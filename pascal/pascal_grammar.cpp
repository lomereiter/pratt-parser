#include "../parser/parser.h"

#include "pascal_grammar.h"

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

using namespace grammar;

typedef std::shared_ptr<Node> PNode;

/* Grammar definition */

std::function<PNode(PNode, PNode)> PascalGrammar::createLed(Operator op) {
    return [op](PNode x, PNode y) -> std::shared_ptr<ExpressionNode> {
        std::shared_ptr<ExpressionNode> expr = std::make_shared<ExpressionNode>(2, op);
        expr -> args.push_front(y);
        expr -> args.push_front(x);
        return expr;
    };
}

std::function<PNode(PNode)> PascalGrammar::createNud(Operator op) {
    return [op](PNode x) -> std::shared_ptr<ExpressionNode> {
        std::shared_ptr<ExpressionNode> expr = std::make_shared<ExpressionNode>(1, op);
        expr -> args.push_front(x);
        return expr;
    };
}

std::function<PNode(PNode)> PascalGrammar::createSignNud(char sign) {
    return [sign](PNode x) -> std::shared_ptr<SignNode> {
        return std::make_shared<SignNode>(sign, x);
    };
}

PascalGrammar::PascalGrammar() : Grammar<PNode>("(end)") {
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
                        sign_eq = &infix("=", 50, createLed(opEq));       
    infix("<>", 50, createLed(opNotEq));    infix("in", 50, createLed(opIn));
    infix("<", 50, createLed(opLess));      infix("<=", 50, createLed(opLessEq));
    infix(">", 50, createLed(opMore));      infix(">=", 50, createLed(opMoreEq));

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

    semicolon = &add_symbol_to_dict(";", 0);

    add_symbol_to_dict("end", 0);
    add_symbol_to_dict("record", std::numeric_limits<int>::max())
    .nud = [this](PrattParser<PNode>& p) -> PNode {
                lbp_guard semicolon_guard(*(this -> semicolon), 0);

                behaviour_guard<RightAssociative> comma_guard(*(this -> comma), 
                    [](PNode x, PNode y) {
                        return ListVisitor<IdentifierNode>(x, y).get_expression();
                    });
                
                std::forward_list<PNode> variable_declarations;

                while (true) {
                    auto next = p.next_token_as_string();
                    if (next == ";") {
                        p.advance();
                        next = p.next_token_as_string();
                        if (variable_declarations.empty()) {
                            p.advance("end"); // either ';' follows field declaration
                            break;            // or ';' is followed by 'end'
                        }
                    }
                    if (next == "end") {
                        p.advance(); // skip 'end' and return the node
                        variable_declarations.reverse();
                        break;
                    } 
                    PNode x = p.parse(0);
                    if (!node_traits::has_type<VariableDeclNode>(x))
                        throw SyntaxError("expected field declaration");
                    variable_declarations.push_front(x);
                }

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

    add_symbol_to_dict("var", 1)
    .nud = [this](PrattParser<PNode>& p) -> PNode {
        lbp_guard semicolon_guard(*(this -> semicolon), 0);

        behaviour_guard<RightAssociative> comma_guard(*(this -> comma), 
            [](PNode x, PNode y) {
                return ListVisitor<IdentifierNode>(x, y).get_expression();
            });
        
        std::forward_list<PNode> variable_declarations;

        do {
            PNode x = p.parse(0);
            if (!node_traits::has_type<VariableDeclNode>(x))
                throw SyntaxError("expected variable declaration");
            variable_declarations.push_front(x);

            if (p.next_token_as_string() != ";") 
                throw SyntaxError("expected ';' after variable declaration");
            else
                p.advance();

            std::string next = p.next_token_as_string();
            if (next == "begin" || next == "procedure" || next == "function" ||
                next == "const" || next == "type" || next == "var" || next == "") 
                break;
        } while (true);

        variable_declarations.reverse();

        return std::make_shared<VariableSectionNode>(
                 std::make_shared<VariableDeclListNode>(
                         std::move(variable_declarations)));
    };

    add_symbol_to_dict("type", 1)
    .nud = [this](PrattParser<PNode>& p) -> PNode {
        lbp_guard semicolon_guard(*(this -> semicolon), 0);
        lbp_guard equal_sign_guard(*(this -> sign_eq), 0);

        std::forward_list<PNode> type_definitions;

        do {
            PNode id = p.parse(0);
            if (!node_traits::has_type<IdentifierNode>(id))
                throw SyntaxError("expected identifier as a type name");

            if (p.next_token_as_string() != "=") 
                throw SyntaxError("expected '=' after type name");
            else
                p.advance();

            PNode type = p.parse(0);
            if (!node_traits::is_type(type))
                throw SyntaxError("expected type definition after '='");
            
            if (p.next_token_as_string() != ";")
                throw SyntaxError("expected ';' after type definition");
            else
                p.advance();

            type_definitions.push_front(std::make_shared<TypeDefinitionNode>(id, type));

            std::string next = p.next_token_as_string();
            if (next == "begin" || next == "procedure" || next == "function" ||
                next == "const" || next == "type" || next == "var" || next == "") 
                break;
        } while (true);

        type_definitions.reverse();
        return std::make_shared<TypeSectionNode>(std::move(type_definitions));
    };

    add_symbol_to_dict("packed", 1)
    .nud = [this](PrattParser<PNode>& p) -> PNode {
        PNode type = p.parse(0);
        if (!node_traits::is_unpacked_structured_type(type)) {
            throw SyntaxError("expected unpacked structured type after 'packed'");
        } else {
            return std::make_shared<PackedTypeNode>(type);
        }
    };

}
