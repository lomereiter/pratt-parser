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
        size_t start_position, length;

        Token(const Symbol<T>& sym, size_t start=0, size_t end=0);
        virtual ~Token();

        const std::string& id() const;
        int lbp() const;

        /* overloaded by LiteralToken */
        virtual T nud(PrattParser<T>& parser) const;

        T led(PrattParser<T>& parser, T left) const;

        /* Token<T>::iterator class */
        class iterator {
            const std::string& str;
            const SymbolDict<T>& symbols;
            size_t start, end;
            const Symbol<T>* match;

            static bool is_white_space(char c);

            public:
            iterator(const std::string& s, 
                     const SymbolDict<T>& symbols);

            iterator& operator++();
            std::unique_ptr<Token<T>> operator*();
        };
};

template <typename T>
struct LiteralToken : public Token<T> {
    T value;
    LiteralToken(const Symbol<T>& sym, T value, size_t start=0, size_t end=0);
    virtual T nud(PrattParser<T>& parser) const;
};
#endif
