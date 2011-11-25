//#include <memory>

//#include "pascal_grammar.h"
//#include "node_traits.h"
#include "ast_visitors.h"

namespace pascal_grammar {

    void add_expressions(PascalGrammar& g) {
        
        typedef PascalGrammar::RightAssociative RightAssociative;

        auto parse_expression_list_and_closing_bracket = 
        [&g](PrattParser<PNode>& p) -> PNode {
            PascalGrammar::behaviour_guard<RightAssociative> comma_guard(*(g.comma),
                    [&g](PNode x, PNode y) {
                        return ListVisitor<ExpressionNode>(x, y, &g, "expression")
                               .get_expression();
                    });

            PNode elements = p.parse(40); /* must be less than lbp of subexpressions */

            if (!node_traits::is_list_of<ExpressionNode>(elements))
                g.error("expected list of expressions after '['");

            g.advance("]", "expected ']' after list of expressions");
            return elements;
        };

       g.add_symbol_to_dict("]", 0);
       g.add_symbol_to_dict("[", 1000) 
        .nud = [parse_expression_list_and_closing_bracket](PrattParser<PNode>& p) -> PNode {
            if (p.next_token_as_string() == "]") {
                p.advance();
                return std::make_shared<SetNode>(
                        std::make_shared<ExpressionListNode>());
            }
            return std::make_shared<SetNode>(parse_expression_list_and_closing_bracket(p));
        };

       g.add_symbol_to_dict("[", 1000)/* big enough for parsing expressions like 'x + a[2]' */
        .led = [&g, parse_expression_list_and_closing_bracket]
        (PrattParser<PNode>& p, PNode left) -> PNode {
            if (!node_traits::is_convertible_to<VariableNode>(left))
                g.error("expected a variable before '['");
            return std::make_shared<IndexedVariableNode>(left,
                    parse_expression_list_and_closing_bracket(p));
        };

       g.postfix("^", 1000, [&g](PNode node) -> PNode {
                if (!node_traits::is_convertible_to<VariableNode>(node)) 
                    g.error("expected a variable before '^'");
                return std::make_shared<ReferencedVariableNode>(node);
            });

       g.dot = &g.infix(".", 1000, [&g](PNode var, PNode field) -> PNode {
            if (!node_traits::is_convertible_to<VariableNode>(var))
                g.error("expected a variable before '.'");
            if (!node_traits::has_type<IdentifierNode>(field)) 
                g.error("expected identifier after '.'");
                return std::make_shared<FieldDesignatorNode>(var, field);
            });
       g.dot -> lbp = 0; // 0 is changed to 1000 in 'begin' handler

       g.add_symbol_to_dict(")", 0);
       g.add_symbol_to_dict("(", 1000) /* foo ( x, y, ... ) */
        .led = [&g](PrattParser<PNode>& p, PNode left) -> PNode {
                if (!node_traits::has_type<IdentifierNode>(left))
                    g.error("expected identifier before '(' token");
                PascalGrammar::behaviour_guard<RightAssociative> comma_guard(*(g.comma),
                    [&g](PNode left, PNode right) {
                        return ListVisitor<ExpressionNode>(left, right, &g, "expression")
                               .get_expression();
                    });
                PNode params = p.parse(0);
                if (!node_traits::is_list_of<ExpressionNode>(params))
                    g.error("expected list of parameters after '(' token");
                g.advance(")", "expected ')' token after the list of parameters");
                return std::make_shared<FunctionDesignatorNode>(left, params);
            };
    }
}
