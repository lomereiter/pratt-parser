#include <limits>
//#include <functional>
//#include <memory>

#include "pascal_grammar.h"
//#include "operator.h"
//#include "node.h"
#include "node_traits.h"

namespace pascal_grammar {

    void add_operators(PascalGrammar& g) {

        auto createLed = [&g](Operator op) -> std::function<PNode(PNode, PNode)> {
            return [&g, op](PNode x, PNode y) -> std::shared_ptr<OperationNode> {
                if (!node_traits::is_convertible_to<ExpressionNode>(x) ||
                    !node_traits::is_convertible_to<ExpressionNode>(y))
                    g.error("expected expression");
                std::shared_ptr<OperationNode> expr = std::make_shared<OperationNode>(2, op);
                expr -> args.push_front(y);
                expr -> args.push_front(x);
                return expr;
            };
        };

        auto createNud = [&g](Operator op) -> std::function<PNode(PNode)> {
            return [&g, op](PNode x) -> std::shared_ptr<OperationNode> {
                if (!node_traits::is_convertible_to<ExpressionNode>(x))
                    g.error("expected expression");
                std::shared_ptr<OperationNode> expr = std::make_shared<OperationNode>(1, op);
                expr -> args.push_front(x);
                return expr;
            };
        };

        auto createSignNud = [&g](char sign) -> std::function<PNode(PNode)> {
            return [&g, sign](PNode x) -> PNode {
                if (node_traits::has_type<UIntegerNumberNode>(x))
                    return std::make_shared<IntegerNumberNode>(x, sign);
                if (node_traits::has_type<URealNumberNode>(x))
                    return std::make_shared<RealNumberNode>(x, sign);
                if (!node_traits::is_convertible_to<ExpressionNode>(x))
                    g.error(std::string("expected expression after ") + sign);
                return std::make_shared<SignNode>(sign, x);
            };
        };
                 /* negating operator */
        g.prefix("not", 200, createNud(opNot));

                            /* multiplicative operators */
        g.infix("*", 150, createLed(opMul));      g.infix("/", 150, createLed(opDiv));
        g.infix("div", 150, createLed(opIntDiv)); g.infix("mod", 150, createLed(opIntMod));
        g.infix("shl", 150, createLed(opShl));    g.infix("shr", 150, createLed(opShr));
                            g.infix("and", 150, createLed(opAnd));

                            /* additive operators */
        g.infix("+", 100, createLed(opAdd));      g.infix("-", 100, createLed(opSub));
        g.infix("or", 100, createLed(opOr));      g.infix("xor", 100, createLed(opXor));

                            /* relational operators */
                            g.sign_eq = &g.infix("=", 50, createLed(opEq));       
        g.infix("<>", 50, createLed(opNotEq));    g.infix("in", 50, createLed(opIn));
        g.infix("<", 50, createLed(opLess));      g.infix("<=", 50, createLed(opLessEq));
        g.infix(">", 50, createLed(opMore));      g.infix(">=", 50, createLed(opMoreEq));

                            /* sign operators */
        g.prefix("+", 150, createSignNud('+'), grammar::keep_symbol_lbp);   
        g.prefix("-", 150, createSignNud('-'), grammar::keep_symbol_lbp);
        /* </operators> */

        g.add_symbol_to_dict(")", 0);

        (g.opening_bracket = &g.add_symbol_to_dict("(", std::numeric_limits<int>::max()))
        -> nud = [&g](PrattParser<PNode>& p) -> PNode {
                PNode x = p.parse(0);
                g.advance(")", "expected closing ')'");

                if (!node_traits::is_convertible_to<ExpressionNode>(x)) 
                    g.error("expected expression after '('");

                return x;
        };
    }

}
