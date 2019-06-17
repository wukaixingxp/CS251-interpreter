//This is a scheme interpreter created by Kaixing Wu and Lucy Wu
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "talloc.h"
#include "linkedlist.h"
#include "value.h"
#include "tokenizer.h"
#include "parser.h"
#include "interpreter.h"
//Helper function for counting the length:
int countLength(Value *args){
    int count = 0;
    Value *cur = args;
    while (cur->type != NULL_TYPE) {
        count++;
        cur = cdr(cur);
    }
    return count;
}


/*** Primitives ***/
void bindPrimitives(Frame *);
void bind(char *name, Value *(*function)(struct Value *), Frame *);
Value *primitiveAdd(Value *);
Value *primitiveSub(Value *);
Value *primitiveMult(Value *);
Value *primitiveDiv(Value *);
Value *primitiveMod(Value *);
Value *primitiveGreater(Value *);
Value *primitiveLess(Value *);
Value *primitiveEq(Value *);
Value *primitiveNull(Value *);
Value *nullHelper(Value *);
Value *primitiveCar(Value *);
Value *primitiveCdr(Value *);
Value *primitiveCons(Value *);

// Function that would initialize a new Frame and set its parent frame
Frame *newFrame(Frame *parent) {
    Frame *f = talloc(sizeof(Frame));
    f->parent = parent;
    f->bindings = makeNull();
    return f;
}
// Print the all the frames
void printFrame(Frame *frame){
    int count = 0;
    bool done = 0;
    while (!done){
        printf("this is %i level \n",count);
        displayTokens(frame->bindings);
        count++;
        if(frame->parent == NULL){
            done = 1;
        }else{
            frame = frame->parent;
        }
    }
}
// Function that would display the content of tree
void displayResult(Value *list){
    switch (list->type) {
        case INT_TYPE:
            printf("%i \n",list->i);
            break;
        case DOUBLE_TYPE:
            printf("%f \n",list->d);
            break;
        case NULL_TYPE:
            break;
        case STR_TYPE:
            printf("%s \n",list->s);
            break;
        case CONS_TYPE:
            displayResult(car(list));
            displayResult(cdr(list));
            break;
        case PTR_TYPE:
            printf("%p \n",list->p);
            break;
        case OPEN_TYPE:
            printf("%s \n", list->s);
            break;
        case CLOSE_TYPE:
            printf("%s \n", list->s);
            break;
        case BOOL_TYPE:
            printf("%s \n", list->s);
            break;
        case SYMBOL_TYPE:
            printf("%s \n", list->s);
            break;
        case CLOSURE_TYPE:
            printf("a CLOSURE_TYPE\n");
            break;
        default:
            break;
    }
    return;
}

// Function that would print out evaluation error message
void evaluationError(){
    printf("Evaluation Error\n");
}

// Main Function that calls eval on each top level of S expression
void interpret(Value *tree){
    Frame *outerFrame = newFrame(NULL);
    bindPrimitives(outerFrame); 
    // Iterate through each expression
    // and display result of evaluation
    while (tree->type != NULL_TYPE) {
        Value *result = eval(car(tree), outerFrame);
        if (result->type != VOID_TYPE) {
            if (result->type == CONS_TYPE){
            //Add one more CONS_TYPE for the printing purpose.
                Value *head = makeNull();
                result = cons(result,head);
            } else if (result->type == NULL_TYPE){
                Value *empty = makeNull();
                empty->type = STR_TYPE;
                empty->s = "emp";
                result = cons(result, empty);;
            }
            printTree(result);
            printf("\n");
        }
        tree = cdr(tree);
    }
}

Value *lookUpSymbol(Value *tree,Frame *frame){
    Value *curBindings = frame->bindings;
    //Loop through all bindings
    while (curBindings->type != NULL_TYPE) {
        
        // If variable is found in binding, return its value
        if(strcmp(tree->s, car(car(curBindings))->s) == 0){
            if (cdr(car(curBindings))->type == CONS_TYPE){
                return car(cdr(car(curBindings)));
            }
            else {
                return cdr(car(curBindings));
            }
        }
        curBindings = cdr(curBindings);
    }
    
    // Error if variable is not bound
    // in either current or parent frames
    if(frame->parent == NULL){
        printf("Frame Parent Null Error 404: variable not found: \n");
        displayTokens(tree);
        texit(1);
    }
    
    // Recursive call on parent frame
    return lookUpSymbol(tree, frame->parent);
}

