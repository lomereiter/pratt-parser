#ifndef PARSER_GRAMMAR_H
#define PARSER_GRAMMAR_H

#include "forward.h"

#include <string>
#include <functional>

namespace grammar {

    template <typename T>
    class Grammar {
        public:

            Grammar(const std::string& end_id);

            Symbol<T>& add_symbol_to_dict(const std::string& sym, int lbp=0);
            Symbol<T>& prefix(const std::string&, int, std::function<T(T)>);
            Symbol<T>& postfix(const std::string&, int, std::function<T(T)>);
            Symbol<T>& infix(const std::string&, int, std::function<T(T, T)>);
            Symbol<T>& infix_r(const std::string&, int, std::function<T(T, T)>);
            Symbol<T>& brackets(const std::string&, const std::string&, 
                    int, std::function<T(T)>);
           
            T parse(const std::string& text) const;
            T parse(const char* text) const;
            
            struct Prefix {  typedef std::function<T(T)> handler_type; };
            struct Postfix { typedef std::function<T(T)> handler_type; };
            struct LeftAssociative {  typedef std::function<T(T,T)> handler_type; };
            struct RightAssociative { typedef std::function<T(T,T)> handler_type; };

            template <typename U> struct type {};

            template <typename _Semantics> static void 
            set_behaviour(Symbol<T>& sym, typename _Semantics::handler_type func);

            template <typename _Semantics> static void 
            set_behaviour(Symbol<T>& sym, int rbp, typename _Semantics::handler_type func);

            template <typename _Semantics>
            struct behaviour_guard {
                behaviour_guard(Symbol<T>& sym, typename _Semantics::handler_type func);
                behaviour_guard(Symbol<T>& sym, int rbp, typename _Semantics::handler_type func);
                ~behaviour_guard();
            private:
                typename _Semantics::handler_type old_func;        
                Symbol<T>& sym;
            };

            struct lbp_guard {
                lbp_guard(Symbol<T>&, int);
                ~lbp_guard();
            private:
                Symbol<T>& sym;
                int old_lbp;
            };

        private:
            static void set_behaviour_helper
                (type<Prefix>, Symbol<T>& sym, std::function<T(T)> f);

            static void set_behaviour_helper
                (type<Prefix>, Symbol<T>& sym, std::function<T(T)> f, int rbp);

            static void set_behaviour_helper
                (type<Postfix>, Symbol<T>& sym, std::function<T(T)> f, int rbp=0);

            static void set_behaviour_helper
                (type<LeftAssociative>, Symbol<T>& sym, std::function<T(T, T)> f);

            static void set_behaviour_helper
                (type<LeftAssociative>, Symbol<T>& sym, std::function<T(T, T)> f, int rbp);

            static void set_behaviour_helper
                (type<RightAssociative>, Symbol<T>& sym, std::function<T(T, T)> f);

            static void set_behaviour_helper
                (type<RightAssociative>, Symbol<T>& sym, std::function<T(T, T)> f, int rbp);
            
            SymbolDict<T> symbols;
    };

} // namespace

#endif
