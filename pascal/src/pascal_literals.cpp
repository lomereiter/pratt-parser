#include <string>
#include <sstream>

namespace pascal {

    size_t number_scanner(const std::string& str, size_t pos) {
        size_t i = pos;
        if (i >= str.length() || !isdigit(str[i])) return pos;
        while (i < str.length() && isdigit(str[i])) 
            ++i; // reading digits before dot
        if (i < str.length() && str[i] == '.') { // if dot is presented
            ++i;
            if (i >= str.length() || !isdigit(str[i])) return i - 1;
            while (i < str.length() && isdigit(str[i]))
                ++i; // read digits after dot
        }
        if (i < str.length() && (str[i] == 'E' || str[i] == 'e')) {
            ++i; // read optional exponent
            if (i >= str.length()) return pos;
            if (str[i] == '+' || str[i] == '-')
                ++i;
            if (i >= str.length() || !isdigit(str[i])) return pos;
            while (i < str.length() && isdigit(str[i]))
                ++i;
        }
        return i;
    }

    std::string number_parser(const std::string& str, size_t beg, size_t end) {
        return str.substr(beg, end - beg);
    }

    size_t string_scanner(const std::string& str, size_t pos) {
        size_t i = pos;
        for ( ; ; ) {
            if (i >= str.length() || str[i] != '\'') return i;
            ++i;
            while (i < str.length() && str[i] != '\'')
                ++i;
            if (i >= str.length()) return pos;
            ++i;
        }
    }

    std::string string_parser(const std::string& str, size_t beg, size_t end) {
        std::stringstream sstr;
        for (size_t pos = beg + 1; pos < end - 1; ++pos) {
            if (str[pos] != '\'') sstr << str[pos];
            else { 
                sstr << '\'';
                ++pos;
            }
        }
        return sstr.str();
    }

    /* scans [_\w][_\w\d]+ */
    size_t identifier_scanner(const std::string& str, size_t pos) {
        size_t i = pos;
        if (i < str.length() && (isalpha(str[i]) || str[i] == '_'))
            ++i;
        else
            return pos; 
        while (i < str.length() && (isalnum(str[i]) || str[i] == '_'))
            ++i;
        return i;
    }

    std::string identifier_parser(const std::string& str, size_t beg, size_t end) {
        return str.substr(beg, end - beg);
    }
}
