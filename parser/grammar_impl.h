#ifndef PARSER_GRAMMAR_IMPL_H
#define PARSER_GRAMMAR_IMPL_H

#include "forward.h"

#include <string>
#include <functional>

#ifdef DEBUG
#include <iostream>
#endif

namespace grammar {

    template <typename T>
    class Grammar {
        public:

            Grammar(const std::string& end_id) : symbols(end_id) {
                symbols[end_id].set_scanner(
                    [](const std::string& s, size_t pos){ return pos; }
                );
            }

            Symbol<T>& add_symbol_to_dict(const std::string& sym, int lbp=0) {
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

            Symbol<T>& prefix(const std::string& op, int binding_power, 
                    std::function<T(T)> selector) {
                Symbol<T>& sym = add_symbol_to_dict(op, binding_power); 
                set_behaviour<Prefix>(sym, binding_power, selector);
                return sym;
            }

            Symbol<T>& postfix(const std::string& op, int binding_power,
                    std::function<T(T)> selector) {
                Symbol<T>& sym = add_symbol_to_dict(op, binding_power);
                set_behaviour<Postfix>(sym, binding_power, selector);
                return sym;
            }

            Symbol<T>& infix(const std::string& op, int binding_power,
                    std::function<T(T, T)> selector) {
                Symbol<T>& sym = add_symbol_to_dict(op, binding_power);
                set_behaviour<LeftAssociative>(sym, binding_power, selector);
                return sym;
            }

            Symbol<T>& infix_r(const std::string& op, int binding_power,
                    std::function<T(T, T)> selector) {
                Symbol<T>& sym = add_symbol_to_dict(op, binding_power);
                set_behaviour<RightAssociative>(sym, binding_power, selector);
                return sym;
            }

            Symbol<T>& brackets(const std::string& ob, const std::string& cb, 
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
            
            T parse(const std::string& text) const {
                return PrattParser<T>(text, symbols).parse();
            }

            T parse(const char* text) const {
                std::string str(text);
                return parse(str);
            }
            
            /* functions for changing the behaviour of a particular symbol */

            struct Prefix { 
                typedef std::function<T(T)> handler_type;
                typedef std::function<T(PrattParser<T>&)> func_type;
            };
            struct Postfix { 
                typedef std::function<T(T)> handler_type;
                typedef std::function<T(PrattParser<T>&, T)> func_type;
            };
            struct LeftAssociative {  
                typedef std::function<T(T,T)> handler_type;
                typedef std::function<T(PrattParser<T>&, T)> func_type;
            };
            struct RightAssociative { 
                typedef std::function<T(T,T)> handler_type; 
                typedef std::function<T(PrattParser<T>&, T)> func_type;
            };

            template <typename U> struct type {};

            /* Notice: this function takes sym.lbp because it can't know about
                   binding_powers implicitly stored in led or nud of this symbol.
                   Therefore it shouldn't be used with symbols which contain
                   different lbps in nud and led. The reason is that it can change 
                   the behaviour of the symbol in an undesired way. 
                   Although, the function is useful when lbp of a symbol is constant. */
            template <typename _Semantics> static void 
            set_behaviour(Symbol<T>& sym, typename _Semantics::handler_type func) {
                set_behaviour_helper(type<_Semantics>(), sym, func);
            }

            /* this version of 'set_behaviour' allows to specify 'rbp' used by
                    PrattParser<T>::parse explicitly, thus avoiding the issue 
                    mentioned above */
            template <typename _Semantics> static void 
            set_behaviour(Symbol<T>& sym, int rbp, typename _Semantics::handler_type func) {
                set_behaviour_helper(type<_Semantics>(), sym, func, rbp);
            }

            template <typename _Semantics>
            struct behaviour_guard {
                behaviour_guard(Symbol<T>& sym, typename _Semantics::handler_type func) : 
                                old_func(get_handler<typename _Semantics::func_type>(sym)), 
                                    sym(sym) {
#ifdef DEBUG
                    std::cout << "Behaviour guard construction for " << sym.id << std::endl;
#endif
                    set_behaviour<_Semantics>(sym, func);
                }

                behaviour_guard(Symbol<T>& sym, int rbp, typename _Semantics::handler_type func) : 
                                old_func(get_handler<typename _Semantics::func_type>(sym)), 
                                                   sym(sym) {
#ifdef DEBUG
                    std::cout << "Behaviour guard construction for " << sym.id << std::endl;
#endif
                    set_behaviour<_Semantics>(sym, rbp, func);
                }

                ~behaviour_guard() {
#ifdef DEBUG
                    std::cout << "Behaviour guard destruction for " << sym.id << std::endl;
#endif
                    restore_behaviour<typename _Semantics::func_type>(sym, old_func);
                }
            private:
                typename _Semantics::func_type old_func;        
                Symbol<T>& sym;
            };

            struct lbp_guard {
                lbp_guard(Symbol<T>& sym, int lbp) : sym(sym), old_lbp(sym.lbp) {
#ifdef DEBUG
                    std::cout << "Lbp guard construction... lbp = " << lbp << std::endl;
#endif
                    sym.lbp = lbp;
                }
                ~lbp_guard() { 
#ifdef DEBUG
                    std::cout << "Lbp guard destruction... old_lbp = " << old_lbp << std::endl;
#endif
                    sym.lbp = old_lbp; 
                }
            private:
                Symbol<T>& sym;
                int old_lbp;
            };

        private:
            /************************ helper functions *****************************/

            template <typename _Func>
            static const _Func get_handler(const Symbol<T>& sym) {
                return get_handler_helper(type<_Func>(), sym); }

            static const std::function<T(PrattParser<T>&)> get_handler_helper
                (type<std::function<T(PrattParser<T>&)>>, const Symbol<T>& sym) {
                     return sym.nud; }

            static const std::function<T(PrattParser<T>&, T)> get_handler_helper
                (type<std::function<T(PrattParser<T>&, T)>>, const Symbol<T>& sym) {
                     return sym.led; }

            /***********************************************************************/

            template <typename _Func>
            static void restore_behaviour(Symbol<T>& sym, _Func& f) {
                restore_behaviour_helper(type<_Func>(), sym, f);
            }

            static void restore_behaviour_helper
                (type<std::function<T(PrattParser<T>&)>>, Symbol<T>& sym, 
                      std::function<T(PrattParser<T>&)>& f) {
                           sym.nud = f; }

            static void restore_behaviour_helper
                (type<std::function<T(PrattParser<T>&, T)>>, Symbol<T>& sym, 
                      std::function<T(PrattParser<T>&, T)>& f) {
                           sym.led = f; }

            /**********************************************************************/

            static void set_behaviour_helper
                (type<Prefix>, Symbol<T>& sym, std::function<T(T)> f) {
                    sym.nud = [&sym, f](PrattParser<T>& p) -> T {
                        return f(p.parse(sym.lbp)); }; }

            static void set_behaviour_helper
                (type<Prefix>, Symbol<T>& sym, std::function<T(T)> f, int rbp) {
                    sym.nud = [rbp, f](PrattParser<T>& p) -> T {
                        return f(p.parse(rbp)); }; }

            static void set_behaviour_helper
                (type<Postfix>, Symbol<T>& sym, std::function<T(T)> f, int rbp=0) {
                    sym.led = [f](PrattParser<T>& p, T left) -> T {
                        return f(left); }; }

            static void set_behaviour_helper
                (type<LeftAssociative>, Symbol<T>& sym, std::function<T(T, T)> f) {
                    sym.led = [&sym, f](PrattParser<T>& p, T left) -> T {
                        return f(left, p.parse(sym.lbp)); }; }

            static void set_behaviour_helper
                (type<LeftAssociative>, Symbol<T>& sym, std::function<T(T, T)> f, int rbp) {
                    sym.led = [rbp, f](PrattParser<T>& p, T left) -> T {
                        return f(left, p.parse(rbp)); }; }

            static void set_behaviour_helper
                (type<RightAssociative>, Symbol<T>& sym, std::function<T(T, T)> f) {
                    sym.led = [&sym, f](PrattParser<T>&p, T left) -> T {
                        return f(left, p.parse(sym.lbp - 1)); }; }

            static void set_behaviour_helper
                (type<RightAssociative>, Symbol<T>& sym, std::function<T(T, T)> f, int rbp) {
                    sym.led = [rbp, f](PrattParser<T>&p, T left) -> T {
                        return f(left, p.parse(rbp - 1)); }; }
            
            SymbolDict<T> symbols;
    };

} // namespace

#endif
