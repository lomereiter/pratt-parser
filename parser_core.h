#ifndef PARSER_CORE_H
#define PARSER_CORE_H

#include "forward.h"

#include <string>

template <typename T>
class PrattParser {
        typename Token<T>::iterator curr;
        Token<T> token;
 
    public:
        PrattParser(const std::string& str, 
                    const typename SymbolDict<T>::type& symbols) :
             curr(str, symbols), token(next())  {
        }
        
        Token<T> next() {
            Token<T> tok = *curr;
            ++curr;
            return tok;
        }

        T parse(int rbp = 0) {
            Token<T> t = token;
            token = next();
            T left = t.nud(*this);
            while (rbp < token.lbp) {
                t = token;
                token = next();
                left = t.led(*this, left);
            }
            return left;
        }
        
        void advance(const std::string& s) {
            if (token.id() != s) {
                throw "unexpected character"; /* FIXME! */
            }
            token = next();
        }
};

#endif
