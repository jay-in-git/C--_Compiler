#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "symbolTable.h"
#include "header.h"
#include "codeGenerate.h"
#define PROLOGUE "sd ra,0(sp)\nsd fp,-8(sp)\nadd fp,sp,-8\nadd sp,sp,-16\nla ra,_frameSize_%s\nlw ra,0(ra)\nsub sp,sp,ra\nsd t0,8(sp)\nsd t1,16(sp)\nsd t2,24(sp)\nsd t3,32(sp)\nsd t4,40(sp)\nsd t5,48(sp)\nsd t6,56(sp)\nsd s2,64(sp)\nsd s3,72(sp)\nsd s4,80(sp)\nsd s5,88(sp)\nsd s6,96(sp)\nsd s7,104(sp)\nsd s8,112(sp)\nsd s9,120(sp)\nsd s10,128(sp)\nsd s11,136(sp)\nsd fp,144(sp)\nfsw ft0,152(sp)\nfsw ft1,156(sp)\nfsw ft2,160(sp)\nfsw ft3,164(sp)\nfsw ft4,168(sp)\nfsw ft5,172(sp)\nfsw ft6,176(sp)\nfsw ft7,180(sp)\nfsw fs1,184(sp)\nfsw fs2,188(sp)\nfsw fs3,192(sp)\nfsw fs4,196(sp)\nfsw fs5,200(sp)\nfsw fs6,204(sp)\nfsw fs7,208(sp)\n"
#define EPILOGUE "_end_%s:\nld t0,8(sp)\nld t1,16(sp)\nld t2,24(sp)\nld t3,32(sp)\nld t4,40(sp)\nld t5,48(sp)\nld t6,56(sp)\nld s2,64(sp)\nld s3,72(sp)\nld s4,80(sp)\nld s5,88(sp)\nld s6,96(sp)\nld s7,104(sp)\nld s8,112(sp)\nld s9,120(sp)\nld s10,128(sp)\nld s11,136(sp)\nld fp,144(sp)\nflw ft0,152(sp)\nflw ft1,156(sp)\nflw ft2,160(sp)\nflw ft3,164(sp)\nflw ft4,168(sp)\nflw ft5,172(sp)\nflw ft6,176(sp)\nflw ft7,180(sp)\nflw fs1,184(sp)\nflw fs2,188(sp)\nflw fs3,192(sp)\nflw fs4,196(sp)\nflw fs5,200(sp)\nflw fs6,204(sp)\nflw fs7,208(sp)\nld ra,8(fp)\nmv sp,fp\nadd sp,sp,8\nld fp,0(fp)\njr ra\n"


FILE* output;

int getIntRegister() {
    int now = int_reg_next;
    while (int_avail_regs[now].is_used) {
        now = (now + 1) % INT_REG_NUM;
        if (now == int_reg_next) {
            fprintf(stderr, "No int reg available...\n");
            exit(1);
        }
    }
    int_reg_next = (int_reg_next + 1) % INT_REG_NUM;
    int_avail_regs[now].is_used = 1;
    return now;
}

int getFloatRegister() {
    int now = float_reg_next;
    while (float_avail_regs[now].is_used) {
        now = (now + 1) % FLOAT_REG_NUM;
        if (now == float_reg_next) {
            fprintf(stderr, "No float reg available...\n");
            exit(1);
        }
    }
    float_reg_next = (now + 1) % FLOAT_REG_NUM;
    float_avail_regs[now].is_used = 1;
    return now;
}

void freeIntRegister(int place) {
    int_avail_regs[place].is_used = 0;
}

void freeFloatRegister(int place) {
    float_avail_regs[place].is_used = 0;
}

void genHead(char *name) {
	fprintf(output, ".text\n");
	fprintf(output, "_start_%s:\n", name);
}

void genPrologue(char* name) {
    fprintf(output, PROLOGUE, name);
}

void genEpilogue(char* name, int size) {
    fprintf(output, EPILOGUE, name);
    fprintf(output, ".data\n");
    fprintf(output, "_frameSize_%s: .word %d\n", name, 208 - size);
}

void codeGenerate(AST_NODE* program_node) {
    output = fopen("output.s", "w+");
    if (output == NULL) {
        puts("Can't open target file");
        exit(1);
    }
    genProgram(program_node);
    fclose(output);
}

void genProgram(AST_NODE* programNode) {
    AST_NODE* current = programNode->child;
    while (current != NULL) {
        switch (current->nodeType) {
            case VARIABLE_DECL_LIST_NODE:
                genGlobalDecl(current->child);
                break;
            case DECLARATION_NODE:
                genFunctionDecl(current);
                break;
            default:
                puts("Unknown node type when processing program node...");
                break;
        }
        current = current->rightSibling;
    }
}

void genGlobalDecl(AST_NODE* decl_node) {
    fprintf(output, ".data\n");
    while (decl_node) {
        if (decl_node->semantic_value.declSemanticValue.kind == TYPE_DECL) {
            decl_node = decl_node->rightSibling;
            continue;
        }
        AST_NODE* type_node = decl_node->child;
        AST_NODE* name_node = type_node->rightSibling;
	int* cvt;
        while (name_node) {
            AST_NODE* init_node = name_node->child;
            SymbolTableEntry* id_entry = name_node->semantic_value.identifierSemanticValue.symbolTableEntry;
            if (id_entry->attribute->attr.typeDescriptor->kind == SCALAR_TYPE_DESCRIPTOR) {
                if (init_node != NULL) {
                    // only support constant
                    switch (init_node->semantic_value.const1->const_type) {
                        case INTEGERC:
                            fprintf(output, "_GLOBAL_%s: .word %d\n"
                                , id_entry->name
                                , init_node->semantic_value.const1->const_u.intval);
                            break;
                        case FLOATC:
                            fprintf(output, "_GLOBAL_%s: .float %.38lf\n"
                                , id_entry->name
                                , init_node->semantic_value.const1->const_u.fval);
                            break;
                        default:
                            break;
                    }
                }
                else {
                    fprintf(output, "_GLOBAL_%s: .word 0\n", id_entry->name);
                }
            }
            else if (id_entry->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR) {
                int array_size = 4;
                for (int i = 0; i < id_entry->attribute->attr.typeDescriptor->properties.arrayProperties.dimension; i++) {
                    array_size *= id_entry->attribute->attr.typeDescriptor->properties.arrayProperties.sizeInEachDimension[i];
                }
                fprintf(output, "_GLOBAL_%s: .space %d\n", id_entry->name, array_size);
            }
            name_node = name_node->rightSibling;
        }
        decl_node = decl_node->rightSibling;
    }
    fprintf(output, ".text\n");
}

