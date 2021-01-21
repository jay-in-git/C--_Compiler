#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include "symbolTable.h"
// This file is for reference only, you are not required to follow the implementation. //
// You only need to check for errors stated in the hw4 document. //
int g_anyErrorOccur = 0;
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_YELLOW  "\033[0;33m"

DATA_TYPE getBiggerType(DATA_TYPE dataType1, DATA_TYPE dataType2);
void processProgramNode(AST_NODE *programNode);
void processDeclList(AST_NODE* declarationNode, int isGlobal);
void processDeclarationNode(AST_NODE* declarationNode, int isGlobal);
void declareFunction(AST_NODE *type);
void declareParamList(AST_NODE* paramList, SymbolAttribute* attr);
void declareParam(AST_NODE* type, SymbolAttribute* attr, Parameter* tail);
void processDeclDimList(AST_NODE* idNode, TypeDescriptor* typeDescriptor, int isParam);
void declareType(AST_NODE* idNodeAsType);
void processBlockNode(AST_NODE* blockNode);
void processExprNode(AST_NODE* exprNode);
TypeDescriptor* declareVariable(AST_NODE* type, int isParam, int isGlobal);
int checkTypeExist(char* id);
void checkFunctionCall(AST_NODE* functionCallNode);
void processStmtNode(AST_NODE* stmtNode);
void checkAssignmentStmt(AST_NODE* assignmentNode);
void processVariablePtrValue(AST_NODE* idNode);
void processVariableValue(AST_NODE* idNode);
void processConstValueNode(AST_NODE* constValueNode);
void processExprOperandNode(AST_NODE* operandNode);
void checkParameterPassing(Parameter* formalParameter, AST_NODE* actualParameter);
DATA_TYPE getType(char* id);
TypeDescriptor* copyTypeDescriptor(TypeDescriptor* oldType);

typedef enum ErrorMsgKind
{
    SYMBOL_IS_NOT_TYPE,
    SYMBOL_REDECLARE,
    SYMBOL_UNDECLARED,
    NOT_FUNCTION_NAME,
    TRY_TO_INIT_ARRAY,
    GLOBAL_INIT_NOT_CONST,/* I added myself */
    REDECLARE_OF_TYPE,
    IS_NOT_FUNCTION_TYPE, /* added */
    EXCESSIVE_ARRAY_DIM_DECLARATION,
    RETURN_ARRAY,
    VOID_VARIABLE,
    DECL_VOID_VARIABLE,
    TYPEDEF_VOID_ARRAY,
    PARAMETER_TYPE_UNMATCH,
    TOO_FEW_ARGUMENTS,
    TOO_MANY_ARGUMENTS,
    VOID_RETURN_NON_VOID,
    NON_VOID_RETURN_VOID,
    INCOMPATIBLE_ARRAY_DIMENSION,
    NOT_ASSIGNABLE,
    ARRAY_EXPR,
    IS_TYPE_NOT_VARIABLE,
    IS_FUNCTION_NOT_VARIABLE,
    STRING_OPERATION,
    ARRAY_SIZE_NOT_INT,
    ARRAY_SIZE_NEGATIVE,
    ARRAY_SUBSCRIPT_NOT_INT,
    PASS_ARRAY_TO_SCALAR,
    PASS_SCALAR_TO_ARRAY
} ErrorMsgKind;

void printErrorMsgSpecial(AST_NODE* node1, char* name2, ErrorMsgKind errorMsgKind)
{
    // g_anyErrorOccur = 1;
    printf("Error found in line %d\n", node1->linenumber);
    
}

void printErrorMsg(AST_NODE* node, ErrorMsgKind errorMsgKind)
{
    g_anyErrorOccur = 1;
    printf(ANSI_COLOR_RED "Error" ANSI_COLOR_RESET " found in line %d\n", node->linenumber);
    switch(errorMsgKind)
    {
        case SYMBOL_IS_NOT_TYPE:
            printf(ANSI_COLOR_YELLOW"   \'%s\' was not declared in this scope\n"ANSI_COLOR_RESET, node->semantic_value.identifierSemanticValue.identifierName);
            break;
        case SYMBOL_REDECLARE:
            printf(ANSI_COLOR_YELLOW"    redeclaration of \'%s %s\'\n"ANSI_COLOR_RESET, node->leftmostSibling->semantic_value.identifierSemanticValue.identifierName, node->semantic_value.identifierSemanticValue.identifierName);
            break; 
        case SYMBOL_UNDECLARED:
            printf(ANSI_COLOR_YELLOW"    \'%s\' was not declared in this scope\n"ANSI_COLOR_RESET, node->semantic_value.identifierSemanticValue.identifierName);
            break; 
        case REDECLARE_OF_TYPE:
            printf(ANSI_COLOR_YELLOW"    conflicting types for \'%s\'\n"ANSI_COLOR_RESET, node->semantic_value.identifierSemanticValue.identifierName);
            break;
        case IS_NOT_FUNCTION_TYPE:
            printf(ANSI_COLOR_YELLOW"    function cannot return array type '%s'\n"ANSI_COLOR_RESET, node->semantic_value.identifierSemanticValue.identifierName);
            break;
        case NOT_FUNCTION_NAME:
            printf(ANSI_COLOR_YELLOW"    called object \'%s\' is not a function or function pointer\n"ANSI_COLOR_RESET, node->semantic_value.identifierSemanticValue.identifierName);
            break; 
        case TRY_TO_INIT_ARRAY:
            printf(ANSI_COLOR_YELLOW"    try to init array \'%s\'\n"ANSI_COLOR_RESET, node->semantic_value.identifierSemanticValue.identifierName);
            break; 
        case GLOBAL_INIT_NOT_CONST:
            printf(ANSI_COLOR_YELLOW"    initializer element is not constant\n"ANSI_COLOR_RESET);
            break; 
        case EXCESSIVE_ARRAY_DIM_DECLARATION:
            break; 
        case RETURN_ARRAY:
            printf(ANSI_COLOR_YELLOW"    can not return an array\n"ANSI_COLOR_RESET);
            break; 
        case VOID_VARIABLE:
            printf(ANSI_COLOR_YELLOW"    void type im expression\n"ANSI_COLOR_RESET);
            break;
        case DECL_VOID_VARIABLE:
            printf(ANSI_COLOR_YELLOW"    variable has incomplete type 'void'\n"ANSI_COLOR_RESET);
            break;
        case TYPEDEF_VOID_ARRAY:
            printf(ANSI_COLOR_YELLOW"    array has incomplete type 'void'\n"ANSI_COLOR_RESET);
            break; 
        case PARAMETER_TYPE_UNMATCH:
            printf(ANSI_COLOR_YELLOW"    passing parameter to incomparable type\n"ANSI_COLOR_RESET);
            break; 
        case TOO_FEW_ARGUMENTS:
            printf(ANSI_COLOR_YELLOW"    too few arguments to function '%s'\n"ANSI_COLOR_RESET, node->semantic_value.identifierSemanticValue.identifierName);
            break; 
        case TOO_MANY_ARGUMENTS:
            printf(ANSI_COLOR_YELLOW"    too many arguments to function \'%s\'\n"ANSI_COLOR_RESET, node->semantic_value.identifierSemanticValue.identifierName);
            break; 
        case VOID_RETURN_NON_VOID:
            printf(ANSI_COLOR_YELLOW"    void function '%s' should not return a value\n"ANSI_COLOR_RESET, node->semantic_value.identifierSemanticValue.identifierName);
            break;
        case NON_VOID_RETURN_VOID:
            printf(ANSI_COLOR_YELLOW"    non-void function '%s' should return a value\n"ANSI_COLOR_RESET, node->semantic_value.identifierSemanticValue.identifierName);
            break;
        case INCOMPATIBLE_ARRAY_DIMENSION:
            printf(ANSI_COLOR_YELLOW"    subscripted value is neither array nor pointer nor vector\n"ANSI_COLOR_RESET);
            break; 
        case NOT_ASSIGNABLE:
            printf(ANSI_COLOR_YELLOW"    assignment to expression with array type\n"ANSI_COLOR_RESET);
            break; 
        case ARRAY_EXPR:
            printf(ANSI_COLOR_YELLOW"    array type in expression\n"ANSI_COLOR_RESET);
            break; 
        case IS_TYPE_NOT_VARIABLE:
            printf(ANSI_COLOR_YELLOW"   \'%s\' is a type, not variable\n"ANSI_COLOR_RESET, node->semantic_value.identifierSemanticValue.identifierName);
            break; 
        case IS_FUNCTION_NOT_VARIABLE:
            printf(ANSI_COLOR_YELLOW"   \'%s\' is a function, not variable\n"ANSI_COLOR_RESET, node->semantic_value.identifierSemanticValue.identifierName);
            break; 
        case STRING_OPERATION:
            printf(ANSI_COLOR_YELLOW"   string is not operable\n"ANSI_COLOR_RESET);
            break; 
        case ARRAY_SIZE_NOT_INT:
            printf(ANSI_COLOR_YELLOW"   size of array has non-integer type\n"ANSI_COLOR_RESET);
            break; 
        case ARRAY_SIZE_NEGATIVE:
            printf(ANSI_COLOR_YELLOW"   size of array \'%s\' is negative\n"ANSI_COLOR_RESET, node->semantic_value.identifierSemanticValue.identifierName);
            break; 
        case ARRAY_SUBSCRIPT_NOT_INT:
            printf(ANSI_COLOR_YELLOW"   array subscript is not an integer\n"ANSI_COLOR_RESET);
            break;
        default:
            printf("Unhandled case in void printErrorMsg(AST_NODE* node, ERROR_MSG_KIND* errorMsgKind)\n");
            break;
    }
}


