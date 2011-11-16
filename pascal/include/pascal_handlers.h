#ifndef PASCAL_HANDLERS_H
#define PASCAL_HANDLERS_H

class PascalGrammar;

namespace pascal_grammar {
    void add_literals(PascalGrammar&);
    void add_operators(PascalGrammar&);
    void add_expressions(PascalGrammar&);
    void add_types(PascalGrammar&);
    void add_sections(PascalGrammar&);
}

#endif
