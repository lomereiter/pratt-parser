#ifndef PARSER_CORE_IMPL_H
#define PARSER_CORE_IMPL_H

#include "forward.h"

#include <string>

#ifdef DEBUG
#include <iostream>
#endif

template <typename T>
class PrattParser {
        const std::string& str;
        typename Token<T>::iterator curr;
        Token<T> token;
  
        Token<T> next() {
            Token<T> tok = *curr;
            ++curr;
            return tok;
        }

    public:
        PrattParser(const std::string& str, 
                    const SymbolDict<T>& symbols) :
             str(str), curr(str, symbols), token(next()) {
        }
       
        T parse(int rbp = 0) {
            Token<T> t = std::move(token);
            token = next();
#ifdef DEBUG
            std::cout << "Calling nud of " << t.id();
            std::cout << " (t.lbp = " << t.lbp() << ", rbp = " << rbp << ")" << std::endl;
#endif
            T left = t.nud(*this);
            while (rbp < token.lbp()) {
                t = std::move(token);
                token = next();
#ifdef DEBUG
                std::cout << "Calling led of " << t.id();
                std::cout << " (t.lbp = " << t.lbp() << ", rbp = " << rbp << ")" << std::endl;
#endif
                left = t.led(*this, left);
            }
            return left;
        }
        
        const std::string next_token_as_string() {
            return str.substr(token.start_position, token.length);
        }

        PrattParser<T>& advance() { token = next(); return *this; }

        PrattParser<T>& advance(const std::string& s) {
            if (next_token_as_string() != s) {
                throw "unexpected character"; /* FIXME! */
            }
            token = next();
            return *this;
        }
};

#endif