void semanticAnalysis(AST_NODE *root)
{
    processProgramNode(root);
}


DATA_TYPE getBiggerType(DATA_TYPE dataType1, DATA_TYPE dataType2)
{
    if(dataType1 == FLOAT_TYPE || dataType2 == FLOAT_TYPE) {
        return FLOAT_TYPE;
    } else {
        return INT_TYPE;
    }
}

FunctionSignature* initFunctionSignature(DATA_TYPE type) {
    FunctionSignature* tmp = (FunctionSignature*) malloc(sizeof(FunctionSignature));
    tmp->parameterList = NULL;
    tmp->parametersCount = 0;
    tmp->returnType = type;
    return tmp;
}

Parameter* initParameter() {
    Parameter* tmp = (Parameter*) malloc(sizeof(Parameter));
    tmp->next = NULL;
    tmp->type = NULL;
    tmp->parameterName = NULL;
    return tmp;
}

SymbolAttribute* initAttribute (SymbolAttributeKind attributeKind) {
    SymbolAttribute* tmp = (SymbolAttribute*) malloc(sizeof(SymbolAttribute));
    tmp->attributeKind = attributeKind;
    switch(attributeKind) {
        case FUNCTION_SIGNATURE:
            tmp->attr.functionSignature = NULL;
            break;
        case VARIABLE_ATTRIBUTE:
        case TYPE_ATTRIBUTE:
            tmp->attr.typeDescriptor = NULL;
            break;
        default:
            printf("Wrong type when initAttr... got %d\n", attributeKind);
            break;
    }
    return tmp;
}

int checkTypeExist(char* id) {
    SymbolTableEntry* entry = retrieveSymbol(id);
    if (entry != NULL && entry->attribute->attributeKind == TYPE_ATTRIBUTE) {
        return 1;
    }
    return 0;
}

DATA_TYPE getType(char* id) {
    SymbolTableEntry* entry = retrieveSymbol(id);
    if (entry != NULL
             && entry->attribute->attributeKind == TYPE_ATTRIBUTE
              && entry->attribute->attr.typeDescriptor->kind == SCALAR_TYPE_DESCRIPTOR){
        return entry->attribute->attr.typeDescriptor->properties.dataType;
    }
    else if (entry != NULL
             && entry->attribute->attributeKind == TYPE_ATTRIBUTE
              && entry->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR){
        return entry->attribute->attr.typeDescriptor->properties.arrayProperties.elementType;
    }
    return ERROR_TYPE;
}

void preInsert(char *id) {
    
    if (!strcmp(id, SYMBOL_TABLE_INT_NAME)) {
        SymbolAttribute* attr = initAttribute(TYPE_ATTRIBUTE);
        attr->attr.typeDescriptor = (TypeDescriptor*) malloc(sizeof(TypeDescriptor));
        attr->attr.typeDescriptor->kind = SCALAR_TYPE_DESCRIPTOR;
        attr->attr.typeDescriptor->properties.dataType = INT_TYPE;
        enterSymbol(SYMBOL_TABLE_INT_NAME, attr);
    }
    else if (!strcmp(id, SYMBOL_TABLE_FLOAT_NAME)) {
        SymbolAttribute* attr = initAttribute(TYPE_ATTRIBUTE);
        attr->attr.typeDescriptor = (TypeDescriptor*) malloc(sizeof(TypeDescriptor));
        attr->attr.typeDescriptor->kind = SCALAR_TYPE_DESCRIPTOR;
        attr->attr.typeDescriptor->properties.dataType = FLOAT_TYPE;
        enterSymbol(SYMBOL_TABLE_FLOAT_NAME, attr);
    }
    else if (!strcmp(id, SYMBOL_TABLE_VOID_NAME)) {
        SymbolAttribute* attr = initAttribute(TYPE_ATTRIBUTE);
        attr->attr.typeDescriptor = (TypeDescriptor*) malloc(sizeof(TypeDescriptor));
        attr->attr.typeDescriptor->kind = SCALAR_TYPE_DESCRIPTOR;
        attr->attr.typeDescriptor->properties.dataType = VOID_TYPE;
        enterSymbol(SYMBOL_TABLE_VOID_NAME, attr);
    }
    else if (!strcmp(id, SYMBOL_TABLE_SYS_LIB_READ)) {
        SymbolAttribute* attr = initAttribute(FUNCTION_SIGNATURE);
        attr->attr.functionSignature = initFunctionSignature(INT_TYPE);
        enterSymbol(SYMBOL_TABLE_SYS_LIB_READ, attr);
    }
    else if (!strcmp(id, SYMBOL_TABLE_SYS_LIB_FREAD)) {
        SymbolAttribute* attr = initAttribute(FUNCTION_SIGNATURE);
        attr->attr.functionSignature = initFunctionSignature(FLOAT_TYPE);
        enterSymbol(SYMBOL_TABLE_SYS_LIB_FREAD, attr);
    }
}

void processProgramNode(AST_NODE* programNode)
{
    AST_NODE* current = programNode->child;
    while (current != NULL) {
        switch (current->nodeType) {
            case VARIABLE_DECL_LIST_NODE:
                processDeclList(current->child, 1);
                break;
            case DECLARATION_NODE:
                declareFunction(current->child);
                break;
            default:
                puts("Unknown node type when processing program node...");
                break;
        }
        current = current->rightSibling;
    }
}