void genFunctionDecl(AST_NODE* decl_node) {
    AST_NODE* type_node = decl_node->child;
    AST_NODE* name_node = type_node->rightSibling;
    SymbolTableEntry* id_entry = name_node->semantic_value.identifierSemanticValue.symbolTableEntry;
    char* id_name = id_entry->name;
    int AR_offset = 0;
    genHead(id_name);
    genPrologue(id_name);
    genParameter(name_node->rightSibling);
    genBlock(name_node->rightSibling->rightSibling, &AR_offset);
    genEpilogue(id_name, AR_offset);
}

void genParameter(AST_NODE* para_decl) {
    
}

void genBlock(AST_NODE* block_node, int *AR_offset) {
    AST_NODE* decl_or_stmt = block_node->child;
    while (decl_or_stmt != NULL) {
        switch (decl_or_stmt->nodeType) {
            case VARIABLE_DECL_LIST_NODE:
                genLocalVariable(decl_or_stmt->child, AR_offset);
                break;
            case STMT_LIST_NODE:
                genStmt(decl_or_stmt->child, AR_offset);
                break;
            default:
                puts("Unknown node type in block");
                break;
        }
        decl_or_stmt = decl_or_stmt->rightSibling;
    }

}

void genLocalVariable(AST_NODE* decl_node, int* AR_offset) {
    while (decl_node) {
        AST_NODE* name_node = decl_node->child->rightSibling;
        while (name_node) {
            AST_NODE* init_node = name_node->child;
            SymbolTableEntry* id_entry = name_node->semantic_value.identifierSemanticValue.symbolTableEntry;
            if (id_entry->attribute->attr.typeDescriptor->kind == SCALAR_TYPE_DESCRIPTOR) { // int or float
                // printf("%s scalar\n", id_entry->name);
                (*AR_offset) -= 4;
                id_entry->offset = (*AR_offset);
                if (init_node) {
                    genExprRelated(init_node);
                    int tmp_reg = getIntRegister();
                    if (init_node->semantic_value.const1->const_type == INTEGERC) {
                        fprintf(output, ".data\n _CONSTANT_%d: .word %d\n.text\n", const_count, id_entry->offset);
                        fprintf(output, "lw %s, _CONSTANT_%d\n", int_avail_regs[tmp_reg].name, const_count++);
                        fprintf(output, "add %s, %s, fp\n", int_avail_regs[tmp_reg].name, int_avail_regs[tmp_reg].name);
                        fprintf(output, "sw %s, 0(%s)\n", int_avail_regs[init_node->place].name, int_avail_regs[tmp_reg].name);
                        freeIntRegister(init_node->place);
                    }
                    else { //float
                        fprintf(output, ".data\n _CONSTANT_%d: .word %d\n.text\n", const_count, id_entry->offset);
                        fprintf(output, "lw %s, _CONSTANT_%d\n", int_avail_regs[tmp_reg].name, const_count++);
                        fprintf(output, "add %s, %s, fp\n", int_avail_regs[tmp_reg].name, int_avail_regs[tmp_reg].name);
                        fprintf(output, "fsw %s, 0(%s)\n", float_avail_regs[init_node->place].name, int_avail_regs[tmp_reg].name);
                        freeFloatRegister(init_node->place);
                    }
                    freeIntRegister(tmp_reg);
                }
            }
            else { //array
                // printf("%s array\n", id_entry->name);
                int offset = 4;
                for (int i = 0; i < id_entry->attribute->attr.typeDescriptor->properties.arrayProperties.dimension; i++) {
                    offset *= id_entry->attribute->attr.typeDescriptor->properties.arrayProperties.sizeInEachDimension[i];
                }
                (*AR_offset) -= offset;
                id_entry->offset = (*AR_offset);
            }
            name_node = name_node->rightSibling;
        }
        decl_node = decl_node->rightSibling;
    }
}

void genStmt(AST_NODE* stmt_node, int *AR_offset) {
    while(stmt_node != NULL){
        if (stmt_node == NULL) {
            //return;
        }
        else if(stmt_node->nodeType == NUL_NODE){
            //return;
        }
        else if(stmt_node->nodeType == BLOCK_NODE){
            genBlock(stmt_node, AR_offset);
            //return;
        }
        else if(stmt_node->nodeType == STMT_NODE){
            if (stmt_node->nodeType == NUL_NODE) break;
            switch (stmt_node->semantic_value.stmtSemanticValue.kind)
            {
            case WHILE_STMT:
                genWhileStmt(stmt_node, AR_offset);
                break;
            case FOR_STMT:
                // not in this assignment
                genForStmt(stmt_node, AR_offset);
                break;
            case ASSIGN_STMT:
                genAssignmentStmt(stmt_node);
                break;
            case IF_STMT:
                genIfStmt(stmt_node, AR_offset);
                break;
            case FUNCTION_CALL_STMT:
                genFunctionCall(stmt_node);
                break;
            case RETURN_STMT:
                genReturnStmt(stmt_node);
                break;
            default:
                puts("can't handle stmt kind in processstmt_node");
                stmt_node->dataType = ERROR_TYPE;
                break;
            }
        }
        stmt_node = stmt_node->rightSibling;
    }
    // if (stmt_node == NULL) {
    //     return;
    // }
    // else if(stmt_node->nodeType == NUL_NODE){
    //     return;
    // }
    // else if(stmt_node->nodeType == BLOCK_NODE){
    //     genBlock(stmt_node, AR_offset);
    //     return;
    // }
    // else if(stmt_node->nodeType == STMT_NODE){
    //     while (stmt_node != NULL) {
    //         if (stmt_node->nodeType == NUL_NODE) break;
    //         switch (stmt_node->semantic_value.stmtSemanticValue.kind)
    //         {
    //         case WHILE_STMT:
    //             genWhileStmt(stmt_node, AR_offset);
    //             break;
    //         case FOR_STMT:
    //             // not in this assignment
    //             genForStmt(stmt_node, AR_offset);
    //             break;
    //         case ASSIGN_STMT:
    //             genAssignmentStmt(stmt_node);
    //             break;
    //         case IF_STMT:
    //             genIfStmt(stmt_node, AR_offset);
    //             break;
    //         case FUNCTION_CALL_STMT:
    //             genFunctionCall(stmt_node);
    //             break;
    //         case RETURN_STMT:
    //             genReturnStmt(stmt_node);
    //             break;
    //         default:
    //             puts("can't handle stmt kind in processstmt_node");
    //             stmt_node->dataType = ERROR_TYPE;
    //             break;
    //         }
    //         stmt_node = stmt_node->rightSibling;
    //     }
    // }
}

