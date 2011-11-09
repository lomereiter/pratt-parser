#ifndef VISITOR_H
#define VISITOR_H

#include <vector>
#include <memory>
#include <type_traits>

#include "node.h"
#include "syntax_error.h"
#include "node_tags.h"

namespace detail {
    /* VTable is created for each VisitorImpl class */
    template <typename Func>
    struct VTable {
        std::vector<Func> table;

        template <typename Visitable>
        void add(Func f) {
            size_t index = node_traits::get_tag_value<Visitable>();
            
            if (index >= table.size()) {
                size_t root_tag = node_traits::get_tag<Node>::value;
                Func default_func = root_tag >= table.size() ? 
                                                           0 :
                                              table[root_tag];
                table.resize(index + 1, default_func);
                // the default function is that taking Node 
                //  (which is the root of the hierarchy) as a parameter
            }
            table[index] = f;
        }

        Func operator[](size_t index) const {
            if (index >= table.size())
                index = node_traits::get_tag<Node>::value;
            return table[index];
        }
    };

    template <class Func> void apply(Func&) {}
    template <class Func, class _Head, class... _Tail>
    void apply(Func& f) { 
        f.template operator()<_Head>(); 
        apply<Func, _Tail...>(f);
    }

    template <class Visitor, class... VisitedList>
    struct CreateVTable {
        typename Visitor::VTableType vtable;

        template <typename Visitable> 
        void operator()() {
            vtable.template add<Visitable>(&Visitor::template thunk<Visitor, Visitable>);
        }

        CreateVTable() {
            this->template operator()<Node>();
            detail::apply<decltype(*this), VisitedList...>(*this);
        }
    };

    template <class Visitor, class... VisitedList>
    struct GetStaticVTable {
        operator const typename Visitor::VTableType* () const {
            return &(table.vtable);
        }
    private:
        static CreateVTable< Visitor, VisitedList... > table;
    };

    template <class Visitor, class... VisitedList>
    CreateVTable<Visitor, VisitedList...> GetStaticVTable<Visitor, VisitedList...>::table;
}

#ifdef DEBUG
#include <iostream>
#include <typeinfo>
#endif

template <typename T> struct non_const { typedef T type; };

/* by default, Visitor doesn't change nodes it visits */
template <template <typename T> class _TypeQualifier = std::add_const>
class Visitor {
    typedef typename _TypeQualifier<std::shared_ptr<Node>>::type& PNodeRef;
public:
    template <class VisitorType, class... VisitedList>
    void Visits() {
        vtable = detail::GetStaticVTable< VisitorType, VisitedList... >();
    }

    template <typename VisitorImpl, typename Visitable>
    void thunk(PNodeRef node) {
        VisitorImpl& visitor = static_cast<VisitorImpl&>(*this);
        typename _TypeQualifier<std::shared_ptr<Visitable>>::type visitable = 
                                std::static_pointer_cast<Visitable>(node);
#ifdef DEBUG
        std::cout << "Visitor class: " << typeid(VisitorImpl).name() << 
            "; Visitable class: " << typeid(Visitable).name() << std::endl;
#endif
        /* Visitor shall have 'visit' methods */
        visitor.visit(visitable);
    }

    typedef void (Visitor::*Thunk) (PNodeRef);
    typedef detail::VTable<Thunk> VTableType;

    void travel(PNodeRef node) {
        Thunk th = (*vtable)[node -> tag()]; // tag() shall be virtual
#ifdef DEBUG
        std::cout << "Travel to node with tag " << node -> tag() << std::endl;
#endif
        (this->*th)(node);
    }

    void visit(PNodeRef node) {}

    const VTableType* vtable;
};

namespace detail {
    template <class _DefaultBehaviour>
    struct AstVisitor : public Visitor<std::add_const> {
        /* constant Visitor */
        AstVisitor() : func() {}
        void visit(const std::shared_ptr<Node>&) { func(); }

        private:
            _DefaultBehaviour func;
    };

    struct ThrowPolicy {
        void operator()() const { throw SyntaxError("unexpected symbol"); }
    };

    struct IgnorePolicy {
        void operator()() const {}
    };
}

typedef detail::AstVisitor<detail::ThrowPolicy> AstThrowVisitor;
typedef detail::AstVisitor<detail::IgnorePolicy> AstIgnoreVisitor;

#endif
