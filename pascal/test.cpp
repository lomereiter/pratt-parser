#include "pascal_grammar.h"
#include "pretty_printer.h"

#include <string>
#include <iostream>
using namespace std;

int main() {
    PascalGrammar pg;

    try {
        string s;
        getline(cin, s);
        PNode e = pg.parse(s);
        PrettyPrinter pp;
        pp.travel(e);
    } catch (SyntaxError& e) {
        cout << "Syntax error: " << e.what() << endl;
    } catch (const char* msg) {
        cout << "Syntax error; " << msg << endl;
    }
}
