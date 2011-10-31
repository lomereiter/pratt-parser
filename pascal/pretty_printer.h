#ifndef PRETTY_PRINTER_H
#define PRETTY_PRINTER_H

#include "visitor.h"
#include "expression_fwd.h"

#include <memory>
#include <iosfwd>
#include <functional>

struct PrettyPrinter : public Visitor {
    PrettyPrinter(int sw=2);

    void visit(std::shared_ptr<Node>);
    void visit(std::shared_ptr<NumberNode>);
    void visit(std::shared_ptr<IdentifierNode>);
    void visit(std::shared_ptr<ExpressionNode>);
    void visit(std::shared_ptr<IdentifierListNode>);
    void visit(std::shared_ptr<StringNode>);
    void visit(std::shared_ptr<SignNode>);
    void visit(std::shared_ptr<ConstantNode>);
    void visit(std::shared_ptr<SubrangeTypeNode>);
    void visit(std::shared_ptr<EnumeratedTypeNode>);
    void visit(std::shared_ptr<PointerTypeNode>);
    void visit(std::shared_ptr<VariableDeclNode>);
    void visit(std::shared_ptr<VariableDeclListNode>);
    void visit(std::shared_ptr<RecordTypeNode>);
    void visit(std::shared_ptr<SetTypeNode>);
    void visit(std::shared_ptr<FileTypeNode>);
    void visit(std::shared_ptr<IndexTypeNode>);
    void visit(std::shared_ptr<IndexTypeListNode>);
    void visit(std::shared_ptr<ArrayTypeNode>);
    void visit(std::shared_ptr<VariableSectionNode>);
    void visit(std::shared_ptr<TypeDefinitionNode>);
    void visit(std::shared_ptr<TypeSectionNode>);
    void visit(std::shared_ptr<PackedTypeNode>);

private:
    int _indent, sw;
    std::ostream& indent();
    void indented(std::function<void()> func);
    void noindent(std::function<void()> func);

    bool is_var_section;
};


#endif
