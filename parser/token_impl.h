#ifndef PARSER_TOKEN_IMPL_H
#define PARSER_TOKEN_IMPL_H

#include "forward.h"

#include <string>
#include <functional>
#include <memory>
#include <locale>

template <typename T>
class Token {
    public: /* TODO: change visibility */
        const Symbol<T>* sym_ptr;
        std::shared_ptr<T> value_ptr;
        size_t start_position, length;

        Token(const Symbol<T>& sym, std::shared_ptr<T> val_p=nullptr,
                size_t start=0, size_t end=0) :
            sym_ptr(&sym),
            value_ptr(val_p), 
            start_position(start),
            length(end - start) {}

        Token<T>& operator=(Token<T>&& tok) {
            sym_ptr = std::move(tok.sym_ptr);
            value_ptr = std::move(tok.value_ptr);
            start_position = tok.start_position;
            length = tok.length;
            return *this;
        }

        const std::string& id() const { return sym_ptr -> id; }
        int lbp() const { return sym_ptr -> lbp; }

        T nud(PrattParser<T>& parser) const {
            if (!sym_ptr -> nud) {
                if (value_ptr) { /* literal token */
                    return *value_ptr;
                } else {
                    /* TODO: throw meaningful exception */
                    throw "no nud!";
                }
            }
            return sym_ptr -> nud(parser);
        }

        T led(PrattParser<T>& parser, T left) const {
            if (!sym_ptr -> led) {
                /* TODO: throw meaningful exception */
                throw "no led!";
            }
            return sym_ptr -> led(parser, left);
        }

        /* Token<T>::iterator class */
        class iterator {
            const std::string& str;
            const SymbolDict<T>& symbols;
            size_t start, end;
            const Symbol<T>* match;

            bool is_white_space(char c) {
                static std::locale loc;
                return std::isspace(c, loc);
            }

            public:
            iterator(const std::string& s, 
                     const SymbolDict<T>& symbols) :
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
                    for (auto it = symbols.cbegin(); it != symbols.cend(); ++it) {
                        const Symbol<T>& sym = it -> second;
                        size_t p = sym.scan(str, start);
                        /* longest match with highest precedence */
                        if (p > end ||
                                (match != nullptr && 
                                 sym.lbp > match->lbp &&
                                 p == end)) {
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
                    return Token<T>(symbols.end_symbol());
                }
                Token<T> token(*match, 
                        match -> has_parser() ? 
                            std::make_shared<T>(match->parse(str, start, end)) :
                            nullptr, start, end);
                start = end;
                return token;
            }
        };
};

#endif
