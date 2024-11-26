#include "../hpp/diff.hpp"

#define MEOW fprintf(stderr, RED "meow\n" RESET);

const int64_t MAX_BUFF       = 128;
const int64_t MAXLEN_COMMAND = 128;
const int64_t MAXDOT_BUFF    = 16;

const int64_t MINUS_VIBE     = -52;

//dump related
static int      DoDot            (expr_t* expr);
static int      HTMLGenerateHead (expr_t* expr);
static int      HTMLGenerateBody (expr_t* expr);
static int      StartExprDump    (expr_t* expr);
static int      NodeDump         (expr_t* expr, node_t* node, int depth, int param);
static int      EndExprDump      (expr_t* expr);
//dump related

static int      ExprVerify      (expr_t* expr);
static int      NodeVerify      (expr_t* expr, node_t* node, int depth);

static double   NodeEval        (expr_t* expr, node_t* node);
static int      NodePrint       (expr_t* expr, node_t* node, int depth, FILE* file);
static int      GetOp           (int num, char* buff);

static int      NewNode         (expr_t* expr, double data, int param, node_t* left, node_t* right, node_t** retNode, node_t* parent);
static int      LoadNode        (expr_t* expr, node_t* node, int param);
static int      CopyNode        (expr_t* expr, node_t* node, node_t** retNode);
static int      NodeDiff        (expr_t* diff, node_t* node, node_t** retNode);

enum param_t{

    //newnode params
    LEFT    = -1,
    RIGHT   = -2,
    ROOT    = -3,
    //newnode params

    //node types
    NUM     = 50,
    VAR     = 51,
    OP      = 52,
    //node types

    DETAILED = 20,
    SIMPLE   = 21
};

enum operations_t{
    ADD = 43,
    SUB = 45,
    MUL = 42,
    DIV = 47
};

enum errors{

    OK              = 0,
    ERR             = 1,
    RECURSION_ERR   = 2,
    TYPE_ERR        = 3

};

/*==============================================================================*/

opName_t opList[] = {
    {"+", ADD},
    {"-", SUB},
    {"*", MUL},
    {"/", DIV}
};

/*==============================================================================*/

int NodeDel(expr_t* expr, node_t* node, int param){
    if (node->left)  NodeDel(expr, node->left,  LEFT);
    if (node->right) NodeDel(expr, node->right, RIGHT);

    if (param == LEFT)  node->parent->left  = nullptr;
    if (param == RIGHT) node->parent->right = nullptr;
    free(node);
    expr->numElem--;

    return OK;
}

/*==============================================================================*/

static int ExprVerify(expr_t* expr){
    int err = NodeVerify(expr, expr->root, 0);

    if (err){
        MEOW
        return err;
    }

    return OK;
}

static int NodeVerify(expr_t* expr, node_t* node, int depth){
    if (!node) return OK;

    if (depth > expr->numElem) return RECURSION_ERR;

    if ((node->type == NUM || node->type == VAR) && (node->left || node->right)) return TYPE_ERR;

    if (node->left)  return NodeVerify(expr, node->left,  depth + 1);
    if (node->right) return NodeVerify(expr, node->right, depth + 1);

    return OK;
}

/*==============================================================================*/
int ExprDiff(expr_t* expr, expr_t* diff){
    NodeDiff(diff, expr->root, &diff->root);

    return OK;
}

