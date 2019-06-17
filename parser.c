#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "value.h"
#include "talloc.h"
#include "linkedlist.h"
#include "tokenizer.h"

Value *addToParseTree(Value *tree, int *depth, Value *token){
    // if the token is not Close paren, just add it to the tree
    if (token->type != CLOSE_TYPE){
        if (token->type == OPEN_TYPE){
            *depth = *depth + 1;
        }
        tree = cons(token,tree);

    }
    else{
        // the token is a CLOST_TYPE, find next open paren
        Value *new = makeNull();
        assert(tree != NULL);
        Value *cur = car(tree);
        tree = cdr(tree);
        //while the current token isn't a open paren,add the
        //current token to the new tree and remove from original tree
        while (cur->type != OPEN_TYPE){
            if (*depth == 0){
                printf("Syntax error: too many close parentheses.\n");
                texit(1);
            } 
            new = cons(cur,new);
            cur = car(tree);
            tree = cdr(tree);   
        }
        //add the new tree to the original tree and decrement depth
        tree = cons(new,tree);
        *depth = *depth - 1;
    }
    return tree;
}

// Takes a list of tokens from a Racket program, and returns a pointer to a
// parse tree representing that program.
Value *parse(Value *tokens){

    Value *tree = makeNull();
    int depth = 0;
    Value *current = tokens;
    assert(current != NULL && "Error (parse): null pointer");
    while (current->type != NULL_TYPE) {
        Value *token = car(current);
        tree = addToParseTree(tree,&depth,token);
        current = cdr(current);
    }
    if (depth != 0) {
        // depth must be bigger than 0, so print the error
        printf("Syntax error: not enough close parentheses.\n");
        texit(1);      
    }
    // to print the correct tree we need to reverse it
    tree = reverse(tree);
    return tree;
}
//print the Car of the cons-type value based on its type
void printCar(Value *val) {
    switch(val->type){
            case NULL_TYPE:
                break;
            case CONS_TYPE:
                break;
            case INT_TYPE:
                printf("%i", val->i);
                break;
            case DOUBLE_TYPE:
                printf("%f", val->d);
                break;
            case STR_TYPE:
                printf("\"%s\"", val->s);
                break;
            case BOOL_TYPE:
                printf("%s", val->s);
                break;
            case OPEN_TYPE:
                break;
            case CLOSE_TYPE:
                break;
            case SYMBOL_TYPE:
                printf("%s", val->s);
                break;
            case PTR_TYPE:
                break;
            default:
                break;
    }
}
void printTree(Value *tree){
    if (tree->type == NULL_TYPE) {
        return;
    }
     //if the head is not a cons type, just print it
    else if (tree->type != CONS_TYPE) {
        printCar(tree);
        return;
    }
    //Check if the head of the tree is a CONS_TYPE
    else {
        // If the head of the tree is also a nested list, then go to the sub-tree
        if (car(tree)->type == CONS_TYPE) {
            printf("(");
            printTree(car(tree));
            printf(")");
            if (cdr(tree)->type != NULL_TYPE){
                printf(" ");
            }
            //then go back to main tree
            printTree(cdr(tree));
        }
        //If it is not a nested list, print the car and keep going
        else{
            printTree(car(tree));
            if (cdr(tree)->type != NULL_TYPE){
                printf(" ");
            }
            printTree(cdr(tree));
            }
        } 
}