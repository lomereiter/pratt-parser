#include "node_traits.h"

namespace node_traits {
namespace detail {

bool __is_convertible_helper(type<ExpressionNode>, const std::shared_ptr<Node>& node) {
    static IsExpr is_expr;
    return is_expr(node);
}

bool __is_convertible_helper(type<VariableNode>, const std::shared_ptr<Node>& node) {
    static IsVar is_var;
    return is_var(node);
}

bool __is_convertible_helper(type<IndexTypeNode>, const std::shared_ptr<Node>& node) {
    static IsIndexType is_index_type_node;
    return is_index_type_node(node);
}

bool __is_convertible_helper(type<StatementNode>, const std::shared_ptr<Node>& node) {
    static IsStatement is_statement;
    return is_statement(node);
}

bool __is_convertible_helper(type<ConstantNode>, const std::shared_ptr<Node>& node) {
    static AreConvertibleTo<ConstantNode,
        StringNode, NumberNode, IdentifierNode> is_surely_constant;
    return is_surely_constant(node) ||
           ( has_type<SignNode>(node) &&
             is_surely_constant(std::static_pointer_cast<SignNode>(node) -> child) );
}

} // namespace detail
} // namespace node_traits
