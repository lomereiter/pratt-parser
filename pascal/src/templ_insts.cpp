#include "parser_impl.h"

#include <memory>
#include <string>
struct Node;

namespace token {

    /// Specialization to treat {...} and (* ... *) as white space
    template <>
    struct SkipWhiteSpace<std::shared_ptr<Node>> {
        void operator()(const std::string& s, size_t& start, 
                                              size_t& last_new_line,
                                              size_t& new_lines) {
            while (start < s.length()) {
                if (isspace(s[start])) {
                    if (s[start] == '\n') last_new_line = start, ++new_lines;
                    ++start;
                } else if (s[start] == '{') {
                    while (start < s.length() && s[start] != '}') {
                        if (s[start] == '\n') last_new_line = start, ++new_lines;
                        ++start;
                    }
                    ++start; // skip '}'
                } else if (s[start] == '(' && (start + 1) < s.length() && s[start+1]=='*') {
                    start += 2;
                    while (start < s.length() && 
                            !( s[start] == '*' && (start + 1) < s.length() && s[start + 1] == ')')) {
                        if (s[start] == '\n') last_new_line = start, ++new_lines;
                        ++start;
                    }
                    start += 2; // skip '*)'
                } else {
                    return;
                }
            }
        }
    };
}

template class Symbol<std::shared_ptr<Node>>;
template class Token<std::shared_ptr<Node>>;
template class PrattParser<std::shared_ptr<Node>>;
template class grammar::Grammar<std::shared_ptr<Node>>;

#define PG grammar::Grammar<std::shared_ptr<Node>>
template struct PG::behaviour_guard<PG::Prefix>;
template struct PG::behaviour_guard<PG::Postfix>;
template struct PG::behaviour_guard<PG::LeftAssociative>;
template struct PG::behaviour_guard<PG::RightAssociative>;
#undef PG
