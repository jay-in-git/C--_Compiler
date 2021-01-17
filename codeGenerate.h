#define INT_REG_NUM 17
#define FLOAT_REG_NUM 14
typedef struct Register {
    char name[4];
    int is_used;
}Register;
Register int_avail_regs[INT_REG_NUM] = {
    {.name = "t0", .is_used = 0},
    {.name = "t1", .is_used = 0},
    {.name = "t2", .is_used = 0},
    {.name = "t3", .is_used = 0},
    {.name = "t4", .is_used = 0},
    {.name = "t5", .is_used = 0},
    {.name = "t6", .is_used = 0},
    {.name = "s2", .is_used = 0},
    {.name = "s3", .is_used = 0},
    {.name = "s4", .is_used = 0},
    {.name = "s5", .is_used = 0},
    {.name = "s6", .is_used = 0},
    {.name = "s7", .is_used = 0},
    {.name = "s8", .is_used = 0},
    {.name = "s9", .is_used = 0},
    {.name = "s10", .is_used = 0},
    {.name = "s11", .is_used = 0}
};
Register float_avail_regs[FLOAT_REG_NUM] = {
    {.name = "ft0", .is_used = 0},
    {.name = "ft1", .is_used = 0},
    {.name = "ft2", .is_used = 0},
    {.name = "ft3", .is_used = 0},
    {.name = "ft4", .is_used = 0},
    {.name = "ft5", .is_used = 0},
    {.name = "ft6", .is_used = 0},
    {.name = "fs1", .is_used = 0},
    {.name = "fs2", .is_used = 0},
    {.name = "fs3", .is_used = 0},
    {.name = "fs4", .is_used = 0},
    {.name = "fs5", .is_used = 0},
    {.name = "fs6", .is_used = 0},
    {.name = "fs7", .is_used = 0}
};
int const_count = 0;
int int_reg_next = 0;
int float_reg_next = 0;
int short_circuit_count = 0;
int if_count = 0;
int while_count = 0;

void codeGenerate(AST_NODE* program_node);
void genHead(char *name);
void genPrologue(char* name);
void genEpilogue(char* name, int offset);
int getIntRegister();
int getFloatRegister();

void freeIntRegister(int place);
void freeFloatRegister(int place);
void genProgram(AST_NODE* programNode);
void genGlobalDecl(AST_NODE* decl_node);
void genFunction(AST_NODE* decl_node);
void genParameter(AST_NODE* para_list);
void genBlock(AST_NODE* block_node, int *AR_offset);
void genStmt(AST_NODE* stmt_node, int *AR_offset);
void genFunctionDecl(AST_NODE* decl_node);
void genLocalVariable(AST_NODE* type_node, int* AR_offset);
void genStmt(AST_NODE* stmt_node, int *AR_offset);
void genWhileStmt(AST_NODE* while_node, int* AR_offset);
void genForStmt(AST_NODE* for_node, int* AR_offset);
void genAssignmentStmt(AST_NODE* assignment_node);
void genAssignOrExpr(AST_NODE* assignOrExprRelatedNode);
void genConstValue(AST_NODE* constValueNode);
void genValuateBinaryExprValue(AST_NODE* exprNode);
void genIfStmt(AST_NODE* if_node, int* AR_offset);
void genReturnStmt(AST_NODE* return_node);
void genFunctionCall(AST_NODE* function_node);
void  genExprRelated(AST_NODE* operandNode);
void genExpr(AST_NODE* exprNode);
void genValuateUnaryExprValue(AST_NODE* exprNode);
void genWrite(AST_NODE *node);
void genRead();
void genFread();