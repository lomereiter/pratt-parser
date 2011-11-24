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
            return std::make_shared<NumberNode>(str.substr(beg, end - beg));
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
