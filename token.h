#ifndef TOKEN_H
#define TOKEN_H

#include "forward.h"

#include <string>
#include <functional>
#include <memory>
#include <locale>

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
                static std::locale loc;
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

#endif
