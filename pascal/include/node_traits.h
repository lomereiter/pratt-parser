#ifndef NODE_TRAITS_H
#define NODE_TRAITS_H

//#include <memory>
//#include <type_traits>

//#include "node.h"
#include "node_fwd.h"
#include "visitor.h"
//#include "node_tags.h"
#include "utils.h"

#ifdef DEBUG
#include "debug.h"
#endif

namespace node_traits {

    namespace visitors {
        using utils::type;

        struct TraitsVisitor : public AstIgnoreVisitor {
            using AstIgnoreVisitor::visit;
            bool operator()(const PNode& node) {
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

            /// Type to be converted to
            typedef _Node node_type;

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
    bool has_type(const PNode& node) {
#ifdef DEBUG
        std::cout << "Node tag: " << node -> tag() << "; type tag: " 
                  << node_traits::get_tag_value<T>() << '\n';
#endif
        return node -> tag() == node_traits::get_tag_value<T>();
    }

    namespace detail {
       
        using namespace utils;
        using namespace visitors;

        template <typename... ConversionLists>
        struct ConversionTable {
            typedef type<typename ConversionLists::node_type...> Keys;
            typedef type<type<typename ConversionLists::node_type, ConversionLists>...> Table;

            template <typename T>
            struct has_key {
                enum { value = utils::belongs_to<T, Keys>::value };
            };

            template <typename T>
            struct take {
                typedef typename utils::typemap::find<T, Table>::type type;
            };
        };

        struct dummy_type {};

        typedef AreConvertibleTo< dummy_type,
            RecordTypeNode, SetTypeNode, FileTypeNode, ArrayTypeNode> IsUST;

        typedef AreConvertibleTo< IndexTypeNode,
            SubrangeTypeNode, EnumeratedTypeNode, IdentifierNode> IsIndexType;

        typedef AreConvertibleTo< dummy_type,
            flatten< IsUST::list, IsIndexType::list, 
                PointerTypeNode, PackedTypeNode>::list > IsType;

        typedef AreConvertibleTo< dummy_type,
                UCArraySchemaNode, PCArraySchemaNode> IsConformantArraySchema;

        typedef AreConvertibleTo< dummy_type,
            flatten< IsConformantArraySchema::list, IdentifierNode>::list> IsParamType;

        typedef AreConvertibleTo< VariableNode,
            IdentifierNode, IndexedVariableNode, ReferencedVariableNode,
            FieldDesignatorNode> IsVar;

        typedef AreConvertibleTo< dummy_type,
            UIntegerNumberNode, URealNumberNode> IsUnsignedNumber;

        typedef AreConvertibleTo< dummy_type,
            UIntegerNumberNode, URealNumberNode, 
            IntegerNumberNode,  RealNumberNode> IsNumber;

        typedef AreConvertibleTo< ExpressionNode,
            flatten< IsNumber::list, IsVar::list,
                OperationNode, StringNode, SetNode, SignNode, 
                FunctionDesignatorNode>::list> IsExpr;

        typedef ConversionTable<
            IsIndexType, IsVar, IsExpr,
            
            AreConvertibleTo< IntegerNumberNode, UIntegerNumberNode>,
            AreConvertibleTo< RealNumberNode, URealNumberNode>,

            AreConvertibleTo< SetExpressionNode,
                flatten<  IsExpr::list, SubrangeNode>::list>,

            AreConvertibleTo< StatementNode,
                IfThenNode, IfThenElseNode, CompoundStatementNode, ForStatementNode,
                WhileStatementNode, RepeatStatementNode, WriteNode, WriteLineNode,
                FunctionDesignatorNode, LabeledStatementNode, AssignmentStatementNode,
                WithStatementNode, CaseStatementNode, GotoStatementNode, EmptyNode>,

            AreConvertibleTo< ParameterNode,
                VariableParameterNode, ValueParameterNode, 
                ProcedureHeadingNode, FunctionHeadingNode>,

            AreConvertibleTo< DeclarationNode,
                VariableSectionNode, TypeSectionNode, ConstSectionNode,
                FunctionNode, FunctionForwardDeclNode, FunctionIdentificationNode,
                ProcedureNode, ProcedureForwardDeclNode, LabelSectionNode>,

            AreConvertibleTo< ProgramHeadingNode, IdentifierNode>
                               > conversions;

        template <typename NodeType, bool has_conversions> struct is_convertible_helper;

        template <typename NodeType>
        struct is_convertible_helper<NodeType, false> {
            typedef struct { 
                bool operator()(const PNode&) { 
                    return false; 
                }
            } type;
        };

        template <typename NodeType>
        struct is_convertible_helper<NodeType, true> {
            typedef typename conversions::take<NodeType>::type type;
        };

        template <>
        struct is_convertible_helper<ConstantNode, true> {
            typedef struct {
                bool operator()(const PNode& node) {
                    typedef AreConvertibleTo<ConstantNode,
                        flatten< IsNumber::list, IdentifierNode>::list> IsNumConst;
                    static IsNumConst is_num_const;
                    static AreConvertibleTo<ConstantNode,
                        flatten< IsNumConst::list, StringNode>::list> is_surely_constant;
                    return is_surely_constant(node) ||
                           ( has_type<SignNode>(node) &&
                             is_num_const(std::static_pointer_cast<SignNode>(node) -> child) );
                }
            } type;
        };

    } // namespace detail

    static detail::IsUnsignedNumber is_unsigned_number;
    static detail::IsNumber is_number;
    static detail::IsUST is_unpacked_structured_type;
    static detail::IsType is_type;
    static detail::IsConformantArraySchema is_conformant_array_schema;
    static detail::IsParamType is_parameter_type;

    template <typename _Node> struct there_exist_coercions_to { 
        enum { value = detail::conversions::has_key<_Node>::value }; 
    };

    template <> struct there_exist_coercions_to<ConstantNode> { enum { value = true }; };

    template <typename _Node>
    bool is_convertible_to(const PNode& node) { 
        if (node -> tag() == node_traits::get_tag_value<_Node>())
            return true;
        static typename detail::is_convertible_helper<
            _Node, there_exist_coercions_to<_Node>::value>::type checker;
        return checker(node);
    }

    template <typename T> struct list_of { typedef ListOf<T> type; };
    
    template <typename T>
    bool is_list_of(const PNode& node) {
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
    std::shared_ptr<T> convert_to(const PNode& node) {
        if (node_traits::has_type<T>(node))
            return std::static_pointer_cast<T>(node);
        return std::make_shared<T>(node);
    }

} // namespace node

#endif
