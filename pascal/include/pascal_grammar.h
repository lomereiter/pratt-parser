#ifndef PASCAL_GRAMMAR_H
#define PASCAL_GRAMMAR_H

#include "parser.h"

#include "pascal_handlers.h"

#include <memory>
#include <string>
#include <functional>

#include "operator.h"
#include "node_fwd.h"

class PascalGrammar;
typedef std::shared_ptr<Node> PNode;

namespace pascal_grammar {
    namespace detail {
        struct bound_specification_guard;
        template <typename ExprType> struct ExpressionListParser;
        PNode parse_formal_parameter_list(PrattParser<PNode>&, PascalGrammar&);
    }
}

/* Grammar definition */
class PascalGrammar : public grammar::Grammar<PNode> {

    friend void pascal_grammar::add_operators(PascalGrammar&);
    friend void pascal_grammar::add_expressions(PascalGrammar&);
    friend void pascal_grammar::add_types(PascalGrammar&);
    friend void pascal_grammar::add_sections(PascalGrammar&);
    friend void pascal_grammar::add_statements(PascalGrammar&);

    friend void pascal_grammar::add_procedures_and_functions(PascalGrammar&);
    friend PNode pascal_grammar::detail::parse_formal_parameter_list
                                       (PrattParser<PNode>&, PascalGrammar&);
    friend struct pascal_grammar::detail::bound_specification_guard;
    friend struct pascal_grammar::detail::ExpressionListParser<ExpressionNode>;
    friend struct pascal_grammar::detail::ExpressionListParser<SetExpressionNode>;

    Symbol<PNode> *comma, *semicolon, *sign_eq, 
                  *colon, *opening_bracket, *end,
                  *range, *array, *packed, *var, *dot;

    std::unique_ptr<PrattParser<PNode>> parser;

    PascalGrammar();
    PascalGrammar(const PascalGrammar&) = delete;
    PascalGrammar(PascalGrammar&&) = delete;
    PascalGrammar& operator=(const PascalGrammar&) = delete;
    PascalGrammar& operator=(PascalGrammar&&) = delete;
    public:
        static PNode parse(const std::string&);
        void error(const std::string&) const;
        void advance(const std::string&, const std::string&);
};

#endif
