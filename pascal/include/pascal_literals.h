#ifndef PASCAL_LITERALS_H
#define PASCAL_LITERALS_H

#include <string>

namespace pascal {
    size_t number_scanner(const std::string&, size_t);
    std::string number_parser(const std::string&, size_t, size_t);

    size_t string_scanner(const std::string&, size_t);
    std::string string_parser(const std::string&, size_t, size_t);

    /* scans [_\w][_\w\d]+ */
    size_t identifier_scanner(const std::string&, size_t);
    std::string identifier_parser(const std::string&, size_t, size_t);
}
#endif
