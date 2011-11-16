#ifndef PARSER_CORE_IMPL_H
#define PARSER_CORE_IMPL_H

#include "parser_core.h"

#ifdef DEBUG
#include <iostream>
#endif

template <typename T> 
std::unique_ptr<Token<T>> PrattParser<T>::next() {
    std::unique_ptr<Token<T>> tok = *token_iter;
    ++token_iter;
    return tok;
}

template <typename T>
PrattParser<T>::PrattParser(const std::string& str, 
            const SymbolDict<T>& symbols) :
     str(str), token_iter(str, symbols), token(next()) {
}
   
template <typename T>
T PrattParser<T>::parse(int rbp) {
    std::unique_ptr<Token<T>> t = std::move(token);
    token = next();
#ifdef DEBUG
    std::cout << "Calling nud of " << t -> id();
    std::cout << " (t.lbp = " << t -> lbp() << ", rbp = " << rbp << ")" << std::endl;
#endif
    T left = t -> nud(*this); /* value for terminals, result of func. call otherwise */
    while (rbp < token -> lbp()) {
        t = std::move(token);
        token = next();
#ifdef DEBUG
        std::cout << "Calling led of " << t -> id();
        std::cout << " (t.lbp = " << t -> lbp() << ", rbp = " << rbp << ")" << std::endl;
#endif
        left = t -> led(*this, left);
    }
    return left;
}

template <typename T>
const std::string PrattParser<T>::next_token_as_string() const {
    return str.substr(token -> start_position, token -> length);
}

template <typename T>
PrattParser<T>& PrattParser<T>::advance() { token = next(); return *this; }

template <typename T>
PrattParser<T>& PrattParser<T>::advance(const std::string& s) {
    if (next_token_as_string() != s) {
        throw "unexpected character"; /* FIXME! */
    }
    token = next();
    return *this;
}

template <typename T>
SourcePosition PrattParser<T>::current_position() const {
    SourcePosition sp;
    sp.position = token->start_position;
    sp.line = token_iter.current_line();
    sp.column = sp.position - token_iter.last_new_line() + 1;
    return sp;
}

template <typename T>
const std::string& PrattParser<T>::code() const {
    return str;
}

#endif
