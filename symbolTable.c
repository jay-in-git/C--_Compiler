#include "symbolTable.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
// This file is for reference only, you are not required to follow the implementation. //

int HASH(char * str) {
	int idx=0;
	while (*str){
		idx = idx << 1;
		idx+=*str;
		str++;
	}
	return (idx & (HASH_TABLE_SIZE-1));
}

SymbolTable symbolTable;


SymbolTableEntry* newSymbolTableEntry(int nestingLevel)
{
    SymbolTableEntry* symbolTableEntry = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
    symbolTableEntry->nextInHashChain = NULL;
    symbolTableEntry->prevInHashChain = NULL;
    symbolTableEntry->nextInSameLevel = NULL;
    symbolTableEntry->sameNameInOuterLevel = NULL;
    symbolTableEntry->attribute = NULL;
    symbolTableEntry->name = NULL;
    symbolTableEntry->nestingLevel = nestingLevel;
    symbolTableEntry->offset = 0;
    return symbolTableEntry;
}

void removeFromHashChain(int hashIndex, SymbolTableEntry* entry)
{
    /*If the entry is the head of the chain*/
    if(entry->prevInHashChain == NULL){
        /*It's also the end of the chain*/
        symbolTable.hashTable[hashIndex] = entry->nextInHashChain;
    }
    else{
        entry->prevInHashChain->nextInHashChain = entry->nextInHashChain;   
    }
    if(entry->nextInHashChain != NULL){
        entry->nextInHashChain->prevInHashChain = entry->prevInHashChain;
    }
    entry->nextInHashChain = NULL;
    entry->prevInHashChain = NULL;
}

void enterIntoHashChain(int hashIndex, SymbolTableEntry* entry)
{
    if(symbolTable.hashTable[hashIndex] == NULL){
        symbolTable.hashTable[hashIndex] = entry;
    }
    else{
        symbolTable.hashTable[hashIndex]->prevInHashChain = entry;
        entry->nextInHashChain = symbolTable.hashTable[hashIndex];
        symbolTable.hashTable[hashIndex] = entry;
    }
}

void initializeSymbolTable()
{   
    symbolTable.currentLevel = 0;
    symbolTable.scopeDisplayElementCount = 10;
    symbolTable.scopeDisplay = (SymbolTableEntry**)malloc(sizeof(SymbolTableEntry*) * symbolTable.scopeDisplayElementCount);
    for(int i=0 ; i<symbolTable.scopeDisplayElementCount ; i++){
        symbolTable.scopeDisplay[i] = NULL;
    }
    for(int i=0 ; i<HASH_TABLE_SIZE ; i++){
        symbolTable.hashTable[i] = NULL;
    }
    preInsert(SYMBOL_TABLE_VOID_NAME);
    preInsert(SYMBOL_TABLE_INT_NAME);
    preInsert(SYMBOL_TABLE_FLOAT_NAME);
    preInsert(SYMBOL_TABLE_SYS_LIB_READ);
    preInsert(SYMBOL_TABLE_SYS_LIB_FREAD);
}

void symbolTableEnd()
{
}

/*Retrieve the symbol named symbolName in the out-most scope(not necessary current scope)*/
SymbolTableEntry* retrieveSymbol(char* symbolName)
{
    int hashV = HASH(symbolName);
    SymbolTableEntry* curr_entry = symbolTable.hashTable[hashV];
    while(curr_entry != NULL){
        if(strcmp(curr_entry->name, symbolName) == 0){
            return curr_entry;
        }
        else{
            curr_entry = curr_entry->nextInHashChain;
        }
    }
    return NULL;
}

SymbolTableEntry* enterSymbol(char* symbolName, SymbolAttribute* attribute)
{
    int hashV = HASH(symbolName);
    // printf("%s with value %d\n", symbolName, hashV);
    SymbolTableEntry* curr_entry = symbolTable.hashTable[hashV];
    // printf("exist? %d\n", curr_entry != NULL);
    SymbolTableEntry* entered_entry = newSymbolTableEntry(symbolTable.currentLevel);
    entered_entry->name = symbolName;
    entered_entry->attribute = attribute; // can directly assign
    while(curr_entry != NULL){
        if(strcmp(curr_entry->name, symbolName) == 0){
            if(symbolTable.currentLevel == curr_entry->nestingLevel){
                free(entered_entry);
                return NULL;
            }
            else if(symbolTable.currentLevel > curr_entry->nestingLevel){
                removeFromHashChain(hashV, curr_entry);
                entered_entry->sameNameInOuterLevel = curr_entry;
                break;
            }
            else{
                printf("This is not supposed to be happend ...\n");
                return NULL;
            }
        }
        else{
            curr_entry = curr_entry->nextInHashChain;
        }
    }
    enterIntoHashChain(hashV, entered_entry);
    entered_entry->nextInSameLevel = symbolTable.scopeDisplay[symbolTable.currentLevel];
    symbolTable.scopeDisplay[symbolTable.currentLevel] = entered_entry;
    return entered_entry;
}

