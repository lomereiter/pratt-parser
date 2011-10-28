#include "../parser/parser.h"

#include <memory>
#include <string>
#include <functional>
#include <sstream>
#include <forward_list>
#include <iostream>

#include "operator.h"
#include "expression.h"
#include "ast_visitors.h"
#include "syntax_error.h"
#include "node_traits.h"
#include "visitor.h"

using namespace std;
using namespace grammar;

typedef std::shared_ptr<Node> PNode;

/* Grammar definition */
class PascalGrammar : public Grammar<PNode> {

    static std::function<PNode(PNode, PNode)> createLed(Operator op) {
        return [op](PNode x, PNode y) -> std::shared_ptr<ExpressionNode> {
            std::shared_ptr<ExpressionNode> expr = std::make_shared<ExpressionNode>(2, op);
            expr -> args.push_front(y);
            expr -> args.push_front(x);
            return expr;
        };
    }

    static std::function<PNode(PNode)> createNud(Operator op) {
        return [op](PNode x) -> std::shared_ptr<ExpressionNode> {
            std::shared_ptr<ExpressionNode> expr = std::make_shared<ExpressionNode>(1, op);
            expr -> args.push_front(x);
            return expr;
        };
    }

    static std::function<PNode(PNode)> createSignNud(char sign) {
        return [sign](PNode x) -> std::shared_ptr<SignNode> {
            return std::make_shared<SignNode>(sign, x);
        };
    }

    Symbol<PNode> *comma, *semicolon;

