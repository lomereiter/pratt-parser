#ifndef PASCAL_GRAMMAR_H
#define PASCAL_GRAMMAR_H

#include "parser.h"
#include "pascal_handlers.h"

//#include <memory>
//#include <string>
//#include <functional>

#include "operator.h"
#include "node_fwd.h"

typedef std::shared_ptr<Node> PNode;

/* Grammar definition */
class PascalGrammar : public grammar::Grammar<PNode> {

    friend void pascal_grammar::add_operators(PascalGrammar&);
    friend void pascal_grammar::add_expressions(PascalGrammar&);
    friend void pascal_grammar::add_types(PascalGrammar&);
    friend void pascal_grammar::add_sections(PascalGrammar&);

    Symbol<PNode> *comma, *semicolon, *sign_eq, 
                  *colon, *opening_bracket, *end;

    std::unique_ptr<PrattParser<PNode>> parser;

    PascalGrammar();
    PascalGrammar(const PascalGrammar&) = delete;
    PascalGrammar(PascalGrammar&&) = delete;
    PascalGrammar& operator=(const PascalGrammar&) = delete;
    PascalGrammar& operator=(PascalGrammar&&) = delete;
    public:
        static PNode parse(const std::string&);
        void error(const std::string&) const;
};

#endif
