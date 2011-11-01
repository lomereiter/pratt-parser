#ifndef PARSER_SYMBOL_H
#define PARSER_SYMBOL_H

#include "forward.h"

#include <functional>
#include <string>
#include <map>
#include <limits>

template <typename T>
class Symbol {

        typedef std::function<size_t(const std::string& str, size_t pos)> ScannerType;
        typedef std::function<T(const std::string& str, size_t beg, size_t end)> ParserType;

        ScannerType scanner;
        ParserType parser;
    public:
        std::string id;
        int lbp;
        std::function<T(PrattParser<T>&)> nud;
        std::function<T(PrattParser<T>&, T)> led;

        Symbol(std::string id=std::string(), int lbp=0);

        size_t scan(const std::string& str, size_t pos) const;
        T parse(const std::string& str, size_t beg, size_t end) const;

        bool has_scanner() const;
        bool has_parser() const;

        Symbol& set_scanner(const ScannerType& s);
        Symbol& set_parser(const ParserType& p);
};

template <typename T>
class SymbolDict {
    typedef std::map<std::string, Symbol<T>> MapType;

    std::string end_id;
    MapType dict;
public:
    typedef typename MapType::iterator iterator;
    typedef typename MapType::const_iterator const_iterator;

    SymbolDict(std::string end_id);

    iterator begin();
    iterator end();
    const_iterator cbegin() const;
    const_iterator cend() const;
    iterator find(const std::string& id);
    Symbol<T>& operator[](const std::string& id);
    const Symbol<T>& end_symbol() const;
    const std::string& get_end_id();
};

#endif
