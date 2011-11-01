#ifndef AST_VISITORS_H
#define AST_VISITORS_H

#include <memory>
#include <forward_list>

#ifdef DEBUG
#include "debug.h"
#endif 

#include "syntax_error.h"
#include "node_traits.h"
#include "visitor.h"
#include "expression.h"

/* ConvertHelper is a strategy class used by ListVisitor.
   It determines whether to try to convert one type to another or not. */
template <typename T, int> struct ConvertHelper;

template <typename T>
struct ConvertHelper<T, 0> {
    bool operator() (const std::shared_ptr<Node>&) { return true; }
};

template <typename T>
struct ConvertHelper<T, 1> {
    bool operator() (std::shared_ptr<Node>& node) {
        if (node_traits::is_convertible_to<T>(node)) {
            node = node::convert_to<T>(node);
            return true;
        } else { return false; }
    }
};

/*  ListVisitor is a visitor class used for implementing 
   behaviour of an operator which builds a list of nodes
   which have type _Type (or are convertible to _Type).
    The list produced has a type _ListType usually 
   determined by using node_traits::list_of struct. */
template <typename _Type, 
          int try_to_convert = node_traits::there_exist_coercions_to<_Type>::value,
          typename _ListType = typename node_traits::list_of<_Type>::type>
struct ListVisitor : public AstThrowVisitor {
    using AstThrowVisitor::visit;
    /* for right-associative operators */
    /* references are non-const because conversion might be performed */
    ListVisitor(std::shared_ptr<Node>& left,
                std::shared_ptr<Node>& right) : visited_right(false) {

        Visits<ListVisitor<_Type, try_to_convert, _ListType>, _Type, _ListType>();

        check_type<_ListType>(right);
        check_type<_Type>(left);

        travel(right);
        travel(left);
    }

    void visit(const std::shared_ptr<_Type>& id) {
        if (!visited_right) {
#ifdef DEBUG
            std::cout << "visited node on the right" << std::endl;
#endif
            visited_right = true;
            ids.push_front(id);
        } else {
#ifdef DEBUG
            std::cout << "visited node on the left" << std::endl;
#endif
            ids.push_front(id);
            expr = std::make_shared<_ListType>(std::move(ids));
        }
    }

    void visit(const std::shared_ptr<_ListType>& id_list) {
        /* List visitor firstly visits right node */
        if (!visited_right) {
            visited_right = true;
#ifdef DEBUG
            std::cout << "visited list on the right" << std::endl;
#endif
            ids = std::move(id_list -> list());
        }
    }

    std::shared_ptr<Node> get_expression() { return expr; }

private:
    bool visited_right;
    std::forward_list<std::shared_ptr<Node>> ids;
    std::shared_ptr<Node> expr;

    static ConvertHelper<_Type, try_to_convert> convert_helper;

    template <typename T>
    void check_type(std::shared_ptr<Node>& node) {
        if (node_traits::has_type<T>(node)) 
            return;
        if (!convert_helper(node))
            throw SyntaxError("inconsistent types"); /* FIXME! */
    }
};

template <typename T, int c, typename U> /* static member definition */
ConvertHelper<T, c> ListVisitor<T, c, U>::convert_helper;

typedef ListVisitor<VariableDeclNode> SemicolonVisitor;

struct OpenBracketVisitor : public AstThrowVisitor {
    using AstThrowVisitor::visit;

    OpenBracketVisitor() { 
        Visits< OpenBracketVisitor, 
                ExpressionNode, NumberNode, StringNode, SignNode,
                IdentifierNode, IdentifierListNode >();
    }

    void visit(const std::shared_ptr<ExpressionNode>& x) { expr = x; }
    void visit(const std::shared_ptr<NumberNode>& x) { expr = x; }
    void visit(const std::shared_ptr<StringNode>& x) { expr = x; }
    void visit(const std::shared_ptr<SignNode>& x) { expr = x; }

    void visit(const std::shared_ptr<IdentifierNode>& x) { 
        expr = std::make_shared<EnumeratedTypeNode>(node::make_list(x));
    }

    void visit(const std::shared_ptr<IdentifierListNode>& x) {
        expr = std::make_shared<EnumeratedTypeNode>(x);
    }

    std::shared_ptr<Node> get_expression() { return expr; }
private:
    std::shared_ptr<Node> expr;
};

#endif
