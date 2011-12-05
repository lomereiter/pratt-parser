//#include <memory>

//#include "pascal_grammar.h"
//#include "node_traits.h"
#include "list_guard.h"

namespace pascal_grammar {
    namespace detail {
        template <typename ExprType>
        struct ExpressionListParser {
            PNode operator()(PascalGrammar& g) {
                PrattParser<PNode>& p = *(g.parser);
                PascalGrammar::lbp_guard comma_lbp_guard(*(g.comma), 1);
                PascalGrammar::list_guard<ExprType> comma_guard(g, g.comma, "expression");

                PNode elements = p.parse(0); /* must be less than lbp of subexpressions */

                if (!node_traits::is_list_of<ExprType>(elements))
                    g.error("expected list of expressions");

                return elements;
            }
        };
    } // namespace detail

    void add_expressions(PascalGrammar& g) {
        
        typedef PascalGrammar::RightAssociative RightAssociative;

        
       g.add_symbol_to_dict("]", 0);
       g.add_symbol_to_dict("[", 1000) 
        .nud = [&g](PrattParser<PNode>& p) -> PNode {
            PascalGrammar::behaviour_guard<PascalGrammar::LeftAssociative> range_guard(*(g.range),
                [&g](PNode left, PNode right) -> PNode {
                    if (!node_traits::is_convertible_to<ExpressionNode>(left))
                        g.error("expected expression as subrange lower bound");
                    if (!node_traits::is_convertible_to<ExpressionNode>(right))
                        g.error("expected expression as subrange upper bound");
                    return std::make_shared<SubrangeNode>(left, right);
                });
            if (p.next_token_as_string() == "]") {
                p.advance();
                return std::make_shared<SetNode>(
                        std::make_shared<ExpressionListNode>());
            }
            static detail::ExpressionListParser<SetExpressionNode> parse_set_expressions;
            PNode set = std::make_shared<SetNode>(parse_set_expressions(g));
            g.advance("]", "expected ']' after list of expressions/subranges");
            return set;
        };

       g.add_symbol_to_dict("[", 1000)/* big enough for parsing expressions like 'x + a[2]' */
        .led = [&g]
        (PrattParser<PNode>& p, PNode left) -> PNode {
            if (!node_traits::is_convertible_to<VariableNode>(left))
                g.error("expected a variable before '['");
            static detail::ExpressionListParser<ExpressionNode> parse_indices;
            PNode indices =  std::make_shared<IndexedVariableNode>(left, parse_indices(g));
            g.advance("]", "expected ']' after list of indices");
            return indices;
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
                static detail::ExpressionListParser<ExpressionNode> parse_params;
                PNode params = parse_params(g);
                g.advance(")", "expected ')' token after the list of parameters");
                return std::make_shared<FunctionDesignatorNode>(left, params);
            };
    }
}
