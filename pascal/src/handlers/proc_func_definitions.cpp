//#include "pascal_grammar.h"
#include "ast_visitors.h"

/* TODO:
 *
 * functions
 *
 * proc/func definitions
 * -------------------------------------------------------------------
 * use unique_ptrs instead of shared_ptr ???
 *
 * error handling in OperationNode
 *
 * get rid of p.advance("...")
 */

namespace pascal_grammar {
    namespace detail {
        struct bound_specification_guard {
            PascalGrammar::lbp_guard colon_guard;
            PascalGrammar::led_guard range_guard;
            PascalGrammar::lbp_guard sl_guard;
            PascalGrammar::behaviour_guard<PascalGrammar::RightAssociative> sb_guard;
            bound_specification_guard(PascalGrammar& g) :
                colon_guard(*(g.colon), 0),
                range_guard(*(g.range),
                [&g](PrattParser<PNode>& p, PNode left) -> PNode {
                    if (!node_traits::has_type<IdentifierNode>(left))
                        g.error("expected identifier before '..'");
                    PNode right = p.parse(0);

                    if (!node_traits::has_type<IdentifierNode>(right))
                        g.error("expected identifier after '..'");
                    g.advance(":", "expected ':' in bound specification");
                    
                    PNode type = p.parse(1); // semicolon ~ 1

                    if (!node_traits::has_type<IdentifierNode>(type))
                        g.error("expected ordinal type identifier after ':'");

                    return std::make_shared<BoundSpecificationNode>(left, right, type);
                }),
                sl_guard(*(g.semicolon), 1),
                sb_guard(*(g.semicolon), 
                [&g](PNode left, PNode right) {
                    return ListVisitor<BoundSpecificationNode>(left, right, &g,
                                       "bound specification").get_expression();
                }) {}
        };

PNode parse_formal_parameter_list(PrattParser<PNode>& p, PascalGrammar& g) {

    typedef PascalGrammar::RightAssociative RightAssociative;
    typedef PascalGrammar::LeftAssociative LeftAssociative;
    typedef PascalGrammar::Prefix Prefix;

    PascalGrammar::nud_guard array_guard(*(g.array), 
        [&g](PrattParser<PNode>& p) -> PNode {
            g.advance("[", "expected '[' after 'array'");
           
            pascal_grammar::detail::bound_specification_guard bg(g);

            /// scan list of bound specifications
            PNode bounds = p.parse(0); // semicolon led gets executed
            if (!node_traits::is_list_of<BoundSpecificationNode>(bounds))
                g.error("expected list of bound specifications after 'array ['");

            g.advance("]", "expected ']' in array parameter type declaration");
            g.advance("of", "expected 'of' after ']'");

            PNode type = p.parse(1); // parsing stops before semicolon
            if (!node_traits::has_type<IdentifierNode>(type) &&
                !node_traits::is_conformant_array_schema(type))
                g.error("expected type identifier or conformant-array-schema after 'of'");

            return std::make_shared<UCArraySchemaNode>(bounds, type);
        });

    PascalGrammar::nud_guard packed_guard(*(g.packed),
        [&g](PrattParser<PNode>& p) -> PNode {
            g.advance("array", "expected 'array' after 'packed'");

            pascal_grammar::detail::bound_specification_guard bg(g);

            g.advance("[", "expected '[' after 'packed array'");

            PNode bounds = p.parse(0);
            if (!node_traits::has_type<BoundSpecificationNode>(bounds))
                g.error("expected bound specification after 'packed array ['");

            g.advance("]", "expected ']' after bound specification");
            g.advance("of", "expected 'of' after ']'");
            
            PNode id = p.parse(1); // see array_guard

            if (!node_traits::has_type<IdentifierNode>(id))
                g.error("expected type identifier after 'of'");

            return std::make_shared<PCArraySchemaNode>(bounds, id);
        });

    PascalGrammar::nud_guard var_guard(*(g.var),
        [&g](PrattParser<PNode>& p) -> PNode {

            /* comma ~ 80, semicolon ~ 1, colon ~ 70 */
            PNode id_list = p.parse(70);
            if (!node_traits::is_list_of<IdentifierNode>(id_list))
                g.error("expected list of identifiers before ':'");

            g.advance(":", "expected ':' after identifier list");

            PNode param_type = p.parse(1); // stop before semicolon
            if (!node_traits::is_parameter_type(param_type))
                g.error("expected parameter type after ':'");

            return std::make_shared<VariableParameterNode>(id_list, param_type);
        });

    PascalGrammar::behaviour_guard<RightAssociative> semicolon_guard(*(g.semicolon),
        [&g](PNode left, PNode right) {
            return ListVisitor<ParameterNode>(left, right, &g,
                    "formal parameter section").get_expression();
        });

    PascalGrammar::behaviour_guard<LeftAssociative> colon_guard(*(g.colon), 30,
        [&g](PNode id_list, PNode param_type) -> PNode {
            if (!node_traits::is_list_of<IdentifierNode>(id_list))
                g.error("expected list of identifiers before ':'");
            if (!node_traits::is_parameter_type(param_type))
                g.error("expected parameter type after ':'");

            return std::make_shared<ValueParameterNode>(id_list, param_type);
        });

    // --------------- scan parameters -------------------------------------- 
    
    PNode params = p.parse(0);
    if (!node_traits::is_list_of<ParameterNode>(params))
        g.error("expected list of formal parameters");
    return params;
}

} // namespace detail
} // namespace pascal_grammar

