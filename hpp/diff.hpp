#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "colors.hpp"

typedef struct files_t{

    FILE* log;
    char* logName;

    FILE* html;
    char* htmlName;

    FILE* dot;
    char* dotName;

    FILE* save;
    char* saveName;

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

    node_t* root;

    uint64_t numElem;
    uint64_t numDump;

    files_t files;

} expr_t;

typedef struct opName_t{
    char    name[5];
    int     opNum;
} opName_t;

int ExprCtor    (expr_t* expr);
int ExprDtor    (expr_t* expr);
int ExprDump    (expr_t* expr);
int NewNode     (expr_t* expr, double data, int param, node_t* left, node_t* right, node_t** retNode);

int HTMLDumpGenerate    (expr_t* expr);
int ExprEval            (expr_t* expr, double* ret);
int ExprTEX             (expr_t* expr);
int LoadExpr            (expr_t* expr);
