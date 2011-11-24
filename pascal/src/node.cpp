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

SignNode::SignNode(char sign, const PNode& child) : 
    child(child), _sign(sign) {}
char SignNode::sign() { return _sign; }

NumberNode::NumberNode(std::string val) : value(val) {}
IdentifierNode::IdentifierNode(std::string s) : name(s) {}
StringNode::StringNode(std::string s) : str(s) {}
ConstantNode::ConstantNode(const PNode& node) : child(node) {}

SubrangeTypeNode::SubrangeTypeNode(const PNode& lb, const PNode& ub) : 
        lower_bound(lb), upper_bound(ub) {}

EnumeratedTypeNode::EnumeratedTypeNode(const PNode& id_list) 
        : identifiers(id_list) {}

VariableDeclNode::VariableDeclNode(const PNode& id_list, const PNode& type) :
        id_list(id_list), type(type) {}

RecordTypeNode::RecordTypeNode(const PNode& node) : child(node) {}
SetTypeNode::SetTypeNode(const PNode& type) : type(type) {}
FileTypeNode::FileTypeNode(const PNode& type) : type(type) {}
PointerTypeNode::PointerTypeNode(const PNode& type) : type(type) {}
IndexTypeNode::IndexTypeNode(const PNode& node) : type(node) {}

ArrayTypeNode::ArrayTypeNode(const PNode& list, const PNode& type) :
            index_type_list(list), type(type) {}

VariableSectionNode::VariableSectionNode(const PNode& decls) 
        : declarations(decls) {}

TypeDefinitionNode::TypeDefinitionNode(const PNode& name, const PNode& type) :
        name(name), type(type) {}

PackedTypeNode::PackedTypeNode(const PNode& type) : type(type) {}
DeclarationNode::DeclarationNode(const PNode& child) : child(child) {}
ExpressionNode::ExpressionNode(const PNode& child) : child(child) {}
SetNode::SetNode(const PNode& elems) : elements(elems) {}

IndexedVariableNode::IndexedVariableNode(const PNode& array_var, const PNode& indices) :
        array_variable(array_var), indices(indices) {}

ReferencedVariableNode::ReferencedVariableNode(const PNode& var) : 
        variable(var) {}

FieldDesignatorNode::FieldDesignatorNode(const PNode& var, const PNode& field) : 
        variable(var), field(field) {}

FunctionDesignatorNode::FunctionDesignatorNode(const PNode& func, const PNode& params) :
        function(func), parameters(params) {}

AssignmentStatementNode::AssignmentStatementNode(const PNode& var, const PNode& expr) :
        variable(var), expression(expr) {}

StatementNode::StatementNode(const PNode& child) : child(child) {}

CompoundStatementNode::CompoundStatementNode(const PNode& child) : child(child) {}

WhileStatementNode::WhileStatementNode(const PNode& cond, const PNode& body) : 
        condition(cond), body(body) {}

RepeatStatementNode::RepeatStatementNode(const PNode& body, const PNode& cond) :
        body(body), condition(cond) {}

ForStatementNode::ForStatementNode(const std::shared_ptr<AssignmentStatementNode>& assignment,
                                   int dir, const PNode& final_expression, const PNode& body) :
        variable(assignment -> variable), initial_expression(assignment -> expression),
        final_expression(final_expression), body(body), direction(dir) {}

IfThenNode::IfThenNode(const PNode& cond, const PNode& body) :
        condition(cond), body(body) {}

IfThenElseNode::IfThenElseNode(const PNode& cond, const PNode& _then, const PNode& _else) : 
        condition(cond), then_body(_then), else_body(_else) {}

VariableNode::VariableNode(const PNode& var) : variable(var) {}

WithStatementNode::WithStatementNode(const PNode& vars, const PNode& body) :
        record_variables(vars), body(body) {}

CaseLimbNode::CaseLimbNode(const PNode& constants, const PNode& body) 
        : constants(constants), body(body) {}

CaseStatementNode::CaseStatementNode(const PNode& expr, const PNode& limbs) :
        expression(expr), limbs(limbs) {}

ConstDefinitionNode::ConstDefinitionNode(const PNode& id, const PNode& c) :
        identifier(id), constant(c) {}

BoundSpecificationNode::BoundSpecificationNode(const PNode& lb, const PNode& ub, const PNode& t) :
        lower_bound(lb), upper_bound(ub), type(t) {}

UCArraySchemaNode::UCArraySchemaNode(const PNode& bs, const PNode& t) :
        bounds(bs), type(t) {}

PCArraySchemaNode::PCArraySchemaNode(const PNode& bs, const PNode& t) :
        bounds(bs), type(t) {}

VariableParameterNode::VariableParameterNode(const PNode& ids, const PNode& t) :
        identifiers(ids), type(t) {}

ValueParameterNode::ValueParameterNode(const PNode& ids, const PNode& t) :
        identifiers(ids), type(t) {}

ProcedureHeadingNode::ProcedureHeadingNode(const std::string& name, const PNode& params) :
        name(name), params(params) {}

FunctionHeadingNode::FunctionHeadingNode(const std::string& name, const PNode& params,
        const PNode& return_type) : name(name), params(params), return_type(return_type) {}

ParameterNode::ParameterNode(const PNode& child) : child(child) {}
