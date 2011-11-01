#ifndef NODE_TRAITS_H
#define NODE_TRAITS_H

#include <memory>

#include "expression.h"
#include "visitor.h"
#include "node_tags.h"

#ifdef DEBUG
#include "debug.h"
#endif

namespace node_traits {

    namespace visitors {

        template <class T>
        struct TraitsVisitor : public AstIgnoreVisitor {
            using AstIgnoreVisitor::visit;
            bool operator()(const std::shared_ptr<Node>& node) {
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
                Visits< ConstVisitor,
                        StringNode, NumberNode, IdentifierNode, 
                        ConstantNode, SignNode>();
            }

            void visit(const std::shared_ptr<StringNode>&) { set(); }
            void visit(const std::shared_ptr<NumberNode>&) { set(); }
            void visit(const std::shared_ptr<IdentifierNode>&) { set(); }

            void visit(const std::shared_ptr<SignNode>& node) { 
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
   } // namespace

    template <typename T>
    bool has_type(const std::shared_ptr<Node>& node) {
#ifdef DEBUG
        std::cout << "Node tag: " << node -> tag() << "; type tag: " << node_traits::get_tag_value<T>() << '\n';
#endif
        return node -> tag() == node_traits::get_tag_value<T>();
    }

    template <typename _Node> struct list_of {};
    template <> struct list_of<VariableDeclNode> { typedef VariableDeclListNode type; };
    template <> struct list_of<IdentifierNode> { typedef IdentifierListNode type; };
    template <> struct list_of<IndexTypeNode> { typedef IndexTypeListNode type; };

    template <typename T>
    bool is_list_of(const std::shared_ptr<Node>& node) {
        return has_type<T>(node) || 
               has_type<typename node_traits::list_of<T>::type>(node);
    }

    static visitors::ConstVisitor is_constant;

    static bool is_unpacked_structured_type(const std::shared_ptr<Node>& node) {
        if (!node) return false;
        size_t tag = node -> tag();
        static size_t record = get_tag_value<RecordTypeNode>(),
                      set = get_tag_value<SetTypeNode>(),
                      file = get_tag_value<FileTypeNode>(),
                      array = get_tag_value<ArrayTypeNode>();
        return tag == record || tag == set || tag == file || tag == array;
    }

    static bool is_type(const std::shared_ptr<Node>& node) {
        if (!node) return false;
        static size_t id = get_tag_value<IdentifierNode>(),
                      ptr = get_tag_value<PointerTypeNode>(),
                      subrange = get_tag_value<SubrangeTypeNode>(),
                      enumeration = get_tag_value<EnumeratedTypeNode>(),
                      packed = get_tag_value<PackedTypeNode>();
        size_t tag = node -> tag();
        return is_unpacked_structured_type(node) || tag == id || tag == ptr ||
               tag == subrange || tag == enumeration || tag == packed;
    }

    namespace detail {
        template <typename U> struct type {};

        static bool __is_convertible_helper(type<ConstantNode>, const std::shared_ptr<Node>& node) {
            return is_constant(node);
        }

        static bool __is_convertible_helper(type<IndexTypeNode>, const std::shared_ptr<Node>& node) {
            static int subrange_tag = get_tag_value<SubrangeTypeNode>();
            static int enum_tag = get_tag_value<EnumeratedTypeNode>();
            if (!node) return false;
            int node_tag = node -> tag();
            return node_tag == subrange_tag || node_tag == enum_tag;
        }

        template <typename _Node>
        static bool __is_convertible_helper(type<_Node>, const std::shared_ptr<Node>& node) { 
            return node -> tag() == node_traits::get_tag_value<_Node>();
        }

    }

    template <typename _Node>
    bool is_convertible_to(const std::shared_ptr<Node>& node) { 
        return detail::__is_convertible_helper(detail::type<_Node>(), node);
    }

    template <typename _Node> struct there_exist_coercions_to  { enum { value = 0 }; };
    template <> struct there_exist_coercions_to<IndexTypeNode> { enum { value = 1 }; };
    template <> struct there_exist_coercions_to<ConstantNode>  { enum { value = 1 }; };


} // namespace node_traits

namespace node {

    template <typename T>
    std::shared_ptr<typename node_traits::list_of<T>::type> 
    make_list(const std::shared_ptr<T>& node) {
        return std::make_shared<typename node_traits::list_of<T>::type>(node);
    }

    template <typename T>
    std::shared_ptr<T> convert_to(const std::shared_ptr<Node>& node) {
        if (node_traits::has_type<T>(node))
            return std::static_pointer_cast<T>(node);
        return std::make_shared<T>(node);
    }

} // namespace node

#endif
