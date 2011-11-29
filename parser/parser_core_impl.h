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
    prev_token = std::move(token);
    token = next();
#ifdef DEBUG
    std::cout <<  "Calling nud of " << prev_token -> id();
    std::cout << " (token.lbp = " << token -> lbp() << ", rbp = " << rbp << ")" << std::endl;
#endif
    T left = prev_token -> nud(*this); /* value for terminals, result of func. call otherwise */
    while (rbp < token -> lbp()) {
        prev_token = std::move(token);
        token = next();
#ifdef DEBUG
        std::cout << "Calling led of " << prev_token -> id();
        std::cout << " (token.lbp = " << token -> lbp() << ", rbp = " << rbp << ")" << std::endl;
#endif
        left = prev_token -> led(*this, left);
    }
    return left;
}

template <typename T>
const std::string PrattParser<T>::next_token_as_string() const {
    return str.substr(token -> start_position, token -> length);
}

template <typename T>
const Token<T>& PrattParser<T>::next_token() const {
    return *token; // slicing is done here 
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
    const std::unique_ptr<Token<T>>& tok = token ? token : prev_token;
    sp.position = tok ? tok -> start_position : 0;
    sp.line = token_iter.current_line();
    sp.column = sp.position - token_iter.last_new_line() + 1;
    return sp;
}

template <typename T>
const std::string& PrattParser<T>::code() const {
    return str;
}

#endif
