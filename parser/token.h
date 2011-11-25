#ifndef PARSER_TOKEN_H
#define PARSER_TOKEN_H

#include "forward.h"

#include <string>
#include <functional>
#include <memory>

namespace token {
    /** Specializations of this class can be provided in order to treat
     *  comments as white space. See Token::iterator.
     */
    template <typename T>
    struct SkipWhiteSpace {
        void operator()(const std::string& str, size_t& start, 
                                                size_t& last_new_line,
                                                size_t& current_line_) 
        {
            static std::locale loc;
            while (start < str.length() && std::isspace(str[start], loc)) {
                if (str[start] == '\n') {
                    last_new_line = start;
                    ++current_line_;
                }
                ++start;
            }
        }
    };
}

/// Represents an atomic entity used by PrattParser instance.
/** Is linked with a Symbol instance via pointer. That allows to
 * change the behaviour of already produced token via changing that
 * of the corresponding symbol.
 *
 * All accessor functions correspond to variables/member functions of
 * the symbol.
 */
template <typename T>
class Token {
        const Symbol<T>* sym_ptr; ///< Pointer to the corresponding symbol
    public:
        size_t start_position; ///< Position of the beginning of the token in the string being parsed.
        size_t length; ///< Length of token string representation.

        Token(const Symbol<T>& sym, size_t start=0, size_t end=0);
        virtual ~Token();

        const std::string& id() const;
        const Symbol<T>& symbol() const;
        int lbp() const;
        virtual T nud(PrattParser<T>& parser) const;
        T led(PrattParser<T>& parser, T left) const;

        /// Delivers instances of Token/LiteralToken to PrattParser instance.
        /** Instances are delivered via \a unique_ptr so as to use polymorhic
         *  behaviour of Token with respect to Token.nud
         */
        class iterator {
            const std::string& str; ///< references the string being parsed
            const SymbolDict<T>& symbols; ///< references symbols of the Grammar used
            size_t start; ///< position in #str of the beginning of current Token
            size_t end;   ///< position in #str after the end of current Token
            const Symbol<T>* match; ///< points to Symbol which matches current Token
            size_t last_new_line_;  ///< position in #str of last '\n' character
            size_t current_line_;   ///< one-indexed current line

            public:
            /// initializes #str and #symbols
            iterator(const std::string& str,
                     const SymbolDict<T>& symbols);

            /** token::SkipWhiteSpace shall have
             *      void operator()(const std::string&, size_t& start, 
             *                                          size_t& last_new_line,
             *                                          size_t& new_lines);
             *      which shall update the number of new lines encountered,
             *                  update \a start position and \a last_new_line appropriately.
             */
            static typename token::SkipWhiteSpace<T> skip_white_space;

            /** Skips whitespace and tries to scan #str for all symbols of Grammar in turn 
             *  starting from #end. If the end of #str is reached returns symbols.end_symbol()
             *
             *  Uses longest-match highest-precedence rule.
             */
            iterator& operator++();
            /// Returns pointer to Token/LiteralToken instance.
            std::unique_ptr<Token<T>> operator*();

            /// Zero-indexed position of last '\n' encountered in #str
            size_t last_new_line() const;

            /// One-indexed current line
            size_t current_line() const; 
        };
};

/// Represents terminal token.
template <typename T>
struct LiteralToken : public Token<T> {
    T value;
    LiteralToken(const Symbol<T>& sym, T value, size_t start=0, size_t end=0);

    /** Just returns #value whereas non-literal tokens usually need to 
     * parse some subsequent part of string.
     */
    virtual T nud(PrattParser<T>&) const;
};
#endif