Value *evalIf(Value *args, Frame *frame){
    Value *current = args;
    
    // Make sure there are three arguments in if expresion
    int count = countLength(args);
    if (count != 3) {
        printf("Syntax Error: \"if\" statement does not contain three arguments.\n");
        texit(1);
        return args;
    }
    
    
    // See if condition is true or false.
    Value *statement = car(args);
    Value *truthValue = eval(statement, frame);
    // If false, evaluate third  element in args.
    if (truthValue->type == BOOL_TYPE && !strcmp(truthValue->s, "#f")){
        return eval(car(cdr(cdr(args))), frame);
    }
    // Otherwise, evaluate second element in args.
    else {
        return eval(car(cdr(args)), frame);
    }
}

Value *evalLet(Value *args, Frame *frame){
    // Initialize a new frame
    Frame *newf = newFrame(frame);
    
    // Make sure that args is a nested list
    if (args->type != CONS_TYPE ||
        car(args)->type != CONS_TYPE ||
        car(car(args))->type != CONS_TYPE) {
        printf("Syntax Error: list of bindings for let does not contain a nested list\n");
        texit(1);
        return args;
    }
    
    Value *bindList = car(args);
    
    // Loop to bind each binding to frame
    while (bindList->type != NULL_TYPE) {
        Value *current = car(bindList);
        
        // Check if each binding contains one variable and one value
        if(cdr(current)->type == NULL_TYPE || cdr(cdr(current))->type != NULL_TYPE){
            printf("Syntax Error: \"let\" variable binding is not correct.\n");
            texit(1);
            return args;
        }
        
        // Get the value of binding
        Value *val = eval(car(cdr(current)), frame);
        
        // Create binding cons cell
        Value *binding = makeNull();
        binding = cons(val, binding);
        binding = cons(car(current), binding);
        
        // Bind the binding to frame
        newf->bindings = cons(binding, newf->bindings);
        
        // Go to next binding
        bindList = cdr(bindList);
        
    }
    if(cdr(args)->type == NULL_TYPE){
        printf("Syntax Error: \"let\" statement is not formatted properly.\n");
        texit(1);
        return args;
    }
    
    // Unwrap extra cons cells to get to actual body of let and return result
    Value *result;
    if (cdr(cdr(args))->type == CONS_TYPE) {
        Value *curr = cdr(args);
        result = eval(car(curr), newf);
        if (cdr(curr)->type!= NULL_TYPE){
            result = eval(car(cdr(curr)), newf);
        }
    } else {
        result = eval(car(cdr(args)), newf);
    }
    return result;
}

Value *evalLetS(Value *args, Frame *frame){
    // Initialize a new frame and pointer to parentFrame
    Frame *cur = newFrame(frame);
    Frame *parentFrame = frame;
    // Make sure that args is a nested list
    if (args->type != CONS_TYPE ||
        car(args)->type != CONS_TYPE ||
        car(car(args))->type != CONS_TYPE) {
        printf("Syntax Error: LetS list of bindings for let does not contain a nested list\n");
        texit(1);
        return args;
    }
    
    Value *bindList = car(args);
    // Loop to bind each binding to frame
    while (bindList->type != NULL_TYPE) {
        Value *current = car(bindList);
        // Check if each binding contains one variable and one value
        if(cdr(current)->type == NULL_TYPE || cdr(cdr(current))->type != NULL_TYPE){
            printf("Syntax Error: \"LetS\" variable binding is not correct.\n");
            texit(1);
            return args;
        }
        
        // Get the value of binding
        Value *val = eval(car(cdr(current)), cur);
        
        // Create binding cons cell
        Value *binding = makeNull();
        binding = cons(val, binding);
        binding = cons(car(current), binding);
        
        // Bind the binding to frame
        cur->bindings = cons(binding, cur->bindings);
        parentFrame = cur;
        // Initialize a new frame again
        cur = newFrame(parentFrame);
        // Go to next binding
        bindList = cdr(bindList);    
    }
    
    if(cdr(args)->type == NULL_TYPE){
        printf("Syntax Error: \"LetS\" statement is not formatted properly.\n");
        texit(1);
        return args;
    }
    // Unwrap extra cons cells to get to actual body of let and return result
    Value *result = NULL;
    if (cdr(cdr(args))->type == CONS_TYPE) {
        Value *curr = cdr(cdr(args));
        while(cdr(curr)->type != NULL_TYPE){
            curr = cdr(curr);
        }
        result = eval(car(curr), cur);
    } else {
        result = eval(car(cdr(args)), cur);
    }
    return result;
}

