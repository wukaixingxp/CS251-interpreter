#include <stdlib.h>
#include <stdio.h>
#include "value.h"
#include "linkedlist.h"
Value *tTree;
int started = 0;
void start(){
    if (!started){
    tTree= calloc(sizeof(Value),1);
    tTree->type = NULL_TYPE;
    started = 1; 
    }  
}

Value *tCons(Value *car, Value *cdr){
    Value *v;
    v = calloc(sizeof(Value),1);
    v->type = CONS_TYPE;
    v->c.car = car;
    v->c.cdr = cdr;
    return v;
}
// Replacement for malloc that stores the pointers allocated. It should store
// the pointers in some kind of list; a linked list would do fine, but insert
// here whatever code you'll need to do so; don't call functions in the
// pre-existing linkedlist.h. Otherwise you'll end up with circular
// dependencies, since you're going to modify the linked list to use talloc.
void *talloc(size_t size){
    start(); 
    Value *new = malloc(size);
    Value *pointer = malloc(sizeof(Value));
    pointer->type = PTR_TYPE;
    pointer->p = new;
    tTree = tCons(pointer,tTree);
    return new;
}

// Free all pointers allocated by talloc, as well as whatever memory you
// allocated in lists to hold those pointers.
void tfree(){
    int done = 0;
    Value *carV;
    Value *cdrV;
    while(!done){
        switch (tTree->type) {
            case INT_TYPE:
                free(tTree);
                done = 1;
                break;
            case DOUBLE_TYPE:
                free(tTree);
                done = 1;
                break;
            case NULL_TYPE:
                free(tTree);
                done = 1;
                break;
            case STR_TYPE:
                free(tTree);
                free(tTree->s);
                done = 1;
                break;
            case PTR_TYPE:
                free(tTree);
                free(tTree->p);
                done = 1;
                break;
            case CONS_TYPE: 
                carV = car(tTree);
                cdrV = cdr(tTree);
                if (carV-> type == PTR_TYPE){
                    free(carV->p);
                }
                free(carV);
                free(tTree);
                tTree = cdrV;
                break;
            case OPEN_TYPE:
                free(tTree);
                done = 1;
                break;
            case CLOSE_TYPE:
                free(tTree);
                done = 1;
                break;
            case BOOL_TYPE:
                free(tTree);
                done = 1;
                break;
            case SYMBOL_TYPE:
                free(tTree);
                done = 1;
                break;
            case PRIMITIVE_TYPE:
                free(tTree);
                done = 1;
                break;
            case VOID_TYPE:
                free(tTree);
                done = 1;
                break;
            default:
                free(tTree);
                done = 1;
                break;
        }
    }
    started = 0;
}

// Replacement for the C function "exit", that consists of two lines: it calls
// tfree before calling exit. It's useful to have later on; if an error happens,
// you can exit your program, and all memory is automatically cleaned up.
void texit(int status){
    tfree();
    exit(status);
}



