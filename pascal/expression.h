#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <memory>
#include <forward_list>
#include <string>

#include "operator.h"
#include "node_tags.h"

struct Node : public std::enable_shared_from_this<Node> {
    /* is default constructible */
    virtual ~Node() {}
    virtual size_t tag() {
        return node_traits::get_tag<Node>::value;
    }
};

template <class NodeType>
struct VisitableNode : public Node {
    virtual size_t tag() {
        return node_traits::get_tag<NodeType>::value;
    }
};

template <typename T>
struct ListOf : public VisitableNode<ListOf<T>> {
    typedef std::forward_list<std::shared_ptr<Node>> ListT;
    ListOf(ListT&& lst) : lst(lst) {}
    ListOf(std::shared_ptr<Node> node) {
        lst.push_front(node);
    }
    ListT& list() { return lst; }
private:
    ListT lst;
};

struct ExpressionNode : public VisitableNode<ExpressionNode> {
    std::forward_list<std::shared_ptr<Node>> args;

    ExpressionNode(int arity, Operator op) : _arity(arity), _op(op) {}

    int arity() { return _arity; }
    int op() { return _op; }

private:
    int _arity;
    Operator _op;
};

struct SignNode : public VisitableNode<SignNode> {
    std::shared_ptr<Node> child;
    SignNode(char sign, std::shared_ptr<Node> child) : child(child), _sign(sign) {}
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

typedef ListOf<IdentifierNode> IdentifierListNode;

typedef IdentifierNode FieldIdentifierNode;
typedef IdentifierNode ConstantIdentifierNode;
typedef IdentifierNode TypeIdentifierNode;
typedef IdentifierNode VariableIdentifierNode;
typedef IdentifierNode ProcedureIdentifierNode;
typedef IdentifierNode FunctionIdentifierNode;
typedef IdentifierNode BoundIdentifierNode;

struct StringNode : public VisitableNode<StringNode> { 
    std::string str;
    StringNode(std::string s) : str(s) {}
};

struct ConstantNode : public VisitableNode<ConstantNode> {
    std::shared_ptr<Node> child;
    ConstantNode(std::shared_ptr<Node> node) : child(node) {}
};

struct SubrangeTypeNode : public VisitableNode<SubrangeTypeNode> {
    std::shared_ptr<ConstantNode> lower_bound;
    std::shared_ptr<ConstantNode> upper_bound;

    SubrangeTypeNode(std::shared_ptr<ConstantNode> lb, 
                     std::shared_ptr<ConstantNode> ub) : 
        lower_bound(lb), upper_bound(ub) {}
};

struct EnumeratedTypeNode : public VisitableNode<EnumeratedTypeNode> {
    std::shared_ptr<IdentifierListNode> identifiers;
    EnumeratedTypeNode(std::shared_ptr<IdentifierListNode> id_list) : identifiers(id_list) {}
};

struct VariableDeclNode : public VisitableNode<VariableDeclNode> {
    std::shared_ptr<Node> id_list;
    std::shared_ptr<Node> type;

    VariableDeclNode(std::shared_ptr<Node> id_list, 
                     std::shared_ptr<Node> type) :
        id_list(id_list), type(type) {}
};

typedef ListOf<VariableDeclNode> VariableDeclListNode;
typedef VariableDeclListNode RecordFixedPartNode;
typedef RecordFixedPartNode RecordFieldListNode;

/* May have child == nullptr */
struct RecordTypeNode : public VisitableNode<RecordTypeNode> {
    std::shared_ptr<Node> child;
    RecordTypeNode(std::shared_ptr<Node> node) : child(node) {}
};

struct SetTypeNode : public VisitableNode<SetTypeNode> {
    std::shared_ptr<Node> type;
    SetTypeNode(std::shared_ptr<Node> type) : type(type) {}
};

struct FileTypeNode : public VisitableNode<FileTypeNode> {
    std::shared_ptr<Node> type;
    FileTypeNode(std::shared_ptr<Node> type) : type(type) {}
};

struct PointerTypeNode : public VisitableNode<PointerTypeNode> {
    std::shared_ptr<Node> type;
    PointerTypeNode(std::shared_ptr<Node> type) : type(type) {}
};

struct IndexTypeNode : public VisitableNode<IndexTypeNode> {
    std::shared_ptr<Node> type;
    IndexTypeNode(std::shared_ptr<Node> node) : type(node) {}
};

typedef ListOf<IndexTypeNode> IndexTypeListNode;

struct ArrayTypeNode : public VisitableNode<ArrayTypeNode> {
    std::shared_ptr<Node> index_type_list;
    std::shared_ptr<Node> type;
    ArrayTypeNode(std::shared_ptr<Node> list, std::shared_ptr<Node> type) :
            index_type_list(list), type(type) {}
};

#endif