Value *evalLetRec(Value *args, Frame *frame){
    // Initialize a new frame and pointer to parentFrame
    Frame *curFrame = newFrame(frame);
    Frame *parentFrame = frame;
    // Make sure that args is a nested list
    if (args->type != CONS_TYPE || 
        car(args)->type != CONS_TYPE || 
        car(car(args))->type != CONS_TYPE) {
        printf("Error: Letrec list of bindings for let does not contain a nested list\n");
        texit(1);
    }
    // Isolate list of bindings to make
    Value *bindList = car(args);
    // For each binding, add it to the current frame, then create a new frame
    //for the next binding
    while (bindList->type == CONS_TYPE) {
        
        Value *cur = car(bindList);
        //make sure binding has 1 variable name and 1 value
        if(cdr(cur)->type == NULL_TYPE || cdr(cdr(cur))->type != NULL_TYPE){
            printf("Error: \"let\" statement does not bind variables correctly.\n");
            texit(1);
        }
        
        // Let val be the result of evaluating cur value in 
        // Frame frame.
        Value *val = eval(car(cdr(cur)), curFrame);
        
        // Create new binding (cons cell) that includes both the variable (in car) 
        // and result of evaluation of value (in cdr)
        Value *binding = makeNull();
        binding = cons(val, binding);
        binding = cons(car(cur), binding);
        
        // Add this new binding to f->bindings
        frame->bindings = cons(binding, frame->bindings);
        
        //set the parent frame to be the current frame,
        //then create a new frame for the next binding
        parentFrame = curFrame;
        curFrame = newFrame(parentFrame);
        bindList = cdr(bindList);
    }
    
    // Evaluate body in Frame curFrame and return the result.
    // There should only be one arg after the bindings
    // but if there are more, go to the last one, else error.
    if(cdr(args)->type == NULL_TYPE){
        printf("Error: \"letRec\" statement is not formatted properly.\n");
        texit(1);
    }
    // Unwrap extra cons cells to get to actual let body and return.
    Value *toReturn = NULL;
    if (cdr(cdr(args))->type == CONS_TYPE) {
        Value *curr = cdr(cdr(args));
        while(cdr(curr)->type != NULL_TYPE){
            curr = cdr(curr);
        }
        toReturn = eval(car(curr), curFrame);
    } else {
        toReturn = eval(car(cdr(args)), curFrame);
    }
    return toReturn;
}

Value *evalQuote(Value *args){
    
    // Make sure that quote has exactly one argument
    if (args->type == NULL_TYPE) {
        printf("Syntax Error: \"quote\" need argument\n");
        texit(1);
    }
    if (cdr(args)->type != NULL_TYPE) {
        printf("Syntax Error: \"quote\" too many arguments\n");
        texit(1);
    }
    
    // Return the whole tree including "quote".
    return car(args);
}


Value *evalDefine(Value *args, Frame *frame) {
    
    // Make sure size of args is 2
    Value *current = args;
    int count = countLength(args);
    if (count != 2) {
        evaluationError();
        printf(" Syntax Error: \"define\" does not contain two arguments.\n");
        texit(1);
    }
    // Eval the next argument
    Value *result = eval(car(cdr(args)), frame);
    // Initialize new binding that binds the variable and the result value
    Value *binding = makeNull();
    binding = cons(result, binding);
    binding = cons(car(args), binding);
    // Add the binding to frame
    frame->bindings = cons(binding, frame->bindings);
    Value* final = makeNull();
    final->type = VOID_TYPE;
    return final;
}
Value *evalLambda(Value *args, Frame *frame) {
    // Make sure size of args is 2
    Value *cur = args;
    int count = countLength(args);
    if (count < 2) {
        printf("Syntax Error: \"lambda\" statement does not contain one or more arguments.\n");
        texit(1);
    } 
    // Make a new closure that contains the names of the
    // parameters for the function, the function code, and the environment.
    Value *closure = makeNull();
    closure->type = CLOSURE_TYPE;
    
    closure->cl.paramNames = car(args);
    closure->cl.functionCode = cdr(args);
    closure->cl.frame = frame;
    return closure;
}


Value *evalEach(Value *args, Frame *frame){
    // Eval each of the given arguments and return a list of
    // results
    Value *current = args;
    Value* final = makeNull();
    while (current->type != NULL_TYPE) {
        final = cons(eval(car(current),frame),final);
        current = cdr(current);
    }
    
    final = reverse(final);
    return final;
}

