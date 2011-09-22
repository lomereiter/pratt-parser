#include <iostream>
#include <vector>
#include <iterator>
#include <map>
#include <string>
#include <algorithm>
#include <exception>
#include <functional>
#include <limits>
#include <sstream>

#include <boost/regex.hpp>
using boost::regex;
using boost::sregex_iterator;
using boost::smatch;

/* TODO: divide the code into several headers */

template <typename T> class Grammar;
template <typename T> class PrattParser;
template <typename T> class Symbol;
template <typename T> class Token;

template <typename T>
struct SymbolDict {
    typedef std::map<std::string, Symbol<T>> type;
    typedef typename type::iterator iterator;
};

const std::string LITERAL_SYMBOL_NAME = "(literal)";
const std::string END_SYMBOL_NAME = "(end)";

template <typename T>
class Grammar {
    public:
        typename SymbolDict<T>::type symbols;

        Grammar() {
            add_symbol_to_dict(LITERAL_SYMBOL_NAME, 0);
            add_symbol_to_dict(END_SYMBOL_NAME, std::numeric_limits<int>::min());
        }

        Symbol<T>& add_symbol_to_dict(std::string sym, int lbp=0) {
            typename SymbolDict<T>::iterator it = symbols.find(sym);
            if (it != symbols.end()) {
                Symbol<T>& s = it -> second;
                s.lbp = std::max(s.lbp, lbp);
                return s;
            } else {
                Symbol<T> s(sym, lbp);
                symbols[sym] = s;
                return symbols[sym];
            }
        }      

        Symbol<T>& prefix(std::string op, int binding_power, 
                std::function<T(T)> selector) {
            Symbol<T>& sym = add_symbol_to_dict(op); /* no left denotation => no lbp */
            sym.nud = [selector, binding_power](PrattParser<T>& p) -> T {
                        return selector(p.parse(binding_power));
            };
            return sym;
        }

        Symbol<T>& postfix(std::string op, int binding_power,
                std::function<T(T)> selector) {
            Symbol<T>& sym = add_symbol_to_dict(op, binding_power);
            sym.led = [selector](PrattParser<T>& p, T left) -> T {
                        return selector(left);
            };
            return sym;
        }


        Symbol<T>& infix(std::string op, int binding_power,
                std::function<T(T, T)> selector) {
            Symbol<T>& sym = add_symbol_to_dict(op, binding_power);
            sym.led = [selector, binding_power](PrattParser<T>&p, T left) -> T {
                        return selector(left, p.parse(binding_power));
            };
            return sym;
        }

        Symbol<T>& infix_r(std::string op, int binding_power,
                std::function<T(T, T)> selector) {
            Symbol<T>& sym = add_symbol_to_dict(op, binding_power);
            sym.led = [selector, binding_power](PrattParser<T>&p, T left) -> T {
                        return selector(left, p.parse(binding_power - 1));
            };
            return sym;
        }
        
        T parse(std::string& text) {
            PrattParser<T> pp(text, symbols);
            return pp.parse();
        }

        T parse(const char* text) {
            std::string str(text);
            return parse(str);
        }
};

template <typename T>
class PrattParser {
        typename SymbolDict<T>::type symbols;
        typename Token<T>::iterator curr, end;
        Token<T> token;
    public:
        PrattParser(std::string& str, typename SymbolDict<T>::type& symbols) : 
            symbols(symbols), curr(str, symbols), token(next()) {
        }
        
        Token<T> next() {
            Token<T> tok = *curr;
            ++curr;
            return tok;
        }

        T parse(int rbp = 0) {
            Token<T> t = token;
            token = next();
            T left = t.nud(*this);
            while (rbp < token.lbp) {
                t = token;
                token = next();
                left = t.led(*this, left);
            }
            return left;
        }
};

template <typename T>
class Symbol {
    /* Basic class containing identifier of symbol,
       its binding power, led and nud */
    public:
        std::string id;
        int lbp;
        std::function<T(PrattParser<T>&)> nud;
        std::function<T(PrattParser<T>&, T)> led;

        Symbol(std::string id="", int lbp = 0) : id(id), lbp(lbp) {}
};

template <typename T>
class Token {
    public: /* TODO: change visibility */
        std::string id;
        int lbp;
        std::function<T(PrattParser<T>&)> null_denotation;
        std::function<T(PrattParser<T>&, T)> left_denotation;
        std::shared_ptr<T> value_ptr;
        Token(Symbol<T>& sym, std::shared_ptr<T> val_p=nullptr) :
            id(sym.id), lbp(sym.lbp), null_denotation(sym.nud), 
            left_denotation(sym.led), value_ptr(val_p) {}

        T nud(PrattParser<T>& parser) {
            if (!null_denotation) {
                if (value_ptr) { /* literal token */
                    return *value_ptr;
                } else {
                    /* TODO: throw meaningful exception */
                    throw "no nud!";
                }
            }
            return null_denotation(parser);
        }

        T led(PrattParser<T>& parser, T left) {
            if (!left_denotation) {
                /* TODO: throw meaningful exception */
                throw "no led!";
            }
            return left_denotation(parser, left);
        }

        /* Token<T>::iterator class */
        class iterator {
            regex token_pattern;
            sregex_iterator curr;
            sregex_iterator end;
            typename SymbolDict<T>::type& symbols;
            public:

            iterator() : symbols(*static_cast<typename SymbolDict<T>::type*>(0)) {}

            iterator(std::string& s, typename SymbolDict<T>::type& symbols) :
                token_pattern("\\s*(?:(\\d+)|(\\+|\\*|-|/))"),
                curr(s.begin(), s.end(), token_pattern),
                symbols(symbols) {
            }

            iterator& operator++() {
                if (curr != end)
                    ++curr;
                return *this;
            }

            Token<T> operator*() {
                if (curr == end) {
                    return Token<T>(symbols[END_SYMBOL_NAME]);
                }
                smatch token_match = *curr;
                if ((*curr)[1].matched) { /* number */
                    std::stringstream ss;
                    ss << (*curr)[1].str();
                    std::shared_ptr<T> value_ptr = std::make_shared<T>();
                    ss >> *value_ptr;
                    return Token<T>(symbols[LITERAL_SYMBOL_NAME], value_ptr);
                } else {
                    return Token<T>(symbols[(*curr)[2].str()]);
                }
            }
        };
};

class Calculator : public Grammar<int> {
        static int add(int lhs, int rhs) { return lhs + rhs; }
        static int sub(int lhs, int rhs) { return lhs - rhs; }
        static int mul(int lhs, int rhs) { return lhs * rhs; }
        static int div(int lhs, int rhs) { return lhs / rhs; }
        static int neg(int lhs) { return -lhs; }
        static int pos(int lhs) { return lhs; }
    public:
        Calculator() {
            infix("+", 10, add); infix("-", 10, sub);
            infix("*", 20, mul); infix("/", 20, div);
            prefix("+", 100, pos); prefix("-", 100, neg);
        }
};

int main() {
    Calculator calc;
    std::string str;
    std::cout << "Enter expression: (+, -, *, / are supported)" << std::endl;
    std::getline(std::cin, str);
    std::cout << calc.parse(str) << std::endl;
    return 0;
}
