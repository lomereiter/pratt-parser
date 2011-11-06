#ifndef PARSER_CORE_H
#define PARSER_CORE_H

#include "forward.h"

#include <string>
#include <memory>

template <typename T>
class PrattParser {
        const std::string& str;
        typename Token<T>::iterator curr;
        std::unique_ptr<Token<T>> token;
        std::unique_ptr<Token<T>> next();

    public:
        PrattParser(const std::string&, const SymbolDict<T>&);
       
        T parse(int rbp = 0);
        const std::string next_token_as_string();
        PrattParser<T>& advance();
        PrattParser<T>& advance(const std::string& s);
};

#endif
