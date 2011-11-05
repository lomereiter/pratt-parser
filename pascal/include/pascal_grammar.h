#ifndef PASCAL_GRAMMAR_H
#define PASCAL_GRAMMAR_H

#include "parser.h"

#include <memory>
#include <string>
#include <functional>

#include "operator.h"
#include "node.h"

typedef std::shared_ptr<Node> PNode;

/* Grammar definition */
class PascalGrammar : public grammar::Grammar<PNode> {

    static std::function<PNode(PNode, PNode)> createLed(Operator op);
    static std::function<PNode(PNode)> createNud(Operator op);
    static std::function<PNode(PNode)> createSignNud(char sign);

    Symbol<PNode> *comma, *semicolon, *sign_eq;

    public:
        PascalGrammar();
        PNode parse(const std::string&);
};

#endif
