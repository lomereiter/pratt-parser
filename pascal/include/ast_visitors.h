#ifndef AST_VISITORS_H
#define AST_VISITORS_H

//#include <memory>
//#include <forward_list>
#include <cassert>
#include <sstream>
//#include <string>

#ifdef DEBUG
#include "debug.h"
#endif 

//#include "syntax_error.h"
//#include "node.h"
#include "node_traits.h"
//#include "visitor.h"
#include "pascal_grammar.h"

/* ConvertHelper is a strategy class used by ListVisitor.
   It determines whether to try to convert one type to another or not. */
template <typename T, int> struct ConvertHelper;

template <typename T>
struct ConvertHelper<T, 0> {
    bool operator() (const std::shared_ptr<Node>& node) { 
        return node_traits::has_type<T>(node);
    }
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

/** Visitor class used for implementing 
 *  behaviour of an operator which builds a list of nodes
 *  which have type _Type (or are convertible to _Type).
 *
 *  The list produced has a type _ListType usually 
 *  determined by using node_traits::list_of struct. 
 */
template <typename _Type, 
          int try_to_convert = node_traits::there_exist_coercions_to<_Type>::value,
          typename _ListType = typename node_traits::list_of<_Type>::type>
struct ListVisitor : public AstThrowVisitor {
    using AstThrowVisitor::visit;
    /* for right-associative operators */

    /// references are non-const because conversion might be performed
    ListVisitor(std::shared_ptr<Node>& left,
                std::shared_ptr<Node>& right,
                const PascalGrammar* const pg,
                std::string expected) : visited_right(false),
                                        grammar(pg), expected(expected) {

        Visits<ListVisitor<_Type, try_to_convert, _ListType>, _Type, _ListType>();

        if (!node_traits::has_type<_ListType>(right)) {
            try_to_convert_to<_Type>(right, true);
            right = node::make_list(std::static_pointer_cast<_Type>(right));
            /* otherwise ListOf<Node> will be created */
        }

        try_to_convert_to<_Type>(left);

        travel(right);
        travel(left);
    }

    /// supposed to visit left node
    void visit(const std::shared_ptr<_Type>& id) {
        if (visited_right) {
#ifdef DEBUG
            std::cout << "visited node on the left" << std::endl;
#endif
            ids.push_front(id);
            expr = std::make_shared<_ListType>(std::move(ids));
        } else {
            assert(0);
        }
    }

    /// supposed to visit right node
    void visit(const std::shared_ptr<_ListType>& id_list) {
        if (!visited_right) {
            visited_right = true;
#ifdef DEBUG
            std::cout << "visited list on the right" << std::endl;
#endif
            ids = std::move(id_list -> list());
        } else {
            assert(0);
        }
    }

    /// Returns the node generated during the visit
    std::shared_ptr<Node> get_expression() { return expr; }

private:
    bool visited_right;
    const PascalGrammar* const grammar;
    std::string expected;

    std::forward_list<std::shared_ptr<Node>> ids;
    std::shared_ptr<Node> expr;

    static ConvertHelper<_Type, try_to_convert> convert_helper;

    template <typename T>
    void try_to_convert_to(std::shared_ptr<Node>& node, bool list=false) {
        if (!convert_helper(node)) {
            std::stringstream err;
            err << "expected " << expected;
            if (list) {
                err << " list";
            }
            grammar -> error(err.str());
        }
    }
};

template <typename T, int c, typename U> /* static member definition */
ConvertHelper<T, c> ListVisitor<T, c, U>::convert_helper;

#endif
