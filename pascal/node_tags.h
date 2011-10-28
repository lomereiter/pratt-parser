#ifndef NODE_TAGS_H
#define NODE_TAGS_H

#ifdef DEBUG
#include <iostream>
#include <typeinfo>
#endif

namespace node_traits {

    struct tag_counter { static int value; };
    int tag_counter::value = 0;

    template <typename T> struct get_tag { static int value; };
    template <typename T>
    int get_tag_value() {
        int& tag = get_tag<const T>::value;
        if (tag == 0) { /* if not yet initialized */
            tag = ++tag_counter::value;
#ifdef DEBUG
            std::cout << tag << ": " << typeid(T).name() << std::endl;
#endif
        }
        return tag;
    }

/* getting unique tags for all node classes */
    template <typename T> int get_tag<T>::value = get_tag_value<T>();
}

#endif
