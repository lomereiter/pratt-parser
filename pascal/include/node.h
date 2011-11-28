#ifndef NODE_H
#define NODE_H

#include <memory>
#include <forward_list>
#include <string>

#include "node_tags.h"
#include "operator.h"

struct Node {
    virtual ~Node();
    virtual size_t tag();
};

typedef std::shared_ptr<Node> PNode;

template <class NodeType>
struct VisitableNode : public Node {
    virtual size_t tag() {
        return node_traits::get_tag_value<NodeType>();
    }
};

template <typename T>
struct ListOf : public VisitableNode<ListOf<T>> {
    typedef std::forward_list<PNode> ListT;
    ListOf() {}
    ListOf(ListT&& lst) : lst(lst) {}
    ListOf(const PNode& node) {
        lst.push_front(node);
    }
    ListT& list() { return lst; }
private:
    ListT lst;
};

struct EmptyNode : public VisitableNode<EmptyNode> {};

struct OperationNode : public VisitableNode<OperationNode> {
    std::forward_list<PNode> args;

    OperationNode(int arity, Operator op);

    int arity();
    int op();

private:
    int _arity;
    Operator _op;
};

struct SignNode : public VisitableNode<SignNode> {
    PNode child;
    SignNode(char sign, const PNode& child);
    char sign();
    private:
        char _sign;
};

struct NumberNode : public VisitableNode<NumberNode> {
    std::string value;
    NumberNode(std::string val);
};

struct IdentifierNode : public VisitableNode<IdentifierNode> {
    std::string name;
    IdentifierNode(std::string s);
};

struct StringNode : public VisitableNode<StringNode> { 
    std::string str;
    StringNode(std::string s);
};

struct ConstantNode : public VisitableNode<ConstantNode> {
    PNode child;
    ConstantNode(const PNode& node);
};

struct SubrangeNode : public VisitableNode<SubrangeNode> {
    PNode lower_bound;
    PNode upper_bound;

    SubrangeNode(const PNode& lb, const PNode& ub);
};

struct EnumeratedTypeNode : public VisitableNode<EnumeratedTypeNode> {
    PNode identifiers;
    EnumeratedTypeNode(const PNode& id_list);
};

struct VariableDeclNode : public VisitableNode<VariableDeclNode> {
    PNode id_list;
    PNode type;

    VariableDeclNode(const PNode& id_list, const PNode& type);
};

struct RecordTypeNode : public VisitableNode<RecordTypeNode> {
    PNode child;
    RecordTypeNode(const PNode& node);
};

struct SetTypeNode : public VisitableNode<SetTypeNode> {
    PNode type;
    SetTypeNode(const PNode& type);
};

struct FileTypeNode : public VisitableNode<FileTypeNode> {
    PNode type;
    FileTypeNode(const PNode& type);
};

struct PointerTypeNode : public VisitableNode<PointerTypeNode> {
    PNode type;
    PointerTypeNode(const PNode& type);
};

struct IndexTypeNode : public VisitableNode<IndexTypeNode> {
    PNode type;
    IndexTypeNode(const PNode& node);
};

struct ArrayTypeNode : public VisitableNode<ArrayTypeNode> {
    PNode index_type_list;
    PNode type;
    ArrayTypeNode(const PNode& list, const PNode& type);
};

struct VariableSectionNode : public VisitableNode<VariableSectionNode> {
    PNode declarations;
    VariableSectionNode(const PNode& decls);
};

struct TypeDefinitionNode : public VisitableNode<TypeDefinitionNode> {
    PNode name;
    PNode type;
    TypeDefinitionNode(const PNode& name, const PNode& type);
};

struct PackedTypeNode : public VisitableNode<PackedTypeNode> {
    PNode type;
    PackedTypeNode(const PNode& type);
};

struct DeclarationNode : public VisitableNode<DeclarationNode> {
    PNode child;
    DeclarationNode(const PNode& child);
};

struct ExpressionNode : public VisitableNode<ExpressionNode> {
    PNode child;
    ExpressionNode(const PNode& child);
};

struct SetExpressionNode : public VisitableNode<SetExpressionNode> {
    PNode child;
    SetExpressionNode(const PNode& child);
};

struct SetNode : public VisitableNode<SetNode> {
    PNode elements;
    SetNode(const PNode& elems);
};

struct IndexedVariableNode : public VisitableNode<IndexedVariableNode> {
    PNode array_variable;
    PNode indices;
    IndexedVariableNode(const PNode& array_var, const PNode& indices);
};

struct ReferencedVariableNode : public VisitableNode<ReferencedVariableNode> {
    PNode variable;
    ReferencedVariableNode(const PNode& var);
};

struct FieldDesignatorNode : public VisitableNode<FieldDesignatorNode> {
    PNode variable;
    PNode field;
    FieldDesignatorNode(const PNode& var, const PNode& field);
};

struct FunctionDesignatorNode : public VisitableNode<FunctionDesignatorNode> {
    PNode function;
    PNode parameters;
    FunctionDesignatorNode(const PNode& func, const PNode& params);
};

struct AssignmentStatementNode : public VisitableNode<AssignmentStatementNode> {
    PNode variable;
    PNode expression;
    AssignmentStatementNode(const PNode& var, const PNode& expr);
};

struct StatementNode : public VisitableNode<StatementNode> {
    PNode child;
    StatementNode(const PNode& child);
};

struct CompoundStatementNode : public VisitableNode<CompoundStatementNode> {
    PNode child;
    CompoundStatementNode(const PNode& child);
};

struct WhileStatementNode : public VisitableNode<WhileStatementNode> {
    PNode condition;
    PNode body;
    WhileStatementNode(const PNode& cond, const PNode& body);
};