Value *evalCond(Value *args, Frame *frame) {
    Value *current = args;
    while (current->type != NULL_TYPE) {
        
        // Check if current is cons type
        if (current->type != CONS_TYPE) {
            printf("Syntax Error: \"cond\" not formatted correctly.\n");
            texit(1);
        }
        
        Value *curExp = car(current);
        
        if (curExp->type != CONS_TYPE) {
            printf("Syntax Error: \"cond\" not formatted correctly.\n");
            texit(1);
        }
        
        Value *condition = car(curExp);
        Value *body = cdr(curExp);
        
        // Check there is exactly one expression in the body
        if (body->type != CONS_TYPE) {
            printf("Syntax Error: \"cond\" clause does not have a body.\n");
            texit(1);
        }
        if (cdr(body)->type != NULL_TYPE) {
            printf("Syntax Error: \"cond\" body given too many arguments.\n");
            texit(1);
        }
        
        if (condition->type == SYMBOL_TYPE &&
            !strcmp(condition->s, "else")) {
            return eval(car(body), frame);
        }
        
        // Evaluate the condition if this is not the else case.
        condition = eval(condition, frame);
        
        // The condition must be a boolean if it is not the else case.
        if (condition->type != BOOL_TYPE) {
            printf("Syntax Error: \"cond\" condition does not evaluate to boolean.\n");
            texit(1);
        }
        
    
        if (!strcmp(condition->s, "#t")) {
            return eval(car(body), frame);
        }
        current = cdr(current);
    }
    // If reach the end of args but no return, return VOID_TYPE.
    Value *result = makeNull();
    result->type = VOID_TYPE;
    return result;
}


Value *evalAnd(Value *args, Frame *frame) {
    Value *current = args;
    while (current->type != NULL_TYPE) {
        Value *curEvaled = eval(car(current), frame);
        // Returns false when find an expression evaluates to false.
        if (curEvaled->type == BOOL_TYPE) {
            if (!strcmp(curEvaled->s, "#f")) {
                return curEvaled;
            }
        }
        // There should not be any non-boolean arguments.
        else {
            printf("Syntax Error: \"and\" should not have non-boolean arguments.\n");
            texit(1);
        }
        current = cdr(current);
    }
    // If reach the end of the given args without finding anything false, return true.
    Value *result = makeNull();
    result->type = BOOL_TYPE;
    result->s = "#t";
    return result;
}

Value *evalOr(Value *args, Frame *frame) {
    Value *current = args;
    while (current->type != NULL_TYPE) {
        Value *curEvaled = eval(car(current), frame);
         // Returns true when find an expression evaluates to true.
        if (curEvaled->type == BOOL_TYPE) {
            if (!strcmp(curEvaled->s, "#t")) {
                return curEvaled;
            }
        }
        // There should not be any non-boolean arguments.
        else {
            printf("Syntax Error: \"or\" should not have non-boolean arguments.\n");
            texit(1);
        }
        current = cdr(current);
    }
    // If reach the end of the given args without finding anything true, return false.
    Value *result = makeNull();
    result->type = BOOL_TYPE;
    result->s = "#f";
    return result;
}


Value *evalSet(Value *args, Frame *frame) {  
    // Check if size of args is 2
    int count = countLength(args);
    Value *cur = args;
    if (count != 2) {
        printf("Syntax Error: \"set!\" statement does not contain two arguments.\n");
        texit(1);
    } 
    // Let vali be the result of evaluating value in cur in frame frame.
    Value *vali = eval(car(cdr(args)), frame);
    
    Value *symbolToChange = car(args);
    
    //find the appropriate binding in frame/parent frames
    //iterate through the bindings in frame->bindings
    Frame *curFrame = frame;
    int foundMatch = 0;
     while(curFrame != NULL){
        Value *bindingList = curFrame->bindings;
        Value *curBinding = car(bindingList);
        //check all levels of bindings

        while(curBinding->type != NULL_TYPE){
            if(!strcmp(car(curBinding)->s, symbolToChange->s)){
                //change binding value by creating new one to replace
                
                foundMatch = 1;
                Value *newBinding = makeNull();
                newBinding = cons(vali, newBinding);
                newBinding = cons(car(curBinding), newBinding);

                //make the pointer to the old binding now point to the new binding
                curBinding = newBinding;
                curFrame->bindings = cons(curBinding, curFrame->bindings);
            }
            bindingList = cdr(bindingList);
            if(bindingList->type != NULL_TYPE){
                curBinding = car(bindingList);
            }
            else{
                break;
            }
        }
        curFrame = curFrame->parent;
    }
    if(foundMatch == 0){
        printf("Syntax Error: \"set!\" must modify an existing symbol.\n");
        texit(1);
    }  
    Value* toReturn = makeNull();
    toReturn->type = VOID_TYPE;    
    return toReturn;
}


