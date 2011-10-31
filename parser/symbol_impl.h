#ifndef PARSER_SYMBOL_IMPL_H
#define PARSER_SYMBOL_IMPL_H

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

        Symbol(std::string id=std::string(), int lbp=0) : id(id), lbp(lbp) {}

        size_t scan(const std::string& str, size_t pos) const {
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

        T parse(const std::string& str, size_t beg, size_t end) const {
            if (parser) {
                return parser(str, beg, end);
            } else {
                return T();
            }
        }

        bool has_scanner() const { return !!scanner; }
        bool has_parser() const { return !!parser; }

        Symbol& set_scanner(const ScannerType& s) {
            scanner = s;
            return *this;
        }

        Symbol& set_parser(const ParserType& p) {
            parser = p;
            return *this;
        }
};

template <typename T>
class SymbolDict {
    typedef std::map<std::string, Symbol<T>> MapType;

    std::string end_id;
    MapType dict;
public:
    typedef typename MapType::iterator iterator;
    typedef typename MapType::const_iterator const_iterator;

    SymbolDict(std::string end_id) : end_id(end_id) {
        Symbol<T> s(end_id, std::numeric_limits<int>::min());
        dict[end_id] = s;
    }

    iterator begin() { return dict.begin(); }
    iterator end() { return dict.end(); }
    const_iterator cbegin() const { return dict.cbegin(); }
    const_iterator cend() const { return dict.cend(); }
    iterator find(const std::string& id) { return dict.find(id); }
    Symbol<T>& operator[](const std::string& id) { return dict[id]; }
    const Symbol<T>& end_symbol() const { return dict.find(end_id) -> second; }
    const std::string& get_end_id() { return end_id; }
};

#endif
