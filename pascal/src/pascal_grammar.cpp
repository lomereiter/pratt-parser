//#include "pascal_grammar.h"

//#include "parser.h"

//#include <sstream>
//#include <forward_list>

//#include "node.h"
#include "ast_visitors.h"
//#include "syntax_error.h"
//#include "node_traits.h"
#include <iostream>
using namespace grammar;


/* Grammar definition */
PascalGrammar::PascalGrammar() : Grammar<PNode>("(end)") {

    pascal_grammar::add_literals(*this); 
    pascal_grammar::add_operators(*this); // initializes opening_bracket, sign_eq
    pascal_grammar::add_types(*this);     // initializes comma, semicolon, end, range, array, packed
    pascal_grammar::add_sections(*this);  // initializes colon, var
    pascal_grammar::add_expressions(*this); // initializes dot
    pascal_grammar::add_statements(*this);
    pascal_grammar::add_procedures_and_functions(*this);

}

PNode PascalGrammar::parse(const std::string& str) {
    static PascalGrammar pg;
    pg.parser = std::unique_ptr<PrattParser<PNode>>(
                    new PrattParser<PNode>( str, pg.get_symbols() )
                );

    static struct {
        PNode operator()() {
            static auto next_symbol = [&pg]() -> PNode {
                if (!pg.parser -> next_token().symbol().nud) {
                    pg.error(std::string("unexpected symbol: ")
                           + pg.parser -> next_token_as_string());
                }
                return pg.parser -> parse(1); // because of keywords
            };


            std::forward_list<PNode> declarations;
            PNode node;
            while (true) {
                if (pg.parser -> next_token_as_string() == "" ||
                    pg.parser -> next_token_as_string() == ".") 
                {
                    pg.error("expected statement part");
                }
                node = next_symbol();
                if (node_traits::has_type<ConstSectionNode>(node)    ||
                    node_traits::has_type<VariableSectionNode>(node) ||
                    node_traits::has_type<TypeSectionNode>(node))
                {
                    declarations.push_front(node);
                } 
                else if (node_traits::has_type<ProcedureHeadingNode>(node))
                {
                    pg.advance(";", "expected ';' after procedure heading");
                    auto next = pg.parser -> next_token_as_string();
                    if (next == "forward") {
                            declarations.push_front(
                                    std::make_shared<ProcedureForwardDeclNode>(node));
                            pg.parser -> advance();
                    } else {
                        declarations.push_front(std::make_shared<ProcedureNode>(node, operator()()));
                    }
                    pg.advance(";", "expected ';' after procedure declaration");
                } 
                else if (node_traits::has_type<FunctionHeadingNode>(node))
                {    
                    pg.advance(";", "expected ';' after function heading");
                    auto next = pg.parser -> next_token_as_string();
                    if (next == "forward") {
                            declarations.push_front(
                                    std::make_shared<FunctionForwardDeclNode>(node));
                            pg.parser -> advance();
                    } else {
                        declarations.push_front(std::make_shared<FunctionNode>(node, operator()()));
                    }
                    pg.advance(";", "expected ';' after function declaration");
                } 
                else if (node_traits::has_type<CompoundStatementNode>(node)) {
                    declarations.push_front(
                            std::static_pointer_cast<CompoundStatementNode>(node) -> child);
                    break;
                } else {
                    break;
                }
            } // while
            auto statements = declarations.front();
            declarations.pop_front();
            if (!node_traits::is_list_of<StatementNode>(statements))
                pg.error("expected statement part");

            declarations.reverse();
            return std::make_shared<BlockNode>(
                    std::make_shared<DeclarationListNode>(std::move(declarations)),
                    std::move(statements));
        }
    } parse_block;

    PNode block = parse_block();
    pg.advance(".", "expected '.' after 'end'");
    pg.advance("", "unexpected symbol after 'end.'");
    return block;
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

void PascalGrammar::advance(const std::string& expected, const std::string& desc) {
    if (parser -> next_token_as_string() != expected)
        error(desc);
    parser -> advance();
}