void genWhileStmt(AST_NODE* whileNode, int* AR_offset) {
    AST_NODE* boolExpr = whileNode->child;
    AST_NODE* body = boolExpr->rightSibling;
    int while_tmp = while_count;
    while_count++;
    fprintf(output, "_WHILE_LABEL_%d:\n", while_tmp);
    genExprRelated(boolExpr);
    if(boolExpr->dataType == INT_TYPE){
        fprintf(output, "beq %s, x0, _WHILE_EXIT_LABEL_%d\n", int_avail_regs[boolExpr->place].name, while_tmp);
        freeIntRegister(boolExpr->place);
    }
    else if(boolExpr->dataType == FLOAT_TYPE){
        int tmp_float_reg = getFloatRegister();
        int tmp_int_reg = getIntRegister();
        fprintf(output, "fmv.w.x %s, x0\n", float_avail_regs[tmp_float_reg].name);
        fprintf(output, "feq.s %s, %s, %s\n", int_avail_regs[tmp_int_reg].name, float_avail_regs[boolExpr->place].name, float_avail_regs[tmp_float_reg].name);
        fprintf(output, "bne %s, x0, _WHILE_EXIT_LABEL_%d\n", int_avail_regs[tmp_int_reg].name, while_tmp);
        freeFloatRegister(tmp_float_reg);
        freeIntRegister(tmp_int_reg);
        freeFloatRegister(boolExpr->place);
    }
    else{
        puts("boolExpr's datatype should be int or float");
    }
    genStmt(body, AR_offset);
    fprintf(output, "j _WHILE_LABEL_%d\n", while_tmp);
    fprintf(output, "_WHILE_EXIT_LABEL_%d:\n", while_tmp);
}

void genForStmt(AST_NODE* for_node, int* AR_offset) {

}

void genAssignmentStmt(AST_NODE* assignment_node) {
    AST_NODE* left = assignment_node->child;
    AST_NODE* right = left->rightSibling;
    genExprRelated(right);
    SymbolTableEntry* left_entry = left->semantic_value.identifierSemanticValue.symbolTableEntry;
    int offset_reg = getIntRegister();
    if (left_entry->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR) {
        AST_NODE* dim = left->child;
        int* size_of_dimension = left_entry->attribute->attr.typeDescriptor->properties.arrayProperties.sizeInEachDimension;
        int next_index = 1;
        int max_index = left_entry->attribute->attr.typeDescriptor->properties.arrayProperties.dimension;
        fprintf(output, "addi %s, x0, 0\n", int_avail_regs[offset_reg].name);
        while (dim) {
            genExprRelated(dim);
            fprintf(output, "add %s, %s, %s\n", int_avail_regs[offset_reg].name, int_avail_regs[offset_reg].name, int_avail_regs[dim->place].name);
            if (next_index < max_index) {
                fprintf(output, "li %s, %d\n", int_avail_regs[dim->place].name, size_of_dimension[next_index++]);
                fprintf(output, "mul %s, %s, %s\n", int_avail_regs[offset_reg].name, int_avail_regs[offset_reg].name, int_avail_regs[dim->place].name);
            }
            else
                fprintf(output, "slli %s, %s, 2\n", int_avail_regs[offset_reg].name, int_avail_regs[offset_reg].name);
            freeIntRegister(dim->place);
            dim = dim->rightSibling;
        }
    }
    else {
        fprintf(output, "mv %s, x0\n", int_avail_regs[offset_reg].name);
    }
    if(left_entry->nestingLevel == 0){
        if(left->dataType == INT_TYPE){
            if(right->dataType == FLOAT_TYPE){
                int tmp_int_reg = getIntRegister();
                int tmp_address_reg = getIntRegister();
                fprintf(output, "fcvt.w.s %s, %s\n", int_avail_regs[tmp_int_reg].name, float_avail_regs[right->place].name);
                fprintf(output, "la %s, _GLOBAL_%s\n", int_avail_regs[tmp_address_reg].name, left_entry->name);
                fprintf(output, "add %s, %s, %s\n", int_avail_regs[tmp_address_reg].name, int_avail_regs[tmp_address_reg].name, int_avail_regs[offset_reg].name);
                fprintf(output, "sw %s, 0(%s)\n", int_avail_regs[tmp_int_reg].name, int_avail_regs[tmp_address_reg].name);
                freeIntRegister(tmp_int_reg);
                freeIntRegister(tmp_address_reg);
                freeFloatRegister(right->place);
            }
            else{
                int tmp_address_reg = getIntRegister();
                fprintf(output, "la %s, _GLOBAL_%s\n", int_avail_regs[tmp_address_reg].name, left_entry->name);
                fprintf(output, "add %s, %s, %s\n", int_avail_regs[tmp_address_reg].name, int_avail_regs[tmp_address_reg].name, int_avail_regs[offset_reg].name);
                fprintf(output, "sw %s, 0(%s)\n", int_avail_regs[right->place].name, int_avail_regs[tmp_address_reg].name);
                freeIntRegister(tmp_address_reg);
                freeIntRegister(right->place);
            }
        }
        else if(left->dataType == FLOAT_TYPE){
            if(right->dataType == INT_TYPE){
                int tmp_float_reg = getFloatRegister();
                fprintf(output, "fcvt.s.w %s, %s\n", float_avail_regs[tmp_float_reg].name, int_avail_regs[right->place].name);
                fprintf(output, "la %s, _GLOBAL_%s\n", int_avail_regs[right->place].name, left_entry->name);
                fprintf(output, "add %s, %s, %s\n", int_avail_regs[right->place].name, int_avail_regs[right->place].name, int_avail_regs[offset_reg].name);
                fprintf(output, "fsw %s, 0(%s)\n", float_avail_regs[tmp_float_reg].name, int_avail_regs[right->place].name);
                freeFloatRegister(tmp_float_reg);
                freeIntRegister(right->place);
            }
            else{
                int tmp_address_reg = getIntRegister();
                fprintf(output, "la %s, _GLOBAL_%s\n", int_avail_regs[tmp_address_reg].name, left_entry->name);
                fprintf(output, "add %s, %s, %s\n", int_avail_regs[tmp_address_reg].name, int_avail_regs[tmp_address_reg].name, int_avail_regs[offset_reg].name);
                fprintf(output, "fsw %s, 0(%s)\n", float_avail_regs[right->place].name, int_avail_regs[tmp_address_reg].name);
                freeFloatRegister(right->place);
                freeIntRegister(tmp_address_reg);
            }
        }
        else{

        }
    }
    else{
        //genVariable(left);
        // offset_reg: the array offset
        // tmp_offset_reg: the stack offset
        int tmp_address_reg = getIntRegister();
        int tmp_offset_reg = getIntRegister();
        fprintf(output, ".data\n_CONSTANT_%d: .word %d\n.text\n", const_count, left_entry->offset);
        fprintf(output, "lw %s, _CONSTANT_%d\n", int_avail_regs[tmp_offset_reg].name, const_count++);
        fprintf(output, "add %s, %s, %s\n", int_avail_regs[tmp_address_reg].name, int_avail_regs[tmp_offset_reg].name, int_avail_regs[offset_reg].name);
        fprintf(output, "add %s, %s, fp\n", int_avail_regs[tmp_address_reg].name, int_avail_regs[tmp_address_reg].name);
        if(left->dataType == INT_TYPE){
            if(right->dataType == FLOAT_TYPE){
                int tmp_int_reg = getIntRegister();
                fprintf(output, "fcvt.w.s %s, %s\n", int_avail_regs[tmp_int_reg].name, float_avail_regs[right->place].name);
                fprintf(output, "sw %s, 0(%s)\n", int_avail_regs[tmp_int_reg].name, int_avail_regs[tmp_address_reg].name);
                freeIntRegister(tmp_int_reg);
                freeFloatRegister(right->place);
            }
            else{
                fprintf(output, "sw %s, 0(%s)\n", int_avail_regs[right->place].name, int_avail_regs[tmp_address_reg].name);
                freeIntRegister(right->place);
            }
        }
        else if(left->dataType == FLOAT_TYPE){
            if(right->dataType == INT_TYPE){
                int tmp_float_reg = getFloatRegister();
                fprintf(output, "fcvt.s.w %s, %s\n", float_avail_regs[tmp_float_reg].name, int_avail_regs[right->place].name);
                fprintf(output, "fsw %s, 0(%s)\n", float_avail_regs[tmp_float_reg].name, int_avail_regs[tmp_address_reg].name);
                freeFloatRegister(tmp_float_reg);
                freeIntRegister(right->place);
            }
            else{
                fprintf(output, "fsw %s, 0(%s)\n", float_avail_regs[right->place].name, int_avail_regs[tmp_address_reg].name);
                freeFloatRegister(right->place);
            }
        }
        else{

        }
        freeIntRegister(tmp_offset_reg);
        freeIntRegister(tmp_address_reg);
    }
    freeIntRegister(offset_reg);
}