Value *evalBegin(Value *args, Frame *frame) {
    // Check if the size of args is 2
    Value *cur = args;
    int count = countLength(args);
    if (count < 1) {
        printf("Syntax Error: \"begin\" does not contain two arguments.\n");
        texit(1);
    }
    
    // Eval each statement in the function code
    Value *command = args;
    cur = car(command);
    while(cur->type != NULL_TYPE){
        eval(cur, frame);
        command = cdr(command);
        if(command->type != NULL_TYPE){
            cur = car(command);
        }
        else{
            break;
        }
    }
    
    // Get the last thing in the list of things that happen in the closure and return that
    Value *last = car(reverse(args));
    
    return eval(last, frame);
}



void bindPrimitives(Frame *frame){
    bind("+", primitiveAdd, frame);
    bind("-", primitiveSub, frame);
    bind("*", primitiveMult, frame);
    bind("/", primitiveDiv, frame);
    bind(">", primitiveGreater, frame);
    bind("<", primitiveLess, frame);
    bind("=", primitiveEq, frame);
    bind("modulo", primitiveMod, frame);
    bind("null?", primitiveNull, frame);
    bind("car", primitiveCar, frame);
    bind("cdr", primitiveCdr, frame);
    bind("cons", primitiveCons, frame);
}



void bind(char *name, Value *(*function)(struct Value *), Frame *frame) {
    
    Value *nameHolder = talloc(sizeof(Value));
    nameHolder->type = SYMBOL_TYPE;
    nameHolder->s = name;
    
    // Add primitive functions to top-level bindings list
    Value *value = talloc(sizeof(Value));
    value->type = PRIMITIVE_TYPE;
    value->pf = function;
    Value *binding = makeNull();
    binding = cons(value, binding);
    binding = cons(nameHolder, binding);
    frame->bindings = cons(binding, frame->bindings);
}

Value *primitiveMult(Value *multList){
    double result = 1.0;
    //check number of args, if 0 return 0, if 1 return that, otherwise add them
    
    //loop through all arguments, add them
    while(multList->type != NULL_TYPE){
        Value *number = car(multList);
        //check to make sure args are ints or doubles
        if (number->type != INT_TYPE &&
            number->type != DOUBLE_TYPE) {
            printf("Syntax Error: Mult expect a INT_TYPE or DOUBLE_TYPE!\n");
            texit(1);
        }
        if(number->type == INT_TYPE){
            result = result * number->i;
        }
        else {
            result = result * number->d;
        }
        multList = cdr(multList);
    }
    Value *total = talloc(sizeof(Value));
    total->type = DOUBLE_TYPE;
    total->d = result;
    
    return total;
}

Value *primitiveSub(Value *subList){
    //Checking the length of sublist.
    double first,second;
    if (length(subList) != 2){
        printf("Syntax Error: Subtraction expect only two arguments!\n");
        texit(1);
    }
    Value *firstArg = car(subList);
    Value *secondArg = car(cdr(subList));
    //Checking the type of sublist, only allow INT_TYPE and DOUBLE_TYPE.
    if ((firstArg->type != INT_TYPE && firstArg->type != DOUBLE_TYPE) || 
        (secondArg->type != INT_TYPE && secondArg->type != DOUBLE_TYPE)){
        printf("Syntax Error: Subtraction expect a INT_TYPE or DOUBLE_TYPE!\n");
        texit(1);
    }
    if (firstArg->type == INT_TYPE){
        first = (double)firstArg->i;
    }
    else {
        first = firstArg->d;
    }
    if (secondArg->type == INT_TYPE){
        second = (double)secondArg->i;
    }
    else {
        second = secondArg->d;
    }
    double result = first - second;
    Value *total = talloc(sizeof(Value));
    total->type = DOUBLE_TYPE;
    total->d = result;
    return total;
}

