//#include "pascal_grammar.h"
#include "list_guard.h"

#ifdef PRINT_DEBUG
#include <iostream>
using std::cout;
using std::endl;
#endif

#include <limits>

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

       static auto parse_statement = [&g]() -> PNode {
           static auto process_if_identifier = [](PNode& node) {
                if (node_traits::has_type<IdentifierNode>(node)) {
                     // function/procedure call
                    node = std::make_shared<FunctionDesignatorNode>(node,
                               std::make_shared<ExpressionListNode>());
                }
           };

           static auto statement_is_empty = [&g]() -> bool {
               auto next = g.parser -> next_token_as_string();
               return (next == ";" || next == "end" || next == "until" || next == "else");
           };

           PrattParser<PNode>& p = *(g.parser);
           PascalGrammar::lbp_guard colon_lbp_guard(*(g.colon), 1);
           PascalGrammar::led_guard colon_guard(*(g.colon),
               [&g](PrattParser<PNode>& p, PNode left) -> PNode {
                    if (!node_traits::is_convertible_to<IntegerNumberNode>(left))
                        g.error("expected integer number as label");
                    if (statement_is_empty())
                        return std::make_shared<EmptyNode>();
                    PNode node = p.parse(0);
                    process_if_identifier(node);
                    if (!node_traits::is_convertible_to<StatementNode>(node))
                        g.error("expected statement");
                    return std::make_shared<LabeledStatementNode>(left, node);
               });

           if (statement_is_empty())
               return std::make_shared<EmptyNode>();
           PNode node = p.parse(0);
           process_if_identifier(node);
           if (!node_traits::is_convertible_to<StatementNode>(node))
               g.error("expected statement");
           return node;
       };

       static auto parse_statement_sequence = [&g]() -> PNode {
           PrattParser<PNode>& p = *(g.parser);
           std::forward_list<PNode> statements;
           while (true) {
               auto next = p.next_token_as_string();
               if (next == ";") { // some support for empty statements
                   p.advance();
               }
               next = p.next_token_as_string();
               if (next == "end" || next == "until")
                   break;
               PNode statement = parse_statement();
               statements.push_front(statement);
               next = p.next_token_as_string();
               if (next != ";") {
                   break; // handling errors is duty of the caller
               }
           }
           statements.reverse();
           return std::make_shared<StatementListNode>(std::move(statements));
       };


       g.add_symbol_to_dict("begin", 1)
        .nud = [&g](PrattParser<PNode>& p) -> PNode {
#ifdef PRINT_DEBUG
            cout << "ENTERING BEGIN" << endl;
#endif
            PascalGrammar::lbp_guard end_guard(*(g.end), 0);
            PascalGrammar::lbp_guard dot_guard(*(g.dot), 1000);

            PNode statements = parse_statement_sequence();

            g.advance("end", "expected 'end' after statement-sequence");
            return std::make_shared<CompoundStatementNode>(statements);
        };

       g.add_symbol_to_dict("do", 0);
       g.add_symbol_to_dict("while", 1)
        .nud = [&g](PrattParser<PNode>& p) -> PNode {
#ifdef PRINT_DEBUG
            cout << "ENTERING WHILE LOOP" << endl;
#endif
            PNode condition = p.parse(0);
            if (!node_traits::is_convertible_to<ExpressionNode>(condition))
                g.error("expected expression after 'while'");
            g.advance("do", "expected 'do' after expression");
            PNode body = parse_statement();
            return std::make_shared<WhileStatementNode>(condition, body);
        };

       g.add_symbol_to_dict("until", 0);
       g.add_symbol_to_dict("repeat", 1)
        .nud = [&g](PrattParser<PNode>& p) -> PNode {
#ifdef PRINT_DEBUG
            cout << "ENTERING REPEAT LOOP" << endl;
#endif
            PNode body = parse_statement_sequence();
            g.advance("until", "expected 'until' after statement-sequence");
            PNode condition = p.parse(1); // stop before semicolon
            if (!node_traits::is_convertible_to<ExpressionNode>(condition))
                g.error("expected expression after 'until'");
            return std::make_shared<RepeatStatementNode>(body, condition);
        };

       g.add_symbol_to_dict("to", 0);
       g.add_symbol_to_dict("downto", 0);
       g.add_symbol_to_dict("for", 1)
        .nud = [&g](PrattParser<PNode>& p) -> PNode {
#ifdef PRINT_DEBUG
            cout << "ENTERING FOR LOOP" << endl;
#endif
            PNode assignment = parse_statement();
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
            g.advance("do", "expected 'do' after final-expression");
            PNode body = parse_statement();
            return std::make_shared<ForStatementNode>(_assignment, sign, final_expr, body);
        };

       g.add_symbol_to_dict("then", 0);
       g.add_symbol_to_dict("else", 0);
       g.add_symbol_to_dict("if", 1)
        .nud = [&g](PrattParser<PNode>& p) -> PNode {
#ifdef PRINT_DEBUG
            cout << "ENTERING IF STATEMENT" << endl;
#endif
            PNode expr = p.parse(1);
            if (!node_traits::is_convertible_to<ExpressionNode>(expr))
                g.error("expected expression after 'if'");
            g.advance("then", "expected 'then'");
            PNode st = parse_statement();
            if (p.next_token_as_string() != "else") {
                return std::make_shared<IfThenNode>(expr, st);
            } else {
                p.advance();
                return std::make_shared<IfThenElseNode>(expr, st, parse_statement());
            }
        };

       g.add_symbol_to_dict("do", 0);
       g.add_symbol_to_dict("with", 1)
        .nud = [&g](PrattParser<PNode>& p) -> PNode {
#ifdef PRINT_DEBUG
            cout << "ENTERING WITH STATEMENT" << endl;
#endif
            PascalGrammar::list_guard<VariableNode> comma_guard(g, g.comma, "record variable");
            PNode list = p.parse(0);
            if (!node_traits::is_list_of<VariableNode>(list))
                g.error("expected list of record variables after 'with'");
            g.advance("do", "expected 'do' in with-statement");
            PNode st = parse_statement();
            return std::make_shared<WithStatementNode>(list, st);
        };

       g.add_symbol_to_dict("case", 1)
        .nud = [&g](PrattParser<PNode>& p) -> PNode {
#ifdef PRINT_DEBUG
            cout << "ENTERING CASE STATEMENT" << endl;
#endif
            PNode expr = p.parse(0);
            if (!node_traits::is_convertible_to<ExpressionNode>(expr))
                g.error("expected expression after 'case'");
            g.advance("of", "expected 'of' after expression");

            PascalGrammar::list_guard<ConstantNode> comma_guard(g, g.comma, "constant");
            PascalGrammar::lbp_guard colon_lbp_guard(*(g.colon), 1);
            PascalGrammar::led_guard colon_guard(*(g.colon),
                [&g](PrattParser<PNode>& p, PNode left) -> PNode {
                    if (!node_traits::is_list_of<ConstantNode>(left))
                        g.error("expected list of constants before ':'");
                    return std::make_shared<CaseLimbNode>(left, parse_statement());
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

       auto parse_output_list = [&g]() -> PNode {
            PrattParser<PNode>& p = *(g.parser);
            PascalGrammar::lbp_guard comma_guard(*(g.comma), 0);
            PascalGrammar::lbp_guard colon_guard(*(g.colon), 0);
            std::forward_list<PNode> output;
            do {
                PNode val = p.parse(0);
                if (!node_traits::is_convertible_to<ExpressionNode>(val))
                    g.error("expected expression as output value");

                output.push_front(val);
                PNode& last_value = output.front();

                auto next = p.next_token_as_string();
                if (next == ",") {
                    p.advance();
                    continue;
                }
                else if (next == ")") {
                    break;
                }

                if (next != ":")
                    g.error("expected ':', ',' or ')'");

                p.advance(); // skip ':'
                PNode field_width, fraction_length;
                field_width = p.parse(0);
                if (!node_traits::is_convertible_to<ExpressionNode>(field_width))
                    g.error("expected expression as field width");
                if (p.next_token_as_string() == ":") {
                    p.advance();
                    fraction_length = p.parse(0);
                    if (!node_traits::is_convertible_to<ExpressionNode>(fraction_length))
                        g.error("expected expression as fraction length");
                }
                last_value = std::make_shared<OutputValueNode>(last_value, field_width, 
                                         fraction_length ? 
                                         fraction_length : 
                                         std::make_shared<OutputValueNode>(last_value,
                                             field_width, std::make_shared<EmptyNode>()));
                next = p.next_token_as_string();
                if (next == ",") {
                    p.advance();
                    continue;
                }
                else if (next == ")") {
                    break;
                }
                else g.error("expected ',' or ')' after output value");
            } while (true);
            output.reverse();
            return std::make_shared<OutputValueListNode>(std::move(output));
       };

       g.add_symbol_to_dict("write", 1)
        .nud = [&g, parse_output_list](PrattParser<PNode>& p) -> PNode {
            g.advance("(", "expected list of values to output");
            PNode output = parse_output_list();
            g.advance(")", "expected closing ')' in 'write'");
            return std::make_shared<WriteNode>(output);
        };

       g.add_symbol_to_dict("writeln", 1)
        .nud = [&g, parse_output_list](PrattParser<PNode>& p) -> PNode {
            if (p.next_token_as_string() != "(")
                return std::make_shared<WriteLineNode>(std::make_shared<OutputValueListNode>());
            p.advance();
            PNode output = parse_output_list();
            g.advance(")", "expected closing ')' in 'writeln'");
            return std::make_shared<WriteLineNode>(output);
        };

      g.prefix("goto", std::numeric_limits<int>::max(), [&g](PNode node) -> PNode {
            if (!node_traits::is_convertible_to<IntegerNumberNode>(node))
                g.error("expected label");
            return std::make_shared<GotoStatementNode>(node);
          });
    }
}
