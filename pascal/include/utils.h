#ifndef UTILS_H
#define UTILS_H

namespace utils {

    template <typename... Ts> struct type {};

    template <typename T, typename... Tail>
    struct belongs_to { enum { value = 0 }; };

    template <typename T, typename... Tail>
    struct belongs_to<T, T, Tail...> { 
        enum { value = 1 }; 
    };
    
    template <typename T, typename U, typename... Tail>
    struct belongs_to<T, U, Tail...> {
        enum { value = belongs_to<T, Tail...>::value };
    };

    template <typename T, typename... Tail>
    struct belongs_to<T, type<Tail...>> {
        enum { value = belongs_to<T, Tail...>::value };
    };

    template <typename... T> struct cons;
    template <typename H, typename... T> 
    struct cons<H, type<T...>> {
        typedef type<H, T...> list;
    };

    template <typename... T> struct flatten {
        typedef typename flatten<type<T...>>::list list;
    };

    template <> struct flatten<type<>> { typedef type<> list; };

    template <typename... T, typename... U>
    struct flatten<type<type<T...>, U...>> { 
        typedef typename flatten<type<T..., U...>>::list list; 
    };

    template <typename H, typename... T>
    struct flatten<type<H, T...>> {
        typedef typename cons<H, typename flatten<type<T...>>::list>::list list;
    };

    namespace typemap {
        /// Table is type<type<K1, V1>, type<K2, V2>, ...>
        template <typename K, typename Table> struct find; 

        template <typename K> struct find<K, type<>> {};

        template <typename K, typename V, typename... Tail> 
        struct find<K, type<type<K, V>, Tail...>> { 
            typedef V type; 
        };

        template <typename K, typename Head, typename... Tail>
        struct find<K, type<Head, Tail...>> { 
            typedef typename find<K, utils::type<Tail...>>::type type;
        };
    }
}
#endif
