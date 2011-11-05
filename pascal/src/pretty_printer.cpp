#include "visitor.h"
#include "node.h"
#include "pretty_printer.h"
#include "operator.h"
#include <memory>
#include <iostream>
#include <functional>
PrettyPrinter::PrettyPrinter(int sw) : indent(0), sw(sw),
is_var_section(false)
{
Visits<PrettyPrinter, Node, NumberNode, IdentifierNode, IdentifierListNode, ConstantNode, StringNode, EnumeratedTypeNode, PointerTypeNode, RecordTypeNode, SetTypeNode, FileTypeNode, IndexTypeNode, IndexTypeListNode, PackedTypeNode, VariableSectionNode, TypeDefinitionNode, TypeSectionNode, OperationNode, SignNode, SubrangeTypeNode, VariableDeclNode, VariableDeclListNode, ArrayTypeNode, DeclarationNode, DeclarationListNode, ExpressionNode, ExpressionListNode, SetNode, IndexedVariableNode, ReferencedVariableNode, FieldDesignatorNode, FunctionDesignatorNode, AssignmentStatementNode, StatementNode, StatementListNode, CompoundStatementNode, WhileStatementNode, RepeatStatementNode, ForStatementNode>();
}
void PrettyPrinter::visit(const std::shared_ptr<Node>& e) {
std::cout << std::string(indent, ' ') << "IMPLEMENT ME!";
std::cout << std::endl;

}
void PrettyPrinter::visit(const std::shared_ptr<NumberNode>& e) {
std::cout << std::string(indent, ' ') << e -> value;
std::cout << std::endl;

}
void PrettyPrinter::visit(const std::shared_ptr<IdentifierNode>& e) {
std::cout << std::string(indent, ' ') << "IDENTIFIER ";
{ int old_indent = indent;
indent = 0;
std::cout << std::string(indent, ' ') << e -> name;
std::cout << std::endl;
indent = old_indent;
}
}
void PrettyPrinter::visit(const std::shared_ptr<IdentifierListNode>& e) {
std::cout << std::string(indent, ' ') << "IDENTIFIER LIST:";
std::cout << std::endl;
indent += sw;
for (auto child : e -> list()) travel(child);
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<ConstantNode>& e) {
std::cout << std::string(indent, ' ') << "CONSTANT";
std::cout << std::endl;
indent += sw;
travel(e -> child);
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<StringNode>& e) {
std::cout << std::string(indent, ' ') << "STRING LITERAL: ";
std::cout << std::string(indent, ' ') << e -> str;
std::cout << std::endl;

}
void PrettyPrinter::visit(const std::shared_ptr<EnumeratedTypeNode>& e) {
std::cout << std::string(indent, ' ') << "ENUMERATED TYPE:";
std::cout << std::endl;
indent += sw;
travel(e -> identifiers);
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<PointerTypeNode>& e) {
std::cout << std::string(indent, ' ') << "POINTER TYPE: ";
{ int old_indent = indent;
indent = 0;
travel(e -> type);
indent = old_indent;
}
}
void PrettyPrinter::visit(const std::shared_ptr<RecordTypeNode>& e) {
std::cout << std::string(indent, ' ') << "RECORD TYPE";
std::cout << std::endl;
indent += sw;
travel(e -> child);
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<SetTypeNode>& e) {
std::cout << std::string(indent, ' ') << "SET OF";
std::cout << std::endl;
indent += sw;
travel(e -> type);
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<FileTypeNode>& e) {
std::cout << std::string(indent, ' ') << "FILE OF";
std::cout << std::endl;
indent += sw;
travel(e -> type);
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<IndexTypeNode>& e) {
travel(e -> type);

}
void PrettyPrinter::visit(const std::shared_ptr<IndexTypeListNode>& e) {
for (auto child : e -> list()) travel(child);

}
void PrettyPrinter::visit(const std::shared_ptr<PackedTypeNode>& e) {
std::cout << std::string(indent, ' ') << "PACKED TYPE:";
std::cout << std::endl;
indent += sw;
travel(e -> type);
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<VariableSectionNode>& e) {
std::cout << std::string(indent, ' ') << "VARIABLE DECLARATION SECTION";
std::cout << std::endl;
indent += sw;
travel(e -> declarations);
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<TypeDefinitionNode>& e) {
std::cout << std::string(indent, ' ') << "TYPE DEFINITION: ";
{ int old_indent = indent;
indent = 0;
travel(e -> name);
indent = old_indent;
}indent += sw;
travel(e -> type);
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<TypeSectionNode>& e) {
std::cout << std::string(indent, ' ') << "TYPE DEFINITION SECTION";
std::cout << std::endl;
indent += sw;
for (auto child : e -> list()) travel(child);
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<OperationNode>& e) {
std::cout << std::string(indent, ' ') << operators::operatorName[e -> op()];
std::cout << std::endl;
indent += sw;
for (const auto& node : e -> args) travel(node);indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<SignNode>& e) {
std::cout << std::string(indent, ' ') << (e -> sign() == '+' ? "PLUS SIGN" : "MINUS SIGN");
std::cout << std::endl;
indent += sw;
travel(e -> child);
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<SubrangeTypeNode>& e) {
std::cout << std::string(indent, ' ') << "SUBRANGE TYPE:";
std::cout << std::endl;
indent += sw;
std::cout << std::string(indent, ' ') << "LOWER BOUND";
std::cout << std::endl;
indent += sw;
travel(e -> lower_bound);
indent -= sw;
std::cout << std::string(indent, ' ') << "UPPER BOUND";
std::cout << std::endl;
indent += sw;
travel(e -> upper_bound);
indent -= sw;
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<VariableDeclNode>& e) {
std::cout << std::string(indent, ' ') << "VARIABLE DECLARATION: ";
std::cout << std::endl;
indent += sw;
travel(e -> id_list);
std::cout << std::string(indent, ' ') << "OF TYPE";
std::cout << std::endl;
indent += sw;
travel(e -> type);
indent -= sw;
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<VariableDeclListNode>& e) {
for (auto child : e -> list()) travel(child);

}
void PrettyPrinter::visit(const std::shared_ptr<ArrayTypeNode>& e) {
std::cout << std::string(indent, ' ') << "ARRAY WITH BOUNDS";
std::cout << std::endl;
indent += sw;
travel(e -> index_type_list);
indent -= sw;
std::cout << std::string(indent, ' ') << "OF TYPE";
std::cout << std::endl;
indent += sw;
travel(e -> type);
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<DeclarationNode>& e) {
travel(e -> child);

}
void PrettyPrinter::visit(const std::shared_ptr<DeclarationListNode>& e) {
for (auto child : e -> list()) travel(child);

}
void PrettyPrinter::visit(const std::shared_ptr<ExpressionNode>& e) {
travel(e -> child);

}
void PrettyPrinter::visit(const std::shared_ptr<ExpressionListNode>& e) {
std::cout << std::string(indent, ' ') << "LIST OF EXPRESSIONS:";
std::cout << std::endl;
indent += sw;
for (auto child : e -> list()) travel(child);
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<SetNode>& e) {
std::cout << std::string(indent, ' ') << "SET: ";
std::cout << std::endl;
indent += sw;
travel(e -> elements);
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<IndexedVariableNode>& e) {
std::cout << std::string(indent, ' ') << "INDEXED VARIABLE:";
std::cout << std::endl;
indent += sw;
std::cout << std::string(indent, ' ') << "ARRAY VARIABLE:";
std::cout << std::endl;
indent += sw;
travel(e -> array_variable);
indent -= sw;
std::cout << std::string(indent, ' ') << "INDICES:";
std::cout << std::endl;
indent += sw;
travel(e -> indices);
indent -= sw;
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<ReferencedVariableNode>& e) {
std::cout << std::string(indent, ' ') << "REFERENCED VARIABLE:";
std::cout << std::endl;
indent += sw;
travel(e -> variable);
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<FieldDesignatorNode>& e) {
std::cout << std::string(indent, ' ') << "RECORD FIELD VARIABLE:";
std::cout << std::endl;
indent += sw;
std::cout << std::string(indent, ' ') << "RECORD VARIABLE:";
std::cout << std::endl;
indent += sw;
travel(e -> variable);
indent -= sw;
std::cout << std::string(indent, ' ') << "FIELD NAME: ";
{ int old_indent = indent;
indent = 0;
travel(e -> field);
indent = old_indent;
}indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<FunctionDesignatorNode>& e) {
std::cout << std::string(indent, ' ') << "PROCEDURE/FUNCTION CALL: ";
{ int old_indent = indent;
indent = 0;
travel(e -> function);
indent = old_indent;
}indent += sw;
travel(e -> parameters);
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<AssignmentStatementNode>& e) {
std::cout << std::string(indent, ' ') << "ASSIGNMENT:";
std::cout << std::endl;
indent += sw;
travel(e -> variable);
travel(e -> expression);
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<StatementNode>& e) {
travel(e -> child);

}
void PrettyPrinter::visit(const std::shared_ptr<StatementListNode>& e) {
std::cout << std::string(indent, ' ') << "STATEMENT SEQUENCE:";
std::cout << std::endl;
indent += sw;
for (auto child : e -> list()) travel(child);
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<CompoundStatementNode>& e) {
std::cout << std::string(indent, ' ') << "COMPOUND STATEMENT:";
std::cout << std::endl;
indent += sw;
travel(e -> child);
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<WhileStatementNode>& e) {
std::cout << std::string(indent, ' ') << "WHILE STATEMENT:";
std::cout << std::endl;
indent += sw;
std::cout << std::string(indent, ' ') << "LOOP CONDITION:";
std::cout << std::endl;
indent += sw;
travel(e -> condition);
indent -= sw;
std::cout << std::string(indent, ' ') << "LOOP BODY:";
std::cout << std::endl;
indent += sw;
travel(e -> body);
indent -= sw;
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<RepeatStatementNode>& e) {
std::cout << std::string(indent, ' ') << "REPEAT STATEMENT:";
std::cout << std::endl;
indent += sw;
std::cout << std::string(indent, ' ') << "LOOP BODY:";
std::cout << std::endl;
indent += sw;
travel(e -> body);
indent -= sw;
std::cout << std::string(indent, ' ') << "LOOP CONDITION:";
std::cout << std::endl;
indent += sw;
travel(e -> condition);
indent -= sw;
indent -= sw;

}
void PrettyPrinter::visit(const std::shared_ptr<ForStatementNode>& e) {
std::cout << std::string(indent, ' ') << "FOR STATEMENT:";
std::cout << std::endl;
indent += sw;
std::cout << std::string(indent, ' ') << "LOOP VARIABLE: ";
{ int old_indent = indent;
indent = 0;
travel(e -> variable);
indent = old_indent;
}indent += sw;
std::cout << std::string(indent, ' ') << "DIRECTION: ";
std::cout << ((e -> direction) > 0 ? "TO" : "DOWNTO");std::cout << "\n";std::cout << std::string(indent, ' ') << "INITIAL EXPRESSION:";
std::cout << std::endl;
indent += sw;
travel(e -> initial_expression);
indent -= sw;
std::cout << std::string(indent, ' ') << "FINAL EXPRESSION:";
std::cout << std::endl;
indent += sw;
travel(e -> final_expression);
indent -= sw;
indent -= sw;
std::cout << std::string(indent, ' ') << "LOOP BODY:";
std::cout << std::endl;
indent += sw;
travel(e -> body);
indent -= sw;
indent -= sw;

}
