#include "parser_impl.h"
#include "pascal_literals.h"

#include <string>
#include <sstream>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <vector>
using namespace std;
using namespace grammar;

class PPrintPreprocessor : private Grammar<string> {
private:
    vector<string> node_names;
    string code;
public:
    PPrintPreprocessor(const string& input_file_name) : Grammar<string>("(end)") {

        add_symbol_to_dict("(string literal)", 0).set_scanner(pascal::string_scanner)
                                                 .set_parser(pascal::string_parser);

        add_symbol_to_dict("(identifier)", 0).set_scanner(pascal::identifier_scanner)
                                             .set_parser(pascal::identifier_parser);

        infix("->", 10, [&node_names](string node_name, string body) -> string {
            node_names.push_back(node_name);
            return "void PrettyPrinter::visit(const std::shared_ptr<" + node_name + ">& e) {" 
                + '\n' + body + '\n' + '}' + '\n';
        });

        infix(",", 20, [](string left, string right) { return left + right; });
        infix(";", 5, [](string left, string right) { return left + right; });
        prefix("visit", 50, [](string s) { return "travel(e -> " + s + ");\n"; });
       
        prefix("indented", 40, [](string s) {
                return "indent += sw;\n" + s + "indent -= sw;\n"; });

        prefix("no_indent", 40, [](string s) {
                return "{ int old_indent = indent;\nindent = 0;\n" + s + "indent = old_indent;\n" + '}'; });
                       
        add_symbol_to_dict("visit_children", 50)
        .set_parser([](const std::string&, size_t, size_t) {
                return "for (auto child : e -> list()) travel(child);\n"; });

        brackets("{", "}", 100500, [](string s) { return s; });

        const string verbatim = "__verbatim__";

        brackets("<", ">", 100500, [=](string s) { return verbatim + "e -> " + s; }); 
        prefix("!", 200, [=](string s) { return verbatim + s; });

        auto print = [=](string s) -> string {
                string to_print = s;
                if (to_print.substr(0, verbatim.length()) == verbatim) {
                    to_print = to_print.substr(verbatim.length());
                } else {
                    to_print = '"' + to_print + '"';
                }
                return "std::cout << std::string(indent, ' ') << " + to_print + ";\n";
        };

        prefix("println", 50, [=](string s) { return print(s) + "std::cout << std::endl;\n";});
        prefix("print", 50, print);

        ifstream in(input_file_name);
        code = parse(string((istreambuf_iterator<char>(in)), istreambuf_iterator<char>()));
    }

    void generate_source(const string& filename) {
        ofstream out(filename);
        out << "#include \"visitor.h\"\n"
               "#include \"node.h\"\n"
               "#include \"pretty_printer.h\"\n"
               "#include \"operator.h\"\n"
               "#include <memory>\n"
               "#include <iostream>\n"
               "#include <functional>\n"
               "PrettyPrinter::PrettyPrinter(int sw) : indent(0), sw(sw),\n"
                                                       "is_var_section(false)\n"
                "{\n"
                "Visits<PrettyPrinter, ";

        for (auto it = node_names.cbegin(); it != node_names.cend(); ) {
            out << *it++;
            if (it != node_names.cend()) out << ", ";
        }

        out << ">();\n}\n" << code;
    }

    void generate_header(const string& filename) {
        ofstream out(filename);
        out << "#ifndef PRETTY_PRINTER_H\n"
               "#define PRETTY_PRINTER_H\n"
               "#include \"visitor.h\"\n"
               "#include \"node_fwd.h\"\n"
               "#include <memory>\n"
               "#include <iosfwd>\n"
               "#include <functional>\n"
               "\n"
               "struct PrettyPrinter : public Visitor<std::add_const> {\n"
               "PrettyPrinter(int sw=2);\n";

        for (const auto& node_name : node_names) {
            out << "void visit(const std::shared_ptr<" + node_name + " >&);\n";
        }
        out << "private:\n"
               "int indent, sw;\n"
               "bool is_var_section;\n"
               "};\n"
               "#endif";

        out.close();
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) { 
        cout << "usage: pp_gen input_file output_dir" << '\n'
             << "    the program creates output_dir/src/pretty_printer.cpp and \n"
                "                        output_dir/include/pretty_printer.h\n";
        return 0;
    }

    string input = argv[1];
    string output = argv[2];
    PPrintPreprocessor proc(input);
    proc.generate_source(output + "/src/pretty_printer.cpp");
    proc.generate_header(output + "/include/pretty_printer.h");
}
