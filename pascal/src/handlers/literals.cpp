//#include <memory>
//#include <string>

#include "node.h"
#include "pascal_grammar.h"
#include "pascal_literals.h"

namespace pascal_grammar {

    void add_literals(PascalGrammar& g) {

       g.add_symbol_to_dict("(number)", 0)
        .set_scanner(pascal::number_scanner)
        .set_parser([](const std::string& str, size_t beg, size_t end) -> PNode {
            bool is_real = false;
            for (size_t i = beg; i != str.length() && i < end; ++i) {
                if (str[i] == '.') is_real = true;
                if (str[i] == 'e') {
                    return std::make_shared<URealNumberNode>(str.substr(0, i),
                                                            str.substr(i + 1, end - i - 1));
                }
            }
            if (is_real) {
                return std::make_shared<URealNumberNode>(str.substr(beg, end - beg));
            } else {
                return std::make_shared<UIntegerNumberNode>(str.substr(beg, end - beg));
            }
        });

       g.add_symbol_to_dict("(identifier)", 0)
        .set_scanner(pascal::identifier_scanner)
        .set_parser([](const std::string& str, size_t beg, size_t end) {
            return std::make_shared<IdentifierNode>(str.substr(beg, end - beg));
        });

       g.add_symbol_to_dict("(string literal)", 0)
        .set_scanner(pascal::string_scanner)
        .set_parser([](const std::string& str, size_t beg, size_t end) -> PNode {
            return std::make_shared<StringNode>(pascal::string_parser(str, beg, end));
        });

    }
}