void processDeclList(AST_NODE* declarationNode, int isGlobal) {
    while (declarationNode != NULL) {
        processDeclarationNode(declarationNode, isGlobal);
        declarationNode = declarationNode->rightSibling;
    }
}

void processDeclarationNode(AST_NODE *declarationNode, int isGlobal)
{
    switch (declarationNode->semantic_value.declSemanticValue.kind) {
        case VARIABLE_DECL:
            declareVariable(declarationNode->child, 0, isGlobal);
            break;
        case TYPE_DECL:
            declareType(declarationNode->child);
            break;
        default:
            printf("Wrong type in Declaration Node, got %d\n", declarationNode->semantic_value.declSemanticValue.kind);
            break;
    }
}

void declareFunction(AST_NODE *type) 
{
    AST_NODE* name = (type->rightSibling);
    AST_NODE* paramList = name->rightSibling, *block = name->rightSibling->rightSibling;
    if (!checkTypeExist(type->semantic_value.identifierSemanticValue.identifierName)) {
        printErrorMsg(type, SYMBOL_IS_NOT_TYPE);
    }
    if (declaredLocally(name->semantic_value.identifierSemanticValue.identifierName)) {
        printErrorMsg(name, SYMBOL_REDECLARE);
    }
    SymbolAttribute* attr = initAttribute(FUNCTION_SIGNATURE);
    DATA_TYPE declType = getType(type->semantic_value.identifierSemanticValue.identifierName);
    SymbolTableEntry* typeEntry = retrieveSymbol(type->semantic_value.identifierSemanticValue.identifierName);
    if (typeEntry != NULL && typeEntry->attribute->attributeKind == TYPE_ATTRIBUTE && typeEntry->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR) {
        printErrorMsg(type, IS_NOT_FUNCTION_TYPE);
        declType = ERROR_TYPE;
    }
    type->dataType = declType;
    type->semantic_value.identifierSemanticValue.symbolTableEntry = typeEntry;
    name->dataType = declType;
    attr->attr.functionSignature = initFunctionSignature(declType);
    name->semantic_value.identifierSemanticValue.symbolTableEntry = enterSymbol(name->semantic_value.identifierSemanticValue.identifierName, attr); 
    openScope();
    declareParamList(paramList, attr);
    processBlockNode(block);
    closeScope();
}

void declareParamList(AST_NODE* paramList, SymbolAttribute* attr)
{
    AST_NODE* param = paramList->child;
    Parameter* tail = NULL;
    while (param != NULL) {
        Parameter* tmp = initParameter();
        if (tail == NULL) {
            attr->attr.functionSignature->parameterList = tmp;
            tail = attr->attr.functionSignature->parameterList;
            tmp->next = NULL;
        }
        else {
            tmp->next = NULL;
            tail->next = tmp;
            tail = tail->next;
        }
        declareParam(param->child, attr, tail);
        (attr->attr.functionSignature->parametersCount)++;
        param = param->rightSibling;
    }
}

void declareParam(AST_NODE* type, SymbolAttribute* attr, Parameter* tail) {
    AST_NODE* name = (type->rightSibling);
    TypeDescriptor* tmp = declareVariable(type, 1, 0);
    tail->type = tmp;
    attr->attr.functionSignature->parameterList->parameterName = name->semantic_value.identifierSemanticValue.identifierName;
}

TypeDescriptor* declareVariable(AST_NODE* type, int isParam, int isGlobal)
{
    AST_NODE* name = (type->rightSibling);
    DATA_TYPE declType = getType(type->semantic_value.identifierSemanticValue.identifierName);
    type->dataType = declType;
    TypeDescriptor* ptrToReturn = NULL;
    SymbolTableEntry* typeEntry = retrieveSymbol(type->semantic_value.identifierSemanticValue.identifierName);
    type->semantic_value.identifierSemanticValue.symbolTableEntry = typeEntry;
    if (!checkTypeExist(type->semantic_value.identifierSemanticValue.identifierName)) {
        printErrorMsg(type, SYMBOL_IS_NOT_TYPE);
    }
    else {
        while (name != NULL) {
            if (declaredLocally(name->semantic_value.identifierSemanticValue.identifierName)) {
                printErrorMsg(name, SYMBOL_REDECLARE);
            }
            SymbolAttribute* attr = initAttribute(VARIABLE_ATTRIBUTE);
            AST_NODE* init = (name->child);
            switch (typeEntry->attribute->attr.typeDescriptor->kind) {
                case SCALAR_TYPE_DESCRIPTOR:
                    if (declType == VOID_TYPE) {
                        printErrorMsg(name, DECL_VOID_VARIABLE);
                        break;
                    }
                    switch (name->semantic_value.identifierSemanticValue.kind) {
                        case NORMAL_ID:
                            attr->attr.typeDescriptor = (TypeDescriptor*) malloc(sizeof(TypeDescriptor));
                            attr->attr.typeDescriptor->kind = SCALAR_TYPE_DESCRIPTOR;
                            attr->attr.typeDescriptor->properties.dataType = declType;
                            break;
                        case WITH_INIT_ID:
                            attr->attr.typeDescriptor = (TypeDescriptor*) malloc(sizeof(TypeDescriptor));
                            attr->attr.typeDescriptor->kind = SCALAR_TYPE_DESCRIPTOR;
                            attr->attr.typeDescriptor->properties.dataType = declType;
                            processExprOperandNode(init);
                            if (isGlobal) {
                                switch (init->nodeType) {
                                    case CONST_VALUE_NODE:
                                        break;
                                    case EXPR_NODE:
                                        if (init->semantic_value.exprSemanticValue.isConstEval){
                                            break;
                                        }
                                    default:
                                        printErrorMsg(name, GLOBAL_INIT_NOT_CONST);
                                        break;
                                }
                            }
                            if (init->dataType == INT_PTR_TYPE || init->dataType == FLOAT_PTR_TYPE || init->dataType == CONST_STRING_TYPE) {
                                printErrorMsg(init, ARRAY_EXPR);
                            }
                            break;
                        case ARRAY_ID:
                            attr->attr.typeDescriptor = (TypeDescriptor*) malloc(sizeof(TypeDescriptor));
                            attr->attr.typeDescriptor->kind = ARRAY_TYPE_DESCRIPTOR;
                            attr->attr.typeDescriptor->properties.arrayProperties.elementType = declType;
                            processDeclDimList(name, attr->attr.typeDescriptor, isParam);
                            break;
                    }
                    break;
                case ARRAY_TYPE_DESCRIPTOR:
                    switch (name->semantic_value.identifierSemanticValue.kind) {
                        case NORMAL_ID:
                            attr->attr.typeDescriptor = copyTypeDescriptor(typeEntry->attribute->attr.typeDescriptor);
                            break;
                        case WITH_INIT_ID:
                            printErrorMsg(name, TRY_TO_INIT_ARRAY);
                            attr->attr.typeDescriptor = copyTypeDescriptor(typeEntry->attribute->attr.typeDescriptor);
                            processExprOperandNode(init);
                            if (isGlobal) {
                                switch (init->nodeType) {
                                    case CONST_VALUE_NODE:
                                        break;
                                    case EXPR_NODE:
                                        if (init->semantic_value.exprSemanticValue.isConstEval){
                                            break;
                                        }
                                    default:
                                        printErrorMsg(name, GLOBAL_INIT_NOT_CONST);
                                        break;
                                }
                            }
                            break;
                        case ARRAY_ID:
                            attr->attr.typeDescriptor = copyTypeDescriptor(typeEntry->attribute->attr.typeDescriptor);
                            processDeclDimList(name, attr->attr.typeDescriptor, isParam);
                            break;
                    }
                    break;
                default:
                    puts("Error type in declare variable");
                    break;
            }
            ptrToReturn = attr->attr.typeDescriptor;
            name->dataType = declType;
            name->semantic_value.identifierSemanticValue.symbolTableEntry = enterSymbol(name->semantic_value.identifierSemanticValue.identifierName, attr);
            name = name->rightSibling;
        }
    }
    if(isParam) return ptrToReturn;
    return NULL;
}

