#ifndef VISITOR_H
#define VISITOR_H

#include <vector>
#include <memory>

#include "expression.h"
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
            
            if (index >= table.size()) { // not yet in table. let's fix it.
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

class Visitor {
public:
    template <typename VisitorImpl, typename Visitable>
    void thunk(std::shared_ptr<Node> node) {
        VisitorImpl& visitor = static_cast<VisitorImpl&>(*this);
        std::shared_ptr<Visitable> visitable = std::static_pointer_cast<Visitable>(node);
#ifdef DEBUG
        std::cout << "Visitor class: " << typeid(VisitorImpl).name() << 
            "; Visitable class: " << typeid(Visitable).name() << std::endl;
#endif
        /* Visitor must have 'visit' methods */
        visitor.visit(visitable);
    }

    typedef void (Visitor::*Thunk) (std::shared_ptr<Node>);
    typedef detail::VTable<Thunk> VTableType;

    void travel(std::shared_ptr<Node> node) {
        Thunk th = (*vtable)[node -> tag()];
        (this->*th)(node);
    }

    void visit(std::shared_ptr<Node> node) {}

    const VTableType* vtable;
};

template <class Visitor, class... VisitedList>
void Visits(Visitor& visitor) {
    visitor.vtable = detail::GetStaticVTable< Visitor, VisitedList... >();
}

namespace detail {
    template <class _Policy>
    struct AstVisitor : public Visitor {

        AstVisitor() : func() {}

        void visit(std::shared_ptr<Node>) { func(); }

        virtual std::shared_ptr<Node> get_expression() { throw 0xBADF00D; }

        private:
            _Policy func;
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