namespace pascal_grammar {

    void add_procedures_and_functions(PascalGrammar& g) {

        g.add_symbol_to_dict("procedure", 1)
         .nud = [&g](PrattParser<PNode>& p) -> PNode {
              PascalGrammar::lbp_guard ob_guard(*(g.opening_bracket), 0);
              PascalGrammar::lbp_guard semicolon_lbp_guard(*(g.semicolon), 1);
              PascalGrammar::behaviour_guard<PascalGrammar::RightAssociative> comma_guard(*(g.comma),
                [&g](PNode left, PNode right) {
                    return ListVisitor<IdentifierNode>(left, right, &g, "identifier")
                           .get_expression();
                });

              PNode name_ = p.parse(1);
              std::string name;
              if (!node_traits::has_type<IdentifierNode>(name_)) {
                  g.error("expected procedure name");
              } else {
                  name = std::static_pointer_cast<IdentifierNode>(name_) -> name;
              }
              auto next = p.next_token_as_string();
              if (next != "(") {
                  return std::make_shared<ProcedureHeadingNode>(name,
                          std::make_shared<ParameterListNode>());
              } else {
                  p.advance();
                  PNode params = pascal_grammar::detail::parse_formal_parameter_list(p, g);
                  g.advance(")", "expected ')' after formal parameter list");
                  return std::make_shared<ProcedureHeadingNode>(name, params);
              }
         };

        g.add_symbol_to_dict("function", 1)
         .nud = [&g](PrattParser<PNode>& p) -> PNode {
              PascalGrammar::lbp_guard ob_guard(*(g.opening_bracket), 0);
              PascalGrammar::lbp_guard semicolon_guard(*(g.semicolon), 1);
              PascalGrammar::lbp_guard colon_guard(*(g.colon), 1);
              PascalGrammar::behaviour_guard<PascalGrammar::RightAssociative> comma_guard(*(g.comma),
                [&g](PNode left, PNode right) {
                    return ListVisitor<IdentifierNode>(left, right, &g, "identifier")
                           .get_expression();
                });

              PNode name_ = p.parse(1);
              std::string name;
              if (!node_traits::has_type<IdentifierNode>(name_)) {
                  g.error("expected function name");
              } else {
                  name = std::static_pointer_cast<IdentifierNode>(name_) -> name;
              }

              PNode params = std::make_shared<ParameterListNode>();
              auto next = p.next_token_as_string();
              if (next != "(") {
                  g.advance(":", "expected ':' in function heading");
              } else {
                  p.advance();
                  params = pascal_grammar::detail::parse_formal_parameter_list(p, g);

                  g.advance(")", "expected ')' after formal parameter list");
                  g.advance(":", "expected ':' in function heading");
              }

              PNode ret = p.parse(1);
              if (!node_traits::has_type<IdentifierNode>(ret))
                  g.error("expected type identifier");

              return std::make_shared<FunctionHeadingNode>(name, params, ret);
         };

    }

}
