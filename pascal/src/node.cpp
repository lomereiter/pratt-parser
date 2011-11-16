//#include <memory>
//#include <forward_list>
//#include <string>

#include "node.h"
//#include "node_tags.h"
//#include "operator.h"

Node::~Node() {}
size_t Node::tag() {
    return node_traits::get_tag_value<Node>();
}

OperationNode::OperationNode(int arity, Operator op) : _arity(arity), _op(op) {}
int OperationNode::arity() { return _arity; }
int OperationNode::op() { return _op; }

SignNode::SignNode(char sign, const std::shared_ptr<Node>& child) : 
    child(child), _sign(sign) {}
char SignNode::sign() { return _sign; }

NumberNode::NumberNode(std::string val) : value(val) {}
IdentifierNode::IdentifierNode(std::string s) : name(s) {}
StringNode::StringNode(std::string s) : str(s) {}
ConstantNode::ConstantNode(const std::shared_ptr<Node>& node) : child(node) {}

SubrangeTypeNode::SubrangeTypeNode(const std::shared_ptr<ConstantNode>& lb, 
                     const std::shared_ptr<ConstantNode>& ub) : 
        lower_bound(lb), upper_bound(ub) {}

EnumeratedTypeNode::EnumeratedTypeNode(const std::shared_ptr<Node>& id_list) 
        : identifiers(id_list) {}

VariableDeclNode::VariableDeclNode(const std::shared_ptr<Node>& id_list, 
                     const std::shared_ptr<Node>& type) :
        id_list(id_list), type(type) {}

RecordTypeNode::RecordTypeNode(const std::shared_ptr<Node>& node) : child(node) {}
SetTypeNode::SetTypeNode(const std::shared_ptr<Node>& type) : type(type) {}
FileTypeNode::FileTypeNode(const std::shared_ptr<Node>& type) : type(type) {}
PointerTypeNode::PointerTypeNode(const std::shared_ptr<Node>& type) : type(type) {}
IndexTypeNode::IndexTypeNode(const std::shared_ptr<Node>& node) : type(node) {}

ArrayTypeNode::ArrayTypeNode(const std::shared_ptr<Node>& list,
                             const std::shared_ptr<Node>& type) :
            index_type_list(list), type(type) {}

VariableSectionNode::VariableSectionNode(const std::shared_ptr<Node>& decls) 
        : declarations(decls) {}

TypeDefinitionNode::TypeDefinitionNode(const std::shared_ptr<Node>& name, 
                                       const std::shared_ptr<Node>& type) :
        name(name), type(type) {}

PackedTypeNode::PackedTypeNode(const std::shared_ptr<Node>& type) : type(type) {}
DeclarationNode::DeclarationNode(const std::shared_ptr<Node>& child) : child(child) {}
ExpressionNode::ExpressionNode(const std::shared_ptr<Node>& child) : child(child) {}
SetNode::SetNode(const std::shared_ptr<Node>& elems) : elements(elems) {}

IndexedVariableNode::IndexedVariableNode(const std::shared_ptr<Node>& array_var,
                        const std::shared_ptr<Node>& indices) :
        array_variable(array_var), indices(indices) {}

ReferencedVariableNode::ReferencedVariableNode(const std::shared_ptr<Node>& var)
        : variable(var) {}

FieldDesignatorNode::FieldDesignatorNode(const std::shared_ptr<Node>& var,
                                         const std::shared_ptr<Node>& field) : 
        variable(var), field(field) {}

FunctionDesignatorNode::FunctionDesignatorNode(const std::shared_ptr<Node>& func,
                                               const std::shared_ptr<Node>& params) :
        function(func), parameters(params) {}

AssignmentStatementNode::AssignmentStatementNode(const std::shared_ptr<Node>& var,
                                                 const std::shared_ptr<Node>& expr) :
        variable(var), expression(expr) {}

StatementNode::StatementNode(const std::shared_ptr<Node>& child) : child(child) {}
CompoundStatementNode::CompoundStatementNode(const std::shared_ptr<Node>& child) : child(child) {}

WhileStatementNode::WhileStatementNode(const std::shared_ptr<Node>& cond,
                                       const std::shared_ptr<Node>& body) : 
        condition(cond), body(body) {}

RepeatStatementNode::RepeatStatementNode(const std::shared_ptr<Node>& body,
                                         const std::shared_ptr<Node>& cond) :
        body(body), condition(cond) {}

ForStatementNode::ForStatementNode(const std::shared_ptr<AssignmentStatementNode>& assignment,
                                   int dir, 
                                   const std::shared_ptr<Node>& final_expression,
                                   const std::shared_ptr<Node>& body) :
        variable(assignment -> variable), initial_expression(assignment -> expression),
        final_expression(final_expression), body(body), direction(dir) {}

IfThenNode::IfThenNode(const std::shared_ptr<Node>& cond, const std::shared_ptr<Node>& body) :
        condition(cond), body(body) {}

IfThenElseNode::IfThenElseNode(const std::shared_ptr<Node>& cond, 
                               const std::shared_ptr<Node>& _then, 
                               const std::shared_ptr<Node>& _else) : 
        condition(cond), then_body(_then), else_body(_else) {}

VariableNode::VariableNode(const std::shared_ptr<Node>& var) : variable(var) {}

WithStatementNode::WithStatementNode(const std::shared_ptr<Node>& vars, const std::shared_ptr<Node>& body) :
        record_variables(vars), body(body) {}

CaseLimbNode::CaseLimbNode(const std::shared_ptr<Node>& constants,
                           const std::shared_ptr<Node>& body) 
        : constants(constants), body(body) {}

CaseStatementNode::CaseStatementNode(const std::shared_ptr<Node>& expr, 
                                     const std::shared_ptr<Node>& limbs) :
        expression(expr), limbs(limbs) {}

ConstDefinitionNode::ConstDefinitionNode(const std::shared_ptr<Node>& id, 
                                         const std::shared_ptr<Node>& c) :
        identifier(id), constant(c) {}