void processDeclDimList(AST_NODE* name, TypeDescriptor* typeDescriptor, int isParam)
{
    AST_NODE* dim = name->child;
    if (isParam) {
        if (dim->nodeType == NUL_NODE) {
            typeDescriptor->properties.arrayProperties.sizeInEachDimension[(typeDescriptor->properties.arrayProperties.dimension)++] = -1;
            dim = dim->rightSibling;
        }
    }
    while (dim != NULL) {
        processExprOperandNode(dim);
        if (dim->dataType == INT_TYPE) {
            if (dim->nodeType == CONST_VALUE_NODE) {
                typeDescriptor->properties.arrayProperties.sizeInEachDimension[(typeDescriptor->properties.arrayProperties.dimension)++] = dim->semantic_value.const1->const_u.intval;
            }
            else if (dim->nodeType == EXPR_NODE && dim->semantic_value.exprSemanticValue.isConstEval) {
                if (dim->semantic_value.exprSemanticValue.constEvalValue.iValue < 0) {
                    printErrorMsg(name, ARRAY_SIZE_NEGATIVE);
                }
                typeDescriptor->properties.arrayProperties.sizeInEachDimension[(typeDescriptor->properties.arrayProperties.dimension)++] = dim->semantic_value.exprSemanticValue.constEvalValue.iValue;
            }
        }
        else {
            printErrorMsg(dim, ARRAY_SIZE_NOT_INT);
            typeDescriptor->properties.arrayProperties.sizeInEachDimension[(typeDescriptor->properties.arrayProperties.dimension)++] = -1;
        }
        dim = dim->rightSibling;
    }
}

void processBlockNode(AST_NODE* blockNode)
{
    AST_NODE* declOrStmt = blockNode->child;
    while (declOrStmt != NULL) {
        switch (declOrStmt->nodeType) {
            case VARIABLE_DECL_LIST_NODE:
                processDeclList(declOrStmt->child, 0);
                break;
            case STMT_LIST_NODE:
                processStmtNode(declOrStmt->child);
                break;
            default:
                puts("Unknown node type in block");
                break;
        }
        declOrStmt = declOrStmt->rightSibling;
    }
}

void getExprOrConstValue(AST_NODE* node, int* intValue, float* floatValue){
    if(node->nodeType == EXPR_NODE){
        if(node->dataType == INT_TYPE){
            if(floatValue){
                *floatValue = node->semantic_value.exprSemanticValue.constEvalValue.iValue;
            }
            else{
                *intValue = node->semantic_value.exprSemanticValue.constEvalValue.iValue;
            }
        }
        else{
            if(floatValue){
                *floatValue = node->semantic_value.exprSemanticValue.constEvalValue.fValue;
            }
            else{
                *intValue = node->semantic_value.exprSemanticValue.constEvalValue.fValue;
            }
        }
    }
    else{
        if(node->dataType == INT_TYPE){
            if(floatValue){
                *floatValue = node->semantic_value.const1->const_u.intval;
            }
            else{
                *intValue = node->semantic_value.const1->const_u.intval;
            }
        }   
        else{
            if(floatValue){
                *floatValue = node->semantic_value.const1->const_u.fval;
            }
            else{
                *intValue = node->semantic_value.const1->const_u.fval;
            }
        }
    }
}

void processExprOperandNode(AST_NODE* operandNode){
    switch (operandNode->nodeType)
    {
    case IDENTIFIER_NODE:
        operandNode->semantic_value.identifierSemanticValue.symbolTableEntry = retrieveSymbol(operandNode->semantic_value.identifierSemanticValue.identifierName);
        processVariablePtrValue(operandNode);
        break;
    case STMT_NODE:
        if(operandNode->semantic_value.stmtSemanticValue.kind == FUNCTION_CALL_STMT){
            checkFunctionCall(operandNode);
        }
        else if(operandNode->semantic_value.stmtSemanticValue.kind == ASSIGN_STMT){
            checkAssignmentStmt(operandNode);
        }
        else{
            operandNode->dataType = ERROR_TYPE;
        }
        break;
    case EXPR_NODE:
        processExprNode(operandNode);
        break;
    case CONST_VALUE_NODE:
        processConstValueNode(operandNode);
        break;
    default:
        operandNode->dataType = ERROR_TYPE;
        break;
    }
}

