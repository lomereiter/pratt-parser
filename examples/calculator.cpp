#include "../parser/parser_impl.h"

#include <cctype>
#include <iostream>

#include <string>

using namespace grammar;

template <typename T>
class Calculator : public Grammar<T> {
        static T add(T lhs, T rhs) { return lhs + rhs; }
        static T sub(T lhs, T rhs) { return lhs - rhs; }
        static T mul(T lhs, T rhs) { return lhs * rhs; }
        static T div(T lhs, T rhs) { return lhs / rhs; }
        static T neg(T lhs) { return -lhs; }
        static T pos(T lhs) { return lhs; }
        static T fac(T lhs) { 
            T v = 1; 
            for (int i = 1; i <= lhs; ++i)
                v *= i;
            return v;
        }
    public:
        Calculator() : Grammar<T>("(end)") {
            Grammar<T>::add_symbol_to_dict("(number)", 0)\
            .set_scanner(
            [](const std::string& str, size_t pos) -> size_t {
            std::string::size_type i = pos;
                while(i < str.length() && isdigit(str[i]))
                    ++i;
                return i;
            })\
            .set_parser(
            [](const std::string& str, size_t beg, size_t end) -> T {
                T num = 0;
                for (size_t i = beg; i != end; ++i)
                    num *= 10, num += str[i] - '0';
                return num;
            });
            this->infix("+", 10, add); this->infix("-", 10, sub);
            this->infix("*", 20, mul); this->infix("/", 20, div);
            this->prefix("+", 100, pos); this->prefix("-", 100, neg);
            this->postfix("!", 110, fac);
            Grammar<T>::brackets("(",")", std::numeric_limits<int>::max(),[](int x){return x;});
        }
};

int main() {
    Calculator<double> calc;
    std::string str;
    std::cout << "Enter expression:" << std::endl;
    std::getline(std::cin, str);
    std::cout << calc.parse(str) << std::endl;
    return 0;
}
