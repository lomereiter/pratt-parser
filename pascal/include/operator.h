#ifndef OPERATOR_H
#define OPERATOR_H
enum Operator {
    opAdd, opSub, opMul, opDiv, opPos, opNeg, opNot, 
    opIntDiv, opIntMod, opOr, opAnd, 
    opEq, opNotEq, opLess, opMore, 
    opLessEq, opMoreEq, opIn,
    opXor, opShl, opShr
};

struct operators {
    static const char* operatorName[];
};
#endif
