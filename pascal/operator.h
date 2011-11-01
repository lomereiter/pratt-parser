#ifndef OPERATOR_H
#define OPERATOR_H
enum Operator {
    opAdd, opSub, opMul, opDiv, opPos, opNeg, opNot, 
    opIntDiv, opIntMod, opOr, opAnd, 
    opEq, opNotEq, opLess, opMore, 
    opLessEq, opMoreEq, opIn,
    opXor, opShl, opShr
};
namespace operators {
    static const char *operatorName[] = {
        "ADD", "SUBTRACT", "MULTIPLY", "DIVIDE", "PLUS", "MINUS", "NOT", 
        "INTEGER DIVIDE", "MODULO", "OR", "AND", 
        "EQUAL TO", "NOT EQUAL TO", "LESS THAN", "MORE THAN",
        "LESS OR EQUAL TO", "MORE OR EQUAL TO", "BELONGS TO",
        "EXCLUSIVE OR", "LEFT SHIFT", "RIGHT SHIFT"
    };
}
#endif