void genIfStmt(AST_NODE* ifNode, int* AR_offset) {
    AST_NODE* boolExpr = ifNode->child;
    AST_NODE* ifStmt = boolExpr->rightSibling;
    AST_NODE* elseStmt = ifStmt->rightSibling;
    genExprRelated(boolExpr);
    if(boolExpr->dataType == INT_TYPE){
        fprintf(output, "beq %s, x0, _ELSE_LABEL_%d\n", int_avail_regs[boolExpr->place].name, if_count);
        freeIntRegister(boolExpr->place);
    }
    else if(boolExpr->dataType == FLOAT_TYPE){
        int tmp_float_reg = getFloatRegister();
        int tmp_int_reg = getIntRegister();
        fprintf(output, "fmv.w.x %s, x0\n", float_avail_regs[tmp_float_reg].name);
        fprintf(output, "feq.s %s, %s, %s\n", int_avail_regs[tmp_int_reg].name, float_avail_regs[boolExpr->place].name, float_avail_regs[tmp_float_reg].name);
        fprintf(output, "bne %s, x0, _ELSE_LABEL_%d\n", int_avail_regs[tmp_int_reg].name, if_count);
        freeFloatRegister(tmp_float_reg);
        freeIntRegister(tmp_int_reg);
        freeFloatRegister(boolExpr->place);
    }
    else{
        puts("boolExpr's datatype should be int or float");
    }
    int now_if_count = if_count;
    if_count++; // or else if will use the same label
    ifStmt->rightSibling = NULL;
    genStmt(ifStmt, AR_offset);
    fprintf(output, "j _IF_EXIT_LABEL_%d\n", now_if_count);
    fprintf(output, "_ELSE_LABEL_%d:\n", now_if_count);
    ifStmt->rightSibling = elseStmt;
    genStmt(elseStmt, AR_offset);
    fprintf(output, "_IF_EXIT_LABEL_%d:\n", now_if_count);
}

void genFunctionCall(AST_NODE* function_node) {
    AST_NODE* name_node = function_node->child;
    SymbolTableEntry* id_entry = name_node->semantic_value.identifierSemanticValue.symbolTableEntry;
    if (!strcmp(name_node->semantic_value.identifierSemanticValue.identifierName, "write")) { //write is not in the table
        genWrite(name_node);
        return;
    }
    else if (!strcmp(id_entry->name, "read")) {
        genRead(function_node);
        return;
    }
    else if (!strcmp(id_entry->name, "fread")) {
        genFread(function_node);
        return;
    }
    //param passing
    int int_i = 0, float_i = 0;
    for (AST_NODE* actual_param = name_node->rightSibling->child; actual_param != NULL; actual_param = actual_param->rightSibling) {
        genExprRelated(actual_param);
        int reg_num = actual_param->place;
        if (actual_param->dataType == INT_TYPE) {
            fprintf(output, "mv a%d, %s\n", int_i++, int_avail_regs[reg_num].name);
            freeIntRegister(actual_param->place);
        }
        else if (actual_param->dataType == FLOAT_TYPE) {
            fprintf(output, "fmv.s fa%d, %s\n", float_i++, float_avail_regs[reg_num].name);
            freeFloatRegister(actual_param->place);
        }
    }
    int tmp_addr_reg = getIntRegister();
    fprintf(output, "la %s, _start_%s\n", int_avail_regs[tmp_addr_reg].name, id_entry->name);
    fprintf(output, "jalr ra, 0(%s)\n", int_avail_regs[tmp_addr_reg].name);
    freeIntRegister(tmp_addr_reg);
    if (id_entry->attribute->attr.functionSignature->returnType == INT_TYPE) {
        function_node->place = getIntRegister();
        fprintf(output, "mv %s, a0\n", int_avail_regs[function_node->place].name);
    }
    else if (id_entry->attribute->attr.functionSignature->returnType == FLOAT_TYPE) {
        function_node->place = getFloatRegister();
        fprintf(output, "fmv.s %s, fa0\n", float_avail_regs[function_node->place].name);
    }
}

