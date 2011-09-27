#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "forward.h"

#include <string>
#include <functional>
#include <limits>
#include <algorithm>

template <typename T>
class Grammar {
    enum ErrorKind { NO_CLOSING_BRACKET };
    public:
        typename SymbolDict<T>::type symbols;

        Grammar() {
            add_symbol_to_dict(std::string(END_SYMBOL_NAME), 
                               std::numeric_limits<int>::min())
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

#endif