void processExprNode(AST_NODE* exprNode)
{
    if (exprNode->semantic_value.exprSemanticValue.kind == UNARY_OPERATION) {
        AST_NODE *child = exprNode->child;
        processExprOperandNode(child);
        if(exprNode->child->dataType == INT_PTR_TYPE || exprNode->child->dataType == FLOAT_PTR_TYPE){
            printErrorMsg(exprNode->child, ARRAY_EXPR);
            exprNode->dataType = ERROR_TYPE;
        }
        if(exprNode->child->dataType == CONST_STRING_TYPE){
            printErrorMsg(exprNode, STRING_OPERATION);
            exprNode->dataType = ERROR_TYPE;
        }
        if (exprNode->child->dataType == VOID_TYPE) {
            printErrorMsg(exprNode, VOID_VARIABLE);
            exprNode->dataType = ERROR_TYPE;
        }
        if(exprNode->child->dataType == ERROR_TYPE){
            exprNode->dataType = ERROR_TYPE;
        }
        if(exprNode->dataType != ERROR_TYPE){
            exprNode->dataType = child->dataType;
            if (exprNode->semantic_value.exprSemanticValue.op.unaryOp == UNARY_OP_LOGICAL_NEGATION) exprNode->dataType = INT_TYPE;
            if(child->nodeType == CONST_VALUE_NODE || (child->nodeType == EXPR_NODE && child->semantic_value.exprSemanticValue.isConstEval)){
                if(child->dataType == INT_TYPE){
                    int intV = 0;
                    getExprOrConstValue(child, &intV, NULL);
                    switch (exprNode->semantic_value.exprSemanticValue.op.unaryOp)
                    {
                    case UNARY_OP_POSITIVE:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = intV;
                        break;
                    case UNARY_OP_NEGATIVE:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = -1 * intV;
                        break;
                    case UNARY_OP_LOGICAL_NEGATION:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = !intV;
                    default:
                        puts("Unknown unary op type in process expr...");
                        break;
                    }
                }
                else{
                    float floatV = 0;
                    getExprOrConstValue(child, NULL, &floatV);
                    switch (exprNode->semantic_value.exprSemanticValue.op.unaryOp)
                    {
                    case UNARY_OP_POSITIVE:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = floatV;
                        break;
                    case UNARY_OP_NEGATIVE:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = -1 * floatV;
                        break;
                    case UNARY_OP_LOGICAL_NEGATION:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = !floatV;
                    default:
                        puts("Unknown unary op type in process expr...");
                        break;
                    }
                }
                exprNode->semantic_value.exprSemanticValue.isConstEval = 1;
            }
        }
    }
    else {
        AST_NODE *leftC = exprNode->child, *rightC = exprNode->child->rightSibling;
        processExprOperandNode(leftC);
        processExprOperandNode(rightC);
        if(leftC->dataType == INT_PTR_TYPE || leftC->dataType == FLOAT_PTR_TYPE){
            printErrorMsg(leftC, ARRAY_EXPR);
            exprNode->dataType = ERROR_TYPE;
        }
        if(rightC->dataType == INT_PTR_TYPE || rightC->dataType == FLOAT_PTR_TYPE){
            printErrorMsg(rightC, ARRAY_EXPR);
            exprNode->dataType = ERROR_TYPE;
        }
        if(leftC->dataType == CONST_STRING_TYPE || rightC->dataType == CONST_STRING_TYPE){
            printErrorMsg(exprNode, STRING_OPERATION);
            exprNode->dataType = ERROR_TYPE;
        }
        if (leftC->dataType == VOID_TYPE || rightC->dataType == VOID_TYPE) {
            printErrorMsg(exprNode, VOID_VARIABLE);
            exprNode->dataType = ERROR_TYPE;
        }
        if(leftC->dataType == ERROR_TYPE || rightC->dataType == ERROR_TYPE){
            exprNode->dataType = ERROR_TYPE;
        }
        if(exprNode->dataType != ERROR_TYPE){
            exprNode->dataType = getBiggerType(leftC->dataType, rightC->dataType);
            if (exprNode->semantic_value.exprSemanticValue.kind == BINARY_OPERATION) {
                switch (exprNode->semantic_value.exprSemanticValue.op.binaryOp) {
                    case BINARY_OP_EQ:
                    case BINARY_OP_GE:
                    case BINARY_OP_LE:
                    case BINARY_OP_NE:
                    case BINARY_OP_GT:
                    case BINARY_OP_LT:
                    case BINARY_OP_AND:
                    case BINARY_OP_OR:
                        exprNode->dataType = INT_TYPE;
                        break;
                    default:
                        break;
                }
            }
        }
        if(exprNode->dataType != ERROR_TYPE){
            if(( (leftC->nodeType == EXPR_NODE && leftC->semantic_value.exprSemanticValue.isConstEval) || leftC->nodeType == CONST_VALUE_NODE) 
            && ((rightC->nodeType == EXPR_NODE && rightC->semantic_value.exprSemanticValue.isConstEval) || rightC->nodeType == CONST_VALUE_NODE)){
                if(leftC->dataType == INT_TYPE && rightC->dataType == INT_TYPE){
                    int leftV = 0, rightV = 0;
                    getExprOrConstValue(leftC, &leftV, NULL);
                    getExprOrConstValue(rightC, &rightV, NULL);
                    switch (exprNode->semantic_value.exprSemanticValue.op.binaryOp)
                    {
                    case BINARY_OP_ADD:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = leftV + rightV;
                        break;
                    case BINARY_OP_SUB:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = leftV - rightV;
                        break;
                    case BINARY_OP_MUL:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = leftV * rightV;
                        break;
                    case BINARY_OP_DIV:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = leftV / rightV;
                        break;
                    case BINARY_OP_EQ:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = (leftV == rightV);
                        break;
                    case BINARY_OP_GE:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = (leftV >= rightV);
                        break;
                    case BINARY_OP_LE:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = (leftV <= rightV);
                        break;
                    case BINARY_OP_NE:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = (leftV != rightV);
                        break;
                    case BINARY_OP_GT:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = (leftV > rightV);
                        break;
                    case BINARY_OP_LT:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = (leftV < rightV);
                        break;
                    case BINARY_OP_AND:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = (leftV && rightV);
                        break;
                    case BINARY_OP_OR:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = (leftV || rightV);
                        break;
                    default:
                        puts("This should not happen, missing of operator");
                        break;
                    }
                }
                else{
                    float leftV = 0, rightV = 0;
                    getExprOrConstValue(leftC, NULL, &leftV);
                    getExprOrConstValue(rightC, NULL, &rightV);
                    switch (exprNode->semantic_value.exprSemanticValue.op.binaryOp)
                    {
                    case BINARY_OP_ADD:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = leftV + rightV;
                        break;
                    case BINARY_OP_SUB:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = leftV - rightV;
                        break;
                    case BINARY_OP_MUL:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = leftV * rightV;
                        break;
                    case BINARY_OP_DIV:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = leftV / rightV;
                        break;
                    case BINARY_OP_EQ:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = (leftV == rightV);
                        break;
                    case BINARY_OP_GE:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = (leftV >= rightV);
                        break;
                    case BINARY_OP_LE:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = (leftV <= rightV);
                        break;
                    case BINARY_OP_NE:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = (leftV != rightV);
                        break;
                    case BINARY_OP_GT:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = (leftV > rightV);
                        break;
                    case BINARY_OP_LT:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = (leftV < rightV);
                        break;
                    case BINARY_OP_AND:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = (leftV && rightV);
                        break;
                    case BINARY_OP_OR:
                        exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = (leftV || rightV);
                        break;
                    default:
                        puts("This should not happen, missing of operator");
                        break;
                    }
                }
                exprNode->semantic_value.exprSemanticValue.isConstEval = 1;
            }
        }
    }
}

TypeDescriptor* copyTypeDescriptor(TypeDescriptor* oldType) {
    TypeDescriptor* newType = (TypeDescriptor*) malloc(sizeof(TypeDescriptor));
    newType->kind = oldType->kind;
    newType->properties = oldType->properties;
    return newType;
}