void genReturnStmt(AST_NODE* return_node) {
    if (return_node->child) genExprRelated(return_node->child);
    AST_NODE* parent = return_node->parent;
    while(parent){
        if(parent->nodeType == DECLARATION_NODE){
            if(parent->semantic_value.declSemanticValue.kind == FUNCTION_DECL){
                break;
            }
        }
        parent = parent->parent;
    }
    if(return_node->dataType == INT_TYPE){
        fprintf(output, "mv a0, %s\n", int_avail_regs[return_node->child->place].name);
        freeIntRegister(return_node->child->place);
    }
    else if(return_node->dataType == FLOAT_TYPE){
        fprintf(output, "fmv.s fa0, %s\n", float_avail_regs[return_node->child->place].name);
        freeFloatRegister(return_node->child->place);
    }
    fprintf(output, "j _end_%s\n", parent->child->rightSibling->semantic_value.identifierSemanticValue.identifierName);
}

void genVariableRHS(AST_NODE* idNode){
    SymbolTableEntry* id_entry = idNode->semantic_value.identifierSemanticValue.symbolTableEntry;
    int offset_reg = getIntRegister();
    if (id_entry->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR) {
        AST_NODE* dim = idNode->child;
        int* size_of_dimension = id_entry->attribute->attr.typeDescriptor->properties.arrayProperties.sizeInEachDimension;
        int next_index = 1;
        int max_index = id_entry->attribute->attr.typeDescriptor->properties.arrayProperties.dimension;
        fprintf(output, "addi %s, x0, 0\n", int_avail_regs[offset_reg].name);
        while (dim) {
            genExprRelated(dim);
            fprintf(output, "add %s, %s, %s\n", int_avail_regs[offset_reg].name, int_avail_regs[offset_reg].name, int_avail_regs[dim->place].name);
            if (next_index < max_index) { // offset = (i1*n2 + i2)*n3 ...
                fprintf(output, "li %s, %d\n", int_avail_regs[dim->place].name, size_of_dimension[next_index++]);
                fprintf(output, "mul %s, %s, %s\n", int_avail_regs[offset_reg].name, int_avail_regs[offset_reg].name, int_avail_regs[dim->place].name);
            }
            freeIntRegister(dim->place);
            dim = dim->rightSibling;
        }
        fprintf(output, "slli %s, %s, 2\n", int_avail_regs[offset_reg].name, int_avail_regs[offset_reg].name);
        if (next_index < max_index) { //return a pointer address
            idNode->place = getIntRegister();
            if (id_entry->nestingLevel == 0) {
                fprintf(output, "la %s, _GLOBAL_%s\n", int_avail_regs[idNode->place].name, id_entry->name);
            }
            else {
                fprintf(output, ".data\n_CONSTANT_%d: .word %d\n.text\n", const_count, id_entry->offset);
                fprintf(output, "lw %s, _CONSTANT_%d\n", int_avail_regs[idNode->place].name, const_count++);
            }
            fprintf(output, "add %s, %s, %s\n", int_avail_regs[idNode->place].name, int_avail_regs[idNode->place].name, int_avail_regs[offset_reg].name);
            freeIntRegister(offset_reg);
            return;
        }
    }
    if(id_entry->nestingLevel == 0){
        if(idNode->dataType == INT_TYPE){
            idNode->place = getIntRegister();
            fprintf(output, "la %s, _GLOBAL_%s\n", int_avail_regs[idNode->place].name, id_entry->name);
            if (id_entry->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR) {
                fprintf(output, "add %s, %s, %s\n", int_avail_regs[idNode->place].name, int_avail_regs[idNode->place].name, int_avail_regs[offset_reg].name);
            }
            fprintf(output, "lw %s, 0(%s)\n", int_avail_regs[idNode->place].name, int_avail_regs[idNode->place].name);
        }
        else if(idNode->dataType == FLOAT_TYPE){
            int tmp_reg = getIntRegister();
            idNode->place = getFloatRegister();
            fprintf(output, "la %s, _GLOBAL_%s\n", int_avail_regs[tmp_reg].name, id_entry->name);
            if (id_entry->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR) {
                fprintf(output, "add %s, %s, %s\n", int_avail_regs[tmp_reg].name, int_avail_regs[tmp_reg].name, int_avail_regs[offset_reg].name);
            }
            fprintf(output, "flw %s, 0(%s)\n", float_avail_regs[idNode->place].name, int_avail_regs[tmp_reg].name);
            freeIntRegister(tmp_reg);
        }
        else{

        }
    }
    else{
        if(idNode->dataType == INT_TYPE){
            idNode->place = getIntRegister();
            int tmp_reg = getIntRegister();
            fprintf(output, ".data\n_CONSTANT_%d: .word %d\n.text\n", const_count, id_entry->offset);
            fprintf(output, "lw %s, _CONSTANT_%d\n", int_avail_regs[tmp_reg].name, const_count++);
            if (id_entry->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR) {
                fprintf(output, "add %s, %s, %s\n", int_avail_regs[tmp_reg].name, int_avail_regs[offset_reg].name, int_avail_regs[tmp_reg].name);
            }
            fprintf(output, "add %s, %s, fp\n", int_avail_regs[tmp_reg].name, int_avail_regs[tmp_reg].name);
            fprintf(output, "lw %s, 0(%s)\n", int_avail_regs[idNode->place].name, int_avail_regs[tmp_reg].name);
            freeIntRegister(tmp_reg);
        }
        else if(idNode->dataType == FLOAT_TYPE){
            idNode->place = getFloatRegister();
            int tmp_reg = getIntRegister();
            fprintf(output, ".data\n_CONSTANT_%d: .word %d\n.text\n", const_count, id_entry->offset);
            fprintf(output, "lw %s, _CONSTANT_%d\n", int_avail_regs[tmp_reg].name, const_count++);
            if (id_entry->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR) {
                fprintf(output, "add %s, %s, %s\n", int_avail_regs[tmp_reg].name, int_avail_regs[offset_reg].name, int_avail_regs[tmp_reg].name);
            }
            fprintf(output, "add %s, %s, fp\n", int_avail_regs[tmp_reg].name, int_avail_regs[tmp_reg].name);
            fprintf(output, "flw %s, 0(%s)\n", float_avail_regs[idNode->place].name, int_avail_regs[tmp_reg].name);
            freeIntRegister(tmp_reg);
        }
        else{

        }
    }
    freeIntRegister(offset_reg);
}

