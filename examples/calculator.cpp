#include "../parser/parser.h"

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
                int i = pos;
                while(i < str.length() && isdigit(str[i]))
                    ++i;
                return i;
            })\
            .set_parser(
            [](const std::string& str, size_t beg, size_t end) -> T {
                T num = 0;
                for (int i = beg; i != end; ++i)
                    num *= 10, num += str[i] - '0';
                return num;
            });
            infix("+", 10, add); infix("-", 10, sub);
            infix("*", 20, mul); infix("/", 20, div);
            prefix("+", 100, pos); prefix("-", 100, neg);
            postfix("!", 110, fac);
            Grammar<T>::brackets("(",")", std::numeric_limits<int>::max());
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
