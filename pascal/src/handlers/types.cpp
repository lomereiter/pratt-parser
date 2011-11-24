//#include <memory>
#include <limits>

//#include "pascal_grammar.h"
//#include "node_traits.h"
//#include "node_fwd.h"
#include "ast_visitors.h"

namespace pascal_grammar {

    void add_types(PascalGrammar& g) {
       typedef PascalGrammar::RightAssociative RightAssociative;

       g.comma = &g.infix_r(",", 80, 
            [&g](PNode x, PNode y) -> PNode {
                g.error("unexpected ','");
                return nullptr;
            });

       g.range = &g.infix("..", 90, 
            [&g](PNode x, PNode y) -> PNode {
                if (!node_traits::is_convertible_to<ConstantNode>(x)) 
                    g.error("expected a constant as the lower bound");
                if (!node_traits::is_convertible_to<ConstantNode>(y))
                    g.error("expected a constant as the upper bound");
                return std::make_shared<SubrangeTypeNode>(
                    node::convert_to<ConstantNode>(x), node::convert_to<ConstantNode>(y)
                );
            });

        g.prefix("^", 80, 
             [&g](PNode x) -> PNode {
                 if (!node_traits::has_type<IdentifierNode>(x)) 
                     g.error("expected identifier after '^'");
                 return std::make_shared<PointerTypeNode>(x);
            });

        g.semicolon = &g.add_symbol_to_dict(";", 0);

        g.end = &g.add_symbol_to_dict("end", 0);
        g.add_symbol_to_dict("record", std::numeric_limits<int>::max())
        .nud = [&g](PrattParser<PNode>& p) -> PNode {
            PascalGrammar::lbp_guard semicolon_guard(*(g.semicolon), 0);

            PascalGrammar::behaviour_guard<RightAssociative> comma_guard(*(g.comma), 
                    [&g](PNode x, PNode y) {
                        return ListVisitor<IdentifierNode>(x, y, &g, "identifier")
                               .get_expression();
                    });
                
                std::forward_list<PNode> variable_declarations;

                while (true) {
                    auto next = p.next_token_as_string();
                    if (next == ";") {
                        p.advance();
                        next = p.next_token_as_string();
                        if (variable_declarations.empty()) {
                            p.advance("end"); // either ';' follows field declaration
                            break;            // or ';' is followed by 'end'
                        }
                    }
                    if (next == "end") {
                        p.advance(); // skip 'end' and return the node
                        variable_declarations.reverse();
                        break;
                    } 
                    PNode x = p.parse(1);
                    if (!node_traits::has_type<VariableDeclNode>(x))
                        g.error("expected field declaration");
                    variable_declarations.push_front(x);
                }

                return std::make_shared<RecordTypeNode>(
                        std::make_shared<VariableDeclListNode>(
                             std::move(variable_declarations)));
            };

       g.add_symbol_to_dict("set", 1)
        .nud = [](PrattParser<PNode>& p) -> PNode {
            return std::make_shared<SetTypeNode>( p.advance("of").parse(1) );
        };

       g.add_symbol_to_dict("file", 1)
        .nud = [](PrattParser<PNode>& p) -> PNode {
            return std::make_shared<FileTypeNode>( p.advance("of").parse(1) );
        };

       g.array = &g.add_symbol_to_dict("array", 1);
       g.array -> nud = [&g](PrattParser<PNode>& p) -> PNode {
            PNode bounds;
            {
                PascalGrammar::behaviour_guard<RightAssociative> guard(*(g.comma), 
                    [&g](PNode x, PNode y) {
                        return ListVisitor<IndexTypeNode>(x, y, &g, "index type")
                               .get_expression();
                    });

                if (p.next_token_as_string() != "[")
                    g.error("expected '[' after 'array'");
                p.advance();

                bounds = p.parse(10);
                if (node_traits::is_convertible_to<IndexTypeNode>(bounds))
                    bounds = node::make_list(node::convert_to<IndexTypeNode>(bounds));
                if (!node_traits::is_list_of<IndexTypeNode>(bounds))
                    g.error("expected list of index types");
            }
            if (p.next_token_as_string() != "]")
                g.error("expected ']' in array type definition");
            p.advance();

            if (p.next_token_as_string() != "of")
                g.error("expected 'of' in array type definition");
            p.advance();

            PNode type = p.parse(1);
            if (!node_traits::is_type(type)) 
                g.error("expected type in array type definition");
            return std::make_shared<ArrayTypeNode>(bounds, type);
        };

       g.packed = &g.add_symbol_to_dict("packed", 1);
       g.packed -> nud = [&g](PrattParser<PNode>& p) -> PNode {
            PNode type = p.parse(1);
            if (!node_traits::is_unpacked_structured_type(type)) {
                g.error("expected unpacked structured type after 'packed'");
                return nullptr;
            } else {
                return std::make_shared<PackedTypeNode>(type);
            }
        };
    }
} // namespace pascal_grammar

