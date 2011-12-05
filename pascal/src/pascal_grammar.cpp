//#include "pascal_grammar.h"

//#include "parser.h"

//#include <sstream>
//#include <forward_list>
#include <stdexcept>
//#include "node.h"
#include "list_guard.h"
//#include "syntax_error.h"
//#include "node_traits.h"
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

PNode PascalGrammar::parse(const std::string& program) {
    static PascalGrammar pg;
    std::string str = program;
    {
        bool lower = true;
        enum { BRACE_COMMENT, BRACKET_COMMENT, STRING, DEFAULT } state = DEFAULT;
        for (size_t i = 0; i != str.length(); ++i) {
            switch (state) {
                case DEFAULT:
                    if (str[i] == '\'') lower = false, state = STRING;
                    if (str[i] == '{') lower = false, state = BRACE_COMMENT;
                    if (str[i] == '(' && (i + 1) < str.length() && str[i + 1] == '*')
                        lower = false, state = BRACKET_COMMENT;
                    break;
                case STRING:
                    if (str[i] == '\'') lower = true, state = DEFAULT;
                    break;
                case BRACE_COMMENT:
                    if (str[i] == '}') lower = true, state = DEFAULT;
                    break;
                case BRACKET_COMMENT:
                    if (str[i] == '*' && (i + 1) < str.length() && str[i + 1] == ')')
                        lower = true, state = DEFAULT;
                    break;
            }
            if (lower) str[i] = tolower(str[i]);
        }
    }
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
                    node_traits::has_type<TypeSectionNode>(node)     ||
                    node_traits::has_type<LabelSectionNode>(node))
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
#ifdef PASCAL_6000
                    } else if (next == "extern") {
                            declarations.push_front(
                                    std::make_shared<ProcedureExternDeclNode>(node));
                            pg.parser -> advance();
#endif
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
#ifdef PASCAL_6000
                    } else if (next == "extern") {
                            declarations.push_front(
                                    std::make_shared<FunctionExternDeclNode>(node));
                            pg.parser -> advance();
#endif
                    } else {
                        declarations.push_front(std::make_shared<FunctionNode>(node, operator()()));
                    }
                    pg.advance(";", "expected ';' after function declaration");
                } 
                else if (node_traits::has_type<FunctionIdentificationNode>(node))
                {
                    pg.advance(";", "expected ';' after function identifier");
                    declarations.push_front(std::make_shared<FunctionNode>(node, operator()()));
                    pg.advance(";", "expected ';' after function declaration");
                } 
                else if (node_traits::has_type<CompoundStatementNode>(node)) {
                    declarations.push_front(
                            std::static_pointer_cast<CompoundStatementNode>(node) -> child);
                    break;
                } 
                else {
                    break;
                }
            } // while

            if (declarations.empty())
                pg.error("expected statement part");

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

    PNode block;
    PNode program_heading = std::make_shared<EmptyNode>();

    try {
        if (pg.parser -> next_token_as_string() == "program") {
            pg.parser -> advance();

            PascalGrammar::lbp_guard semi_guard(*(pg.semicolon), 0);
            PascalGrammar::list_guard<IdentifierNode> comma_guard(pg, pg.comma, "identifier");
            PascalGrammar::led_guard open_bracket_guard(*(pg.opening_bracket),
                [&pg](PrattParser<PNode>& p, PNode name) -> PNode {
                    if (!node_traits::has_type<IdentifierNode>(name))
                        pg.error("expected identifier as program name");
                    PNode list = p.parse(0);
                    if (!node_traits::is_list_of<IdentifierNode>(list))
                        pg.error("expected list of identifiers after '('");
                    pg.advance(")", "expected ')' after list of identifiers");
                    return std::make_shared<ProgramHeadingNode>(
                        std::static_pointer_cast<IdentifierNode>(name) -> name, 
                        list);
                });
            program_heading = pg.parser -> parse(0);
            if (!node_traits::is_convertible_to<ProgramHeadingNode>(program_heading))
                pg.error("expected program heading");
            if (node_traits::has_type<IdentifierNode>(program_heading))
                program_heading = std::make_shared<ProgramHeadingNode>(
                    std::static_pointer_cast<IdentifierNode>(program_heading) -> name);
            pg.advance(";", "expected ';' after program heading");
        }

        block = parse_block();
        pg.advance(".", "expected '.' after 'end'");
        pg.advance("", "unexpected symbol after 'end.'");
    } catch (std::runtime_error& e) {
        pg.error(e.what());
    }
    return std::make_shared<ProgramNode>(program_heading, block);
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
