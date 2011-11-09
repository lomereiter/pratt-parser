#include "parser.h"

#include "pascal_grammar.h"
#include "pascal_literals.h"

#include <memory>
#include <string>
#include <functional>
#include <sstream>
#include <forward_list>

#include "operator.h"
#include "node.h"
#include "ast_visitors.h"
#include "syntax_error.h"
#include "node_traits.h"

using namespace grammar;

typedef std::shared_ptr<Node> PNode;

/* Grammar definition */

std::function<PNode(PNode, PNode)> PascalGrammar::createLed(Operator op) {
    return [op](PNode x, PNode y) -> std::shared_ptr<OperationNode> {
        std::shared_ptr<OperationNode> expr = std::make_shared<OperationNode>(2, op);
        expr -> args.push_front(y);
        expr -> args.push_front(x);
        return expr;
    };
}

std::function<PNode(PNode)> PascalGrammar::createNud(Operator op) {
    return [op](PNode x) -> std::shared_ptr<OperationNode> {
        std::shared_ptr<OperationNode> expr = std::make_shared<OperationNode>(1, op);
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
    .set_scanner(pascal::number_scanner)
    .set_parser([](const std::string& str, size_t beg, size_t end) -> PNode {
        return std::make_shared<NumberNode>(str.substr(beg, end - beg));
    });

    add_symbol_to_dict("(identifier)", 0)
    .set_scanner(pascal::identifier_scanner)
    .set_parser([](const std::string& str, size_t beg, size_t end) {
        return std::make_shared<IdentifierNode>(str.substr(beg, end - beg));
    });

    add_symbol_to_dict("(string literal)", 0)
    .set_scanner(pascal::string_scanner)
    .set_parser([](const std::string& str, size_t beg, size_t end) -> PNode {
        return std::make_shared<StringNode>(pascal::string_parser(str, beg, end));
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

    comma= &infix_r(",", 80, [](PNode x, PNode y) -> PNode {
            throw SyntaxError("unexpected ','");
            });

    infix("..", 90, 
            [](PNode x, PNode y) -> PNode {
                if (!node_traits::is_convertible_to<ConstantNode>(x)) 
                    throw SyntaxError("expected a constant as the lower bound");
                if (!node_traits::is_convertible_to<ConstantNode>(y))
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

    colon = &infix(":", 70, 
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


    auto parse_expression_list_and_closing_bracket = 
    [this](PrattParser<PNode>& p) -> PNode {
        behaviour_guard<RightAssociative> comma_guard(*(this->comma),
                [](PNode x, PNode y) {
                    return ListVisitor<ExpressionNode>(x, y).get_expression();
                });

        PNode elements = p.parse(40); /* must be less than lbp of subexpressions */

        if (!node_traits::is_list_of<ExpressionNode>(elements))
            throw SyntaxError("expected list of expressions after '['");
       
        if (p.next_token_as_string() != "]") 
            throw SyntaxError("expected ']' after list of expressions");
        p.advance();

        return elements;
    };

    add_symbol_to_dict("]", 0);
    add_symbol_to_dict("[", 1000) 
    .nud = [parse_expression_list_and_closing_bracket](PrattParser<PNode>& p) -> PNode {
        if (p.next_token_as_string() == "]") {
            p.advance();
            return std::make_shared<SetNode>(
                    std::make_shared<ExpressionListNode>());
        }
        return std::make_shared<SetNode>(parse_expression_list_and_closing_bracket(p));
    };

    add_symbol_to_dict("[", 1000)/* big enough for parsing expressions like 'x + a[2]' */
    .led = [parse_expression_list_and_closing_bracket](PrattParser<PNode>& p, PNode left) -> PNode {
        if (!node_traits::is_convertible_to<VariableNode>(left))
            throw SyntaxError("expected a variable before '['");
        return std::make_shared<IndexedVariableNode>(left,
                parse_expression_list_and_closing_bracket(p));
    };

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
            PNode x = p.parse(1);
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

    add_symbol_to_dict("const", 1)
    .nud = [this](PrattParser<PNode>& p) -> PNode {
        lbp_guard semicolon_guard(*(this -> semicolon), 0);
        lbp_guard equal_sign_guard(*(this -> sign_eq), 0);
        
        std::forward_list<PNode> const_defs;
        do {
            PNode id = p.parse(0);
            if (!node_traits::has_type<IdentifierNode>(id))
                throw SyntaxError("expected identifier in constant definition");
            if (p.next_token_as_string() != "=")
                throw SyntaxError("expected '=' after identifier");
            p.advance();
            PNode constant = p.parse(0);
            if (!node_traits::is_convertible_to<ConstantNode>(constant))
                throw SyntaxError("expected constant after '='");
            else
                constant = node::convert_to<ConstantNode>(constant);
            const_defs.push_front(
                    std::make_shared<ConstDefinitionNode>(id, constant));

            if (p.next_token_as_string() != ";")
                throw SyntaxError("expected ';' after constant definition");
            else
                p.advance();

            std::string next = p.next_token_as_string();
            if (next == "begin" || next == "procedure" || next == "function" ||
                next == "const" || next == "type" || next == "var" || next == "") 
                break;
        } while (true);
        const_defs.reverse();
        return std::make_shared<ConstSectionNode>(std::move(const_defs));
    };

    add_symbol_to_dict("type", 1)
    .nud = [this](PrattParser<PNode>& p) -> PNode {
        lbp_guard semicolon_guard(*(this -> semicolon), 0);
        lbp_guard equal_sign_guard(*(this -> sign_eq), 0);

        std::forward_list<PNode> type_definitions;

        do {
            PNode id = p.parse(1);
            if (!node_traits::has_type<IdentifierNode>(id))
                throw SyntaxError("expected identifier as a type name");

            if (p.next_token_as_string() != "=") 
                throw SyntaxError("expected '=' after type name");
            else
                p.advance();

            PNode type = p.parse(1);
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
        PNode type = p.parse(1);
        if (!node_traits::is_unpacked_structured_type(type)) {
            throw SyntaxError("expected unpacked structured type after 'packed'");
        } else {
            return std::make_shared<PackedTypeNode>(type);
        }
    };

    postfix("^", 1, [](PNode node) -> PNode {
            if (!node_traits::is_convertible_to<VariableNode>(node)) 
                throw SyntaxError("expected a variable before '^'");
            return std::make_shared<ReferencedVariableNode>(node);
        });

    infix(".", 1000, [](PNode var, PNode field) -> PNode {
        if (!node_traits::is_convertible_to<VariableNode>(var))
            throw SyntaxError("expected a variable before '.'");
        if (!node_traits::has_type<IdentifierNode>(field)) 
            throw SyntaxError("expected identifier after '.'");
            return std::make_shared<FieldDesignatorNode>(var, field);
        });

    add_symbol_to_dict(")", 0);
    add_symbol_to_dict("(", 1000)
    .led = [this](PrattParser<PNode>& p, PNode left) -> PNode {
                if (!node_traits::has_type<IdentifierNode>(left))
                    throw SyntaxError("expected identifier before '(' token");
                behaviour_guard<RightAssociative> comma_guard(*(this -> comma),
                    [](PNode left, PNode right) {
                        return ListVisitor<ExpressionNode>(left, right).get_expression();
                    });
                PNode params = p.parse(0);
                if (!node_traits::is_list_of<ExpressionNode>(params))
                    throw SyntaxError("expected list of parameters after '(' token");
                if (p.next_token_as_string() != ")")
                    throw SyntaxError("expected ')' token after the list of parameters");
                p.advance();
                return std::make_shared<FunctionDesignatorNode>(left, params);
            };

    infix(":=", 40,
            [](PNode var, PNode expr) -> PNode {
                if (!node_traits::is_convertible_to<VariableNode>(var))
                    throw SyntaxError("expected variable before ':=' token");
                if (!node_traits::is_convertible_to<ExpressionNode>(expr))
                    throw SyntaxError("expected expression after ':=' token");
                return std::make_shared<AssignmentStatementNode>(var, expr);
            });

    add_symbol_to_dict("begin", 1)
    .nud = [this](PrattParser<PNode>& p) -> PNode {
        lbp_guard sl_guard(*(this -> semicolon), 1);
        behaviour_guard<RightAssociative> sb_guard(*(this -> semicolon),
            [](PNode x, PNode y) {
                return ListVisitor<StatementNode>(x, y).get_expression();
            });
        PNode statements = p.parse(0);
        if (!node_traits::is_list_of<StatementNode>(statements))
            throw SyntaxError("expected statement-sequence after 'begin'");
        if (p.next_token_as_string() != "end") 
            throw SyntaxError("expected 'end' after statement-sequence");
        p.advance();
        return std::make_shared<CompoundStatementNode>(statements);
    };

    add_symbol_to_dict("do", 0);
    add_symbol_to_dict("while", 1)
    .nud = [](PrattParser<PNode>& p) -> PNode {
        PNode condition = p.parse(0);
        if (!node_traits::is_convertible_to<ExpressionNode>(condition))
            throw SyntaxError("expected expression after 'while'");
        if (p.next_token_as_string() != "do")
            throw SyntaxError("expected 'do' after expression");
        p.advance();
        PNode body = p.parse(1);
        if (!node_traits::is_convertible_to<StatementNode>(body))
            throw SyntaxError("expected statement-sequence after 'do'");
        return std::make_shared<WhileStatementNode>(condition, body);
    };

    add_symbol_to_dict("until", 0);
    add_symbol_to_dict("repeat", 1)
    .nud = [](PrattParser<PNode>& p) -> PNode {
        PNode body = p.parse(1);
        if (!node_traits::is_list_of<StatementNode>(body))
            throw SyntaxError("expected statement-sequence after 'repeat'");
        if (p.next_token_as_string() != "until")
            throw SyntaxError("expected 'until' after statement-sequence");
        p.advance();
        PNode condition = p.parse(0);
        if (!node_traits::is_convertible_to<ExpressionNode>(condition))
            throw SyntaxError("expected expression after 'until'");
        return std::make_shared<RepeatStatementNode>(body, condition);
    };

    add_symbol_to_dict("to", 0);
    add_symbol_to_dict("downto", 0);
    add_symbol_to_dict("for", 1)
    .nud = [this](PrattParser<PNode>& p) -> PNode {
        PNode assignment = p.parse(0);
        if (!node_traits::has_type<AssignmentStatementNode>(assignment))
            throw SyntaxError("expected assignment-statement after 'for'");
        auto _assignment = std::static_pointer_cast<AssignmentStatementNode>(assignment);
        if (!node_traits::has_type<IdentifierNode>(_assignment -> variable))
            throw SyntaxError("expected identifier after 'for'");
        int sign;
        std::string next = p.next_token_as_string();
        if (next == "to") {
            sign = 1;
        } else if (next == "downto") {
            sign = -1;
        } else {
            throw SyntaxError("expected 'to' or 'downto' after initial-expression");
        }
        p.advance();
        PNode final_expr = p.parse(1);
        if (!node_traits::is_convertible_to<ExpressionNode>(final_expr)) {
            std::string msg = "expected final-expression after ";
            msg = msg + '\'' + next + '\'';
            throw SyntaxError(std::move(msg));
        }
        if (p.next_token_as_string() != "do")
            throw SyntaxError("expected 'do' after final-expression");
        p.advance();
        PNode body = p.parse(1);
        if (!node_traits::is_convertible_to<StatementNode>(body))
            throw SyntaxError("expected statement after 'do'");
        return std::make_shared<ForStatementNode>(_assignment, sign, final_expr, body);
    };

    add_symbol_to_dict("then", 0);
    add_symbol_to_dict("else", 0);
    add_symbol_to_dict("if", 1)
    .nud = [](PrattParser<PNode>& p) -> PNode {
        PNode expr = p.parse(1);
        if (!node_traits::is_convertible_to<ExpressionNode>(expr))
            throw SyntaxError("expected expression after 'if'");
        if (p.next_token_as_string() != "then")
            throw SyntaxError("expected 'then'");
        p.advance();
        PNode st = p.parse(1);
        if (!node_traits::is_convertible_to<StatementNode>(st))
            throw SyntaxError("expected statement after 'then'");
        if (p.next_token_as_string() != "else") {
            return std::make_shared<IfThenNode>(expr, st);
        } else {
            p.advance();
            PNode _else = p.parse(1);
            if (!node_traits::is_convertible_to<StatementNode>(st))
                throw SyntaxError("expected statement after 'else'");
            return std::make_shared<IfThenElseNode>(expr, st, _else);
        }
    };

    add_symbol_to_dict("do", 0);
    add_symbol_to_dict("with", 1)
    .nud = [this](PrattParser<PNode>& p) -> PNode {
        behaviour_guard<RightAssociative> comma_guard(*(this->comma),
            [](PNode left, PNode right) {
                return ListVisitor<VariableNode>(left, right).get_expression();
            });
        PNode list = p.parse(0);
        if (!node_traits::is_list_of<VariableNode>(list))
            throw SyntaxError("expected list of record variables after 'with'");
        if (p.next_token_as_string() != "do")
            throw SyntaxError("expected 'do' in with-statement");
        p.advance();
        PNode st = p.parse(1);
        if (!node_traits::is_convertible_to<StatementNode>(st))
            throw SyntaxError("expected statement after 'do'");
        return std::make_shared<WithStatementNode>(list, st);
    };

    add_symbol_to_dict("case", 1)
    .nud = [this](PrattParser<PNode>& p) -> PNode {
        PNode expr = p.parse(0);
        if (!node_traits::is_convertible_to<ExpressionNode>(expr))
            throw SyntaxError("expected expression after 'case'");
        if (p.next_token_as_string() != "of")
            throw SyntaxError("expected 'of' after expression");

        p.advance();

        behaviour_guard<RightAssociative> comma_guard(*(this -> comma),
            [](PNode left, PNode right) {
                return ListVisitor<ConstantNode>(left, right).get_expression();
            });

        behaviour_guard<LeftAssociative> colon_guard(*(this -> colon), 1,
            [](PNode left, PNode right) -> PNode {
                if (!node_traits::is_list_of<ConstantNode>(left))
                    throw SyntaxError("expected list of constants before ':'");
                if (!node_traits::is_convertible_to<StatementNode>(right))
                    throw SyntaxError("expected statement after ':'");
                return std::make_shared<CaseLimbNode>(left, right);
            });

        lbp_guard semicolon_guard(*(this -> semicolon), 0);
        
        std::forward_list<PNode> limbs;

        do {
            PNode limb = p.parse(0);
            if (!node_traits::has_type<CaseLimbNode>(limb))
                throw SyntaxError("expected case-limb");
            limbs.push_front(limb);
            std::string next = p.next_token_as_string();
            if (next == ";") {
                p.advance();
                if (p.next_token_as_string() == "end") {
                    p.advance();
                    break;
                }
            } else if (next == "end") {
                p.advance();
                break;
            } else {
                throw SyntaxError("expected ';' or 'end'");
            }
        } while (true);

        limbs.reverse();
        return std::make_shared<CaseStatementNode>(expr,
                std::make_shared<CaseLimbListNode>(std::move(limbs)));
    };
}

PNode PascalGrammar::parse(const std::string& str) {
    PrattParser<PNode> parser(str, get_symbols());
    std::forward_list<PNode> declarations;
    while (parser.next_token_as_string() != "") {
        declarations.push_front(std::make_shared<DeclarationNode>(parser.parse(1)));
    }
    declarations.reverse();
    return std::make_shared<DeclarationListNode>(std::move(declarations));
}
