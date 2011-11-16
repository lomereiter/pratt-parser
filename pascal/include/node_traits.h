#ifndef NODE_TRAITS_H
#define NODE_TRAITS_H

//#include <memory>
//#include <type_traits>

//#include "node.h"
#include "visitor.h"
//#include "node_tags.h"
#include "utils.h"

#ifdef DEBUG
#include "debug.h"
#endif

namespace node_traits {
    using utils::type;

    namespace visitors {

        struct TraitsVisitor : public AstIgnoreVisitor {
            using AstIgnoreVisitor::visit;
            bool operator()(const std::shared_ptr<Node>& node) {
                has_trait = false;
                travel(node);
                return has_trait;
            }
            bool has_trait;
        };

        /** TraitsVisitor used to determine if particular pointer to a _Node instance
         * points to some type from _ConvertibleTo list.
         */
        template <typename _Node, typename... _ConvertibleTo>
        struct AreConvertibleTo : public TraitsVisitor {

            /// Represents a list of convertible types which can be used with utils::append
            typedef type<_ConvertibleTo...> list;

            AreConvertibleTo() {
                this->template Visits<AreConvertibleTo<_Node, _ConvertibleTo...>, _ConvertibleTo...>();
            }
           
            template <typename _NodeType,
                      typename = typename std::enable_if<std::is_same<_NodeType, Node>::value>::type,
                      int dummy = 0>
            void visit(const std::shared_ptr<_NodeType>&) { }

            template <typename _NodeType,
                      typename = typename std::enable_if<
                          utils::belongs_to<_NodeType, _ConvertibleTo...>::value
                          >::type>
            void visit(const std::shared_ptr<_NodeType>&) { has_trait = true; }
        };

        /// Specialization used with utils::append
        template <typename _Node, typename... _ConvertibleTo>
        struct AreConvertibleTo <_Node, type<_ConvertibleTo...>> : 
        public AreConvertibleTo <_Node, _ConvertibleTo...> {};

   } // namespace visitors

    template <typename T>
    bool has_type(const std::shared_ptr<Node>& node) {
#ifdef DEBUG
        std::cout << "Node tag: " << node -> tag() << "; type tag: " << node_traits::get_tag_value<T>() << '\n';
#endif
        return node -> tag() == node_traits::get_tag_value<T>();
    }

    namespace detail {
        
        using utils::type;
        using namespace visitors;
        struct dummy_type {};

        typedef AreConvertibleTo< dummy_type,
            RecordTypeNode, SetTypeNode, FileTypeNode, ArrayTypeNode> IsUST;

        typedef AreConvertibleTo< IndexTypeNode,
            SubrangeTypeNode, EnumeratedTypeNode, IdentifierNode> IsIndexType;

        typedef AreConvertibleTo< dummy_type,
            utils::append<  IsUST::list,
            utils::append<  IsIndexType::list,
                PointerTypeNode, PackedTypeNode
                         >::list>::list> IsType;

        typedef AreConvertibleTo< VariableNode,
            IdentifierNode, IndexedVariableNode, ReferencedVariableNode,
            FieldDesignatorNode> IsVar;

        typedef AreConvertibleTo< ExpressionNode,
            utils::append<  IsVar::list,
                OperationNode, StringNode, NumberNode, SetNode, 
                SignNode, FunctionDesignatorNode>::list> IsExpr;

        typedef AreConvertibleTo< StatementNode,
                AssignmentStatementNode, CompoundStatementNode,
                WhileStatementNode, RepeatStatementNode, ForStatementNode,

                IdentifierNode, FunctionDesignatorNode, // <- indistinguishable during parsing

                IfThenNode, IfThenElseNode,
                WithStatementNode, CaseStatementNode> IsStatement;

        bool __is_convertible_helper(type<ExpressionNode>, const std::shared_ptr<Node>&);
        bool __is_convertible_helper(type<VariableNode>, const std::shared_ptr<Node>&);
        bool __is_convertible_helper(type<IndexTypeNode>, const std::shared_ptr<Node>&);
        bool __is_convertible_helper(type<StatementNode>, const std::shared_ptr<Node>&);
        bool __is_convertible_helper(type<ConstantNode>, const std::shared_ptr<Node>&);

        template <typename _Node>
        bool __is_convertible_helper(type<_Node>, const std::shared_ptr<Node>&) { 
            return false;
        }

    } // namespace detail

    static detail::IsUST is_unpacked_structured_type;
    static detail::IsType is_type;

    template <typename _Node>
    bool is_convertible_to(const std::shared_ptr<Node>& node) { 
        if (node -> tag() == node_traits::get_tag_value<_Node>())
            return true;
        return detail::__is_convertible_helper(detail::type<_Node>(), node);
    }

    template <typename _Node> struct there_exist_coercions_to   { enum { value = 0 }; };
    template <> struct there_exist_coercions_to<IndexTypeNode>  { enum { value = 1 }; };
    template <> struct there_exist_coercions_to<ConstantNode>   { enum { value = 1 }; };
    template <> struct there_exist_coercions_to<ExpressionNode> { enum { value = 1 }; };
    template <> struct there_exist_coercions_to<StatementNode>  { enum { value = 1 }; };
    template <> struct there_exist_coercions_to<VariableNode>   { enum { value = 1 }; };

    template <typename T> struct list_of { typedef ListOf<T> type; };
    
    template <typename T>
    bool is_list_of(const std::shared_ptr<Node>& node) {
        return has_type<typename node_traits::list_of<T>::type>(node) ||
               is_convertible_to<T>(node);
    }

 
} // namespace node_traits

namespace node {

    template <typename T>
    std::shared_ptr<typename node_traits::list_of<T>::type> 
    make_list(const std::shared_ptr<T>& node) {
        return std::make_shared<typename node_traits::list_of<T>::type>(node);
    }

    /** If node runtime type is T returns std::static_pointer_cast to T.
     * Otherwise, calls constructor of T taking node as its argument.
     */
    template <typename T>
    std::shared_ptr<T> convert_to(const std::shared_ptr<Node>& node) {
        if (node_traits::has_type<T>(node))
            return std::static_pointer_cast<T>(node);
        return std::make_shared<T>(node);
    }

} // namespace node

#endif
