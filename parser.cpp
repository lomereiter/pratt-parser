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
#include <memory>

#include <locale>
/* TODO: divide the code into several headers */

template <typename T> class Grammar;
template <typename T> class PrattParser;
template <typename T> class Symbol;
template <typename T> class Token;

template <typename T>
struct SymbolDict {
    typedef std::map<std::string, Symbol<T>> type;
    typedef typename type::iterator iterator;
    typedef typename type::const_iterator const_iterator;
};


std::string END_SYMBOL_NAME = "(end)";

template <typename T>
class Grammar {
    enum ErrorKind { NO_CLOSING_BRACKET };
    public:
        typename SymbolDict<T>::type symbols;

        Grammar() {
            add_symbol_to_dict(END_SYMBOL_NAME, std::numeric_limits<int>::min())\
                .set_scanner([](const std::string& s, size_t pos){ return pos; });
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
            Symbol<T>& sym = add_symbol_to_dict(op); 
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
            sym.led = [selector, binding_power](PrattParser<T>& p, T left) -> T {
                        return selector(left, p.parse(binding_power));
            };
            return sym;
        }

        Symbol<T>& infix_r(std::string op, int binding_power,
                std::function<T(T, T)> selector) {
            Symbol<T>& sym = add_symbol_to_dict(op, binding_power);
            sym.led = [selector, binding_power](PrattParser<T>& p, T left) -> T {
                        return selector(left, p.parse(binding_power - 1));
            };
            return sym;
        }

        Symbol<T>& brackets(std::string ob, std::string cb, int binding_power) {
            Symbol<T>& open_sym = add_symbol_to_dict(ob, binding_power);
            Symbol<T>& close_sym = add_symbol_to_dict(cb, 0);
            open_sym.nud = [cb](PrattParser<T>& p) -> T {
                        T val = p.parse(0);
                        p.advance(cb);
                        return val;
            };
            return open_sym;
        }
        
        T parse(const std::string& text) const {
            return PrattParser<T>(text, symbols).parse();
        }

        T parse(const char* text) const {
            std::string str(text);
            return parse(str);
        }
};

template <typename T>
class PrattParser {
        typename Token<T>::iterator curr;
        Token<T> token;
 
    public:
        PrattParser(const std::string& str, 
                    const typename SymbolDict<T>::type& symbols) :
             curr(str, symbols), token(next())  {
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
        
        void advance(const std::string& s) {
            if (token.id() != s) {
                throw "unexpected character"; /* FIXME! */
            }
            token = next();
        }
};

template <typename T>
class Symbol {
    /* Basic class containing identifier of symbol,
       its binding power, led and nud */

        typedef std::function<size_t(const std::string& str, size_t pos)> ScannerType;
        typedef std::function<T(const std::string& str, size_t beg, size_t end)> ParserType;

        ScannerType scanner;
        ParserType parser;
    public:
        std::string id;
        int lbp;
        std::function<T(PrattParser<T>&)> nud;
        std::function<T(PrattParser<T>&, T)> led;

        Symbol(std::string id=std::string(), int lbp=0) : id(id), lbp(lbp) {}

        size_t scan(const std::string& str, size_t pos) const {
            if (scanner) {
                return scanner(str, pos);
            } else {
                size_t len = id.length();
                for (size_t i = 0; i < len; ++i) {
                    if (i + pos >= str.length() || id[i] != str[i + pos])
                        return pos;
                }
                return pos + len;
            }
        }

        T parse(const std::string& str, size_t beg, size_t end) const {
            if (parser) {
                return parser(str, beg, end);
            } else {
                return T();
            }
        }

        bool has_scanner() const { return !!scanner; }
        bool has_parser() const { return !!parser; }

        Symbol& set_scanner(const ScannerType& s) {
            scanner = s;
            return *this;
        }

