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
                return std::make_shared<SubrangeNode>(x, y);
            });

        g.prefix("^", 80, 
             [&g](PNode x) -> PNode {
                 if (!node_traits::has_type<IdentifierNode>(x)) 
                     g.error("expected identifier after '^'");
                 return std::make_shared<PointerTypeNode>(x);
            });

        g.semicolon = &g.add_symbol_to_dict(";", 0);

        static struct {
            PNode operator()(PascalGrammar& g) {
                PrattParser<PNode>& p = *(g.parser);

                PascalGrammar::lbp_guard semicolon_guard(*(g.semicolon), 0);
                PascalGrammar::behaviour_guard<RightAssociative> comma_guard(*(g.comma), 
                        [&g](PNode x, PNode y) {
                            return ListVisitor<IdentifierNode>(x, y, &g, "identifier")
                                   .get_expression();
                        });
                
                auto next = p.next_token_as_string();
                if (next == ";") {
                    p.advance();
                    next = p.next_token_as_string();
                    if (next == ")" || next == "end") {
                    return std::make_shared<FieldListNode>(
                        std::make_shared<EmptyNode>(), std::make_shared<EmptyNode>());
                    } else {
                        g.error("expected 'end' or ')' after ';'");
                    }
                }
                if (next == ")" || next == "end") {
                    return std::make_shared<FieldListNode>(
                            std::make_shared<EmptyNode>(), std::make_shared<EmptyNode>());
                }
                PNode fixed_part;
                PNode variant_part;
                if (next != "case") {
                    // parse fixed part
                    std::forward_list<PNode> record_sections;
                    while (true) {
                        PNode sect = p.parse(1);
                        if (!node_traits::has_type<VariableDeclNode>(sect))
                            g.error("expected record section");
                        record_sections.push_front(
                            std::make_shared<RecordSectionNode>(
                                std::static_pointer_cast<VariableDeclNode>(sect)));
                        auto next = p.next_token_as_string();
                        if (next == ")" || next == "end") {
                            record_sections.reverse();
                            fixed_part = std::make_shared<FixedPartNode>(std::move(record_sections));
                            break;
                        }
                        g.advance(";", "expected ';' after record section");
                        next = p.next_token_as_string();
                        if (next == "case" || next == ")" || next == "end") {
                            record_sections.reverse();
                            fixed_part = std::make_shared<FixedPartNode>(std::move(record_sections));
                            break;
                        }
                    }
                }
                if (p.next_token_as_string() == "case") {
                    PNode tag_field, type_id;
                    p.advance(); // skip 'case'
                    type_id = p.parse(70); // colon lbp
                    if (!node_traits::has_type<IdentifierNode>(type_id))
                        g.error("expected identifier in variant part");
                    auto next = p.next_token_as_string();
                    if (next == ":") {
                        tag_field = std::move(type_id);
                        type_id = p.parse(0);
                        if (!node_traits::has_type<IdentifierNode>(type_id))
                            g.error("expected type identifier");
                    }
                    g.advance("of", "expected 'of' in variant part");
                    PascalGrammar::behaviour_guard<RightAssociative> comma_guard(*(g.comma), 90,
                        [&g](PNode left, PNode right) -> PNode {
                        return ListVisitor<ConstantNode>(left, right, &g, "constant")
                                                        .get_expression();
                        });
                    std::forward_list<PNode> variants;
                    while (true) {
                        PNode case_label_list = p.parse(89);
                        if (!node_traits::is_list_of<ConstantNode>(case_label_list))
                            g.error("expected case label list");
                        g.advance(":", "expected ':' after case label list");
                        g.advance("(", "expected '(' token after ':'");
                        PNode field_list = operator()(g);
                        if (!node_traits::has_type<FieldListNode>(field_list))
                            g.error("expected field list");
                        g.advance(")", "expected ')' token");

                        variants.push_front(std::make_shared<FieldVariantNode>(
                                    case_label_list, field_list));

                        auto next = p.next_token_as_string();
                        if (next != ";") {
                            variants.reverse();
                            variant_part = std::make_shared<VariantPartNode>(std::move(variants));
                            break;
                        }
                        // next == ";"
                        p.advance();
                        /* this might be the end of field list in two cases:
                         * 1) record ... ; end
                         * 2) case ... of ... : ( ... ; )
                         */
                        next = p.next_token_as_string();
                        if (next == ")" || next == "end")
                            break;
                    }
                }
                return std::make_shared<FieldListNode>(
                        fixed_part ? fixed_part : std::make_shared<EmptyNode>(),
                        variant_part ? variant_part : std::make_shared<EmptyNode>());
            }
        } parse_field_list;

        g.end = &g.add_symbol_to_dict("end", 0);
        g.add_symbol_to_dict("record", std::numeric_limits<int>::max())
        .nud = [&g](PrattParser<PNode>&) -> PNode {
            PNode field_list = parse_field_list(g);
            if (!node_traits::has_type<FieldListNode>(field_list))
                g.error("expected field list");
            g.advance("end", "expected 'end'");
            return std::make_shared<RecordTypeNode>(field_list);
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

                g.advance("[", "expected '[' after 'array'");

                bounds = p.parse(10);
                if (node_traits::is_convertible_to<IndexTypeNode>(bounds))
                    bounds = node::make_list(node::convert_to<IndexTypeNode>(bounds));
                if (!node_traits::is_list_of<IndexTypeNode>(bounds))
                    g.error("expected list of index types");
            }

            g.advance("]", "expected ']' in array type definition");
            g.advance("of", "expected 'of' in array type definition");

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