Value *primitiveDiv(Value *divList){
        //Checking the length of sublist.
    double first,second;
    if (length(divList) != 2){
        printf("Syntax Error: Division expect only two arguments!\n");
        texit(1);
    }
    Value *firstArg = car(divList);
    Value *secondArg = car(cdr(divList));
    //Checking the type of sublist, only allow INT_TYPE and DOUBLE_TYPE.
    if ((firstArg->type != INT_TYPE && firstArg->type != DOUBLE_TYPE) || 
        (secondArg->type != INT_TYPE && secondArg->type != DOUBLE_TYPE)){
        printf("Syntax Error: Division expect a INT_TYPE or DOUBLE_TYPE!\n");
        texit(1);
    }
    if (firstArg->type == INT_TYPE){
        first = (double)firstArg->i;
    }
    else {
        first = firstArg->d;
    }
    if (secondArg->type == INT_TYPE){
        second = (double)secondArg->i;
    }
    else {
        second = secondArg->d;
    }
    double result = first / second;
    Value *total = talloc(sizeof(Value));
    total->type = DOUBLE_TYPE;
    total->d = result;
    return total;
}

Value *primitiveAdd(Value *addList){
    double result = 0.0;
    //check number of args, if 0 return 0, if 1 return that, otherwise add them
    
    //loop through all arguments, add them
    while(addList->type != NULL_TYPE){
        Value *number = car(addList);
        //check to make sure args are ints or doubles
        if (number->type != INT_TYPE &&
            number->type != DOUBLE_TYPE) {
            printf("Syntax Error: Add expect a INT_TYPE or DOUBLE_TYPE!\n");
            texit(1);
        }
        if(number->type == INT_TYPE){
            result += number->i;
        }
        else {
            result += number->d;
        }
        addList = cdr(addList);
    }
    Value *total = talloc(sizeof(Value));
    total->type = DOUBLE_TYPE;
    total->d = result;
    
    return total;
}

Value *primitiveMod(Value *modList){
    //Checking the length of sublist.
    int first,second;
    if (length(modList) != 2){
        printf("Syntax Error: Subtraction expect only two arguments!\n");
        texit(1);
    }
    Value *firstArg = car(modList);
    Value *secondArg = car(cdr(modList));
    //Checking the type of sublist, only allow INT_TYPE and DOUBLE_TYPE.
    if (firstArg->type != INT_TYPE || secondArg->type != INT_TYPE ){
        printf("Syntax Error: Mod expect a INT_TYPE!\n");
        texit(1);
    }
    first = firstArg->i;
    second =secondArg->i;
    Value *total = talloc(sizeof(Value));
    total->type = INT_TYPE;
    total->i = first%second;
    return total;
}

Value *primitiveGreater(Value *list){
    //Checking the length of sublist.
    double first,second;
    if (length(list) != 2){
        printf("Syntax Error: Greater comparison expect only two arguments!\n");
        texit(1);
    }
    Value *firstArg = car(list);
    Value *secondArg = car(cdr(list));
    //Checking the type of sublist, only allow INT_TYPE and DOUBLE_TYPE.
    if ((firstArg->type != INT_TYPE && firstArg->type != DOUBLE_TYPE)|| 
        (secondArg->type != INT_TYPE && secondArg->type != DOUBLE_TYPE) ){
        printf("Syntax Error: Greater comparison expect a INT_TYPE or DOUBLE_TYPE!\n");
        texit(1);
    }
    if (firstArg->type == INT_TYPE){
        first = (double)firstArg->i;
    }
    else {
        first = firstArg->d;
    }
    if (secondArg->type == INT_TYPE){
        second = (double)secondArg->i;
    }
    else {
        second = secondArg->d;
    }
    Value *total = talloc(sizeof(Value));
    total->type = BOOL_TYPE;
    if (first > second){
        total->s = "#t";
    }else{
        total->s = "#f";
    }
    return total;
}

Value *primitiveLess(Value *list){
    //Checking the length of sublist.
    double first,second;
    if (length(list) != 2){
        printf("Syntax Error: Less comparison expect only two arguments!\n");
        texit(1);
    }
    Value *firstArg = car(list);
    Value *secondArg = car(cdr(list));
    //Checking the type of sublist, only allow INT_TYPE and DOUBLE_TYPE.
    if ((firstArg->type != INT_TYPE && firstArg->type != DOUBLE_TYPE)|| 
        (secondArg->type != INT_TYPE && secondArg->type != DOUBLE_TYPE)){
        printf("Syntax Error: Less comparison expect a INT_TYPE or DOUBLE_TYPE!\n");
        texit(1);
    }
    if (firstArg->type == INT_TYPE){
        first = (double)firstArg->i;
    }
    else {
        first = firstArg->d;
    }
    if (secondArg->type == INT_TYPE){
        second = (double)secondArg->i;
    }
    else {
        second = secondArg->d;
    }
    Value *total = talloc(sizeof(Value));
    total->type = BOOL_TYPE;
    if (first >= second){
        total->s = "#f";
    }else{
        total->s = "#t";
    }
    return total;
}