        Symbol& set_parser(const ParserType& p) {
            parser = p;
            return *this;
        }
};

template <typename T>
class Token {
        const std::string* id_ptr;
    public: /* TODO: change visibility */
        int lbp;
        std::function<T(PrattParser<T>&)> null_denotation;
        std::function<T(PrattParser<T>&, T)> left_denotation;
        std::shared_ptr<T> value_ptr;
        Token(const Symbol<T>& sym, std::shared_ptr<T> val_p=nullptr) :
            id_ptr(&sym.id), lbp(sym.lbp), null_denotation(sym.nud), 
            left_denotation(sym.led), value_ptr(val_p) {}

        const std::string& id() { return *id_ptr; }

        T nud(PrattParser<T>& parser) const {
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

        T led(PrattParser<T>& parser, T left) const {
            if (!left_denotation) {
                /* TODO: throw meaningful exception */
                throw "no led!";
            }
            return left_denotation(parser, left);
        }

        /* Token<T>::iterator class */
        class iterator {
            const typename SymbolDict<T>::type& symbols;
            const std::string& str;
            size_t start, end;
            const Symbol<T>* match;

            bool is_white_space(char c) {
                std::locale loc;
                return std::isspace(c, loc);
            }

            public:
            iterator(const std::string& s, 
                     const typename SymbolDict<T>::type& symbols) :
                str(s), symbols(symbols), start(0), end(0) {
                    operator++();
            }

            iterator& operator++() {
                while (start < str.length() &&
                       is_white_space(str[start]))
                    ++start;

                if (start < str.length()) {
                    end = start;
                    match = nullptr;
                    for (const auto& kv : symbols) {
                        const Symbol<T>& sym = kv.second;
                        size_t p = sym.scan(str, start);
                        if (p > end) {
                            match = &sym;
                            end = p;
                        }
                    }
                    if (end == start) {
                        throw "Invalid symbol"; /* FIXME */
                    }
                }
                return *this;
            }

            Token<T> operator*() {
                if (start >= str.length()) {
                    return Token<T>(symbols.find(END_SYMBOL_NAME)->second);
                }
                Token<T> token(*match, 
                        match -> has_parser() ? 
                            std::make_shared<T>(match->parse(str, start, end)) :
                            nullptr);
                start = end;
                return token;
            }
        };
};


#include <cctype>
template <typename T>
class Calculator : public Grammar<T> {
        static T add(T lhs, T rhs) { return lhs + rhs; }
        static T sub(T lhs, T rhs) { return lhs - rhs; }
        static T mul(T lhs, T rhs) { return lhs * rhs; }
        static T div(T lhs, T rhs) { return lhs / rhs; }
        static T neg(T lhs) { return -lhs; }
        static T pos(T lhs) { return lhs; }
        static T fac(T lhs) { 
            T v = 1; 
            for (int i = 1; i <= lhs; ++i)
                v *= i;
            return v;
        }
    public:
        Calculator() {
            Grammar<T>::add_symbol_to_dict("(number)", 0)\
            .set_scanner(
            [](const std::string& str, size_t pos) -> size_t {
                int i = pos;
                while(i < str.length() && isdigit(str[i]))
                    ++i;
                return i;
            })\
            .set_parser(
            [](const std::string& str, size_t beg, size_t end) -> T {
                T num = 0;
                for (int i = beg; i != end; ++i)
                    num *= 10, num += str[i] - '0';
                return num;
            });
            infix("+", 10, add); infix("-", 10, sub);
            infix("*", 20, mul); infix("/", 20, div);
            prefix("+", 100, pos); prefix("-", 100, neg);
            postfix("!", 110, fac);
            Grammar<T>::brackets("(",")", std::numeric_limits<int>::max());
        }
};

int main() {

    Calculator<double> calc;
    std::string str;
    std::cout << "Enter expression:" << std::endl;
    std::getline(std::cin, str);
    std::cout << calc.parse(str) << std::endl;
    return 0;
}