void declareType(AST_NODE* idNodeAsType)
{
    char* typeName = idNodeAsType->semantic_value.identifierSemanticValue.identifierName;
    AST_NODE* name = idNodeAsType->rightSibling;
    DATA_TYPE declType = getType(typeName);
    SymbolTableEntry* typeEntry = retrieveSymbol(typeName);
    SymbolTableEntry* nameEntry = retrieveSymbol(name->semantic_value.identifierSemanticValue.identifierName);
    idNodeAsType->semantic_value.identifierSemanticValue.symbolTableEntry = typeEntry;
    if(!checkTypeExist(typeName)){
        printErrorMsg(idNodeAsType, SYMBOL_IS_NOT_TYPE);
        name->dataType = ERROR_TYPE;
    }
    else{
        while (name != NULL) {
            char* idName = name->semantic_value.identifierSemanticValue.identifierName;
            SymbolAttribute* attr = initAttribute(TYPE_ATTRIBUTE);
            switch (typeEntry->attribute->attr.typeDescriptor->kind) {
                case SCALAR_TYPE_DESCRIPTOR:
                    switch (name->semantic_value.identifierSemanticValue.kind) {
                        case NORMAL_ID:
                            if (declaredLocally(idName)) {
                                if (typeEntry->attribute->attributeKind == TYPE_ATTRIBUTE
                                 && getType(idName) != declType
                                   && nameEntry->attribute->attributeKind == TYPE_ATTRIBUTE) {
                                    printErrorMsg(name, REDECLARE_OF_TYPE);
                                    name->dataType = ERROR_TYPE;
                                }
                                else {
                                    printErrorMsg(name, SYMBOL_REDECLARE);
                                    name->dataType = ERROR_TYPE;
                                }
                            }
                            else {
                                name->dataType = declType;
                                attr->attr.typeDescriptor = (TypeDescriptor*) malloc(sizeof(TypeDescriptor));
                                attr->attr.typeDescriptor->kind = SCALAR_TYPE_DESCRIPTOR;
                                attr->attr.typeDescriptor->properties.dataType = declType;
                                name->semantic_value.identifierSemanticValue.symbolTableEntry = enterSymbol(idName, attr);
                            }
                            break;
                        case ARRAY_ID:
                            if (declType == VOID_TYPE) {
                                printErrorMsg(name, TYPEDEF_VOID_ARRAY);
                                name->dataType = ERROR_TYPE;
                            }
                            if (declaredLocally(idName)) {
                                if (typeEntry->attribute->attributeKind == TYPE_ATTRIBUTE
                                 && getType(idName) != declType
                                   && nameEntry->attribute->attributeKind == TYPE_ATTRIBUTE) {
                                    printErrorMsg(name, REDECLARE_OF_TYPE);
                                    name->dataType = ERROR_TYPE;
                                }
                                else {
                                    printErrorMsg(name, SYMBOL_REDECLARE);
                                    name->dataType = ERROR_TYPE;
                                }
                            }
                            else {
                                name->dataType = declType;
                                attr->attr.typeDescriptor = (TypeDescriptor*) malloc(sizeof(TypeDescriptor));
                                attr->attr.typeDescriptor->kind = ARRAY_TYPE_DESCRIPTOR;
                                attr->attr.typeDescriptor->properties.arrayProperties.elementType = declType;
                                processDeclDimList(name, attr->attr.typeDescriptor, 0);
                                name->semantic_value.identifierSemanticValue.symbolTableEntry = enterSymbol(idName, attr);
                            }
                            break;
                        default:
                            puts("Typedef with unknown id type");
                            break;
                    }
                    break;
                case ARRAY_TYPE_DESCRIPTOR:
                    switch (name->semantic_value.identifierSemanticValue.kind) {
                        case NORMAL_ID:
                            if (declaredLocally(idName)) {
                                if (typeEntry->attribute->attributeKind == TYPE_ATTRIBUTE
                                 && getType(idName) != declType
                                   && nameEntry->attribute->attributeKind == TYPE_ATTRIBUTE) {
                                    printErrorMsg(name, REDECLARE_OF_TYPE);
                                    name->dataType = ERROR_TYPE;
                                }
                                else {
                                    printErrorMsg(name, SYMBOL_REDECLARE);
                                    name->dataType = ERROR_TYPE;
                                }
                            }
                            else {
                                name->dataType = declType;
                                attr->attr.typeDescriptor = copyTypeDescriptor(typeEntry->attribute->attr.typeDescriptor);
                                name->semantic_value.identifierSemanticValue.symbolTableEntry = enterSymbol(idName, attr);
                            }
                            break;
                        case ARRAY_ID:
                            if (declaredLocally(idName)) {
                                if (typeEntry->attribute->attributeKind == TYPE_ATTRIBUTE
                                 && getType(idName) != declType
                                   && nameEntry->attribute->attributeKind == TYPE_ATTRIBUTE) {
                                    printErrorMsg(name, REDECLARE_OF_TYPE);
                                    name->dataType = ERROR_TYPE;
                                }
                                else {
                                    printErrorMsg(name, SYMBOL_REDECLARE);
                                    name->dataType = ERROR_TYPE;
                                }
                            }
                            else {
                                name->dataType = declType;
                                attr->attr.typeDescriptor = copyTypeDescriptor(typeEntry->attribute->attr.typeDescriptor);
                                processDeclDimList(name, attr->attr.typeDescriptor, 0);
                                name->semantic_value.identifierSemanticValue.symbolTableEntry = enterSymbol(idName, attr);
                            }
                            break;
                        default:
                            puts("Typedef with id error");
                            break;
                    }
                    break;
                default:
                    puts("Typedef type error");
                    break;
            }
            name = name->rightSibling;
        }
    }
}

void checkAssignOrExpr(AST_NODE* assignOrExprRelatedNode)
{
    if(assignOrExprRelatedNode->nodeType == STMT_NODE){
        if(assignOrExprRelatedNode->semantic_value.stmtSemanticValue.kind == ASSIGN_STMT){
            checkAssignmentStmt(assignOrExprRelatedNode);
        }
        else if(assignOrExprRelatedNode->semantic_value.stmtSemanticValue.kind == FUNCTION_CALL_STMT){
            checkFunctionCall(assignOrExprRelatedNode);
        }
    }
    else{
        processExprOperandNode(assignOrExprRelatedNode);
    }
}

void checkWhileStmt(AST_NODE* whileNode)
{   
    checkAssignOrExpr(whileNode->child);
    if (whileNode->child->dataType == VOID_TYPE) {
        printErrorMsg(whileNode, VOID_VARIABLE);
    }
    processStmtNode(whileNode->child->rightSibling);
}

void checkForStmt(AST_NODE* forNode)
{
    if (forNode->child->nodeType == NONEMPTY_ASSIGN_EXPR_LIST_NODE) {
        checkAssignOrExpr(forNode->child->child);
    }
    if (forNode->child->rightSibling->nodeType == NONEMPTY_RELOP_EXPR_LIST_NODE) {
        processExprOperandNode(forNode->child->rightSibling->child);
    }
    if (forNode->child->rightSibling->rightSibling->nodeType == NONEMPTY_ASSIGN_EXPR_LIST_NODE) {
        checkAssignOrExpr(forNode->child->rightSibling->rightSibling->child);
    }
    processStmtNode(forNode->child->rightSibling->rightSibling->rightSibling);
}

void checkAssignmentStmt(AST_NODE* assignmentNode)
{
    AST_NODE* LHS_node = assignmentNode->child;
    AST_NODE* RHS_node = LHS_node->rightSibling;
    processVariableValue(LHS_node);
    processExprOperandNode(RHS_node);
    if(LHS_node->dataType == ERROR_TYPE || RHS_node->dataType == ERROR_TYPE){
        assignmentNode->dataType = ERROR_TYPE;
    }
    if(RHS_node->dataType == INT_PTR_TYPE || RHS_node->dataType == FLOAT_PTR_TYPE){
        printErrorMsg(assignmentNode, ARRAY_EXPR);
        assignmentNode->dataType = ERROR_TYPE;
    }
    else{
        assignmentNode->dataType = LHS_node->dataType;
    }
}


void checkIfStmt(AST_NODE* ifNode)
{
    processExprOperandNode(ifNode->child);
    if (ifNode->child->dataType == VOID_TYPE) {
        printErrorMsg(ifNode, VOID_VARIABLE);
    }
    processStmtNode(ifNode->child->rightSibling);
    processStmtNode(ifNode->child->rightSibling->rightSibling);
}

void checkWriteFunction(AST_NODE* functionCallNode)
{
    AST_NODE* idName = functionCallNode->child;
    AST_NODE* relopList = idName->rightSibling;
    AST_NODE* argument = relopList->child;
    if (argument == NULL) {
        printErrorMsg(idName, TOO_FEW_ARGUMENTS);
    }
    else {
        processExprOperandNode(argument);
        if (argument->rightSibling != NULL) {
            printErrorMsg(idName, TOO_MANY_ARGUMENTS);
        }
        if (argument->dataType != CONST_STRING_TYPE && argument->dataType != INT_TYPE && argument->dataType != FLOAT_TYPE) {
            printErrorMsg(idName, PARAMETER_TYPE_UNMATCH);
        }
    }
}

