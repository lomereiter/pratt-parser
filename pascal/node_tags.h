#ifndef NODE_TAGS_H
#define NODE_TAGS_H

#include <cstddef>

#ifdef DEBUG
#include "debug.h"
#endif

namespace node_traits {

    struct tag_counter { static size_t value; };

    template <typename T> struct get_tag { static size_t value; };
    template <typename T>
    size_t get_tag_value() {
        size_t& tag = get_tag<const T>::value;
        if (tag == 0) { /* if not yet initialized */
            tag = ++tag_counter::value;
#ifdef DEBUG
            std::cout << tag << ": " << typeid(T).name() << std::endl;
#endif
        }
        return tag;
    }

    template <typename T> size_t node_traits::get_tag<T>::value = get_tag_value<T>();
}

#endif
