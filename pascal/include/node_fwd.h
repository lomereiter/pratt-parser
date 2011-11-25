#ifndef NODE_FWD_H
#define NODE_FWD_H

struct Node;

template <typename T> struct ListOf;

struct NumberNode;

struct IdentifierNode;
typedef ListOf<IdentifierNode> IdentifierListNode;

struct OperationNode;
struct StringNode;
struct SignNode;
struct ConstantNode;
typedef ListOf<ConstantNode> ConstantListNode;

struct SubrangeTypeNode;
struct EnumeratedTypeNode;
struct PointerTypeNode;

struct VariableDeclNode;
typedef ListOf<VariableDeclNode> VariableDeclListNode;

struct RecordTypeNode;
struct SetTypeNode;
struct FileTypeNode;

struct IndexTypeNode;
typedef ListOf<IndexTypeNode> IndexTypeListNode;

struct ArrayTypeNode;
struct VariableSectionNode;

struct TypeDefinitionNode;
typedef ListOf<TypeDefinitionNode> TypeSectionNode;

struct PackedTypeNode;

struct DeclarationNode;
typedef ListOf<DeclarationNode> DeclarationListNode;

struct ExpressionNode;
typedef ListOf<ExpressionNode> ExpressionListNode;

struct SetNode;
struct IndexedVariableNode;
struct ReferencedVariableNode;
struct FieldDesignatorNode;
struct FunctionDesignatorNode;

struct AssignmentStatementNode;
struct CompoundStatementNode;
struct WhileStatementNode;
struct RepeatStatementNode;
struct ForStatementNode;
struct StatementNode;
typedef ListOf<StatementNode> StatementListNode;

struct IfThenNode;
struct IfThenElseNode;

struct VariableNode;
typedef ListOf<VariableNode> VariableListNode;

struct WithStatementNode;

struct CaseStatementNode;
struct CaseLimbNode;
typedef ListOf<CaseLimbNode> CaseLimbListNode;

struct ConstDefinitionNode;
typedef ListOf<ConstDefinitionNode> ConstSectionNode;

struct BoundSpecificationNode;
typedef ListOf<BoundSpecificationNode> BoundSpecificationListNode;

struct UCArraySchemaNode;
struct PCArraySchemaNode;
struct VariableParameterNode;
struct ValueParameterNode;
struct ProcedureHeadingNode;
struct FunctionHeadingNode;

struct ParameterNode;
typedef ListOf<ParameterNode> ParameterListNode;

struct ProcedureNode;
struct FunctionNode;
struct ProcedureForwardDeclNode;
struct FunctionForwardDeclNode;
struct BlockNode;

#endif