void checkFunctionCall(AST_NODE* functionCallNode)
{   
    AST_NODE* IdNode = functionCallNode->child;
    SymbolTableEntry* function_entry = retrieveSymbol(IdNode->semantic_value.identifierSemanticValue.identifierName);
    IdNode->semantic_value.identifierSemanticValue.symbolTableEntry = function_entry;
    if (!strcmp(IdNode->semantic_value.identifierSemanticValue.identifierName, "write")) {
        checkWriteFunction(functionCallNode);
        functionCallNode->dataType = VOID_TYPE;
        return;
    }
    if(function_entry == NULL){
        printErrorMsg(IdNode, SYMBOL_UNDECLARED);
        functionCallNode->dataType = ERROR_TYPE;
        return;
    }
    else if(function_entry->attribute->attributeKind != FUNCTION_SIGNATURE){
        printErrorMsg(IdNode, NOT_FUNCTION_NAME);
        functionCallNode->dataType = ERROR_TYPE;
        return;
    }
    else{
        AST_NODE* actualparameter = IdNode->rightSibling->child;
        Parameter* formalParameter = function_entry->attribute->attr.functionSignature->parameterList;
        int error = 0;
        while(actualparameter && formalParameter){
            processExprOperandNode(actualparameter);
            checkParameterPassing(formalParameter, actualparameter);
            if(actualparameter->dataType == ERROR_TYPE){
                functionCallNode->dataType = ERROR_TYPE;
            }
            if (actualparameter->dataType == VOID_TYPE) {
                printErrorMsg(IdNode, PARAMETER_TYPE_UNMATCH);
            }
            actualparameter = actualparameter->rightSibling;
            formalParameter = formalParameter->next;
        }
        if(actualparameter != NULL){
            printErrorMsg(IdNode, TOO_MANY_ARGUMENTS);
        }
        else if(formalParameter != NULL){
            printErrorMsg(IdNode, TOO_FEW_ARGUMENTS);
        }
        functionCallNode->dataType = function_entry->attribute->attr.functionSignature->returnType;
    }
}

void checkParameterPassing(Parameter* formalParameter, AST_NODE* actualParameter)
{
    char* typeName[16];
    typeName[INT_TYPE] = "int";
    typeName[INT_PTR_TYPE] = "int";
    typeName[FLOAT_TYPE] = "float";
    typeName[FLOAT_PTR_TYPE] = "float";
    if(formalParameter->type->kind == SCALAR_TYPE_DESCRIPTOR && (actualParameter->dataType == INT_PTR_TYPE || actualParameter->dataType == FLOAT_PTR_TYPE)){
        SymbolTableEntry* paramEntry = retrieveSymbol(actualParameter->semantic_value.identifierSemanticValue.identifierName);
        g_anyErrorOccur = 1;
        printf(ANSI_COLOR_RED "Error" ANSI_COLOR_RESET " found in line %d\n", actualParameter->linenumber);
        printf(ANSI_COLOR_YELLOW"   invalid conversion from ‘%s ", typeName[actualParameter->dataType]);
        for (int i = 0; i < paramEntry->attribute->attr.typeDescriptor->properties.arrayProperties.dimension; i++) {
            printf("[%d]", paramEntry->attribute->attr.typeDescriptor->properties.arrayProperties.sizeInEachDimension[i]);
        }
        printf("' ");
        printf("to ‘%s’\n"ANSI_COLOR_RESET, typeName[formalParameter->type->properties.dataType]);
        actualParameter->dataType = ERROR_TYPE;
    }
    else if(formalParameter->type->kind == ARRAY_TYPE_DESCRIPTOR && !(actualParameter->dataType == INT_PTR_TYPE || actualParameter->dataType == FLOAT_PTR_TYPE)){
        g_anyErrorOccur = 1;
        printf(ANSI_COLOR_RED "Error" ANSI_COLOR_RESET " found in line %d\n", actualParameter->linenumber);
        printf(ANSI_COLOR_YELLOW"   invalid conversion from ‘%s’ to ‘%s ", typeName[actualParameter->dataType], typeName[formalParameter->type->properties.arrayProperties.elementType]);
        if (formalParameter->type->properties.arrayProperties.dimension == 1) {
            printf("*");
        }
        else {
            printf("(*)");
            for (int i = 1; i < formalParameter->type->properties.arrayProperties.dimension; i++) {
                printf("[%d]", formalParameter->type->properties.arrayProperties.sizeInEachDimension[i]);
            }
        }
        printf("'\n"ANSI_COLOR_RESET);
        actualParameter->dataType = ERROR_TYPE;
    }
}

void processVariablePtrValue(AST_NODE* idNode)
{
    SymbolTableEntry *id_entry = retrieveSymbol(idNode->semantic_value.identifierSemanticValue.identifierName);
    idNode->semantic_value.identifierSemanticValue.symbolTableEntry = id_entry;
    if(id_entry == NULL){
        printErrorMsg(idNode, SYMBOL_UNDECLARED);
        idNode->dataType = ERROR_TYPE;
        return;
    }
    else if(id_entry->attribute->attributeKind == TYPE_ATTRIBUTE){
        printErrorMsg(idNode, IS_TYPE_NOT_VARIABLE);
        idNode->dataType = ERROR_TYPE;
        return;
    }
    else if(id_entry->attribute->attributeKind == FUNCTION_SIGNATURE){
        printErrorMsg(idNode, IS_FUNCTION_NOT_VARIABLE);
        idNode->dataType = ERROR_TYPE;
        return;
    }
    else{
        TypeDescriptor *type = id_entry->attribute->attr.typeDescriptor;
        if(idNode->semantic_value.identifierSemanticValue.kind == NORMAL_ID){
            if(type->kind == ARRAY_TYPE_DESCRIPTOR && type->properties.arrayProperties.elementType == INT_TYPE){
                idNode->dataType = INT_PTR_TYPE;
            }
            else if(type->kind == ARRAY_TYPE_DESCRIPTOR && type->properties.arrayProperties.elementType == FLOAT_TYPE){
                idNode->dataType = FLOAT_PTR_TYPE;
            }
            else{
                idNode->dataType = type->properties.dataType;
            }
        }
        else if(idNode->semantic_value.identifierSemanticValue.kind == ARRAY_ID){
            if(type->kind == SCALAR_TYPE_DESCRIPTOR){
                printErrorMsg(idNode, INCOMPATIBLE_ARRAY_DIMENSION);
                idNode->dataType = ERROR_TYPE;
            }
            else{
                int dim = 0;
                AST_NODE* dimNode = idNode->child;
                while(dimNode){
                    dim += 1;
                    processExprOperandNode(dimNode);
                    if(dimNode->dataType != INT_TYPE){
                        printErrorMsg(idNode, ARRAY_SUBSCRIPT_NOT_INT);
                    }
                    dimNode = dimNode->rightSibling;
                }
                if(idNode->dataType != ERROR_TYPE){
                    if(dim == type->properties.arrayProperties.dimension){
                        idNode->dataType = type->properties.arrayProperties.elementType;
                    }
                    else if(dim < type->properties.arrayProperties.dimension){
                        if(type->properties.arrayProperties.elementType == INT_TYPE){
                            idNode->dataType = INT_PTR_TYPE;
                        }
                        else{
                            idNode->dataType = FLOAT_PTR_TYPE;
                        }
                    }
                    else{
                        printErrorMsg(idNode, INCOMPATIBLE_ARRAY_DIMENSION);
                        idNode->dataType = ERROR_TYPE;
                    }
                }
            }
        }
        else{
            puts("processVariable can't be called with init_ID");
        }
    }
}

