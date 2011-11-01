#ifndef PARSER_TOKEN_H
#define PARSER_TOKEN_H

#include "forward.h"

#include <string>
#include <functional>
#include <memory>

template <typename T>
class Token {
    public: /* TODO: change visibility */
        const Symbol<T>* sym_ptr; /* pointer to the constant symbol */
        std::shared_ptr<T> value_ptr;
        size_t start_position, length;

        Token(const Symbol<T>& sym, std::shared_ptr<T> val_p=nullptr,
                size_t start=0, size_t end=0);

        Token<T>& operator=(Token<T>&&);

        const std::string& id() const;
        int lbp() const;
        T nud(PrattParser<T>& parser) const;
        T led(PrattParser<T>& parser, T left) const;

        /* Token<T>::iterator class */
        class iterator {
            const SymbolDict<T>& symbols;
            const std::string& str;
            size_t start, end;
            const Symbol<T>* match;

            bool is_white_space(char c);

            public:
            iterator(const std::string& s, 
                     const SymbolDict<T>& symbols);

            iterator& operator++();
            Token<T> operator*();
        };
};

#endif