static int NodeDiff(expr_t* diff, node_t* node, node_t** retNode){
    node_t* node_left_mul   = nullptr;
    node_t* node_right_mul  = nullptr;

    node_t* node_left       = nullptr;
    node_t* node_right      = nullptr;
    node_t* node_copy_left  = nullptr;
    node_t* node_copy_right = nullptr;

    if (node->type == OP){
        if (node->data == ADD){
            if (node->left)  NodeDiff(diff, node->left,  &node_left);
            if (node->right) NodeDiff(diff, node->right, &node_right);

            NewNode(diff, ADD, OP, node_left, node_right, retNode, nullptr);

        }
        else if (node->data == MUL){
            if (node->left)  NodeDiff(diff, node->left,  &node_left);
            if (node->right) NodeDiff(diff, node->right, &node_right);

            if (node->left)  CopyNode(diff, node->left,  &node_copy_left);
            if (node->right) CopyNode(diff, node->right, &node_copy_right);

            NewNode(diff, MUL, OP, node_left,  node_copy_right, &node_left_mul, nullptr);
            NewNode(diff, MUL, OP, node_right, node_copy_left,  &node_right_mul, nullptr);

            NewNode(diff, ADD, OP, node_left_mul, node_right_mul, retNode, nullptr);
        }
    }
    if (node->type == VAR)  NewNode(diff, 1, NUM, node_left, node_right, retNode, nullptr);
    if (node->type == NUM)  NewNode(diff, 0, NUM, node_left, node_right, retNode, nullptr);

    return OK;
}

static int CopyNode(expr_t* expr, node_t* node, node_t** retNode){
    node_t* node_left = nullptr;
    node_t* node_right = nullptr;

    if (node->left)  CopyNode(expr, node->left,  &node_left);
    if (node->right) CopyNode(expr, node->right, &node_right);

    NewNode(expr, node->data, node->type, node_left, node_right, retNode, nullptr);

    return OK;
}
/*==============================================================================*/

int ExprTEX(expr_t* expr){
    if (ExprVerify(expr)) return ERR;

    FILE* file = fopen("./bin/tex/meow.tex", "w");

    fprintf(file, "\\documentclass[a4paper]{article}\n");
    fprintf(file, "\\begin{document}\n");

    fprintf(file, "$");
    NodePrint(expr, expr->root, 0, file);
    fprintf(file, "$\n");

    fprintf(file, "\\end{document}\n");

    fclose(file);

    char command[MAXLEN_COMMAND] = {};
    snprintf(command, MAXLEN_COMMAND, "pdflatex ./bin/tex/meow.tex");
    system(command);

    snprintf(command, MAXLEN_COMMAND, "rm meow.aux");
    system(command);

    snprintf(command, MAXLEN_COMMAND, "rm meow.log");
    system(command);
    return OK;
}

static int NodePrint(expr_t* expr, node_t* node, int depth, FILE* file){
    if (!node) return OK;
    if (depth >= expr->numElem) return ERR;

    if (node->type != NUM && expr->root != node) fprintf(file, "(");




    if (node->type == OP && (int)node->data == DIV){
        fprintf(file, "\\frac");

        fprintf(file, "{");
        if (node->left)  NodePrint(expr, node->left, depth + 1, file);
        fprintf(file, "}");

        fprintf(file, "{");
        if (node->right) NodePrint(expr, node->right, depth + 1, file);
        fprintf(file, "}");
    }

    else{

        if (node->left)  NodePrint(expr, node->left, depth + 1, file);

        if (node->type == NUM)      fprintf(file, "%0.1lf", node->data);

        else if (node->type == VAR) fprintf(file, "%c", (char)node->data);

        else if (node->type == OP){
            char buff[MAXDOT_BUFF] = {};
            GetOp((int)node->data, buff);
            fprintf(file, "%s", buff);
        }

        if (node->right) NodePrint(expr, node->right, depth + 1, file);
    }

    if (node->type != NUM && expr->root != node) fprintf(file, ")");

    return OK;
}

static int GetOp(int num, char* buff){
    int printed = 0;

    for (int i = 0; i < sizeof(opList) / sizeof(opName_t); i++){
        if (num == opList[i].opNum){
            snprintf(buff, MAXDOT_BUFF, "%s", opList[i].name);
            printed = 1;
        }
    }

    if (!printed) snprintf(buff, MAXDOT_BUFF, "???");

    return OK;
}
/*==============================================================================*/

