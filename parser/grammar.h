#ifndef PARSER_GRAMMAR_H
#define PARSER_GRAMMAR_H

#include "forward.h"

#include <string>
#include <functional>

namespace grammar {

    struct keep_symbol_lbp_t {};
    constexpr keep_symbol_lbp_t keep_symbol_lbp {};

    template <typename T>
    class Grammar {
        public:

            Grammar(const std::string& end_id);

            Symbol<T>& add_symbol_to_dict(const std::string& sym, int lbp=0);
            Symbol<T>& prefix(const std::string&, int, std::function<T(T)>);
            Symbol<T>& prefix(const std::string&, int, std::function<T(T)>, keep_symbol_lbp_t);
            Symbol<T>& postfix(const std::string&, int, std::function<T(T)>);
            Symbol<T>& infix(const std::string&, int, std::function<T(T, T)>);
            Symbol<T>& infix_r(const std::string&, int, std::function<T(T, T)>);
            Symbol<T>& brackets(const std::string&, const std::string&, 
                    int, std::function<T(T)>);
           
            T parse(const std::string& text) const;
            T parse(const char* text) const;
           
            const SymbolDict<T>& get_symbols() const;

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
                typename _Semantics::func_type old_func;        
                Symbol<T>& sym;
            };

#define __DECLARE_GRAMMAR_GUARD__(__field_name, __field_type) \
            struct __field_name##_guard { \
                __field_name##_guard(Symbol<T>&, __field_type); \
                ~__field_name##_guard(); \
            private: \
                Symbol<T>& sym; \
                __field_type old_##__field_name; \
            }

#define __DEFINE_GRAMMAR_GUARD__(__field_name, __field_type) \
template <typename T> \
Grammar<T>::__field_name##_guard::__field_name##_guard(Symbol<T>& s, __field_type f) : \
            sym(s), old_##__field_name(std::move(s.__field_name)) { \
    sym.__field_name = std::move(f); \
} \
\
template <typename T> \
Grammar<T>::__field_name##_guard::~__field_name##_guard() { \
    sym.__field_name = std::move(old_##__field_name); \
}

            __DECLARE_GRAMMAR_GUARD__(lbp, int);
            __DECLARE_GRAMMAR_GUARD__(nud, typename Prefix::func_type);
            __DECLARE_GRAMMAR_GUARD__(led, typename LeftAssociative::func_type);

        private:
            static void set_behaviour_helper
                (Prefix, Symbol<T>& sym, std::function<T(T)> f);
            static void set_behaviour_helper
                (Prefix, Symbol<T>& sym, std::function<T(T)> f, int rbp);
            static void set_behaviour_helper
                (Postfix, Symbol<T>& sym, std::function<T(T)> f, int rbp=0);
            static void set_behaviour_helper
                (LeftAssociative, Symbol<T>& sym, std::function<T(T, T)> f);
            static void set_behaviour_helper
                (LeftAssociative, Symbol<T>& sym, std::function<T(T, T)> f, int rbp);
            static void set_behaviour_helper
                (RightAssociative, Symbol<T>& sym, std::function<T(T, T)> f);
            static void set_behaviour_helper
                (RightAssociative, Symbol<T>& sym, std::function<T(T, T)> f, int rbp);

            template <typename _Func>
            static _Func&& get_handler(Symbol<T>& sym);

            static std::function<T(PrattParser<T>&)>&& get_handler_helper
                (type<std::function<T(PrattParser<T>&)>>, Symbol<T>& sym);

            static std::function<T(PrattParser<T>&, T)>&& get_handler_helper
                (type<std::function<T(PrattParser<T>&, T)>>, Symbol<T>& sym);

            template <typename _Func>
            static void restore_behaviour(Symbol<T>& sym, _Func& f);

            static void restore_behaviour_helper
                (type<std::function<T(PrattParser<T>&)>>, Symbol<T>& sym, 
                      std::function<T(PrattParser<T>&)>& f);

            static void restore_behaviour_helper
                (type<std::function<T(PrattParser<T>&, T)>>, Symbol<T>& sym, 
                      std::function<T(PrattParser<T>&, T)>& f);

            SymbolDict<T> symbols;
    };

} // namespace

#endif