void processVariableValue(AST_NODE* idNode)
{
    SymbolTableEntry *id_entry = retrieveSymbol(idNode->semantic_value.identifierSemanticValue.identifierName);
    idNode->semantic_value.identifierSemanticValue.symbolTableEntry = id_entry;
    if(id_entry == NULL){
        printErrorMsg(idNode, SYMBOL_UNDECLARED);
        idNode->dataType = ERROR_TYPE;
        return;
    }
    else if(id_entry->attribute->attributeKind == TYPE_ATTRIBUTE){
        printErrorMsg(idNode, IS_TYPE_NOT_VARIABLE);
        idNode->dataType = ERROR_TYPE;
        return;
    }
    else if(id_entry->attribute->attributeKind == FUNCTION_SIGNATURE){
        printErrorMsg(idNode, IS_FUNCTION_NOT_VARIABLE);
        idNode->dataType = ERROR_TYPE;
        return;
    }
    else{
        TypeDescriptor *type = id_entry->attribute->attr.typeDescriptor;
        if(idNode->semantic_value.identifierSemanticValue.kind == NORMAL_ID){
            if(type->kind == ARRAY_TYPE_DESCRIPTOR){
                printErrorMsg(idNode, NOT_ASSIGNABLE);
                idNode->dataType = ERROR_TYPE;
            }
            else{
                idNode->dataType = type->properties.dataType;
            }
        }
        else if(idNode->semantic_value.identifierSemanticValue.kind == ARRAY_ID){
            int dim = 0;
            AST_NODE* dimNode = idNode->child;
            while(dimNode){
                dim += 1;
                processExprOperandNode(dimNode);
                if(dimNode->dataType != INT_TYPE){
                    printErrorMsg(idNode, ARRAY_SUBSCRIPT_NOT_INT);
                }
                dimNode = dimNode->rightSibling;
            }
            if(type->kind == ARRAY_TYPE_DESCRIPTOR){
                if(dim == type->properties.arrayProperties.dimension){
                    idNode->dataType = type->properties.arrayProperties.elementType;
                }
                else if (dim < type->properties.arrayProperties.dimension){
                    printErrorMsg(idNode, NOT_ASSIGNABLE);
                    idNode->dataType = ERROR_TYPE;
                }
                else {
                    printErrorMsg(idNode, INCOMPATIBLE_ARRAY_DIMENSION);
                    idNode->dataType = ERROR_TYPE;
                }
            }
            else{
                printErrorMsg(idNode, INCOMPATIBLE_ARRAY_DIMENSION);
                idNode->dataType = ERROR_TYPE;
            }
        }
        else{
            puts("processVariable can't be called with init_ID");
        }
    }
}


void processConstValueNode(AST_NODE* constValueNode)
{
    switch (constValueNode->semantic_value.const1->const_type)
    {
    case INTEGERC:
        constValueNode->dataType = INT_TYPE;
        break;
    case FLOATC:
        constValueNode->dataType = FLOAT_TYPE;
        break;
    case STRINGC:
        constValueNode->dataType = CONST_STRING_TYPE;
        break;
    default:
        break;
    }
}


void checkReturnStmt(AST_NODE* returnNode)
{   
    AST_NODE* parent = returnNode->parent;
    DATA_TYPE returnType = NONE_TYPE;
    while(parent){
        if(parent->nodeType == DECLARATION_NODE){
            if(parent->semantic_value.declSemanticValue.kind == FUNCTION_DECL){
                returnType = parent->child->dataType;
            }
            break;
        }
        parent = parent->parent;
    }
    if(returnNode->child == NULL){
        if(returnType != VOID_TYPE){
            printErrorMsg(parent->child->rightSibling, NON_VOID_RETURN_VOID);
            returnNode->dataType = ERROR_TYPE;
        }
        else{
            returnNode->dataType = returnType;
        }
    }
    else
    {
        processExprOperandNode(returnNode->child);
        if (returnType == VOID_TYPE) {
            printErrorMsg(parent->child->rightSibling, VOID_RETURN_NON_VOID);
            returnNode->dataType = ERROR_TYPE;
        }
        else if(returnNode->child->dataType == returnType){
            returnNode->dataType = returnType;
        }
        else if((returnNode->child->dataType == INT_TYPE || returnNode->child->dataType == FLOAT_TYPE)
        && (returnType == INT_TYPE || returnType == FLOAT_TYPE)){
            returnNode->dataType = returnType;
        }
        else if (returnNode->child->dataType == INT_PTR_TYPE || returnNode->child->dataType == FLOAT_PTR_TYPE){
            printErrorMsg(returnNode, RETURN_ARRAY);
            returnNode->dataType = ERROR_TYPE;
        }
        else {
            returnNode->dataType = ERROR_TYPE;
        }
    }
}

void processStmtNode(AST_NODE* stmtNode)
{   
    while (stmtNode != NULL) {
        if (stmtNode == NULL) {
            //return;
        }
        else if(stmtNode->nodeType == NUL_NODE){
            //return;
        }
        else if(stmtNode->nodeType == BLOCK_NODE){
            openScope();
            processBlockNode(stmtNode);
            closeScope();
            //return;
        }
        else if(stmtNode->nodeType == STMT_NODE){ 
            if (stmtNode->nodeType == NUL_NODE) break;
            switch (stmtNode->semantic_value.stmtSemanticValue.kind)
            {
            case WHILE_STMT:
                checkWhileStmt(stmtNode);
                break;
            case FOR_STMT:
                checkForStmt(stmtNode);
                break;
            case ASSIGN_STMT:
                checkAssignmentStmt(stmtNode);
                break;
            case IF_STMT:
                checkIfStmt(stmtNode);
                break;
            case FUNCTION_CALL_STMT:
                checkFunctionCall(stmtNode);
                break;
            case RETURN_STMT:
                checkReturnStmt(stmtNode);
                break;
            default:
                puts("can't handle stmt kind in processStmtNode");
                stmtNode->dataType = ERROR_TYPE;
                break;
            }
        }
        stmtNode = stmtNode->rightSibling;
    }
    // if (stmtNode == NULL) {
    //     return;
    // }
    // else if(stmtNode->nodeType == NUL_NODE){
    //     return;
    // }
    // else if(stmtNode->nodeType == BLOCK_NODE){
    //     openScope();
    //     processBlockNode(stmtNode);
    //     closeScope();
    //     return;
    // }
    // else if(stmtNode->nodeType == STMT_NODE){
    //     while (stmtNode != NULL) {
    //         if (stmtNode->nodeType == NUL_NODE) break;
    //         switch (stmtNode->semantic_value.stmtSemanticValue.kind)
    //         {
    //         case WHILE_STMT:
    //             checkWhileStmt(stmtNode);
    //             break;
    //         case FOR_STMT:
    //             checkForStmt(stmtNode);
    //             break;
    //         case ASSIGN_STMT:
    //             checkAssignmentStmt(stmtNode);
    //             break;
    //         case IF_STMT:
    //             checkIfStmt(stmtNode);
    //             break;
    //         case FUNCTION_CALL_STMT:
    //             checkFunctionCall(stmtNode);
    //             break;
    //         case RETURN_STMT:
    //             checkReturnStmt(stmtNode);
    //             break;
    //         default:
    //             puts("can't handle stmt kind in processStmtNode");
    //             stmtNode->dataType = ERROR_TYPE;
    //             break;
    //         }
    //         stmtNode = stmtNode->rightSibling;
    //     }
    // }
}
