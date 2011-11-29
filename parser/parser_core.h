#ifndef PARSER_CORE_H
#define PARSER_CORE_H

#include "forward.h"

#include <string>
#include <memory>

struct SourcePosition {
    size_t position;
    size_t line;
    size_t column;
};

template <typename T>
class PrattParser {
        const std::string& str;
        typename Token<T>::iterator token_iter;
        std::unique_ptr<Token<T>> token;
        std::unique_ptr<Token<T>> prev_token;
        std::unique_ptr<Token<T>> next();

    public:
        PrattParser(const std::string&, const SymbolDict<T>&);
       
        T parse(int rbp = 0);
        const Token<T>& next_token() const;
        const std::string next_token_as_string() const;
        PrattParser<T>& advance();
        PrattParser<T>& advance(const std::string& s);

        SourcePosition current_position() const;
        
        const std::string& code() const;
};

#endif
