//#include "pascal_grammar.h"

//#include "parser.h"

//#include <sstream>
//#include <forward_list>

//#include "node.h"
#include "ast_visitors.h"
//#include "syntax_error.h"
//#include "node_traits.h"

using namespace grammar;

/* Grammar definition */

PascalGrammar::PascalGrammar() : Grammar<PNode>("(end)") {

    pascal_grammar::add_literals(*this); 
    pascal_grammar::add_operators(*this); // initializes opening_bracket, sign_eq
    pascal_grammar::add_types(*this);     // initializes comma, semicolon, end
    pascal_grammar::add_sections(*this);  // initializes colon
    pascal_grammar::add_expressions(*this);
    pascal_grammar::add_statements(*this);

    }

PNode PascalGrammar::parse(const std::string& str) {
    static PascalGrammar pg;
    pg.parser = std::unique_ptr<PrattParser<PNode>>(
                    new PrattParser<PNode>( str, pg.get_symbols() )
                );
    std::forward_list<PNode> declarations;
    while (pg.parser -> next_token_as_string() != "") {
        declarations.push_front(std::make_shared<DeclarationNode>(pg.parser -> parse(1)));
    }
    declarations.reverse();
    return std::make_shared<DeclarationListNode>(std::move(declarations));
}

void PascalGrammar::error(const std::string& description) const {
    SourcePosition position = parser -> current_position();
    const std::string& str = parser -> code();
    std::stringstream error_desc;
    static const size_t SNIPPET_LEN = 30;
    error_desc << "syntax error near line " << position.line << ": "
                                            << description;
    error_desc << "\n\t" << "...";
    error_desc << str.substr(position.position > SNIPPET_LEN ? 
                             position.position - SNIPPET_LEN : 
                                                             0, SNIPPET_LEN);
    error_desc << "...";
    throw SyntaxError(error_desc.str());
}
