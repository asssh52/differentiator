#include "../hpp/diff.hpp"

int main(int argc, char* argv[]){

    expr_t expr = {};

    int a = ExprCtor(&expr);
    printf(RED "%d\n" RESET, a);
//     node_t* node1 = 0;
//     node_t* node2 = 0;
//     node_t* node3 = 0;
//     node_t* node4 = 0;
//
//     NewNode(&expr, 1, 50, nullptr, nullptr, &node1);
//     NewNode(&expr, 2, 51, nullptr, nullptr, &node2);
//     NewNode(&expr, 4, 50, nullptr, nullptr, &node3);
//     NewNode(&expr, 6, 50, nullptr, nullptr, &node4);
//
//     NewNode(&expr, 43, 52, node1, node2, &node1);
//     NewNode(&expr, 42, 52, node3, node4, &node2);
//
//     NewNode(&expr, 47, 52, node1, node2, &expr.root);
    LoadExpr(&expr);
    ExprDump(&expr);
    ExprTEX(&expr);

    double b = 0;
    ExprEval(&expr, &b);
    printf(MAG "%lf\n" RESET, b);

    HTMLDumpGenerate(&expr);

    return 0;
}