int NewNode(expr_t* expr, double data, int param, node_t* left, node_t* right, node_t** ret, node_t* parent){
    node_t* newNode = (node_t*)calloc(1, sizeof(*newNode));

    newNode->data = data;

    if (param == NUM){
        newNode->type = NUM;

        newNode->left   = nullptr;
        newNode->right  = nullptr;
    }

    else if (param == VAR){
        newNode->type = VAR;

        newNode->left   = nullptr;
        newNode->right  = nullptr;
    }

    else if (param == OP){
        newNode->type = OP;

        newNode->left   = left;
        newNode->right  = right;
    }

    if (parent) newNode->parent = parent;
    newNode->id = expr->numElem;
    expr->numElem++;

    if (ret) *ret = newNode;

    return OK;
}

/*==============================================================================*/
int ExprEval(expr_t* expr, double* ret){
    if (ExprVerify(expr)) return ERR;

    *ret = NodeEval(expr, expr->root);

    return OK;
}


static double NodeEval(expr_t* expr, node_t* node){

    if (node->type == NUM) return node->data;

    if (node->type == VAR){
        printf("enter num:\n");
        double a = 0;
        scanf("%lf", &a);
        return a;
    }

    if (node->type == OP){
        switch ((int)node->data){
            case ADD: return NodeEval(expr, node->left) + NodeEval(expr, node->right);
            case SUB: return NodeEval(expr, node->left) - NodeEval(expr, node->right);
            case MUL: return NodeEval(expr, node->left) * NodeEval(expr, node->right);
            case DIV: return NodeEval(expr, node->left) / NodeEval(expr, node->right);

            default: return ERR;
        }
    }

    return ERR;
}

/*==============================================================================*/

int ExprCtor(expr_t* expr){
    if (!expr) return ERR;

    expr->files.log = fopen(expr->files.logName, "w");
    if (!expr->files.log) expr->files.log = stdout;

    expr->files.html = fopen(expr->files.htmlName, "w");
    if (!expr->files.html) expr->files.html = fopen("htmldump.html", "w");

    expr->files.dotName = "./bin/dot.dot";

    HTMLGenerateHead(expr);

    if (ExprVerify(expr)) return ERR;

    return OK;
}

/*==============================================================================*/

int ExprDtor(expr_t* expr){
    if (ExprVerify(expr)) return ERR;



    return OK;
}

/*==============================================================================*/

int ExprDump(expr_t* expr){

    StartExprDump(expr);
    NodeDump(expr, expr->root, 0, DETAILED);
    EndExprDump(expr);
    DoDot(expr);
    HTMLGenerateBody(expr);

    return OK;
}

