#include "../hpp/diff.hpp"

int main(int argc, char* argv[]){

    expr_t expr = {};
    expr_t* currentExpr = &expr;

    ExprCtor(currentExpr);
    StartTex(currentExpr);

    LoadExpr(currentExpr);

    ExprDump(currentExpr);

    ExprDiff(&currentExpr);


    //ExprDiff(&currentExpr);

    double b = 0;

    EndTex(currentExpr);
    HTMLDumpGenerate(currentExpr);

    return 0;
}
