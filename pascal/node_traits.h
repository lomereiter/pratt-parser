#ifndef NODE_TRAITS_H
#define NODE_TRAITS_H

#include <memory>

#include "expression.h"
#include "visitor.h"
#include "node_tags.h"

namespace node_traits {

    namespace visitors {

        template <class T>
        struct TraitsVisitor : public AstIgnoreVisitor {
            using AstIgnoreVisitor::visit;
            bool operator()(std::shared_ptr<Node> node) {
                static_cast<T*>(this) -> initialize();
                has_trait = false;
                travel(node);
                return has_trait;
            }
            void initialize() {}
            void set() { has_trait = true; }
            void unset() { has_trait = false; }
            bool has_trait;
        };

        struct ConstVisitor : public TraitsVisitor<ConstVisitor> {
            using TraitsVisitor<ConstVisitor>::visit;
            ConstVisitor() { 
                Visits<ConstVisitor, 
                       StringNode,     NumberNode, 
                       IdentifierNode, ConstantNode,
                       SignNode>(*this);
            }

            void visit(std::shared_ptr<StringNode>) { set(); }
            void visit(std::shared_ptr<NumberNode>) { set(); }
            void visit(std::shared_ptr<IdentifierNode>) { set(); }

            void visit(std::shared_ptr<SignNode> node) { 
                if (visited_sign_node) {
                    unset();
                } else {
                    visited_sign_node = true;
                    travel(node -> child);
                }
            }

            void initialize() { visited_sign_node = false; }
            private:
                bool visited_sign_node;
        };

        struct TypeVisitor : public TraitsVisitor<TypeVisitor> {
            using TraitsVisitor<TypeVisitor>::visit;
            TypeVisitor() {
                Visits<TypeVisitor,
                       IdentifierNode, PointerTypeNode,
                       RecordTypeNode, SetTypeNode,
                       FileTypeNode, ArrayTypeNode,
                       SubrangeTypeNode, EnumeratedTypeNode>(*this); 
            }
            void visit(std::shared_ptr<IdentifierNode>) { set(); }
            void visit(std::shared_ptr<PointerTypeNode>) { set(); }
            void visit(std::shared_ptr<RecordTypeNode>) { set(); }
            void visit(std::shared_ptr<SetTypeNode> node) { set(); }
            void visit(std::shared_ptr<FileTypeNode> node) { set(); }
            void visit(std::shared_ptr<ArrayTypeNode> node) { set(); }
            void visit(std::shared_ptr<SubrangeTypeNode> node) { set(); }
            void visit(std::shared_ptr<EnumeratedTypeNode> node) { set(); }
        };

    } // namespace

    template <typename T>
    bool has_type(std::shared_ptr<Node> node) {
        return node -> tag() == node_traits::get_tag_value<T>();
    }

    template <typename _Node> struct list_of {};
    template <> struct list_of<VariableDeclNode> { typedef VariableDeclListNode type; };
    template <> struct list_of<IdentifierNode> { typedef IdentifierListNode type; };
    template <> struct list_of<IndexTypeNode> { typedef IndexTypeListNode type; };

    template <typename T>
    bool is_list_of(std::shared_ptr<Node> node) {
        return has_type<T>(node) || 
               has_type<typename node_traits::list_of<T>::type>(node);
    }

    static visitors::ConstVisitor                       is_constant;
    static visitors::TypeVisitor                        is_type;

    namespace detail {
        template <typename U> struct type {};

        bool __is_convertible_helper(type<ConstantNode>, std::shared_ptr<Node>& node) {
            return is_constant(node);
        }

        bool __is_convertible_helper(type<IndexTypeNode>, std::shared_ptr<Node>& node) {
            static int subrange_tag = get_tag_value<SubrangeTypeNode>();
            static int enum_tag = get_tag_value<EnumeratedTypeNode>();

            int node_tag = node -> tag();
            return node_tag == subrange_tag || node_tag == enum_tag;
        }

        template <typename _Node>
        bool __is_convertible_helper(type<_Node>, std::shared_ptr<Node>& node) { 
            return node -> tag() == node_traits::get_tag_value<_Node>();
        }

    }

    template <typename _Node>
    bool is_convertible_to(std::shared_ptr<Node>& node) { 
        return detail::__is_convertible_helper(detail::type<_Node>(), node);
    }

    template <typename _Node> struct there_exist_coercions_to  { enum { value = 0 }; };
    template <> struct there_exist_coercions_to<IndexTypeNode> { enum { value = 1 }; };
    template <> struct there_exist_coercions_to<ConstantNode>  { enum { value = 1 }; };


} // namespace node_traits

namespace node {

    template <typename T>
    std::shared_ptr<typename node_traits::list_of<T>::type> 
    make_list(std::shared_ptr<T> node) {
        return std::make_shared<typename node_traits::list_of<T>::type>(node);
    }

    template <typename T>
    std::shared_ptr<T> convert_to(std::shared_ptr<Node> node) {
        if (node_traits::has_type<T>(node))
            return std::static_pointer_cast<T>(node);
        return std::make_shared<T>(node);
    }

} // namespace node

#endif