Value *primitiveEq(Value *list){
    //Checking the length of sublist.
    double first,second;
    if (length(list) != 2){
        printf("Syntax Error: equal comparison expect only two arguments!\n");
        texit(1);
    }
    Value *firstArg = car(list);
    Value *secondArg = car(cdr(list));
    //Checking the type of sublist, only allow INT_TYPE and DOUBLE_TYPE.
    if ((firstArg->type != INT_TYPE && firstArg->type != DOUBLE_TYPE)|| 
        (secondArg->type != INT_TYPE && secondArg->type != DOUBLE_TYPE) ){
        printf("Syntax Error: equal comparison expect a INT_TYPE or DOUBLE_TYPE!\n");
        texit(1);
    }
    if (firstArg->type == INT_TYPE){
        first = (double)firstArg->i;
    }
    else {
        first = firstArg->d;
    }
    if (secondArg->type == INT_TYPE){
        second = (double)secondArg->i;
    }
    else {
        second = secondArg->d;
    }
    Value *total = talloc(sizeof(Value));
    total->type = BOOL_TYPE;
    if (first == second){
        total->s = "#t";
    }else{
        total->s = "#f";
    }
    return total;
}
Value *primitiveNull(Value *args) {
    if(args->type == NULL_TYPE){
        printf("Syntax Error: Wrong number of args for null check.\n");
        texit(1);
    }
    //verify that there is only one arg
    if(args->type == CONS_TYPE){
        if(cdr(args)->type == CONS_TYPE && car(cdr(args))->type != NULL_TYPE){
            printf("Syntax Error: Wrong number of args for null check.\n");
            texit(1);
        }
        else if(cdr(args)->type != NULL_TYPE){
            printf("Syntax Error: Wrong number of args for null check.\n");
            texit(1);
        }
    }
    return nullHelper(args);
}




Value *nullHelper(Value *args){
    //Check to see if we're in a cons cell, if so, go deeper
    if (args->type == CONS_TYPE){
        return nullHelper(car(args));
    }
    //Base case: return if the innermost thing is null or not
    else{
        Value *result = makeNull();
        result->type = BOOL_TYPE;
        if (isNull(args)) {
            result->s = "#t";
        } else {
            result->s = "#f";
        }
        return result;
    }
}




Value *primitiveCar(Value *value){
    // Error Checking
    if (value->type != CONS_TYPE){
        printf("Syntax Error: \"car\" invalid input.\n");
        texit(1);
        return value;
    }
    if (cdr(value)->type!= NULL_TYPE){
        printf("Syntax Error: \"car\" statement expect only one argument.\n");
        texit(1);
        return value;
    }
    if (car(value)->type!= CONS_TYPE){
        printf("Syntax Error: \"car\" statement expect a CONS_TYPE argument.\n");
        texit(1);
        return value;
    }
    Value *result = car(car(value));
    return result;
}


Value *primitiveCdr(Value *value){
    // Error Checking
    if (value->type != CONS_TYPE){
        printf("Syntax Error: \"cdr\" invalid input.\n");
        texit(1);
        return value;
    }
    if (cdr(value)->type!= NULL_TYPE){
        printf("Syntax Error: \"cdr\" statement expect only one argument.\n");
        texit(1);
        return value;
    }
    if (car(value)->type!= CONS_TYPE){
        printf("Syntax Error: \"cdr\" statement expect a CONS_TYPE argument.\n");
        texit(1);
        return value;
    }
    Value *result = cdr(car(value));
    return result;
}


Value *primitiveCons(Value *value){
    
    // Error Checking
    if (value->type != CONS_TYPE){
        printf("Syntax Error: \"cons\" statement expect a CONS_TYPE argument.\n");
        texit(1);
        return value;
    }
    if (cdr(value)->type == NULL_TYPE || cdr(cdr(value))->type!= NULL_TYPE){
        printf("Syntax Error: \"cons\" statement expect only two arguments .\n");
        texit(1);
        return value;
    }
    Value *firstCon = car(value);
    Value *secondCon = cdr(value);
    if (secondCon->type == CONS_TYPE){
        secondCon = car(secondCon);
    }
    return cons(firstCon,secondCon);
}

