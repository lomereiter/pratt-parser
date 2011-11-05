#ifndef SYNTAX_ERROR_H
#define SYNTAX_ERROR_H

#include <exception>
#include <string>

struct SyntaxError : public std::exception {
    SyntaxError(const char* str) : message(str) {}
    SyntaxError(const std::string& str) : message(str) {}
    SyntaxError(std::string&& str) : message(str) {}
    virtual ~SyntaxError() throw() {}
    virtual const char* what() const throw() {
        return message.c_str();
    }
    private:
        std::string message;
};

#endif