static int NodeDump(expr_t* expr, node_t* node, int depth, int param){
    printf(BLU "p:%p\n" RESET, node);
    if (!node) return OK;
    if (depth > expr->numElem) return ERR;

    char outBuff[MAXDOT_BUFF] = {};
/*---------SIMPLE---------*/
    if (param == SIMPLE){
        if (node->type == NUM){
            fprintf(expr->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#e6f2ff\";label = \" { %0.3llu } | { num: %0.2lf }\"];\n",
                node->id, node->id, node->data);
        }

        else if (node->type == OP){
            snprintf(outBuff, MAXDOT_BUFF, "%c", (char)node->data);

            fprintf(expr->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#fff3e6\";label = \" { %0.3llu } | { oper: %s }\"];\n",
                node->id, node->id, outBuff);
        }

        else if (node->type == VAR){
            fprintf(expr->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#e6ffe6\";label = \" { %0.3llu } | { var: %c }\"];\n",
                node->id, node->id, (char)node->data);
        }

        else {
            fprintf(expr->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#ffe6fe\";label = \" { %0.3llu } | { unknown %0.0lf }\"];\n",
                node->id, node->id, node->data);
        }
    }
/*---------SIMPLE---------*/

/*---------DETAILED---------*/
    else if (param == DETAILED){
        if (node->type == NUM){
            fprintf(expr->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#e6f2ff\";label = \" {{ %0.3llu } | { p: %p } | { num: %0.2lf } | {left: %p} | {right: %p} | {parent: %p}}\"];\n",
                node->id, node->id, node, node->data, node->left, node->right, node->parent);
        }

        else if (node->type == OP){
            snprintf(outBuff, MAXDOT_BUFF, "%c", (char)node->data);

            fprintf(expr->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#fff3e6\";label = \" {{ %0.3llu } | { p: %p } | { oper: %s } | {left: %p} | {right: %p} | {parent: %p}}\"];\n",
                node->id, node->id, node, outBuff, node->left, node->right, node->parent);
        }

        else if (node->type == VAR){
            fprintf(expr->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#e6ffe6\";label = \" {{ %0.3llu } | { p: %p } | { var: %c }| {left: %p} | {right: %p} | {parent: %p}}\"];\n",
                node->id, node->id, node, (char)node->data, node->left, node->right, node->parent);
        }

        else {
            fprintf(expr->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#ffe6fe\";label = \" {{ %0.3llu } | { p: %p } | { unknown %0.0lf }| {left: %p} | {right: %p} | {parent: %p}}\"];\n",
                node->id, node->id, node, node->data, node->left, node->right, node->parent);
        }
    }
/*---------DETAILED---------*/

    //edges
    if (node->left){
        fprintf(expr->files.dot,
                "\tnode%0.3llu -> node%0.3llu [ fontname=\"SF Pro\"; weight=1; color=\"#04BF00\"; style=\"bold\"];\n\n",
                node->id, node->left->id);

        NodeDump(expr, node->left, depth + 1, param);
    }

    if (node->right){
        fprintf(expr->files.dot,
                "\tnode%0.3llu -> node%0.3llu [ fontname=\"SF Pro\"; weight=1; color=\"#fd4381\"; style=\"bold\"];\n\n",
                node->id, node->right->id);

        NodeDump(expr, node->right, depth + 1, param);
    }
    //edges

    return OK;
}

static int StartExprDump(expr_t* expr){

    if (!expr->files.dot) expr->files.dot = fopen("./bin/dot.dot", "w");

    fprintf(expr->files.dot, "digraph G{\n");

    fprintf(expr->files.dot, "\trankdir=TB;\n");
    fprintf(expr->files.dot, "\tbgcolor=\"#f8fff8\";\n");

    return OK;
}

static int EndExprDump(expr_t* expr){

    fprintf(expr->files.dot, "}\n");

    fclose(expr->files.dot);

    return OK;
}

/*==============================================================================*/

int LoadExpr(expr_t* expr){

    //open save
    expr->files.save = fopen(expr->files.saveName, "r");
    if (!expr->files.save){
        expr->files.save     = fopen("save.txt", "r");
        expr->files.saveName = "save.txt";
    }
    //open save

    expr->numElem = 0;
    LoadNode(expr, expr->root, ROOT);

    //ExprDump(expr);
    return OK;
}

static int CheckArg(char* buffer){
    printf(MAG "%s\n" RESET, buffer);

    for (int i = 0; i < sizeof(opList) / sizeof(opName_t); i++){
        if (!strcmp(buffer, opList[i].name)){
            return OP;
        }
    }

    if (isalpha(buffer[0]) && buffer[1] == 0){
        return VAR;
    }

    return NUM;
}

static int GetOpNum(char* buffer){
    for (int i = 0; i < sizeof(opList) / sizeof(opName_t); i++){
        if (!strcmp(buffer, opList[i].name)){
            return opList[i].opNum;
        }
    }

    return MINUS_VIBE;
}

static int GetVarNum(char* buffer){
    return buffer[0];
}

static int GetNum(char* buffer){
    return atof(buffer);
}


