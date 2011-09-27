#ifndef SYMBOL_H
#define SYMBOL_H

#include <functional>
#include <string>

template <typename T> class PrattParser;

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

#include <map>
template <typename T>
struct SymbolDict {
    typedef std::map<std::string, Symbol<T>> type;
    typedef typename type::iterator iterator;
    typedef typename type::const_iterator const_iterator;
};

#include <string>
const std::string END_SYMBOL_NAME("(end)");
#endif