void genVariableLHS(AST_NODE* idNode){

}

void genConstValue(AST_NODE* constValueNode){
    int reg;
    int tmp_reg;
    fprintf(output, ".data\n");
    int* cvt;
    switch (constValueNode->semantic_value.const1->const_type) {
        case INTEGERC:
            reg = getIntRegister();
            fprintf(output, "_CONSTANT_%d: .word %d\n", const_count, constValueNode->semantic_value.const1->const_u.intval);
            //align
            fprintf(output, ".text\n");
            fprintf(output, "lw %s, _CONSTANT_%d\n", int_avail_regs[reg].name, const_count++);
            break;
        case FLOATC:
            reg = getFloatRegister();
            tmp_reg = getIntRegister();
            fprintf(output, "_CONSTANT_%d: .float %.38lf\n", const_count, constValueNode->semantic_value.const1->const_u.fval);
            //align
            fprintf(output, ".text\n");
            fprintf(output, "la %s, _CONSTANT_%d\n", int_avail_regs[tmp_reg].name, const_count++);
            fprintf(output, "flw %s, 0(%s)\n", float_avail_regs[reg].name, int_avail_regs[tmp_reg].name);
            freeIntRegister(tmp_reg);
            break;
        case STRINGC:
            reg = getIntRegister();
            fprintf(output, "_CONSTANT_%d: .string %s\n", const_count, constValueNode->semantic_value.const1->const_u.sc);
            //align
            fprintf(output, ".text\n");
            fprintf(output, "la %s, _CONSTANT_%d\n", int_avail_regs[reg].name, const_count++);
            break;
        default:
            break;
    }
    constValueNode->place = reg;
}

void genAssignOrExpr(AST_NODE* assignOrExprRelatedNode){
    if(assignOrExprRelatedNode->nodeType == STMT_NODE){
        if(assignOrExprRelatedNode->semantic_value.stmtSemanticValue.kind == ASSIGN_STMT){
            genAssignmentStmt(assignOrExprRelatedNode);
        }
        else if(assignOrExprRelatedNode->semantic_value.stmtSemanticValue.kind == FUNCTION_CALL_STMT){
            genFunctionCall(assignOrExprRelatedNode);
        }
    }
    else{
        genExprRelated(assignOrExprRelatedNode);
    }
}

