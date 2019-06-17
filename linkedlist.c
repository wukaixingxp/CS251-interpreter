

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "talloc.h"
#include "linkedlist.h"
#include "value.h"


// Create a new NULL_TYPE value node.
Value *makeNull() {
    Value *v;
    v = talloc(sizeof(Value));
    v->type = NULL_TYPE;
    return v;
}

// Create a new CONS_TYPE value node.
Value *cons(Value *car, Value *cdr){
    Value *v;
    v = talloc(sizeof(Value));
    v->type = CONS_TYPE;
    v->c.car = car;
    v->c.cdr = cdr;
    return v;
}

// Display the contents of the linked list to the screen in some kind of
// readable format
void display(Value *list){
    switch (list->type) {
        case INT_TYPE:
            printf("%i:integer\n",list->i);
            break;
        case DOUBLE_TYPE:
            printf("%f:float \n",list->d);
            break;
        case NULL_TYPE:
            break;
        case STR_TYPE:
            printf("%s:string \n",list->s);
            break;
        case CONS_TYPE:
            display(car(list));
            display(cdr(list));
            break;
        case PTR_TYPE:
            printf("%p:pointer \n",list->p);
            break;
        case OPEN_TYPE:
            printf("%s:open", list->s);
        case CLOSE_TYPE:
            printf("%s:close", list->s);
        case BOOL_TYPE:
            printf("%i:boolean", list->i);
        case SYMBOL_TYPE:
            printf("%s:symbol", list->s);
        default:
            break;
    }
    return;
}


void copy_string(char *target, char *source) {
    while (*source) {
        *target = *source;
        source++;
        target++;
    }
    *target = '\0';
}

Value *clone(Value *v){
    Value *new;
    new = makeNull();
    new->type = v->type;
    switch (v->type) {
        case INT_TYPE:
            new->i = v->i;
            break;
        case DOUBLE_TYPE:
            new->d = v->d;
            break;
        case NULL_TYPE:
            break;
        case STR_TYPE:
            new->s = v->s;
            break;
        case CONS_TYPE:
            printf("Cloning a con\n");
            new->c = v->c;
            break;
        case PTR_TYPE:
            new->p = v->p;
            break;
        case OPEN_TYPE:
            new->p = v->p;
            break;
        case CLOSE_TYPE:
            new->p = v->p;
            break;
        case BOOL_TYPE:
            new->p = v->p;
            break;
        case SYMBOL_TYPE:
            new->p = v->p;
            break;
        default:
            break;
    }
    return new;
}
// Return a new list that is the reverse of the one that is passed in. All
// content within the list should be duplicated; there should be no shared
// memory between the original list and the new one.
//
// FAQ: What if there are nested lists inside that list?
// ANS: There won't be for this assignment. There will be later, but that will
// be after we've got an easier way of managing memory.
Value *reverse(Value *list){
    Value *cur = list;
    // Create new list
    Value *new= makeNull();
    while (!isNull(cur)) {
        // Create new cons cell with pointers to old values/nested lists
        new = cons(car(cur),new);
        cur = cdr(cur);
    }
    return new;
}

// Frees up all memory directly or indirectly referred to by list. Note that
// this linked list might consist of linked lists as items, so you'll need to
// clean them up as well.
// FAQ: What if there are nested lists inside that list?
// ANS: There won't be for this assignment. There will be later, but that will
// be after we've got an easier way of managing memory.
void cleanup(Value *list){
    assert(list != NULL);
    assert(list->type == NULL_TYPE| list->type == CONS_TYPE| list->type == INT_TYPE|list->type == DOUBLE_TYPE|list->type == STR_TYPE);
    switch (list->type) {
        case INT_TYPE:
            free(list);
            break;
        case DOUBLE_TYPE:
            free(list);
            break;
        case NULL_TYPE:
            free(list);
            break;
        case STR_TYPE:
            free(list);
            break;
        case CONS_TYPE:
            cleanup(car(list));
            cleanup(cdr(list));
            free(list);
            break;
        case PTR_TYPE:
            free(list);
            break;
        case OPEN_TYPE:
            free(list);
            break;
        case CLOSE_TYPE:
            free(list);
            break;
        case BOOL_TYPE:
            free(list);
            break;
        case SYMBOL_TYPE:
            free(list);
            break;
        default:
            break;
    }
    return;
}
// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.
Value *car(Value *list){
    assert(list != NULL);
    assert(list->type == CONS_TYPE);
    return list->c.car;
}
// Utility to make it less typing to get cdr value. Use assertions to make sure
// that this is a legitimate operation.
Value *cdr(Value *list){
    assert(list != NULL);
    assert(list->type == CONS_TYPE);
    return list->c.cdr;
}
// Utility to check if pointing to a NULL_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isNull(Value *value){
    assert(value != NULL);
    assert(value->type == NULL_TYPE| value->type == CONS_TYPE| value->type == INT_TYPE|value->type == DOUBLE_TYPE|value->type == STR_TYPE);
    if (value->type == NULL_TYPE){
        return 1;
    }
    return 0;
}
// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int length(Value *value){
    switch (value->type) {
        case INT_TYPE:
            return 1;
        case DOUBLE_TYPE:
            return 1;
        case NULL_TYPE:
            return 0;
        case STR_TYPE:
            return 1;
        case PTR_TYPE:
            return 1;
        case CONS_TYPE:
            return length(car(value))+length(cdr(value));
        case OPEN_TYPE:
            return 1;
        case CLOSE_TYPE:
            return 1;
        case BOOL_TYPE:
            return 1;
        case SYMBOL_TYPE:
            return 1;
        default:
            return 0;
        
    }
}

