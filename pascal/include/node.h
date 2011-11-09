#ifndef NODE_H
#define NODE_H

#include <memory>
#include <forward_list>
#include <string>

#include "node_fwd.h"
#include "node_tags.h"
#include "operator.h"

struct Node : public std::enable_shared_from_this<Node> {
    /* is default constructible */
    virtual ~Node() {}
    virtual size_t tag() {
        return node_traits::get_tag_value<Node>();
    }
};

template <class NodeType>
struct VisitableNode : public Node {
    virtual size_t tag() {
        return node_traits::get_tag_value<NodeType>();
    }
};

template <typename T>
struct ListOf : public VisitableNode<ListOf<T>> {
    typedef std::forward_list<std::shared_ptr<Node>> ListT;
    ListOf() {}
    ListOf(ListT&& lst) : lst(lst) {}
    ListOf(const std::shared_ptr<Node>& node) {
        lst.push_front(node);
    }
    ListT& list() { return lst; }
private:
    ListT lst;
};

struct OperationNode : public VisitableNode<OperationNode> {
    std::forward_list<std::shared_ptr<Node>> args;

    OperationNode(int arity, Operator op) : _arity(arity), _op(op) {}

    int arity() { return _arity; }
    int op() { return _op; }

private:
    int _arity;
    Operator _op;
};

struct SignNode : public VisitableNode<SignNode> {
    std::shared_ptr<Node> child;
    SignNode(char sign, const std::shared_ptr<Node>& child) : child(child), _sign(sign) {}
    char sign() { return _sign; }
    private:
        char _sign;
};

struct NumberNode : public VisitableNode<NumberNode> {
    std::string value;
    NumberNode(std::string val) : value(val) {}
};

struct IdentifierNode : public VisitableNode<IdentifierNode> {
    std::string name;
    IdentifierNode(std::string s) : name(s) {}
};

struct StringNode : public VisitableNode<StringNode> { 
    std::string str;
    StringNode(std::string s) : str(s) {}
};

struct ConstantNode : public VisitableNode<ConstantNode> {
    std::shared_ptr<Node> child;
    ConstantNode(const std::shared_ptr<Node>& node) : child(node) {}
};

struct SubrangeTypeNode : public VisitableNode<SubrangeTypeNode> {
    std::shared_ptr<ConstantNode> lower_bound;
    std::shared_ptr<ConstantNode> upper_bound;

    SubrangeTypeNode(const std::shared_ptr<ConstantNode>& lb, 
                     const std::shared_ptr<ConstantNode>& ub) : 
        lower_bound(lb), upper_bound(ub) {}
};

struct EnumeratedTypeNode : public VisitableNode<EnumeratedTypeNode> {
    std::shared_ptr<Node> identifiers;
    EnumeratedTypeNode(const std::shared_ptr<Node>& id_list) 
        : identifiers(id_list) {}
};

struct VariableDeclNode : public VisitableNode<VariableDeclNode> {
    std::shared_ptr<Node> id_list;
    std::shared_ptr<Node> type;

    VariableDeclNode(const std::shared_ptr<Node>& id_list, 
                     const std::shared_ptr<Node>& type) :
        id_list(id_list), type(type) {}
};

struct RecordTypeNode : public VisitableNode<RecordTypeNode> {
    std::shared_ptr<Node> child;
    RecordTypeNode(const std::shared_ptr<Node>& node) : child(node) {}
};

struct SetTypeNode : public VisitableNode<SetTypeNode> {
    std::shared_ptr<Node> type;
    SetTypeNode(const std::shared_ptr<Node>& type) : type(type) {}
};

struct FileTypeNode : public VisitableNode<FileTypeNode> {
    std::shared_ptr<Node> type;
    FileTypeNode(const std::shared_ptr<Node>& type) : type(type) {}
};

struct PointerTypeNode : public VisitableNode<PointerTypeNode> {
    std::shared_ptr<Node> type;
    PointerTypeNode(const std::shared_ptr<Node>& type) : type(type) {}
};

struct IndexTypeNode : public VisitableNode<IndexTypeNode> {
    std::shared_ptr<Node> type;
    IndexTypeNode(const std::shared_ptr<Node>& node) : type(node) {}
};

struct ArrayTypeNode : public VisitableNode<ArrayTypeNode> {
    std::shared_ptr<Node> index_type_list;
    std::shared_ptr<Node> type;
    ArrayTypeNode(const std::shared_ptr<Node>& list, const std::shared_ptr<Node>& type) :
            index_type_list(list), type(type) {}
};

struct VariableSectionNode : public VisitableNode<VariableSectionNode> {
    std::shared_ptr<Node> declarations;
    VariableSectionNode(const std::shared_ptr<Node>& decls) : declarations(decls) {}
};

struct TypeDefinitionNode : public VisitableNode<TypeDefinitionNode> {
    std::shared_ptr<Node> name;
    std::shared_ptr<Node> type;
    TypeDefinitionNode(const std::shared_ptr<Node>& name, const std::shared_ptr<Node>& type) :
        name(name), type(type) {}
};

struct PackedTypeNode : public VisitableNode<PackedTypeNode> {
    std::shared_ptr<Node> type;
    PackedTypeNode(const std::shared_ptr<Node>& type) : type(type) {}
};

struct DeclarationNode : public VisitableNode<DeclarationNode> {
    std::shared_ptr<Node> child;
    DeclarationNode(const std::shared_ptr<Node>& child) : child(child) {}
};

struct ExpressionNode : public VisitableNode<ExpressionNode> {
    std::shared_ptr<Node> child;
    ExpressionNode(const std::shared_ptr<Node>& child) : child(child) {}
};

