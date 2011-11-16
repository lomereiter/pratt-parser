#ifndef PARSER_GRAMMAR_IMPL_H
#define PARSER_GRAMMAR_IMPL_H

#include "grammar.h"
#include <utility>

#ifdef DEBUG
#include <iostream>
#endif

namespace grammar {

template <typename T>
Grammar<T>::Grammar(const std::string& end_id) : symbols(end_id) {
    symbols[end_id].set_scanner(
        [](const std::string& s, size_t pos){ return pos; }
    );
}

template <typename T>
Symbol<T>& Grammar<T>::add_symbol_to_dict(const std::string& sym, int lbp) {
    typename SymbolDict<T>::iterator it = symbols.find(sym);
    if (it != symbols.end()) {
        Symbol<T>& s = it -> second;
        /* Notice: one symbol may have both 'nud' and 'led'.
           Storing maximal lbp in dictionary is needed 
           not for parser but for lexer (see Token::iterator).
           Parser uses binding_power stored in 'nud' & 'led'. 
         */
        s.lbp = s.lbp > lbp ? s.lbp : lbp;
        return s;
    } else {
        Symbol<T> s(sym, lbp);
        symbols[sym] = s;
        return symbols[sym];
    }
}      

template <typename T>
Symbol<T>& Grammar<T>::prefix(const std::string& op, int binding_power, 
        std::function<T(T)> selector) {
    Symbol<T>& sym = add_symbol_to_dict(op, binding_power); 
    set_behaviour<Prefix>(sym, binding_power, selector);
    return sym;
}

template <typename T>
Symbol<T>& Grammar<T>::postfix(const std::string& op, int binding_power,
        std::function<T(T)> selector) {
    Symbol<T>& sym = add_symbol_to_dict(op, binding_power);
    set_behaviour<Postfix>(sym, binding_power, selector);
    return sym;
}

template <typename T>
Symbol<T>& Grammar<T>::infix(const std::string& op, int binding_power,
        std::function<T(T, T)> selector) {
    Symbol<T>& sym = add_symbol_to_dict(op, binding_power);
    set_behaviour<LeftAssociative>(sym, binding_power, selector);
    return sym;
}

template <typename T>
Symbol<T>& Grammar<T>::infix_r(const std::string& op, int binding_power,
        std::function<T(T, T)> selector) {
    Symbol<T>& sym = add_symbol_to_dict(op, binding_power);
    set_behaviour<RightAssociative>(sym, binding_power, selector);
    return sym;
}

template <typename T>
Symbol<T>& Grammar<T>::brackets(const std::string& ob, const std::string& cb, 
        int binding_power, std::function<T(T)> selector=nullptr) {
    Symbol<T>& open_sym = add_symbol_to_dict(ob, binding_power);
    /* Symbol<T>& close_sym = */ add_symbol_to_dict(cb, 0);
    if (!selector) selector = [](T val) -> T { return val; };
    open_sym.nud = [cb, selector](PrattParser<T>& p) -> T {
            T val = p.parse(0);
            p.advance(cb);
            return selector(val);
    };
    return open_sym;
}

template <typename T>
T Grammar<T>::parse(const std::string& text) const {
    return PrattParser<T>(text, symbols).parse();
}

template <typename T>
T Grammar<T>::parse(const char* text) const {
    std::string str(text);
    return parse(str);
}

template <typename T>
const SymbolDict<T>& Grammar<T>::get_symbols() const {
    return symbols;
}

/* functions for changing the behaviour of a particular symbol */
/* Notice: this function takes sym.lbp because it can't know about
       binding_powers implicitly stored in led or nud of this symbol.
       Therefore it shouldn't be used with symbols which contain
       different lbps in nud and led. The reason is that it can change 
       the behaviour of the symbol in an undesired way. 
       Although, the function is useful when lbp of a symbol is constant. */
template <typename T> template <typename _Semantics> void 
Grammar<T>::set_behaviour(Symbol<T>& sym, typename _Semantics::handler_type func) {
    set_behaviour_helper(_Semantics(), sym, func);
}

/* this version of 'set_behaviour' allows to specify 'rbp' used by
        PrattParser<T>::parse explicitly, thus avoiding the issue 
        mentioned above */
template <typename T> template <typename _Semantics> void 
Grammar<T>::set_behaviour(Symbol<T>& sym, int rbp, typename _Semantics::handler_type func) {
    set_behaviour_helper(_Semantics(), sym, func, rbp);
}

template <typename T> template <typename _Semantics>
Grammar<T>::behaviour_guard<_Semantics>::behaviour_guard
                (Symbol<T>& sym, typename _Semantics::handler_type func) : 
                    old_func(Grammar<T>::get_handler<typename _Semantics::func_type>(sym)), 
                    sym(sym) {
#ifdef DEBUG
        std::cout << "Behaviour guard construction for " << sym.id << std::endl;
#endif
        set_behaviour<_Semantics>(sym, func);
}

template <typename T> template <typename _Semantics>
Grammar<T>::behaviour_guard<_Semantics>::behaviour_guard
                (Symbol<T>& sym, int rbp, typename _Semantics::handler_type func) : 
                    old_func(Grammar<T>::get_handler<typename _Semantics::func_type>(sym)),
                    sym(sym) {
#ifdef DEBUG
    std::cout << "Behaviour guard construction for " << sym.id << std::endl;
#endif
    set_behaviour<_Semantics>(sym, rbp, func);
}

template <typename T> template <typename _Semantics>
Grammar<T>::behaviour_guard<_Semantics>::~behaviour_guard() {
#ifdef DEBUG
    std::cout << "Behaviour guard destruction for " << sym.id << std::endl;
#endif
    restore_behaviour<typename _Semantics::func_type>(sym, old_func);
}

__DEFINE_GRAMMAR_GUARD__(lbp, int)
__DEFINE_GRAMMAR_GUARD__(nud, typename Prefix::func_type)
__DEFINE_GRAMMAR_GUARD__(led, typename LeftAssociative::func_type)

/************************ helper functions *****************************/

template <typename T> template <typename _Func> _Func&&
Grammar<T>::get_handler(Symbol<T>& sym) {
    return get_handler_helper(type<_Func>(), sym); }

template <typename T> std::function<T(PrattParser<T>&)>&&
Grammar<T>::get_handler_helper
    (type<std::function<T(PrattParser<T>&)>>, Symbol<T>& sym) {
         return std::move(sym.nud); }

template <typename T> std::function<T(PrattParser<T>&, T)>&&
Grammar<T>::get_handler_helper
    (type<std::function<T(PrattParser<T>&, T)>>, Symbol<T>& sym) {
         return std::move(sym.led); }

/***********************************************************************/

template <typename T> template <typename _Func> void 
Grammar<T>::restore_behaviour(Symbol<T>& sym, _Func& f) {
    restore_behaviour_helper(type<_Func>(), sym, f);
}

template <typename T> void 
Grammar<T>::restore_behaviour_helper
    (type<std::function<T(PrattParser<T>&)>>, Symbol<T>& sym, 
          std::function<T(PrattParser<T>&)>& f) {
               sym.nud = std::move(f); }

template <typename T> void 
Grammar<T>::restore_behaviour_helper
    (type<std::function<T(PrattParser<T>&, T)>>, Symbol<T>& sym, 
          std::function<T(PrattParser<T>&, T)>& f) {
               sym.led = std::move(f); }

/**********************************************************************/

template <typename T> void 
Grammar<T>::set_behaviour_helper (typename Grammar<T>::Prefix, 
        Symbol<T>& sym, std::function<T(T)> f) {
        sym.nud = [&sym, f](PrattParser<T>& p) -> T {
            return f(p.parse(sym.lbp)); }; }

template <typename T> void 
Grammar<T>::set_behaviour_helper (typename Grammar<T>::Prefix, 
        Symbol<T>& sym, std::function<T(T)> f, int rbp) {
        sym.nud = [rbp, f](PrattParser<T>& p) -> T {
            return f(p.parse(rbp)); }; }

template <typename T> void 
Grammar<T>::set_behaviour_helper (typename Grammar<T>::Postfix, 
        Symbol<T>& sym, std::function<T(T)> f, int rbp) {
        sym.led = [f](PrattParser<T>& p, T left) -> T {
            return f(left); }; }

template <typename T> void 
Grammar<T>::set_behaviour_helper (typename Grammar<T>::LeftAssociative, 
        Symbol<T>& sym, std::function<T(T, T)> f) {
        sym.led = [&sym, f](PrattParser<T>& p, T left) -> T {
            return f(left, p.parse(sym.lbp)); }; }

template <typename T> void 
Grammar<T>::set_behaviour_helper (typename Grammar<T>::LeftAssociative, 
        Symbol<T>& sym, std::function<T(T, T)> f, int rbp) {
        sym.led = [rbp, f](PrattParser<T>& p, T left) -> T {
            return f(left, p.parse(rbp)); }; }

template <typename T> void
Grammar<T>::set_behaviour_helper (typename Grammar<T>::RightAssociative, 
        Symbol<T>& sym, std::function<T(T, T)> f) {
        sym.led = [&sym, f](PrattParser<T>&p, T left) -> T {
            return f(left, p.parse(sym.lbp - 1)); }; }

template <typename T> void 
Grammar<T>::set_behaviour_helper (typename Grammar<T>::RightAssociative, 
        Symbol<T>& sym, std::function<T(T, T)> f, int rbp) {
        sym.led = [rbp, f](PrattParser<T>&p, T left) -> T {
            return f(left, p.parse(rbp - 1)); }; }

} // namespace

#endif
