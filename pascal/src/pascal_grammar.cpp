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

    infix(":=", 40,
            [this](PNode var, PNode expr) -> PNode {
                if (!node_traits::is_convertible_to<VariableNode>(var))
                    error("expected variable before ':=' token");
                if (!node_traits::is_convertible_to<ExpressionNode>(expr))
                    error("expected expression after ':=' token");
                return std::make_shared<AssignmentStatementNode>(var, expr);
            });

    add_symbol_to_dict("begin", 1)
    .nud = [this](PrattParser<PNode>& p) -> PNode {
        if (p.next_token_as_string() == "end")
            error("expected non-empty statement-sequence after 'begin'");

        lbp_guard sl_guard(*(this -> semicolon), 1);
        lbp_guard end_guard(*(this -> end), 0);

        behaviour_guard<RightAssociative> sb_guard(*(this -> semicolon),
            [this](PNode x, PNode y) {
                return ListVisitor<StatementNode>(x, y, this, "statement")
                       .get_expression();
            });
        PNode statements = p.parse(0);
        if (!node_traits::is_list_of<StatementNode>(statements))
            error("expected statement-sequence after 'begin'");
        if (p.next_token_as_string() != "end") 
            error("expected 'end' after statement-sequence");
        p.advance();
        return std::make_shared<CompoundStatementNode>(statements);
    };

    add_symbol_to_dict("do", 0);
    add_symbol_to_dict("while", 1)
    .nud = [this](PrattParser<PNode>& p) -> PNode {
        PNode condition = p.parse(0);
        if (!node_traits::is_convertible_to<ExpressionNode>(condition))
            error("expected expression after 'while'");
        if (p.next_token_as_string() != "do")
            error("expected 'do' after expression");
        p.advance();
        PNode body = p.parse(1);
        if (!node_traits::is_convertible_to<StatementNode>(body))
            error("expected statement-sequence after 'do'");
        return std::make_shared<WhileStatementNode>(condition, body);
    };

    add_symbol_to_dict("until", 0);
    add_symbol_to_dict("repeat", 1)
    .nud = [this](PrattParser<PNode>& p) -> PNode {
        PNode body = p.parse(1);
        if (!node_traits::is_list_of<StatementNode>(body))
            error("expected statement-sequence after 'repeat'");
        if (p.next_token_as_string() != "until")
            error("expected 'until' after statement-sequence");
        p.advance();
        PNode condition = p.parse(0);
        if (!node_traits::is_convertible_to<ExpressionNode>(condition))
            error("expected expression after 'until'");
        return std::make_shared<RepeatStatementNode>(body, condition);
    };

    add_symbol_to_dict("to", 0);
    add_symbol_to_dict("downto", 0);
    add_symbol_to_dict("for", 1)
    .nud = [this](PrattParser<PNode>& p) -> PNode {
        PNode assignment = p.parse(0);
        if (!node_traits::has_type<AssignmentStatementNode>(assignment))
            error("expected assignment-statement after 'for'");
        auto _assignment = std::static_pointer_cast<AssignmentStatementNode>(assignment);
        if (!node_traits::has_type<IdentifierNode>(_assignment -> variable))
            error("expected identifier after 'for'");
        int sign;
        std::string next = p.next_token_as_string();
        if (next == "to") {
            sign = 1;
        } else if (next == "downto") {
            sign = -1;
        } else {
            error("expected 'to' or 'downto' after initial-expression");
        }
        p.advance();
        PNode final_expr = p.parse(1);
        if (!node_traits::is_convertible_to<ExpressionNode>(final_expr)) {
            std::string msg = "expected final-expression after ";
            msg = msg + '\'' + next + '\'';
            error(std::move(msg));
        }
        if (p.next_token_as_string() != "do")
            error("expected 'do' after final-expression");
        p.advance();
        PNode body = p.parse(1);
        if (!node_traits::is_convertible_to<StatementNode>(body))
            error("expected statement after 'do'");
        return std::make_shared<ForStatementNode>(_assignment, sign, final_expr, body);
    };

    add_symbol_to_dict("then", 0);
    add_symbol_to_dict("else", 0);
    add_symbol_to_dict("if", 1)
    .nud = [this](PrattParser<PNode>& p) -> PNode {
        PNode expr = p.parse(1);
        if (!node_traits::is_convertible_to<ExpressionNode>(expr))
            error("expected expression after 'if'");
        if (p.next_token_as_string() != "then")
            error("expected 'then'");
        p.advance();
        PNode st = p.parse(1);
        if (!node_traits::is_convertible_to<StatementNode>(st))
            error("expected statement after 'then'");
        if (p.next_token_as_string() != "else") {
            return std::make_shared<IfThenNode>(expr, st);
        } else {
            p.advance();
            PNode _else = p.parse(1);
            if (!node_traits::is_convertible_to<StatementNode>(st))
                error("expected statement after 'else'");
            return std::make_shared<IfThenElseNode>(expr, st, _else);
        }
    };

    add_symbol_to_dict("do", 0);
    add_symbol_to_dict("with", 1)
    .nud = [this](PrattParser<PNode>& p) -> PNode {
        behaviour_guard<RightAssociative> comma_guard(*(this->comma),
            [this](PNode left, PNode right) {
                return ListVisitor<VariableNode>(left, right, this, "record variable")
                       .get_expression();
            });
        PNode list = p.parse(0);
        if (!node_traits::is_list_of<VariableNode>(list))
            error("expected list of record variables after 'with'");
        if (p.next_token_as_string() != "do")
            error("expected 'do' in with-statement");
        p.advance();
        PNode st = p.parse(1);
        if (!node_traits::is_convertible_to<StatementNode>(st))
            error("expected statement after 'do'");
        return std::make_shared<WithStatementNode>(list, st);
    };

    add_symbol_to_dict("case", 1)
    .nud = [this](PrattParser<PNode>& p) -> PNode {
        PNode expr = p.parse(0);
        if (!node_traits::is_convertible_to<ExpressionNode>(expr))
            error("expected expression after 'case'");
        if (p.next_token_as_string() != "of")
            error("expected 'of' after expression");

        p.advance();

        behaviour_guard<RightAssociative> comma_guard(*(this -> comma),
            [this](PNode left, PNode right) {
                return ListVisitor<ConstantNode>(left, right, this, "constant")
                       .get_expression();
            });

        behaviour_guard<LeftAssociative> colon_guard(*(this -> colon), 1,
            [this](PNode left, PNode right) -> PNode {
                if (!node_traits::is_list_of<ConstantNode>(left))
                    error("expected list of constants before ':'");
                if (!node_traits::is_convertible_to<StatementNode>(right))
                    error("expected statement after ':'");
                return std::make_shared<CaseLimbNode>(left, right);
            });

        lbp_guard semicolon_guard(*(this -> semicolon), 0);
        
        std::forward_list<PNode> limbs;

        do {
            PNode limb = p.parse(0);
            if (!node_traits::has_type<CaseLimbNode>(limb))
                error("expected case-limb");
            limbs.push_front(limb);
            std::string next = p.next_token_as_string();
            if (next == ";") {
                p.advance();
                if (p.next_token_as_string() == "end") {
                    p.advance();
                    break;
                }
            } else if (next == "end") {
                p.advance();
                break;
            } else {
                error("expected ';' or 'end'");
            }
        } while (true);

        limbs.reverse();
        return std::make_shared<CaseStatementNode>(expr,
                std::make_shared<CaseLimbListNode>(std::move(limbs)));
    };
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