Value *apply(Value *function, Value *args){
    assert(function->type == CLOSURE_TYPE);
    // Create a new frame
    Frame *new = newFrame(function->cl.frame);
    
    Value *current = args;
    Value *argName = function->cl.paramNames;
    // Go through all the actual argument value
    while(current->type != NULL_TYPE){
        // If no more paraname, print error
        if (argName -> type == NULL_TYPE){
            evaluationError();
            printf("Syntax Error: Apply need more paramNames.\n");
            texit(1);
        }
        Value *vcurrent = car(current);
        Value *vargName = car(argName);
        // Bind the paraname with the actual value
        Value *result = eval(vcurrent, new->parent);
        
        Value *bind = makeNull();
        
        bind = cons(result,bind);
        bind = cons(vargName,bind);
        new->bindings = cons(bind, new->bindings);
        
        current = cdr(current);
        argName = cdr(argName);
    }
    // If there are still some paraname, print error
    if (argName->type != NULL_TYPE){
        evaluationError();
        printf("Syntax Error: Apply has too many arguments.\n");
        texit(1);
    }
    //Eval each statement in the function code
    Value *command = function->cl.functionCode;
    Value *value = car(command);
    while(value->type != NULL_TYPE){
        if(value->type == CONS_TYPE && car(value)->type == SYMBOL_TYPE){
            command = cdr(command);
        }
        else{
            eval(value, new);
            command = cdr(command);
        }
        if(command->type != NULL_TYPE){
            value = car(command);
        }
        else{
            break;
        }
    }
    // Using the last commend to eval
    Value *last = car(reverse(function->cl.functionCode));
    
    return eval(last, new);
}

Value *eval(Value *tree, Frame *frame) {
    switch (tree->type)  {        
            // int, double, boolean, string all evaluate to themselves
        case INT_TYPE: {
            return tree;
            break;
        }
        case DOUBLE_TYPE:
            return tree;
            break;
        case BOOL_TYPE: {
            return tree;
            break;
        }
        case STR_TYPE: {
            return tree;
            break;
        }
        case CLOSURE_TYPE: {
            return tree;
            break;
        }
        case SYMBOL_TYPE: {
            return lookUpSymbol(tree, frame);
            break;
        }
        case CONS_TYPE: {
            // Get car and cdr of the cons cell
            Value *first = car(tree);;
            Value *args = cdr(tree);
            Value *result = talloc(sizeof(Value));
            // Check what is first and act correspondingly
            if (first->type == SYMBOL_TYPE || first->type == CONS_TYPE) {
                if (!strcmp(first->s,"if")) {
                    result = evalIf(args,frame);
                }
                else if (!strcmp(first->s,"let")) {
                    result = evalLet(args, frame);
                }
                else if (!strcmp(first->s,"quote")) {
                    result = evalQuote(args);
                }
                else if (!strcmp(first->s, "define")) {
                    result = evalDefine(args, frame);
                }
                else if (!strcmp(first->s, "lambda")) {
                    result = evalLambda(args, frame);
                }
                else if(!strcmp(first->s, "let*")){
                    result = evalLetS(args, frame);
                }
                else if(!strcmp(first->s, "letrec")){
                    result = evalLetRec(args, frame);
                }
                else if (!strcmp(first->s, "cond")) {
                    result = evalCond(args, frame);
                }
                else if (!strcmp(first->s, "and")) {
                    result = evalAnd(args, frame);
                }
                else if (!strcmp(first->s, "or")) {
                    result = evalOr(args, frame);
                }
                else if(!strcmp(first->s, "set!")){
                    result = evalSet(args, frame);
                }
                else if(!strcmp(first->s, "begin")){
                    result = evalBegin(args, frame);
                }
                else {
                    Value *evaledOperator = eval(first, frame);
                    
                    // If first is a Racket function
                    if (evaledOperator->type == CLOSURE_TYPE) {
                        Value *evaledArgs = evalEach(args, frame);
                        result = apply(evaledOperator, evaledArgs);
                    }
                    // If first is a primitive function
                    else if (evaledOperator->type == PRIMITIVE_TYPE) {
                        Value *evaledArgs = evalEach(args, frame);
                        result = evaledOperator->pf(evaledArgs);
                    }
                
                    else if (evaledOperator->type == SYMBOL_TYPE){
                        printf("Evaluation Syntax Error: This is not a recognized procedure.\n");
                        texit(1);
                    }
                
                    else{
                        return tree;
                    }
            }
            }
            else {
                return tree;
            }
            return result;
            break;
        }
            // To supress warnings
        case NULL_TYPE:{
            return tree;
            break;
        }
        case PTR_TYPE:{
            return tree;
            break;
        }
        case OPEN_TYPE:{
            return tree;
            break;
        }
        case CLOSE_TYPE:{
            return tree;
            break;
        }
        default:
            return tree;
            break;
    }
    return tree;
}