/*remove the symbol from the current scope*/
void removeSymbol(char* symbolName)
{
    int hashV = HASH(symbolName);
    SymbolTableEntry* curr_entry = symbolTable.hashTable[hashV];
    while(curr_entry != NULL){
        if(strcmp(curr_entry->name, symbolName) == 0){
            if(symbolTable.currentLevel == curr_entry->nestingLevel){
                removeFromHashChain(hashV, curr_entry);
                if(curr_entry->sameNameInOuterLevel != NULL){
                    enterIntoHashChain(hashV, curr_entry->sameNameInOuterLevel);
                }
                SymbolTableEntry* curr_scope_entry = symbolTable.scopeDisplay[symbolTable.currentLevel];                
                SymbolTableEntry* prev_scope_entry = NULL;
                while(curr_scope_entry != NULL){
                    if(strcmp(curr_scope_entry->name, curr_entry->name) == 0){
                        if(prev_scope_entry == NULL){
                            symbolTable.scopeDisplay[symbolTable.currentLevel] = curr_scope_entry->nextInSameLevel;
                        }
                        else{
                            prev_scope_entry->nextInSameLevel = curr_scope_entry->nextInSameLevel;
                        }
                    }
                    prev_scope_entry = curr_scope_entry;
                    curr_scope_entry = curr_scope_entry->nextInSameLevel;
                    // free(curr_scope_entry);
                }
                return;
            }
            else{
                printf("The %s hasn't be declared in this scope\n", symbolName);
                return;
            }
        }
        else{
            curr_entry = curr_entry->nextInHashChain;
        }
    }
    printf("There's no symobol %s in current scope\n", symbolName);
}

/*return 1 if the symbol has been declared in current scope, otherwise 0*/
int declaredLocally(char* symbolName)
{
    int hashV = HASH(symbolName);
    SymbolTableEntry* curr_entry = symbolTable.hashTable[hashV];
    // printf("find declare success? %d\n", curr_entry != NULL);
    while(curr_entry != NULL){
        if(strcmp(curr_entry->name, symbolName) == 0){
            if(symbolTable.currentLevel == curr_entry->nestingLevel){
                return 1;
            }
            else{
                return 0;
            }
        }
        curr_entry = curr_entry->nextInHashChain;
    }
    return 0;
}

void openScope()
{
    symbolTable.currentLevel += 1;
    if(symbolTable.currentLevel == symbolTable.scopeDisplayElementCount){
        SymbolTableEntry** new_scopeDisplay;
        if((new_scopeDisplay = (SymbolTableEntry**)malloc(sizeof(SymbolTableEntry*) * (symbolTable.scopeDisplayElementCount + 5))) == NULL){
            printf("No enough memory to alloc symbolTable scope array\n");
            return;
        }
        memcpy(new_scopeDisplay, symbolTable.scopeDisplay, sizeof(SymbolTableEntry*) * symbolTable.scopeDisplayElementCount);
        free(symbolTable.scopeDisplay);
        symbolTable.scopeDisplay = new_scopeDisplay;
        for(int i=symbolTable.scopeDisplayElementCount ; i<symbolTable.scopeDisplayElementCount + 5 ; i++){
            symbolTable.scopeDisplay[i] = NULL;
        }
        symbolTable.scopeDisplayElementCount += 5;
    }
}

void closeScope()
{
    if(symbolTable.currentLevel < 0){
        printf("No need to close current scope, it's already no scope\n");
        return;
    }
    SymbolTableEntry* curr_scope_entry = symbolTable.scopeDisplay[symbolTable.currentLevel];
    while(curr_scope_entry != NULL){
        int hashV = HASH(curr_scope_entry->name);
        SymbolTableEntry* next_scope_entry = curr_scope_entry->nextInSameLevel;
        removeFromHashChain(hashV, curr_scope_entry);
        if(curr_scope_entry->sameNameInOuterLevel != NULL){
            enterIntoHashChain(hashV, curr_scope_entry->sameNameInOuterLevel);
        }
        // free(curr_scope_entry);
        curr_scope_entry = next_scope_entry;
    }
    symbolTable.scopeDisplay[symbolTable.currentLevel] = NULL;
    symbolTable.currentLevel -= 1;
}

void printSymbolTable() {
    SymbolTableEntry* currEntry = symbolTable.scopeDisplay[symbolTable.currentLevel];
    puts("===================");
    while (currEntry != NULL) {
        printf("||  %2d%2d %6s  ||\n", currEntry->nestingLevel, currEntry->attribute->attributeKind, currEntry->name);
        currEntry = currEntry->nextInSameLevel;
    }
    puts("===================");

}
