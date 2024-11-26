#include "../hpp/diff.hpp"

int main(int argc, char* argv[]){

    expr_t expr = {};
    ExprCtor(&expr);

    expr_t diff = {};
    ExprCtor(&diff);


    LoadExpr(&expr);
    //NodeDel(&expr, expr.root);

    //ExprDiff(&expr, &diff);
    //ExprDump(&diff);
    //NodeDel(&diff, diff.root->left->left, -1);
    ExprDump(&expr);
    // ExprTEX(&expr);

    double b = 0;
    //ExprEval(&expr, &b);
    printf(MAG "%lf\n" RESET, b);

    HTMLDumpGenerate(&expr);

    return 0;
}
