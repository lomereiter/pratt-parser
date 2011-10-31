#ifndef EXPRESSION_FWD_H
#define EXPRESSION_FWD_H

struct Node;

template <typename T> struct ListOf;

struct NumberNode;

struct IdentifierNode;
typedef ListOf<IdentifierNode> IdentifierListNode;

struct ExpressionNode;
struct StringNode;
struct SignNode;
struct ConstantNode;
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

#endif