static int LoadNode(expr_t* expr, node_t* node, int param){
    node_t* newNode       = nullptr;
    char buffer[MAX_BUFF] = {};
    char bracket  = 0;
    int  tempData = 0;

    fscanf(expr->files.save, "%[^()]", buffer);
    bracket = fgetc(expr->files.save);

    if (bracket == '('){
        if (param == ROOT){
            NewNode(expr, 0, 0, nullptr, nullptr, &newNode, nullptr);
            expr->root = newNode;
        }
        if (param == LEFT){
            NewNode(expr, 0, 0, nullptr, nullptr, &newNode, node);
            node->left = newNode;
        }
        if (param == RIGHT){
            NewNode(expr, 0, 0, nullptr, nullptr, &newNode, node);
            node->right = newNode;
        }
    } else abort();
    fscanf(expr->files.save, "%[^()]", buffer);
    bracket = fgetc(expr->files.save);

    if (bracket == ')'){
        //read data
        int type = CheckArg(buffer);

        if (type == OP)         tempData = GetOpNum(buffer);
        else if (type == VAR)   tempData = GetVarNum(buffer);
        else if (type == NUM)   tempData = GetNum(buffer);

        newNode->type = type;
        newNode->data = tempData;

        return OK;
    }

    else if (bracket == '('){
        ungetc(bracket, expr->files.save);
        LoadNode(expr, newNode, LEFT);


        fscanf(expr->files.save, "%[^()]", buffer);
        bracket = fgetc(expr->files.save);


        //read data
        int type = CheckArg(buffer);

        printf(BLU "%d\n" RESET, type);

        if (type == OP)         tempData = GetOpNum(buffer);
        else if (type == VAR)   tempData = GetVarNum(buffer);
        else if (type == NUM)   tempData = GetNum(buffer);

        newNode->type = type;
        newNode->data = tempData;

        if (bracket == ')'){
            return OK;
        }

        else if (bracket == '('){
            ungetc(bracket, expr->files.save);
            LoadNode(expr, newNode, RIGHT);
        }

        bracket = fgetc(expr->files.save);
    }

    return OK;
}


/*==============================================================================*/

static int DoDot(expr_t* expr){
    char command[MAXLEN_COMMAND]   = {};
    char out[MAXLEN_COMMAND]       = {};

    const char* startOut= "./bin/png/output";
    const char* endOut  = ".png";

    snprintf(out,     MAXLEN_COMMAND, "%s%llu%s", startOut, expr->numDump, endOut);
    snprintf(command, MAXLEN_COMMAND, "dot -Tpng %s > %s", expr->files.dotName, out);
    system(command);

    expr->numDump++;
    return OK;
}

static int HTMLGenerateHead(expr_t* expr){
    fprintf(expr->files.html, "<html>\n");

    fprintf(expr->files.html, "<head>\n");
    fprintf(expr->files.html, "</head>\n");

    fprintf(expr->files.html, "<body style=\"background-color:#f8fff8;\">\n");

    return OK;
}

static int HTMLGenerateBody(expr_t* expr){
    fprintf(expr->files.html, "<div style=\"text-align: center;\">\n");

    fprintf(expr->files.html, "\t<h2 style=\"font-family: 'Haas Grot Text R Web', 'Helvetica Neue', Helvetica, Arial, sans-serif;'\"> Dump: %llu</h2>\n", expr->numDump);
    fprintf(expr->files.html, "\t<h3 style=\"font-family: 'Haas Grot Text R Web', 'Helvetica Neue', Helvetica, Arial, sans-serif;'\"> Num elems: %llu</h3>\n", expr->numElem);

    fprintf(expr->files.html, "\t<img src=\"./bin/png/output%llu.png\">\n\t<br>\n\t<br>\n\t<br>\n", expr->numDump - 1);

    fprintf(expr->files.html, "</div>\n");

    return OK;
}

int HTMLDumpGenerate(expr_t* expr){

    fprintf(expr->files.html, "</body>\n");
    fprintf(expr->files.html, "</html>\n");

    return OK;
}

/*==============================================================================*/