    public:
        PascalGrammar() : Grammar<PNode>("(end)") {

            add_symbol_to_dict("(number)", 0)
            .set_scanner(
            [](const std::string& str, size_t pos) -> size_t {
                  size_t i = pos;
                  if (i >= str.length() || !isdigit(str[i])) return pos;
                  while (i < str.length() && isdigit(str[i])) 
                      ++i; // reading digits before dot
                  if (i < str.length() && str[i] == '.') { // if dot is presented
                      ++i;
                      if (i >= str.length() || !isdigit(str[i])) return i - 1;
                      while (i < str.length() && isdigit(str[i]))
                          ++i; // read digits after dot
                  }
                  if (i < str.length() && (str[i] == 'E' || str[i] == 'e')) {
                      ++i; // read optional exponent
                      if (i >= str.length()) return pos;
                      if (str[i] == '+' || str[i] == '-')
                          ++i;
                      if (i >= str.length() || !isdigit(str[i])) return pos;
                      while (i < str.length() && isdigit(str[i]))
                          ++i;
                  }
                  return i;
            })
            .set_parser(
            [](const std::string& str, size_t beg, size_t end) -> PNode {
                return std::make_shared<NumberNode>(str.substr(beg, end - beg));
            });

            add_symbol_to_dict("(identifier)", 0)
            .set_scanner(
            [](const std::string& str, size_t pos) -> size_t {
                size_t i = pos;
                if (i < str.length() && (isalpha(str[i]) || str[i] == '_'))
                    ++i;
                else
                    return pos; 
                while (i < str.length() && (isalnum(str[i]) || str[i] == '_'))
                    ++i;
                return i;
            })
            .set_parser([](const std::string& str, size_t beg, size_t end) {
                return std::make_shared<IdentifierNode>(str.substr(beg, end - beg));
            });

            add_symbol_to_dict("(string literal)", 0)
            .set_scanner(
            [](const std::string& str, size_t pos) -> size_t {
                size_t i = pos;
                for ( ; ; ) {
                    if (i >= str.length() || str[i] != '\'') return i;
                    ++i;
                    while (i < str.length() && str[i] != '\'')
                        ++i;
                    if (i >= str.length()) return pos;
                    ++i;
                }
            })
            .set_parser([](const std::string& str, size_t beg, size_t end) -> PNode {
                std::stringstream sstr;
                for (size_t pos = beg + 1; pos < end - 1; ++pos) {
                    if (str[pos] != '\'') sstr << str[pos];
                    else { 
                        sstr << '\'';
                        ++pos;
                    }
                }
                return std::make_shared<StringNode>(sstr.str());
            });

            /* <operators> */
                                /* negating operator */
            prefix("not", 200, createNud(opNot));

                                /* multiplicative operators */
            infix("*", 150, createLed(opMul));      infix("/", 150, createLed(opDiv));
            infix("div", 150, createLed(opIntDiv)); infix("mod", 150, createLed(opIntMod));
            infix("shl", 150, createLed(opShl));    infix("shr", 150, createLed(opShr));
                                infix("and", 150, createLed(opAnd));

                                /* additive operators */
            infix("+", 100, createLed(opAdd));      infix("-", 100, createLed(opSub));
            infix("or", 100, createLed(opOr));      infix("xor", 100, createLed(opXor));

                                /* relational operators */
            infix("=", 50, createLed(opEq));        infix("<>", 50, createLed(opNotEq));
            infix("<", 50, createLed(opLess));      infix("<=", 50, createLed(opLessEq));
            infix(">", 50, createLed(opMore));      infix(">=", 50, createLed(opMoreEq));
                                infix("in", 50, createLed(opIn));

                                /* sign operators */
            prefix("+", 150, createSignNud('+')); prefix("-", 150, createSignNud('-'));
            /* </operators> */

            add_symbol_to_dict(")", 0);

            add_symbol_to_dict("(", std::numeric_limits<int>::max()) 
            .nud = [this](PrattParser<PNode>& p) -> PNode {

                    behaviour_guard<RightAssociative> guard(*(this -> comma), 
                        [](PNode x, PNode y) {
                            return ListVisitor<IdentifierNode>(x, y).get_expression();
                        });

                    PNode x = p.parse(0);
                    p.advance(")");

                    OpenBracketVisitor v;
                    v.travel(x);
                    return v.get_expression();
            };

comma=     &infix_r(",", 80, [](PNode x, PNode y) -> PNode {
                                throw SyntaxError("unexpected ','");
                                return nullptr; });

            infix("..", 90, 
                    [](PNode x, PNode y) -> PNode {
                        if (!node_traits::is_constant(x)) 
                            throw SyntaxError("expected a constant as the lower bound");
                        if (!node_traits::is_constant(y))
                            throw SyntaxError("expected a constant as the upper bound");
                        return std::make_shared<SubrangeTypeNode>(
                            node::convert_to<ConstantNode>(x), node::convert_to<ConstantNode>(y)
                        );
                    });

            prefix("^", 80, 
                    [](PNode x) -> PNode {
                        if (!node_traits::has_type<IdentifierNode>(x)) 
                            throw SyntaxError("expected identifier after '^'");
                        return std::make_shared<PointerTypeNode>(x);
                    });

            infix(":", 70, 
                    [](PNode x, PNode y) -> PNode {
                        if (!node_traits::is_list_of<IdentifierNode>(x)) 
                            throw SyntaxError("expected identifier list");
                        if (!node_traits::is_type(y))
                            throw SyntaxError("expected type name");
                        return std::make_shared<VariableDeclNode>(x, y);
                    });

semicolon= &infix_r(";", 65, [](PNode, PNode) -> PNode {
                                throw SyntaxError("unexpected ';'");
                                return nullptr; });

            add_symbol_to_dict("end", 0);
            add_symbol_to_dict("record", std::numeric_limits<int>::max())
            .nud = [this](PrattParser<PNode>& p) -> PNode {

                        auto next = p.next_token_as_string();

                        if (next == ";") {
                            p.advance().advance("end");
                            return std::make_shared<RecordTypeNode>(nullptr);
                        } else if (next == "end") {
                            p.advance();
                            return std::make_shared<RecordTypeNode>(nullptr);
                        } 

                        behaviour_guard<Postfix> semicolon_guard(*(this -> semicolon),
                            [](PNode x) { return x; }); // identity function

                        behaviour_guard<RightAssociative> comma_guard(*(this -> comma), 
                            [](PNode x, PNode y) {
                                return ListVisitor<IdentifierNode>(x, y).get_expression();
                            });
                        
                        std::forward_list<PNode> variable_declarations;

                        while ( p.next_token_as_string() != "end" ) {
                            PNode x = p.parse(0);
                            if (!node_traits::has_type<VariableDeclNode>(x))
                                throw SyntaxError("expected variable declaration");
                            variable_declarations.push_front(x);
                        }
                        p.advance();

                        variable_declarations.reverse();

                        return std::make_shared<RecordTypeNode>(
                                    std::make_shared<VariableDeclListNode>(
                                         std::move(variable_declarations)));
                    };

            add_symbol_to_dict("[", 10);
            add_symbol_to_dict("]", 0);
            add_symbol_to_dict("of", 1);

            add_symbol_to_dict("set", 1)
            .nud = [](PrattParser<PNode>& p) -> PNode {
                return std::make_shared<SetTypeNode>( p.advance("of").parse(1) );
            };

            add_symbol_to_dict("file", 1)
            .nud = [](PrattParser<PNode>& p) -> PNode {
                return std::make_shared<FileTypeNode>( p.advance("of").parse(1) );
            };

            add_symbol_to_dict("array", 1)
            .nud = [this](PrattParser<PNode>& p) -> PNode {
                PNode bounds;
                {
                    behaviour_guard<RightAssociative> guard(*(this -> comma), 
                        [](PNode x, PNode y) {
                            return ListVisitor<IndexTypeNode>(x, y).get_expression();
                        });

                    bounds = p.advance("[").parse(10);
                    if (node_traits::is_convertible_to<IndexTypeNode>(bounds))
                        bounds = node::make_list(node::convert_to<IndexTypeNode>(bounds));
                    if (!node_traits::is_list_of<IndexTypeNode>(bounds))
                        throw SyntaxError("expected list of index types");
                }
                PNode type = p.advance("]").advance("of").parse(1);
                if (!node_traits::is_type(type)) 
                    throw SyntaxError("expected type in array type definition");
                return std::make_shared<ArrayTypeNode>(bounds, type);
            };
        }
};