struct SetNode : public VisitableNode<SetNode> {
    std::shared_ptr<Node> elements;
    SetNode(const std::shared_ptr<Node>& elems) : elements(elems) {}
};

struct IndexedVariableNode : public VisitableNode<IndexedVariableNode> {
    std::shared_ptr<Node> array_variable;
    std::shared_ptr<Node> indices;
    IndexedVariableNode(const std::shared_ptr<Node>& array_var,
                        const std::shared_ptr<Node>& indices) :
        array_variable(array_var), indices(indices) {}
};

struct ReferencedVariableNode : public VisitableNode<ReferencedVariableNode> {
    std::shared_ptr<Node> variable;
    ReferencedVariableNode(const std::shared_ptr<Node>& var) : variable(var) {}
};

struct FieldDesignatorNode : public VisitableNode<FieldDesignatorNode> {
    std::shared_ptr<Node> variable;
    std::shared_ptr<Node> field;
    FieldDesignatorNode(const std::shared_ptr<Node>& var,
                        const std::shared_ptr<Node>& field) : 
        variable(var), field(field) {}
};

struct FunctionDesignatorNode : public VisitableNode<FunctionDesignatorNode> {
    std::shared_ptr<Node> function;
    std::shared_ptr<Node> parameters;
    FunctionDesignatorNode(const std::shared_ptr<Node>& func,
                           const std::shared_ptr<Node>& params) :
        function(func), parameters(params) {}
};

struct AssignmentStatementNode : public VisitableNode<AssignmentStatementNode> {
    std::shared_ptr<Node> variable;
    std::shared_ptr<Node> expression;
    AssignmentStatementNode(const std::shared_ptr<Node>& var,
                   const std::shared_ptr<Node>& expr) :
        variable(var), expression(expr) {}
};

struct StatementNode : public VisitableNode<StatementNode> {
    std::shared_ptr<Node> child;
    StatementNode(const std::shared_ptr<Node>& child) : child(child) {}
};

struct CompoundStatementNode : public VisitableNode<CompoundStatementNode> {
    std::shared_ptr<Node> child;
    CompoundStatementNode(const std::shared_ptr<Node>& child) : child(child) {}
};

struct WhileStatementNode : public VisitableNode<WhileStatementNode> {
    std::shared_ptr<Node> condition;
    std::shared_ptr<Node> body;
    WhileStatementNode(const std::shared_ptr<Node>& cond,
                       const std::shared_ptr<Node>& body) : 
        condition(cond), body(body) {}
};

struct RepeatStatementNode : public VisitableNode<RepeatStatementNode> {
    std::shared_ptr<Node> body;
    std::shared_ptr<Node> condition;
    RepeatStatementNode(const std::shared_ptr<Node>& body,
                        const std::shared_ptr<Node>& cond) :
        body(body), condition(cond) {}
};

struct ForStatementNode : public VisitableNode<ForStatementNode> {
    std::shared_ptr<Node> variable;
    std::shared_ptr<Node> initial_expression;
    std::shared_ptr<Node> final_expression;
    std::shared_ptr<Node> body;
    int direction;
    ForStatementNode(const std::shared_ptr<AssignmentStatementNode>& assignment,
                     int dir, const std::shared_ptr<Node>& final_expression,
                     const std::shared_ptr<Node>& body) :
        variable(assignment -> variable), initial_expression(assignment -> expression),
        final_expression(final_expression), body(body), direction(dir) {}
};

struct IfThenNode : public VisitableNode<IfThenNode> {
    std::shared_ptr<Node> condition;
    std::shared_ptr<Node> body;
    IfThenNode(const std::shared_ptr<Node>& cond, const std::shared_ptr<Node>& body) :
        condition(cond), body(body) {}
};

struct IfThenElseNode : public VisitableNode<IfThenElseNode> {
    std::shared_ptr<Node> condition;
    std::shared_ptr<Node> then_body;
    std::shared_ptr<Node> else_body;
    IfThenElseNode(const std::shared_ptr<Node>& cond, const std::shared_ptr<Node>& _then,
                   const std::shared_ptr<Node>& _else) : 
        condition(cond), then_body(_then), else_body(_else) {}
};

struct VariableNode : public VisitableNode<VariableNode> {
    std::shared_ptr<Node> variable;
    VariableNode(const std::shared_ptr<Node>& var) : variable(var) {}
};

struct WithStatementNode : public VisitableNode<WithStatementNode> {
    std::shared_ptr<Node> record_variables;
    std::shared_ptr<Node> body;
    WithStatementNode(const std::shared_ptr<Node>& vars, const std::shared_ptr<Node>& body) :
        record_variables(vars), body(body) {}
};

struct CaseLimbNode : public VisitableNode<CaseLimbNode> {
    std::shared_ptr<Node> constants;
    std::shared_ptr<Node> body;
    CaseLimbNode(const std::shared_ptr<Node>& constants,
                 const std::shared_ptr<Node>& body) : constants(constants), body(body) {}
};

struct CaseStatementNode : public VisitableNode<CaseStatementNode> {
    std::shared_ptr<Node> expression;
    std::shared_ptr<Node> limbs;
    CaseStatementNode(const std::shared_ptr<Node>& expr, const std::shared_ptr<Node>& limbs) :
        expression(expr), limbs(limbs) {}
};

struct ConstDefinitionNode : public VisitableNode<ConstDefinitionNode> {
    std::shared_ptr<Node> identifier;
    std::shared_ptr<Node> constant;
    ConstDefinitionNode(const std::shared_ptr<Node>& id, const std::shared_ptr<Node>& c) :
        identifier(id), constant(c) {}
};
#endif
