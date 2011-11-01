#ifndef PRETTY_PRINTER_H
#define PRETTY_PRINTER_H

#include "visitor.h"
#include "expression_fwd.h"

#include <memory>
#include <iosfwd>
#include <functional>

struct PrettyPrinter : public Visitor<std::add_const> {
    PrettyPrinter(int sw=2);

    void visit(const std::shared_ptr<Node>&);
    void visit(const std::shared_ptr<NumberNode>&);
    void visit(const std::shared_ptr<IdentifierNode>&);
    void visit(const std::shared_ptr<ExpressionNode>&);
    void visit(const std::shared_ptr<IdentifierListNode>&);
    void visit(const std::shared_ptr<StringNode>&);
    void visit(const std::shared_ptr<SignNode>&);
    void visit(const std::shared_ptr<ConstantNode>&);
    void visit(const std::shared_ptr<SubrangeTypeNode>&);
    void visit(const std::shared_ptr<EnumeratedTypeNode>&);
    void visit(const std::shared_ptr<PointerTypeNode>&);
    void visit(const std::shared_ptr<VariableDeclNode>&);
    void visit(const std::shared_ptr<VariableDeclListNode>&);
    void visit(const std::shared_ptr<RecordTypeNode>&);
    void visit(const std::shared_ptr<SetTypeNode>&);
    void visit(const std::shared_ptr<FileTypeNode>&);
    void visit(const std::shared_ptr<IndexTypeNode>&);
    void visit(const std::shared_ptr<IndexTypeListNode>&);
    void visit(const std::shared_ptr<ArrayTypeNode>&);
    void visit(const std::shared_ptr<VariableSectionNode>&);
    void visit(const std::shared_ptr<TypeDefinitionNode>&);
    void visit(const std::shared_ptr<TypeSectionNode>&);
    void visit(const std::shared_ptr<PackedTypeNode>&);

private:
    int _indent, sw;
    std::ostream& indent();
    void indented(const std::function<void()>& func);
    void noindent(const std::function<void()>& func);

    bool is_var_section;
};


#endif