void genValuateBinaryExprValue(AST_NODE* exprNode){
    AST_NODE* left = exprNode->child;
    AST_NODE* right = left->rightSibling;
    if(left->dataType == INT_TYPE && right->dataType == INT_TYPE){
        exprNode->place = getIntRegister();
        switch (exprNode->semantic_value.exprSemanticValue.op.binaryOp)
        {
        case BINARY_OP_ADD:
            fprintf(output, "add %s, %s, %s\n", int_avail_regs[exprNode->place].name, int_avail_regs[left->place].name, int_avail_regs[right->place].name);
            break;
        case BINARY_OP_SUB:
            fprintf(output, "sub %s, %s, %s\n", int_avail_regs[exprNode->place].name, int_avail_regs[left->place].name, int_avail_regs[right->place].name);
            break;
        case BINARY_OP_MUL:
            fprintf(output, "mul %s, %s, %s\n", int_avail_regs[exprNode->place].name, int_avail_regs[left->place].name, int_avail_regs[right->place].name);
            break;
        case BINARY_OP_DIV:
            fprintf(output, "div %s, %s, %s\n", int_avail_regs[exprNode->place].name, int_avail_regs[left->place].name, int_avail_regs[right->place].name);
            break;
        case BINARY_OP_EQ:
            fprintf(output, "sub %s, %s, %s\n", int_avail_regs[exprNode->place].name, int_avail_regs[left->place].name, int_avail_regs[right->place].name);
            fprintf(output, "sltiu %s, %s, 1\n", int_avail_regs[exprNode->place].name, int_avail_regs[exprNode->place].name);
            break;
        case BINARY_OP_GE:
            fprintf(output, "slt %s, %s, %s\n", int_avail_regs[exprNode->place].name, int_avail_regs[left->place].name, int_avail_regs[right->place].name);
            fprintf(output, "seqz %s, %s\n", int_avail_regs[exprNode->place].name, int_avail_regs[exprNode->place].name);
            break;
        case BINARY_OP_LE:
            fprintf(output, "slt %s, %s, %s\n", int_avail_regs[exprNode->place].name, int_avail_regs[right->place].name, int_avail_regs[left->place].name);
            fprintf(output, "seqz %s, %s\n", int_avail_regs[exprNode->place].name, int_avail_regs[exprNode->place].name);
            break;
        case BINARY_OP_NE:
            fprintf(output, "sub %s, %s, %s\n", int_avail_regs[exprNode->place].name, int_avail_regs[left->place].name, int_avail_regs[right->place].name);
            fprintf(output, "sltiu %s, %s, 1\n", int_avail_regs[exprNode->place].name, int_avail_regs[exprNode->place].name);
            fprintf(output, "seqz %s, %s\n", int_avail_regs[exprNode->place].name, int_avail_regs[exprNode->place].name);
            break;
        case BINARY_OP_GT:
            fprintf(output, "slt %s, %s, %s\n", int_avail_regs[exprNode->place].name, int_avail_regs[right->place].name, int_avail_regs[left->place].name);
            break;
        case BINARY_OP_LT:
            fprintf(output, "slt %s, %s, %s\n", int_avail_regs[exprNode->place].name, int_avail_regs[left->place].name, int_avail_regs[right->place].name);
            break;
        case BINARY_OP_AND:
            fprintf(output, "mv %s, x0\n", int_avail_regs[exprNode->place].name);
            fprintf(output, "beq %s, x0, _SHORT_CIRCUIT_%d\n", int_avail_regs[left->place].name, short_circuit_count);
            fprintf(output, "sltiu %s, %s, 1\n", int_avail_regs[exprNode->place].name, int_avail_regs[right->place].name);
            fprintf(output, "xori %s, %s, 1\n", int_avail_regs[exprNode->place].name, int_avail_regs[exprNode->place].name);
            fprintf(output, "j _SHORT_CIRCUIT_%d\n", short_circuit_count);
            fprintf(output, "_SHORT_CIRCUIT_%d:\n", short_circuit_count);
            short_circuit_count++;
            break;
        case BINARY_OP_OR:
            fprintf(output, "addi %s, x0, 1\n", int_avail_regs[exprNode->place].name);
            fprintf(output, "bne %s, x0, _SHORT_CIRCUIT_%d\n", int_avail_regs[left->place].name, short_circuit_count);
            fprintf(output, "sltiu %s, %s, 1\n", int_avail_regs[exprNode->place].name, int_avail_regs[right->place].name);
            fprintf(output, "xori %s, %s, 1\n", int_avail_regs[exprNode->place].name, int_avail_regs[exprNode->place].name);
            fprintf(output, "j _SHORT_CIRCUIT_%d\n", short_circuit_count);
            fprintf(output, "_SHORT_CIRCUIT_%d:\n", short_circuit_count);
            short_circuit_count++;
            break;
        default:
            puts("unexpected binary op kind in genValuateBinaryExperValue");
            break;
        }
        freeIntRegister(left->place);
        freeIntRegister(right->place);
    }
    else{
        int tmp_reg;
        exprNode->place = getFloatRegister();
        if(left->dataType == INT_TYPE){
            freeIntRegister(left->place);
            left->place = getFloatRegister();
        }
        if(right->dataType == INT_TYPE){
            freeIntRegister(right->place);
            left->place = getFloatRegister();
        }
        switch (exprNode->semantic_value.exprSemanticValue.op.binaryOp)
        {
        case BINARY_OP_ADD:
            fprintf(output, "fadd.s %s, %s, %s\n", float_avail_regs[exprNode->place].name, float_avail_regs[left->place].name, float_avail_regs[right->place].name);
            break;
        case BINARY_OP_SUB:
            fprintf(output, "fsub.s %s, %s, %s\n", float_avail_regs[exprNode->place].name, float_avail_regs[left->place].name, float_avail_regs[right->place].name);
            break;
        case BINARY_OP_MUL:
            fprintf(output, "fmul.s %s, %s, %s\n", float_avail_regs[exprNode->place].name, float_avail_regs[left->place].name, float_avail_regs[right->place].name);
            break;
        case BINARY_OP_DIV:
            fprintf(output, "fdiv.s %s, %s, %s\n", float_avail_regs[exprNode->place].name, float_avail_regs[left->place].name, float_avail_regs[right->place].name);
            break;
        case BINARY_OP_EQ:
            freeFloatRegister(exprNode->place);
            exprNode->place = getIntRegister();
            fprintf(output, "feq.s %s, %s, %s\n", int_avail_regs[exprNode->place].name, float_avail_regs[left->place].name, float_avail_regs[right->place].name);
            break;
        case BINARY_OP_GE:
            freeFloatRegister(exprNode->place);
            exprNode->place = getIntRegister();
            fprintf(output, "fle.s %s, %s, %s\n", int_avail_regs[exprNode->place].name, float_avail_regs[right->place].name, float_avail_regs[left->place].name);
            break;
        case BINARY_OP_LE:
            freeFloatRegister(exprNode->place);
            exprNode->place = getIntRegister();
            fprintf(output, "fle.s %s, %s, %s\n", int_avail_regs[exprNode->place].name, float_avail_regs[left->place].name, float_avail_regs[right->place].name);
            break;
        case BINARY_OP_NE:
            freeFloatRegister(exprNode->place);
            exprNode->place = getIntRegister();
            fprintf(output, "feq.s %s, %s, %s\n", int_avail_regs[exprNode->place].name, float_avail_regs[left->place].name, float_avail_regs[right->place].name);
            fprintf(output, "seqz %s, %s\n", int_avail_regs[exprNode->place].name, int_avail_regs[exprNode->place].name);
            break;
        case BINARY_OP_GT:
            freeFloatRegister(exprNode->place);
            exprNode->place = getIntRegister();
            fprintf(output, "flt.s %s, %s, %s\n", int_avail_regs[exprNode->place].name, float_avail_regs[right->place].name, float_avail_regs[left->place].name);
            break;
        case BINARY_OP_LT:
            freeFloatRegister(exprNode->place);
            exprNode->place = getIntRegister();
            fprintf(output, "flt.s %s, %s, %s\n", int_avail_regs[exprNode->place].name, float_avail_regs[left->place].name, float_avail_regs[right->place].name);
            break;
        case BINARY_OP_AND:
            freeFloatRegister(exprNode->place);
            exprNode->place = getIntRegister();
            tmp_reg = getFloatRegister();
            fprintf(output, "fcvt.s.w %s, x0\n", float_avail_regs[tmp_reg].name);
            fprintf(output, "feq.s %s, %s, %s\n", int_avail_regs[exprNode->place].name, float_avail_regs[left->place].name, float_avail_regs[tmp_reg].name);
            fprintf(output, "seqz %s, %s\n", int_avail_regs[exprNode->place].name, int_avail_regs[exprNode->place].name);
            fprintf(output, "beq %s, x0, _SHORT_CIRCUIT_%d\n", int_avail_regs[exprNode->place].name, short_circuit_count);
            fprintf(output, "feq.s %s, %s, %s\n", int_avail_regs[exprNode->place].name, float_avail_regs[right->place].name, float_avail_regs[tmp_reg].name);
            fprintf(output, "seqz %s, %s\n", int_avail_regs[exprNode->place].name, int_avail_regs[exprNode->place].name);
            fprintf(output, "j _SHORT_CIRCUIT_%d\n", short_circuit_count);
            fprintf(output, "_SHORT_CIRCUIT_%d:\n", short_circuit_count);
            short_circuit_count++;
            freeFloatRegister(tmp_reg);
            break;
        case BINARY_OP_OR:
            freeFloatRegister(exprNode->place);
            exprNode->place = getIntRegister();
            tmp_reg = getFloatRegister();
            fprintf(output, "fcvt.s.w %s, x0\n", float_avail_regs[tmp_reg].name);
            fprintf(output, "feq.s %s, %s, %s\n", int_avail_regs[exprNode->place].name, float_avail_regs[left->place].name, float_avail_regs[tmp_reg].name);
            fprintf(output, "seqz %s, %s\n", int_avail_regs[exprNode->place].name, int_avail_regs[exprNode->place].name);
            fprintf(output, "bne %s, x0, _SHORT_CIRCUIT_%d\n", int_avail_regs[exprNode->place].name, short_circuit_count);
            fprintf(output, "feq.s %s, %s, %s\n", int_avail_regs[exprNode->place].name, float_avail_regs[right->place].name, float_avail_regs[tmp_reg].name);
            fprintf(output, "seqz %s, %s\n", int_avail_regs[exprNode->place].name, int_avail_regs[exprNode->place].name);
            fprintf(output, "j _SHORT_CIRCUIT_%d\n", short_circuit_count);
            fprintf(output, "_SHORT_CIRCUIT_%d:\n", short_circuit_count);
            short_circuit_count++;
            freeFloatRegister(tmp_reg);
            break;
        default:
            puts("unexpected binary op kind in genValuateBinaryExperValue");
            break;
        }
        freeFloatRegister(left->place);
        freeFloatRegister(right->place);
    }
}

