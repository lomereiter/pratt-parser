#include "visitor.h"
#include "expression.h"
#include "pretty_printer.h"

#include <memory>
#include <iostream>
#include <functional>

/* Currently, in order to add new type of node, the following is needed:
   1) add new function prototype in the header;
   2) add its definition in this file;
   3) add the type name in the template argument of 'Visits'
        in the constructor.
*/

PrettyPrinter::PrettyPrinter(int sw) : _indent(0), sw(sw),
                                       is_var_section(false)
{
    Visits<PrettyPrinter,
           NumberNode, IdentifierNode, ExpressionNode, IdentifierListNode,
           StringNode, SignNode, ConstantNode, SubrangeTypeNode,
           EnumeratedTypeNode, FileTypeNode, SetTypeNode, ArrayTypeNode,
           PointerTypeNode, VariableDeclNode, VariableDeclListNode,
           RecordTypeNode, IndexTypeNode, IndexTypeListNode,
           VariableSectionNode, TypeDefinitionNode, TypeSectionNode,
           PackedTypeNode>(*this);
}

void PrettyPrinter::PrettyPrinter::visit(std::shared_ptr<Node> e) {
    indent() << "IMPLEMENT ME!" << '\n';
}

void PrettyPrinter::visit(std::shared_ptr<NumberNode> e) {
    indent() << e -> value << '\n';
}

void PrettyPrinter::visit(std::shared_ptr<IdentifierNode> e) {
    indent() << "IDENTIFIER " << e -> name << '\n';
}

void PrettyPrinter::visit(std::shared_ptr<ExpressionNode> e) {
    indent() << operators::operatorName[e -> op()] << '\n';
    indented([=]() -> void {
        for (const auto& node : e -> args)
            travel(node);
    });
}

void PrettyPrinter::visit(std::shared_ptr<IdentifierListNode> e) {
    indent() << "IDENTIFIER LIST:" << '\n';
    indented([=]() -> void {
        for (const auto& node : e -> list())
            travel(node);
    });
}

void PrettyPrinter::visit(std::shared_ptr<StringNode> e) {
    indent() << "STRING LITERAL: " << e -> str << '\n';
}

void PrettyPrinter::visit(std::shared_ptr<SignNode> e) {
    indent() << (e -> sign() == '+' ? "PLUS SIGN" : "MINUS SIGN") << '\n';
    indented([=](){ travel(e -> child); });
}

void PrettyPrinter::visit(std::shared_ptr<ConstantNode> e) {
    indent() << "CONSTANT " << '\n';
    indented([=](){ travel(e -> child);});
}

void PrettyPrinter::visit(std::shared_ptr<SubrangeTypeNode> e) {
    indent() << "SUBRANGE TYPE:" << '\n';
    indented([=]() -> void {
            indent() << "LOWER BOUND:" << '\n';
            indented([=](){ travel(e -> lower_bound); });
            indent() << "UPPER BOUND:" << '\n';
            indented([=](){ travel(e -> upper_bound); });
    });
}

void PrettyPrinter::visit(std::shared_ptr<EnumeratedTypeNode> e) {
    indent() << "ENUMERATED TYPE: " << '\n';
    indented([=]() { travel(e -> identifiers); });
}

void PrettyPrinter::visit(std::shared_ptr<PointerTypeNode> e) {
    indent() << "POINTER TYPE: ";
    noindent([=]() { travel(e -> type); });
}

void PrettyPrinter::visit(std::shared_ptr<VariableDeclNode> e) {
    indent() << "VARIABLE DECLARATION: " << '\n';
    indented([=]() {
            travel(e -> id_list);
            indent() << "OF TYPE " << '\n';
            indented([=]() { travel(e -> type); });
    });
}

void PrettyPrinter::visit(std::shared_ptr<VariableDeclListNode> e) {
    for (const auto& node : e -> list())
        travel(node);
}

void PrettyPrinter::visit(std::shared_ptr<RecordTypeNode> e) {
    indent() << "RECORD TYPE" << '\n';
    indented([=]() { travel(e -> child); });
}

void PrettyPrinter::visit(std::shared_ptr<SetTypeNode> e) {
    indent() << "SET OF" << '\n';
    indented([=]() { travel(e -> type); });
}

void PrettyPrinter::visit(std::shared_ptr<FileTypeNode> e) {
    indent() << "FILE OF" << '\n';
    indented([=]() { travel(e -> type); });
}

void PrettyPrinter::visit(std::shared_ptr<IndexTypeNode> e) { travel(e -> type); }
void PrettyPrinter::visit(std::shared_ptr<IndexTypeListNode> e) {
    for (const auto& node : e -> list())
        travel(node);
}

void PrettyPrinter::visit(std::shared_ptr<ArrayTypeNode> e) {
    indent() << "ARRAY WITH BOUNDS" << '\n';
    indented([=]() { travel(e -> index_type_list); });
    indent() << "OF TYPE" << '\n';
    indented([=]() { travel(e -> type); });
}

void PrettyPrinter::visit(std::shared_ptr<VariableSectionNode> e) {
    indent() << "VARIABLE DECLARATION SECTION" << '\n';
    indented([=]() { travel(e -> declarations); });
};

void PrettyPrinter::visit(std::shared_ptr<TypeDefinitionNode> e) {
    indent() << "TYPE DEFINITION: ";
    noindent([=]() { travel(e -> name); });
    indented([=]() { travel(e -> type); });
}

void PrettyPrinter::visit(std::shared_ptr<TypeSectionNode> e) {
    indent() << "TYPE DEFINITION SECTION" << '\n';
    indented([=]() {
        for (const auto& node : e -> list())
            travel(node); });
}

void PrettyPrinter::visit(std::shared_ptr<PackedTypeNode> e) {
    indent() << "PACKED TYPE:" << '\n';
    indented([=]() { travel(e -> type); });
}

std::ostream& PrettyPrinter::indent() {
    return std::cout << std::string(_indent, ' ');
}

void PrettyPrinter::indented(std::function<void()> func) {
    _indent += sw; func(); _indent -= sw;
}

void PrettyPrinter::noindent(std::function<void()> func) {
    int old_indent = _indent;
    _indent = 0;
    func();
    _indent = old_indent;
}
