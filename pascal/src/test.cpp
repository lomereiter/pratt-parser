#include "pascal_grammar.h"

#include "pretty_printer.h"

//#include <string>
#include <iostream>
#include <fstream>
#include <streambuf>
using namespace std;

int main(int argc, const char* argv[]) {
    try {
        string code;

        if (argc == 1) {
            getline(cin, code);
        } else if (argc > 2) {
            cout << "usage: " << argv[0] << " [filename]" << '\n'
                 << "\tif filename is provided, prints its AST" << '\n'
                 << "\totherwise reads a string from stdin\n";
        } else { // argc == 2
            ifstream in(argv[1]);
            code = string(istreambuf_iterator<char>(in),
                          istreambuf_iterator<char>());
        }

        PNode node = PascalGrammar::parse(code);
        PrettyPrinter pp;
        pp.travel(node);
    } catch (SyntaxError& e) {
        cout << e.what() << endl;
    } catch (const char* msg) {
        cout << "Error: " << msg << endl;
    }
}