void genValuateUnaryExprValue(AST_NODE* exprNode){
    AST_NODE* child = exprNode->child;
    int tmp_reg;
    if(child->dataType == INT_TYPE){
        exprNode->place = getIntRegister();
        switch (exprNode->semantic_value.exprSemanticValue.op.unaryOp)
        {
        case UNARY_OP_POSITIVE:
            fprintf(output, "mv %s, %s\n", int_avail_regs[exprNode->place].name, int_avail_regs[child->place].name);
            break;
        case UNARY_OP_NEGATIVE:
            fprintf(output, "sub %s, x0, %s\n", int_avail_regs[exprNode->place].name, int_avail_regs[child->place].name);
            break;
        case UNARY_OP_LOGICAL_NEGATION:
            fprintf(output, "seqz %s, %s\n", int_avail_regs[exprNode->place].name, int_avail_regs[child->place].name);
            break;
        default:
            puts("unexpected unary op kind in genValuateUnaryExprValue");
            break;
        }
        freeIntRegister(child->place);
    }
    else if(child->dataType == FLOAT_TYPE){
        switch (exprNode->semantic_value.exprSemanticValue.op.unaryOp)
        {
        case UNARY_OP_POSITIVE:
            exprNode->place = getFloatRegister();
            fprintf(output, "fmv.x.w %s, %s\n", float_avail_regs[exprNode->place].name, float_avail_regs[child->place].name);
            break;
        case UNARY_OP_NEGATIVE:
            exprNode->place = getFloatRegister();        
            fprintf(output, "fneg.s %s, %s\n", float_avail_regs[exprNode->place].name, float_avail_regs[child->place].name);
            break;
        case UNARY_OP_LOGICAL_NEGATION:
            tmp_reg = getFloatRegister();
            exprNode->place = getIntRegister();
            fprintf(output, "fcvt.s.w %s, x0\n", float_avail_regs[tmp_reg].name);
            fprintf(output, "feq.s %s, %s, %s\n", int_avail_regs[exprNode->place].name, float_avail_regs[tmp_reg].name, float_avail_regs[child->place].name);
            freeFloatRegister(tmp_reg);
            break;
        default:
            puts("unexpected unary op kind in genValuateUnaryExprValue");
            break;
        }
        freeFloatRegister(child->place);
    }
    else{
        puts("unexpected data type in genValuateUnaryExprValue");
    }
}

void genExpr(AST_NODE* exprNode){
    if(exprNode->semantic_value.exprSemanticValue.kind == BINARY_OPERATION){
        AST_NODE* left = exprNode->child;
        AST_NODE* right = left->rightSibling;
        genExprRelated(left);
        genExprRelated(right);
        genValuateBinaryExprValue(exprNode);
        if(left->dataType == INT_TYPE) freeIntRegister(left->place);
        else if(left->dataType == FLOAT_TYPE) freeFloatRegister(left->place);
        if(right->dataType == INT_TYPE) freeIntRegister(right->place);
        else if(right->dataType == FLOAT_TYPE) freeFloatRegister(right->place);
    }
    else if(exprNode->semantic_value.exprSemanticValue.kind == UNARY_OPERATION){
        genExprRelated(exprNode->child);
        genValuateUnaryExprValue(exprNode);
        if(exprNode->child->dataType == INT_TYPE) freeIntRegister(exprNode->child->place);
        else if(exprNode->child->dataType == FLOAT_TYPE) freeFloatRegister(exprNode->child->place);
    }
    else{
        puts("unexpected op kind in genExpr");
    }
}

void genExprRelated(AST_NODE* operandNode) {
    switch (operandNode->nodeType)
    {
    case IDENTIFIER_NODE:
        genVariableRHS(operandNode);
        break;
    case STMT_NODE:
        if(operandNode->semantic_value.stmtSemanticValue.kind == FUNCTION_CALL_STMT){
            genFunctionCall(operandNode);
        }
        else if(operandNode->semantic_value.stmtSemanticValue.kind == ASSIGN_STMT){
            genAssignmentStmt(operandNode);
        }
        else{
            operandNode->dataType = ERROR_TYPE;
        }
        break;
    case EXPR_NODE:
        genExpr(operandNode);
        break;
    case CONST_VALUE_NODE:
        genConstValue(operandNode);
        break;
    default:
        puts("unexpected nodeType in genExprRelated");
        operandNode->dataType = ERROR_TYPE;
        break;
    }
}

void genWrite(AST_NODE *node){
	AST_NODE *param_list_node = node->rightSibling;
    AST_NODE *param_node = param_list_node->child;
	genExprRelated(param_node);
    int reg_num = param_node->place;
	if(param_node->dataType == INT_TYPE) {
		fprintf(output, "mv a0, %s\n", int_avail_regs[reg_num].name);
		fprintf(output, "call _write_int\n");
	    freeIntRegister(reg_num);
	}
	else if(param_node->dataType == FLOAT_TYPE) {
		fprintf(output, "fmv.s fa0, %s\n", float_avail_regs[reg_num].name);
		fprintf(output, "call _write_float\n");
        freeFloatRegister(reg_num);
	}
	else if(param_node->dataType == CONST_STRING_TYPE) {
		fprintf(output, "mv a0, %s\n", int_avail_regs[reg_num].name);
		fprintf(output, "call _write_str\n");
        freeIntRegister(reg_num);
	}
}
    
void genRead(AST_NODE* function_node) {
    function_node->place = getIntRegister();
    fprintf(output, "call _read_int\n");
    fprintf(output, "mv %s, a0\n", int_avail_regs[function_node->place].name);
}
void genFread(AST_NODE* function_node) {
    function_node->place = getFloatRegister();
    fprintf(output, "call _read_float\n");
    fprintf(output, "fmv.s %s, fa0\n", float_avail_regs[function_node->place].name);
}
