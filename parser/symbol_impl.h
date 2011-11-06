#ifndef PARSER_SYMBOL_IMPL_H
#define PARSER_SYMBOL_IMPL_H

#include "forward.h"
#include "symbol.h"

#include <functional>
#include <string>
#include <map>
#include <limits>
template <typename T>
Symbol<T>::Symbol(std::string id, int lbp) : id(id), lbp(lbp) {}

template <typename T>
size_t Symbol<T>::scan(const std::string& str, size_t pos) const {
    if (scanner) {
        return scanner(str, pos);
    } else {
        /* the default behaviour */
        size_t len = id.length();
        for (size_t i = 0; i < len; ++i) {
            if (i + pos >= str.length() || id[i] != str[i + pos])
                return pos;
        }
        return pos + len;
    }
}

template <typename T>
T Symbol<T>::parse(const std::string& str, size_t beg, size_t end) const {
    if (parser) {
        return parser(str, beg, end);
    } else {
        return T();
    }
}

template <typename T>
bool Symbol<T>::has_scanner() const { return !!scanner; }
        
template <typename T>
bool Symbol<T>::has_parser() const { return !!parser; }

template <typename T> 
Symbol<T>& Symbol<T>::set_scanner(const ScannerType& s) {
    scanner = s; return *this;
}

template <typename T>
Symbol<T>& Symbol<T>::set_parser(const ParserType& p) {
    parser = p; return *this;
}

/* SymbolDict functions */

template <typename T>
SymbolDict<T>::SymbolDict(std::string end_id) : end_id(end_id) {
    Symbol<T> s(end_id, std::numeric_limits<int>::min());
    dict[end_id] = s;
}

template <typename T>
typename SymbolDict<T>::iterator SymbolDict<T>::begin() { 
    return dict.begin(); 
}

template <typename T>
typename SymbolDict<T>::iterator SymbolDict<T>::end() {
    return dict.end();
}

template <typename T> 
typename SymbolDict<T>::const_iterator SymbolDict<T>::cbegin() const { 
    return dict.cbegin(); 
}

template <typename T>
typename SymbolDict<T>::const_iterator SymbolDict<T>::cend() const { 
    return dict.cend(); 
}

template <typename T>
typename SymbolDict<T>::iterator SymbolDict<T>::find(const std::string& id) { 
    return dict.find(id); 
}

template <typename T>
Symbol<T>& SymbolDict<T>::operator[](const std::string& id) {
    return dict[id]; 
}

template <typename T>
const Symbol<T>& SymbolDict<T>::end_symbol() const { 
    return dict.find(end_id) -> second; 
}

template <typename T>
const std::string& SymbolDict<T>::get_end_id() { 
    return end_id; 
}

#endif