struct RepeatStatementNode : public VisitableNode<RepeatStatementNode> {
    PNode body;
    PNode condition;
    RepeatStatementNode(const PNode& body, const PNode& cond);
};

struct ForStatementNode : public VisitableNode<ForStatementNode> {
    PNode variable;
    PNode initial_expression;
    PNode final_expression;
    PNode body;
    int direction;
    ForStatementNode(const std::shared_ptr<AssignmentStatementNode>&, int, 
                     const PNode&, const PNode&);
};

struct IfThenNode : public VisitableNode<IfThenNode> {
    PNode condition;
    PNode body;
    IfThenNode(const PNode& cond, const PNode& body);
};

struct IfThenElseNode : public VisitableNode<IfThenElseNode> {
    PNode condition;
    PNode then_body;
    PNode else_body;
    IfThenElseNode(const PNode& cond, const PNode& _then, const PNode& _else);
};

struct VariableNode : public VisitableNode<VariableNode> {
    PNode variable;
    VariableNode(const PNode& var);
};

struct WithStatementNode : public VisitableNode<WithStatementNode> {
    PNode record_variables;
    PNode body;
    WithStatementNode(const PNode& vars, const PNode& body);
};

struct CaseLimbNode : public VisitableNode<CaseLimbNode> {
    PNode constants;
    PNode body;
    CaseLimbNode(const PNode& constants, const PNode& body);
};

struct CaseStatementNode : public VisitableNode<CaseStatementNode> {
    PNode expression;
    PNode limbs;
    CaseStatementNode(const PNode& expr, const PNode& limbs);
};

struct ConstDefinitionNode : public VisitableNode<ConstDefinitionNode> {
    PNode identifier;
    PNode constant;
    ConstDefinitionNode(const PNode& id, const PNode& c);
};

struct BoundSpecificationNode : public VisitableNode<BoundSpecificationNode> {
    PNode lower_bound;
    PNode upper_bound;
    PNode type;
    BoundSpecificationNode(const PNode& lb, const PNode& ub, const PNode& type);
};

struct UCArraySchemaNode : public VisitableNode<UCArraySchemaNode> {
    PNode bounds;
    PNode type;
    UCArraySchemaNode(const PNode& bounds, const PNode& type);
};

struct PCArraySchemaNode : public VisitableNode<PCArraySchemaNode> {
    PNode bounds;
    PNode type;
    PCArraySchemaNode(const PNode& bounds, const PNode& type);
};

struct VariableParameterNode : public VisitableNode<VariableParameterNode> {
    PNode identifiers;
    PNode type;
    VariableParameterNode(const PNode& ids, const PNode& type);
};

struct ValueParameterNode : public VisitableNode<ValueParameterNode> {
    PNode identifiers;
    PNode type;
    ValueParameterNode(const PNode& ids, const PNode& type);
};

struct ProcedureHeadingNode : public VisitableNode<ProcedureHeadingNode> {
    std::string name;
    PNode params;
    ProcedureHeadingNode(const std::string& name, const PNode& params);
};

struct ParameterNode : public VisitableNode<ParameterNode> {
    PNode child;
    ParameterNode(const PNode& child);
};

struct FunctionHeadingNode : public VisitableNode<FunctionHeadingNode> {
    std::string name;
    PNode params;
    PNode return_type;
    FunctionHeadingNode(const std::string& n, const PNode& p, const PNode& r);
};

struct ProcedureNode : public VisitableNode<ProcedureNode> {
    PNode heading;
    PNode body;
    ProcedureNode(const PNode& heading, const PNode& body);
};

struct FunctionNode : public VisitableNode<FunctionNode> {
    PNode heading;
    PNode body;
    FunctionNode(const PNode& heading, const PNode& body);
};

struct ProcedureForwardDeclNode : public VisitableNode<ProcedureForwardDeclNode> {
    PNode heading;
    ProcedureForwardDeclNode(const PNode& heading);
};

struct FunctionForwardDeclNode : public VisitableNode<FunctionForwardDeclNode> {
    PNode heading;
    FunctionForwardDeclNode(const PNode& heading);
};

#ifdef PASCAL_6000
struct ProcedureExternDeclNode : public VisitableNode<ProcedureExternDeclNode> {
    PNode heading;
    ProcedureExternDeclNode(const PNode& heading);
};

struct FunctionExternDeclNode : public VisitableNode<FunctionExternDeclNode> {
    PNode heading;
    FunctionExternDeclNode(const PNode& heading);
};
#endif

struct BlockNode : public VisitableNode<BlockNode> {
    PNode declarations;
    PNode statements;
    BlockNode(const PNode& declarations, const PNode& statements);
};

struct OutputValueNode : public VisitableNode<OutputValueNode> {
    PNode expression;
    PNode field_width;
    PNode fraction_length;
    OutputValueNode(const PNode& expr, const PNode& fw, const PNode& fraction_length);
};

struct WriteNode : public VisitableNode<WriteNode> {
    PNode output_list;
    WriteNode(const PNode& list);
};

struct WriteLineNode : public VisitableNode<WriteLineNode> {
    PNode output_list;
    WriteLineNode(const PNode& list);
};

struct RecordSectionNode : public VisitableNode<RecordSectionNode> { 
    PNode id_list;
    PNode type;
    RecordSectionNode(std::shared_ptr<VariableDeclNode>&&);
};

struct FieldVariantNode : public VisitableNode<FieldVariantNode> {
    PNode case_labels;
    PNode fields;
    FieldVariantNode(const PNode& case_labels, const PNode& fields);
};

struct FieldListNode : public VisitableNode<FieldListNode> {
    PNode fixed_part;
    PNode variant_part;
    FieldListNode(const PNode& f, const PNode& v);
};
#endif
