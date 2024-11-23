#include "../hpp/diff.hpp"

#define MEOW fprintf(stderr, RED "meow\n" RESET);

const int64_t MAX_BUFF       = 128;
const int64_t MAXLEN_COMMAND = 128;
const int64_t MAXDOT_BUFF    = 16;

const int64_t MINUS_VIBE     = -52;

static int DoDot            (expr_t* expr);
static int HTMLGenerateHead (expr_t* expr);
static int HTMLGenerateBody (expr_t* expr);

static int StartExprDump    (expr_t* expr);
static int NodeDump         (expr_t* expr, node_t* node, int depth);
static int EndExprDump      (expr_t* expr);

static double   NodeEval        (expr_t* expr, node_t* node);
static int      NodePrint       (expr_t* expr, node_t* node, int depth, FILE* file);
static int      GetOp           (int num, char* buff);

enum param_t{

    //newnode params
    LEFT    = -1,
    RIGHT   = -2,
    ROOT    = -3,
    //newnode params

    //node types
    NUM     = 50,
    VAR     = 51,
    OP      = 52
    //node types

};

enum operations_t{
    ADD = 43,
    SUB = 45,
    MUL = 42,
    DIV = 47
};

enum errors{

    OK  = 0,
    ERR = 1

};

/*==============================================================================*/

opName_t opList[] = {
    {"+", ADD},
    {"-", SUB},
    {"*", MUL},
    {"/", DIV}
};

/*==============================================================================*/

int ExprPrint(expr_t* expr, FILE* file){
    //verify
    fprintf(file, "$");
    NodePrint(expr, expr->root, 0, file);
    fprintf(file, "$");

    return OK;
}

static int NodePrint(expr_t* expr, node_t* node, int depth, FILE* file){
    if (!node) return OK;
    if (depth >= expr->numElem) return ERR;

    //if (node->type != NUM) fprintf(file, "(");
    fprintf(file, "(");

    if (node->left)  NodePrint(expr, node->left, depth + 1, file);

    if (node->type == NUM)      fprintf(file, "%0.1lf", node->data);

    else if (node->type == VAR) fprintf(file, "%c", (char)node->data);

    else if (node->type == OP){
        char buff[MAXDOT_BUFF] = {};
        GetOp((int)node->data, buff);
        fprintf(file, "%s", buff);
    }

    if (node->right) NodePrint(expr, node->right, depth + 1, file);


    //if (node->type != NUM) fprintf(file, ")");
    fprintf(file, ")");

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

int NewNode(expr_t* expr, double data, int param, node_t* left, node_t* right, node_t** ret){
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

    if (ret) *ret = newNode;

    newNode->id = expr->numElem;
    expr->numElem++;

    return OK;
}

/*==============================================================================*/
int ExprEval(expr_t* expr, double* ret){
    //verify
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

    //verify
    return OK;
}

/*==============================================================================*/

int ExprDtor(expr_t* expr){
    //verify



    return OK;
}

/*==============================================================================*/

int ExprDump(expr_t* expr){

    StartExprDump(expr);
    NodeDump(expr, expr->root, 0);
    EndExprDump(expr);

    DoDot(expr);
    HTMLGenerateBody(expr);

    return OK;
}

static int NodeDump(expr_t* expr, node_t* node, int depth){
    if (!node) return OK;
    if (depth > expr->numElem) return ERR;

    char outBuff[MAXDOT_BUFF] = {};

    //node
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
        //char* varname = "MEOW_VAR";

        fprintf(expr->files.dot,
            "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#e6ffe6\";label = \" { %0.3llu } | { var: %c }\"];\n",
            node->id, node->id, (char)node->data);
    }

    else {
        //char* varname = "MEOW_VAR";

        fprintf(expr->files.dot,
            "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#ffe6fe\";label = \" { %0.3llu } | { unknown %0.0lf }\"];\n",
            node->id, node->id, node->data);
    }
    //node

    //edges
    if (node->left){
        fprintf(expr->files.dot,
                "\tnode%0.3llu -> node%0.3llu [ fontname=\"SF Pro\"; weight=1; color=\"#04BF00\"; style=\"bold\"];\n\n",
                node->id, node->left->id);

        NodeDump(expr, node->left, depth + 1);
    }

    if (node->right){
        fprintf(expr->files.dot,
                "\tnode%0.3llu -> node%0.3llu [ fontname=\"SF Pro\"; weight=1; color=\"#fd4381\"; style=\"bold\"];\n\n",
                node->id, node->right->id);

        NodeDump(expr, node->right, depth + 1);
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
static int LoadNode(expr_t* expr, node_t* node, int param);

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

    ExprDump(expr);
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
            NewNode(expr, 0, 0, nullptr, nullptr, &newNode);
            expr->root = newNode;
        }
        if (param == LEFT){
            NewNode(expr, 0, 0, nullptr, nullptr, &newNode);
            node->left = newNode;
        }
        if (param == RIGHT){
            NewNode(expr, 0, 0, nullptr, nullptr, &newNode);
            node->right = newNode;
        }
    } else abort();
    fscanf(expr->files.save, "%[^()]", buffer);
    bracket = fgetc(expr->files.save);

    if (bracket == ')'){
        //read data
        MEOW
        int type = CheckArg(buffer);

        printf(BLU "%d\n" RESET, type);

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