struct PrettyPrinter : public Visitor {
    PrettyPrinter(int sw=2) : _indent(0), sw(sw) {
        Visits<PrettyPrinter,
               NumberNode, IdentifierNode, ExpressionNode, IdentifierListNode,
               StringNode, SignNode, ConstantNode, SubrangeTypeNode,
               EnumeratedTypeNode, FileTypeNode, SetTypeNode, ArrayTypeNode,
               PointerTypeNode, VariableDeclNode, VariableDeclListNode,
               RecordTypeNode, IndexTypeNode, IndexTypeListNode>(*this);
    }

    void visit(PNode e) {
        indent() << "IMPLEMENT ME!" << endl;
    }

    void visit(std::shared_ptr<NumberNode> e) {
        indent() << e -> value << '\n';
    }

    void visit(std::shared_ptr<IdentifierNode> e) { 
        indent() << "IDENTIFIER " << e -> name << '\n'; 
    }

    void visit(std::shared_ptr<ExpressionNode> e) {
        indent() << operators::operatorName[e -> op()] << '\n';
        indented([=]() -> void {
            for (const auto& node : e -> args)
                travel(node);
        });
    }
    
    void visit(std::shared_ptr<IdentifierListNode> e) {
        indent() << "IDENTIFIER LIST:" << '\n';
        indented([=]() -> void {
            for (const auto& node : e -> list())
                travel(node);
        });
    }

    void visit(std::shared_ptr<StringNode> e) {
        indent() << "STRING LITERAL: " << e -> str << '\n';
    }

    void visit(std::shared_ptr<SignNode> e) {
        indent() << (e -> sign() == '+' ? "PLUS SIGN" : "MINUS SIGN") << '\n';
        indented([=](){ travel(e -> child); });
    }
        
    void visit(std::shared_ptr<ConstantNode> e) {
        indent() << "CONSTANT " << '\n';
        indented([=](){ travel(e -> child);});
    }

    void visit(std::shared_ptr<SubrangeTypeNode> e) {
        indent() << "SUBRANGE TYPE:" << '\n';
        indented([=]() -> void {
                indent() << "LOWER BOUND:" << '\n';
                indented([=](){ travel(e -> lower_bound); });
                indent() << "UPPER BOUND:" << '\n';
                indented([=](){ travel(e -> upper_bound); });
        });
    }

    void visit(std::shared_ptr<EnumeratedTypeNode> e) {
        indent() << "ENUMERATED TYPE: " << '\n';
        indented([=]() { travel(e -> identifiers); });
    }

    void visit(std::shared_ptr<PointerTypeNode> e) {
        indent() << "POINTER TYPE: ";
        noindent([=]() { travel(e -> type); });
    }
    
    void visit(std::shared_ptr<VariableDeclNode> e) {
        indent() << "VARIABLE DECLARATION: " << '\n';
        indented([=]() { 
                travel(e -> id_list);
                indent() << "OF TYPE " << '\n';
                indented([=]() { travel(e -> type); });
        });
    }

    void visit(std::shared_ptr<VariableDeclListNode> e) {
        for (const auto& node : e -> list())
            travel(node);
    }
    
    void visit(std::shared_ptr<RecordTypeNode> e) {
        indent() << "RECORD TYPE" << endl;
        indented([=]() { if (e -> child) travel(e -> child); });
    }

    void visit(std::shared_ptr<SetTypeNode> e) {
        indent() << "SET OF" << endl;
        indented([=]() { travel(e -> type); });
    }

    void visit(std::shared_ptr<FileTypeNode> e) {
        indent() << "FILE OF" << endl;
        indented([=]() { travel(e -> type); });
    }

    void visit(std::shared_ptr<IndexTypeNode> e) { travel(e -> type); }
    void visit(std::shared_ptr<IndexTypeListNode> e) {
        for (const auto& node : e -> list())
            travel(node);
    }
       
    void visit(std::shared_ptr<ArrayTypeNode> e) {
        indent() << "ARRAY WITH BOUNDS" << endl;
        indented([=]() { travel(e -> index_type_list); });
        indent() << "OF TYPE" << endl;
        indented([=]() { travel(e -> type); });
    }

private:
    int _indent, sw;
    std::ostream& indent() { 
        return std::cout << string(_indent, ' ');
    }

    void indented(std::function<void()> func) {
        _indent += sw; func(); _indent -= sw;
    }

    void noindent(std::function<void()> func) {
        int old_indent = _indent;
        _indent = 0;
        func();
        _indent = old_indent;
    }
};

int main() {
    PascalGrammar pg;
    string s;
    getline(cin, s);
    PNode e = pg.parse(s);
    PrettyPrinter pp;
    pp.travel(e);
}
