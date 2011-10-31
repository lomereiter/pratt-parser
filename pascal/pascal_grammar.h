#ifndef PASCAL_GRAMMAR_H
#define PASCAL_GRAMMAR_H

#include "../parser/parser.h"

#include <memory>
#include <string>
#include <functional>
#include <sstream>
#include <forward_list>

#include "operator.h"
#include "expression.h"
#include "ast_visitors.h"
#include "syntax_error.h"
#include "node_traits.h"

typedef std::shared_ptr<Node> PNode;

/* Grammar definition */
class PascalGrammar : public grammar::Grammar<PNode> {

    static std::function<PNode(PNode, PNode)> createLed(Operator op);
    static std::function<PNode(PNode)> createNud(Operator op);
    static std::function<PNode(PNode)> createSignNud(char sign);

    Symbol<PNode> *comma, *semicolon, *sign_eq;

    public:
        PascalGrammar();
};

#endif
