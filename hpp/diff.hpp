#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "colors.hpp"

const int OP_LEN            = 5;
const int SUBST_NAME_LEN    = 5;

typedef struct files_t{

    char* logName;
    char* htmlName;
    char* dotName;
    char* saveName;
    char* TexName;

    FILE* log;
    FILE* html;
    FILE* dot;
    FILE* save;
    FILE* tex;

} files_t;

typedef struct node_t{

    node_t*     parent;
    node_t*     left;
    node_t*     right;

    int         type;
    uint64_t    id;
    double      data;

} node_t;

typedef struct nodeDictPair_t{

    node_t* node;
    char*   name;

} dictPair_t;

typedef struct expr_t{
    files_t files;

    node_t* root;

    uint64_t numElem;
    uint64_t numId;
    uint64_t numDump;
    uint64_t diffCount;

    dictPair_t* dict;
    uint64_t    numDict;
    int*        usedLabels;
    int         currentLabel;

} expr_t;

typedef struct opName_t{
    char    name[OP_LEN];
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
int ExprDiff            (expr_t** expr);

int StartTex(expr_t* expr);
int FillTex(expr_t* expr, node_t* node, int param);
int EndTex(expr_t* expr);
