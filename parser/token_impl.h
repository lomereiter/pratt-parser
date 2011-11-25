#ifndef PARSER_TOKEN_IMPL_H
#define PARSER_TOKEN_IMPL_H

#include "token.h"

#include <locale>

template <typename T>
Token<T>::Token(const Symbol<T>& sym, size_t start, size_t end) :
            sym_ptr(&sym), start_position(start), length(end - start) {}

template <typename T> Token<T>::~Token() {}

template <typename T>
const std::string& Token<T>::id() const { return sym_ptr -> id; }

template <typename T>
const Symbol<T>& Token<T>::symbol() const { return *sym_ptr; }

template <typename T>
int Token<T>::lbp() const { return sym_ptr -> lbp; }

template <typename T>
T Token<T>::nud(PrattParser<T>& parser) const {
    if (!sym_ptr -> nud) {
        /* TODO: throw meaningful exception */
        throw "no nud!";
    }
    return sym_ptr -> nud(parser);
}

template <typename T>
T Token<T>::led(PrattParser<T>& parser, T left) const {
    if (!sym_ptr -> led) {
        /* TODO: throw meaningful exception */
        throw "no led!";
    }
    return sym_ptr -> led(parser, left);
}

template <typename T>
Token<T>::iterator::iterator(const std::string& s, 
         const SymbolDict<T>& symbols) :
    str(s), symbols(symbols), start(0), end(0),
    last_new_line_(0), current_line_(1) {
        operator++();
}

template <typename T>
typename Token<T>::iterator& Token<T>::iterator::operator++() {

    skip_white_space(str, start, last_new_line_, current_line_);

    if (start < str.length()) {
        end = start;
        match = nullptr;
        for (auto it = symbols.cbegin(); it != symbols.cend(); ++it) {
            const Symbol<T>& sym = it -> second;
            size_t p = sym.scan(str, start);
            /* longest match with highest precedence */
            if (p > end || 
                    (match != nullptr && 
                     sym.lbp > match -> lbp &&
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

template <typename T>
std::unique_ptr<Token<T>> Token<T>::iterator::operator*() {
    if (start >= str.length()) {
        return std::unique_ptr<Token<T>>(new Token<T>(symbols.end_symbol()));
    }
    size_t old_start = start;
    start = end;
    if (match -> has_parser()) {
        return std::unique_ptr<Token<T>>(
                     new LiteralToken<T>(*match, match -> parse(str, old_start, end),
                                         old_start, end));
    } else {
        return std::unique_ptr<Token<T>>(new Token<T>(*match, old_start, end));
    }
}

template <typename T>
size_t Token<T>::iterator::last_new_line() const { return last_new_line_; }

template <typename T>
size_t Token<T>::iterator::current_line() const { return current_line_; }

template <typename T>
LiteralToken<T>::LiteralToken(const Symbol<T>& sym, T val, size_t start, size_t end) :
    Token<T>(sym, start, end), value(val) {}

template <typename T>
T LiteralToken<T>::nud(PrattParser<T>& p) const { return value; }

#endif
