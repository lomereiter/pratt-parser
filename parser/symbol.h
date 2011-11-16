#ifndef PARSER_SYMBOL_H
#define PARSER_SYMBOL_H

#include "forward.h"

#include <functional>
#include <string>
#include <map>

/// Represents a particular type of token
template <typename T>
class Symbol {

        typedef std::function<size_t(const std::string& str, size_t pos)> ScannerType;
        typedef std::function<T(const std::string& str, size_t beg, size_t end)> ParserType;

        /** In case token is literal, used to scan string beginning from given position.
         *
         * Shall return a position after the end of token scanned.
         *
         * If no token of this type is at the given position, 
         * shall return the given position.
         */
        ScannerType scanner;

        /** In case token is literal, used to parse string range [beg, end).
         * Returns value of type T.
         */
        ParserType parser;
    public:
        /** Unique identifier of the symbol. 
         * Also used if #scanner is not provided.
         */
        std::string id; 
        int lbp; ///< Left-binding power in Pratt terminology
        std::function<T(PrattParser<T>&)> nud; ///< NUll Denotation in Pratt terminology
        std::function<T(PrattParser<T>&, T)> led; ///< LEft Denotation in Pratt terminology

        Symbol(std::string id=std::string(), ///< identifier of the symbol
                int lbp=0 ///< left-binding power of the symbol
                );

        /** Uses #scanner if provided.
         * Otherwise, scans \a str for \a id.
         */
        size_t scan(const std::string& str, size_t pos) const;

        /** Uses #parser if provided.
         * Otherwise throws. Shall be called for literal tokens only.
         */
        T parse(const std::string& str, size_t beg, size_t end) const;

        bool has_scanner() const;
        bool has_parser() const;

        /** Sets #parser to \a p. Causes instances of PrattParser to treat 
         * the tokens produced by this symbol as literals 
         */
        Symbol<T>& set_parser(const ParserType& p);

        /// Sets #scanner to \a s.
        Symbol<T>& set_scanner(const ScannerType& s);
};

/// Used to store set of symbols.
/** Provides std::map-like interface.
 *  The map is from std::string (Symbol<T>#id) to #Symbol<T>.
 */
template <typename T>
class SymbolDict {
    typedef std::map<std::string, Symbol<T>> MapType;

    /// Each grammar shall have a symbol denoting end of token stream
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

    /// Returns identifier of end symbol
    const std::string& get_end_id();
};

#endif
