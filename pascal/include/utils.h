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

    namespace detail {
        template <typename T, typename List> struct append_one;
        template <typename T, typename... Tail>
        struct append_one <T, type<Tail...>>  {
            typedef type<T, Tail...> list;
        };

        template <typename T> struct head;
        template <typename T, typename... Tail>
        struct head<type<T, Tail...>> {
            typedef T type;
        };
        
        template <typename T> struct tail;
        template <typename T, typename... Tail>
        struct tail<type<T, Tail...>> {
            typedef type<Tail...> list;
        };
    }

    template <typename T, typename... Tail>
    struct append {
        typedef typename detail::append_one< 
                             typename detail::head<T>::type,
                             typename append< typename detail::tail<T>::list,
                                                       Tail...
                                            >::list
                                           >::list list;
    };

    template <typename... Tail>
    struct append <type<>, Tail...> {
        typedef type<Tail...> list;
    };
   
    template <typename T, typename... Tail> 
    struct append <T, type<Tail...>> {
        typedef typename append <T, Tail...>::list list;
    };

    template <typename... Tail>
    struct append <type<>, type<Tail...>> {
        typedef type<Tail...> list;
    };
}

#endif
