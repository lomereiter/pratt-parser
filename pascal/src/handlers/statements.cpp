//#include "pascal_grammar.h"
#include "ast_visitors.h"

namespace pascal_grammar {

    void add_statements(PascalGrammar& g) {
       typedef PascalGrammar::RightAssociative RightAssociative;
       typedef PascalGrammar::LeftAssociative LeftAssociative;

       g.infix(":=", 40,
                [&g](PNode var, PNode expr) -> PNode {
                    if (!node_traits::is_convertible_to<VariableNode>(var))
                        g.error("expected variable before ':=' token");
                    if (!node_traits::is_convertible_to<ExpressionNode>(expr))
                        g.error("expected expression after ':=' token");
                    return std::make_shared<AssignmentStatementNode>(var, expr);
                });

       g.add_symbol_to_dict("begin", 1)
        .nud = [&g](PrattParser<PNode>& p) -> PNode {
            if (p.next_token_as_string() == "end")
                g.error("expected non-empty statement-sequence after 'begin'");

            PascalGrammar::lbp_guard sl_guard(*(g.semicolon), 1);
            PascalGrammar::lbp_guard end_guard(*(g.end), 0);

            PascalGrammar::behaviour_guard<RightAssociative> sb_guard(*(g.semicolon),
                [&g](PNode x, PNode y) {
                    return ListVisitor<StatementNode>(x, y, &g, "statement")
                           .get_expression();
                });
            PNode statements = p.parse(0);
            if (!node_traits::is_list_of<StatementNode>(statements))
                g.error("expected statement-sequence after 'begin'");
            if (p.next_token_as_string() != "end") 
                g.error("expected 'end' after statement-sequence");
            p.advance();
            return std::make_shared<CompoundStatementNode>(statements);
        };

       g.add_symbol_to_dict("do", 0);
       g.add_symbol_to_dict("while", 1)
        .nud = [&g](PrattParser<PNode>& p) -> PNode {
            PNode condition = p.parse(0);
            if (!node_traits::is_convertible_to<ExpressionNode>(condition))
                g.error("expected expression after 'while'");
            if (p.next_token_as_string() != "do")
                g.error("expected 'do' after expression");
            p.advance();
            PNode body = p.parse(1);
            if (!node_traits::is_convertible_to<StatementNode>(body))
                g.error("expected statement-sequence after 'do'");
            return std::make_shared<WhileStatementNode>(condition, body);
        };

       g.add_symbol_to_dict("until", 0);
       g.add_symbol_to_dict("repeat", 1)
        .nud = [&g](PrattParser<PNode>& p) -> PNode {
            PNode body = p.parse(1);
            if (!node_traits::is_list_of<StatementNode>(body))
                g.error("expected statement-sequence after 'repeat'");
            if (p.next_token_as_string() != "until")
                g.error("expected 'until' after statement-sequence");
            p.advance();
            PNode condition = p.parse(0);
            if (!node_traits::is_convertible_to<ExpressionNode>(condition))
                g.error("expected expression after 'until'");
            return std::make_shared<RepeatStatementNode>(body, condition);
        };

       g.add_symbol_to_dict("to", 0);
       g.add_symbol_to_dict("downto", 0);
       g.add_symbol_to_dict("for", 1)
        .nud = [&g](PrattParser<PNode>& p) -> PNode {
            PNode assignment = p.parse(0);
            if (!node_traits::has_type<AssignmentStatementNode>(assignment))
                g.error("expected assignment-statement after 'for'");
            auto _assignment = std::static_pointer_cast<AssignmentStatementNode>(assignment);
            if (!node_traits::has_type<IdentifierNode>(_assignment -> variable))
                g.error("expected identifier after 'for'");
            int sign;
            std::string next = p.next_token_as_string();
            if (next == "to") {
                sign = 1;
            } else if (next == "downto") {
                sign = -1;
            } else {
                g.error("expected 'to' or 'downto' after initial-expression");
            }
            p.advance();
            PNode final_expr = p.parse(1);
            if (!node_traits::is_convertible_to<ExpressionNode>(final_expr)) {
                std::string msg = "expected final-expression after ";
                msg = msg + '\'' + next + '\'';
                g.error(std::move(msg));
            }
            if (p.next_token_as_string() != "do")
                g.error("expected 'do' after final-expression");
            p.advance();
            PNode body = p.parse(1);
            if (!node_traits::is_convertible_to<StatementNode>(body))
                g.error("expected statement after 'do'");
            return std::make_shared<ForStatementNode>(_assignment, sign, final_expr, body);
        };

       g.add_symbol_to_dict("then", 0);
       g.add_symbol_to_dict("else", 0);
       g.add_symbol_to_dict("if", 1)
        .nud = [&g](PrattParser<PNode>& p) -> PNode {
            PNode expr = p.parse(1);
            if (!node_traits::is_convertible_to<ExpressionNode>(expr))
                g.error("expected expression after 'if'");
            if (p.next_token_as_string() != "then")
                g.error("expected 'then'");
            p.advance();
            PNode st = p.parse(1);
            if (!node_traits::is_convertible_to<StatementNode>(st))
                g.error("expected statement after 'then'");
            if (p.next_token_as_string() != "else") {
                return std::make_shared<IfThenNode>(expr, st);
            } else {
                p.advance();
                PNode _else = p.parse(1);
                if (!node_traits::is_convertible_to<StatementNode>(st))
                    g.error("expected statement after 'else'");
                return std::make_shared<IfThenElseNode>(expr, st, _else);
            }
        };

       g.add_symbol_to_dict("do", 0);
       g.add_symbol_to_dict("with", 1)
        .nud = [&g](PrattParser<PNode>& p) -> PNode {
            PascalGrammar::behaviour_guard<RightAssociative> comma_guard(*(g.comma),
                [&g](PNode left, PNode right) {
                    return ListVisitor<VariableNode>(left, right, &g, "record variable")
                           .get_expression();
                });
            PNode list = p.parse(0);
            if (!node_traits::is_list_of<VariableNode>(list))
                g.error("expected list of record variables after 'with'");
            if (p.next_token_as_string() != "do")
                g.error("expected 'do' in with-statement");
            p.advance();
            PNode st = p.parse(1);
            if (!node_traits::is_convertible_to<StatementNode>(st))
                g.error("expected statement after 'do'");
            return std::make_shared<WithStatementNode>(list, st);
        };

       g.add_symbol_to_dict("case", 1)
        .nud = [&g](PrattParser<PNode>& p) -> PNode {
            PNode expr = p.parse(0);
            if (!node_traits::is_convertible_to<ExpressionNode>(expr))
                g.error("expected expression after 'case'");
            if (p.next_token_as_string() != "of")
                g.error("expected 'of' after expression");

            p.advance();

            PascalGrammar::behaviour_guard<RightAssociative> comma_guard(*(g.comma),
                [&g](PNode left, PNode right) {
                    return ListVisitor<ConstantNode>(left, right, &g, "constant")
                           .get_expression();
                });

            PascalGrammar::behaviour_guard<LeftAssociative> colon_guard(*(g.colon), 1,
                [&g](PNode left, PNode right) -> PNode {
                    if (!node_traits::is_list_of<ConstantNode>(left))
                        g.error("expected list of constants before ':'");
                    if (!node_traits::is_convertible_to<StatementNode>(right))
                        g.error("expected statement after ':'");
                    return std::make_shared<CaseLimbNode>(left, right);
                });

            PascalGrammar::lbp_guard semicolon_guard(*(g.semicolon), 0);
            
            std::forward_list<PNode> limbs;

            do {
                PNode limb = p.parse(0);
                if (!node_traits::has_type<CaseLimbNode>(limb))
                    g.error("expected case-limb");
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
                    g.error("expected ';' or 'end'");
                }
            } while (true);

            limbs.reverse();
            return std::make_shared<CaseStatementNode>(expr,
                    std::make_shared<CaseLimbListNode>(std::move(limbs)));
        };
    }
}
