#ifndef PRETTY_PRINTER_H
#define PRETTY_PRINTER_H
#include "visitor.h"
#include "node_fwd.h"
#include <memory>
#include <iosfwd>
#include <functional>

struct PrettyPrinter : public Visitor<std::add_const> {
PrettyPrinter(int sw=2);
void visit(const std::shared_ptr<Node >&);
void visit(const std::shared_ptr<NumberNode >&);
void visit(const std::shared_ptr<IdentifierNode >&);
void visit(const std::shared_ptr<IdentifierListNode >&);
void visit(const std::shared_ptr<ConstantNode >&);
void visit(const std::shared_ptr<StringNode >&);
void visit(const std::shared_ptr<EnumeratedTypeNode >&);
void visit(const std::shared_ptr<PointerTypeNode >&);
void visit(const std::shared_ptr<RecordTypeNode >&);
void visit(const std::shared_ptr<SetTypeNode >&);
void visit(const std::shared_ptr<FileTypeNode >&);
void visit(const std::shared_ptr<IndexTypeNode >&);
void visit(const std::shared_ptr<IndexTypeListNode >&);
void visit(const std::shared_ptr<PackedTypeNode >&);
void visit(const std::shared_ptr<VariableSectionNode >&);
void visit(const std::shared_ptr<TypeDefinitionNode >&);
void visit(const std::shared_ptr<TypeSectionNode >&);
void visit(const std::shared_ptr<OperationNode >&);
void visit(const std::shared_ptr<SignNode >&);
void visit(const std::shared_ptr<SubrangeTypeNode >&);
void visit(const std::shared_ptr<VariableDeclNode >&);
void visit(const std::shared_ptr<VariableDeclListNode >&);
void visit(const std::shared_ptr<ArrayTypeNode >&);
void visit(const std::shared_ptr<DeclarationNode >&);
void visit(const std::shared_ptr<DeclarationListNode >&);
void visit(const std::shared_ptr<ExpressionNode >&);
void visit(const std::shared_ptr<ExpressionListNode >&);
void visit(const std::shared_ptr<SetNode >&);
void visit(const std::shared_ptr<IndexedVariableNode >&);
void visit(const std::shared_ptr<ReferencedVariableNode >&);
void visit(const std::shared_ptr<FieldDesignatorNode >&);
void visit(const std::shared_ptr<FunctionDesignatorNode >&);
void visit(const std::shared_ptr<AssignmentStatementNode >&);
void visit(const std::shared_ptr<StatementNode >&);
void visit(const std::shared_ptr<StatementListNode >&);
void visit(const std::shared_ptr<CompoundStatementNode >&);
void visit(const std::shared_ptr<WhileStatementNode >&);
void visit(const std::shared_ptr<RepeatStatementNode >&);
void visit(const std::shared_ptr<ForStatementNode >&);
private:
int indent, sw;
bool is_var_section;
};
#endif