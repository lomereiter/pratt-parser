#include "node_traits.h"

namespace node_traits {
namespace detail {

bool __is_convertible_helper(type<ExpressionNode>, const PNode& node) {
    static IsExpr is_expr;
    return is_expr(node);
}

bool __is_convertible_helper(type<VariableNode>, const PNode& node) {
    static IsVar is_var;
    return is_var(node);
}

bool __is_convertible_helper(type<IndexTypeNode>, const PNode& node) {
    static IsIndexType is_index_type_node;
    return is_index_type_node(node);
}

bool __is_convertible_helper(type<StatementNode>, const PNode& node) {
    static IsStatement is_statement;
    return is_statement(node);
}

bool __is_convertible_helper(type<DeclarationNode>, const PNode& node) {
    static IsDeclaration is_decl;
    return is_decl(node);
}

bool __is_convertible_helper(type<ConstantNode>, const PNode& node) {
    static AreConvertibleTo<ConstantNode,
        StringNode, NumberNode, IdentifierNode> is_surely_constant;
    return is_surely_constant(node) ||
           ( has_type<SignNode>(node) &&
             is_surely_constant(std::static_pointer_cast<SignNode>(node) -> child) );
}

bool __is_convertible_helper(type<ParameterNode>, const PNode& node) {
    static IsParameterNode is_parameter_node;
    return is_parameter_node(node);
}

} // namespace detail
} // namespace node_traits
