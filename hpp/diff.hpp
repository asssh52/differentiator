#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "colors.hpp"

typedef struct files_t{

    char* logName;
    char* htmlName;
    char* dotName;
    char* saveName;

    FILE* log;
    FILE* html;
    FILE* dot;
    FILE* save;

} files_t;

typedef struct node_t{

    node_t*     parent;
    node_t*     left;
    node_t*     right;

    int         type;
    uint64_t    id;
    double      data;

} node_t;

typedef struct expr_t{
    files_t files;

    node_t* root;

    uint64_t numElem;
    uint64_t numDump;

} expr_t;

typedef struct opName_t{
    char    name[5];
    int     opNum;
} opName_t;

int NodeDel(expr_t* expr, node_t* node, int param);

int ExprCtor    (expr_t* expr);
int ExprDtor    (expr_t* expr);
int ExprDump    (expr_t* expr);


int HTMLDumpGenerate    (expr_t* expr);
int ExprEval            (expr_t* expr, double* ret);
int ExprTEX             (expr_t* expr);
int LoadExpr            (expr_t* expr);
int ExprDiff            (expr_t* expr, expr_t* diff);

