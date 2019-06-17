#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "value.h"
#include "talloc.h"
#include "linkedlist.h"
/*
check if the given character c is a number
*/
int isNumber(char c){
    return (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' ||
        c == '5' || c == '6' || c == '7' || c == '8' || c == '9');
}

/*
check if the given character c is a letter
*/
int isLetter(char c){
    return ((c > 64 && c < 91) || (c > 96 && c < 123));
}

/*
check if the given character c can begin a symbol
*/
int isInitial(char c){
    return (isLetter(c) || c == '!' || c == '$' || c == '%' || c == '&' || 
            c == '*' || c == '/' || c == ':' || c == '<' || c == '=' || 
            c == '>' || c == '?' || c == '~' || c == '_' || c == '^');
}

/*
check if the given character c is a valid non-starter symbol character
*/
int isSubsequent(char c){
    return (isInitial(c) || isNumber(c) || c == '.' || c == '+' || c == '-');
}

/*
check if the given character c is whitespace
*/
int isWhitespace(char c){
    return (c == '\n' || c == '\t' || c == ' ');
}

Value *tokenize() {
    char charRead;
    Value *list = makeNull();
    charRead = fgetc(stdin);
    int quote = 0;
    int bufferSize = 50;
    char *buffer = talloc(sizeof(char)*bufferSize);// Creating a char buffer 
    while (charRead != EOF) {
        // reset set the buffer size
        bufferSize = 50;
        // Checking the open
        if (charRead == '(') {
            Value *val = makeNull();
            val->type =OPEN_TYPE;
            list = cons(val,list);
        // Checking the close    
        } else if (charRead == ')') {
            Value *val = makeNull();
            val->type =CLOSE_TYPE;
            list = cons(val,list);    
        }
        // Do nothing when encounter a space or "\n"
        else if (charRead == ' '| charRead == '\n') {   }
        //Check +-*/ 
        else if (charRead == '+' | charRead == '-' | charRead == '*' | charRead == '/') {
            Value *val = makeNull();
            val->type =SYMBOL_TYPE;
            char *arth = talloc(sizeof(char)*2);;
            arth[0] = charRead;
            arth[1] = '\0';
            val->s = arth;
            list = cons(val,list);
        // Checking numbers
        } else if (isNumber(charRead)) {
            int seenPeriod = 0;
            int count = 1;
            //add first thing to buffer (to check if it's + or - later)
            buffer[0] = charRead;
            charRead = fgetc(stdin);
            //while we haven't yet reached the end of the number
            while (!isWhitespace(charRead) && charRead != ')' && charRead != '(' && charRead != EOF){
                //check if next thing is digit or period
                if (isNumber(charRead) || charRead == '.'){
                    // if it's a period, make sure it's the only period
                    if (charRead == '.'){
                        if (seenPeriod){
                            printf("Bad Syntax: too many decimal points in the number\n");
                            texit(EXIT_FAILURE);
                        }
                        else {
                            seenPeriod = 1;
                        }
                    }
                    // if string too long for storage buffer, increase size
                    if (count >= bufferSize){
                        bufferSize += 50;
                        char *temp = talloc(sizeof(char)*bufferSize);
                        strcpy(temp,buffer);
                        buffer = temp;
                    }
                    buffer[count] = charRead;
                    count++;  
                }
                else {
                    printf("Bad Syntax: Not a Number\n");
                    texit(1);
                }
                charRead = fgetc(stdin);
            }
            //we got one too many things, so put one back
            ungetc(charRead, stdin);
            buffer[count] = '\0';
            count++; 
            //create a string of the correct size, and copy
            //the bufferArray we have into that string
            char *finalStr = talloc(sizeof(char)*count);
            strcpy(finalStr,buffer);
            Value *node = talloc(sizeof(Value));
            //change that string into a float or int, based on type
            if (seenPeriod){
                float finalFloat;
                finalFloat = atof(finalStr);   
                node->type = DOUBLE_TYPE;
                node->d = finalFloat;
            }
            else{
                int finalInt;
                finalInt = atoi(finalStr);     
                node->type = INT_TYPE;
                node->i = finalInt;
            }
            list = cons(node, list);   
        // Check boolean
        } else if (charRead == '#') {
            charRead = fgetc(stdin);
            if (charRead == EOF){
                printf("Error: cannot be tokenized \n");
                printf("Bad Syntax in: %c \n",charRead);
                texit(1);
            } 
            Value *val = makeNull();
            val->type =BOOL_TYPE; 
            if (charRead == 't'){
                val->s = "#t";
                list = cons(val,list);
            } else if (charRead == 'f') {
                val->s = "#f";
                list = cons(val,list);
            }
            // Checking comments
        } else if (charRead == ';'){
            while (charRead != '\n' && charRead != EOF){
                charRead = fgetc(stdin);
            }
        //Checking Strings

        }else if (charRead == '\"') {
            int count = 0;
            charRead = fgetc(stdin);
            // While we have not reached the end of the string, add chars to the buffer
            while (charRead != '\"'){
                // If the file ends before the end of the string, error
                if(charRead == EOF){
                    printf("Bad Syntax: encountered EOF in middle of string\n");
                    texit(EXIT_FAILURE);
                }
                // if string is too long for storage buffer, make the buffer bigger
                if (count + 2 >= bufferSize){
                    bufferSize += bufferSize;
                    char *temp = talloc(sizeof(char)*bufferSize);
                    strcpy(temp,buffer);
                    buffer = temp;
                }
                char nextChar = fgetc(stdin);
                // If the next character is a quote and cur char is a backslash, 
                // it's an escaped quote, not the end of the string
                if(nextChar == '\"' && charRead == '\\'){
                        buffer[count] = charRead;
                        buffer[count + 1] = nextChar;
                        count++;
                        
                        charRead = fgetc(stdin);
                } else {
                    buffer[count] = charRead;
                    charRead = nextChar;
                }
                count++;
            }
            // We have reached the end of the string, null terminate it 
            // and copy into a new char array
            buffer[count] = '\0';
            count++;
            char *finalStr = talloc(sizeof(char)*count);
            strcpy(finalStr,buffer);
            
            // Create a new node, put the string in it, add to the linked list
            Value *node = talloc(sizeof(Value));
            node->type = STR_TYPE;
            node->s = finalStr;
            list = cons(node, list);
        // Checking the Symbol

        } else if(isInitial(charRead)){
            int count = 0;
            while (!isWhitespace(charRead) && charRead != ')' && charRead != '(' && charRead != EOF){               
                if (charRead == EOF){
                    printf("Bad Syntax: unexpected EOF\n");
                    texit(1);                 
                } //check next thing is a valid char for symbol type
                if (isSubsequent(charRead)){
                    // if string too long for storage buffer, increase size
                    if (count >= bufferSize){
                        bufferSize += 50;
                        char *temp = talloc(sizeof(char)*bufferSize);
                        strcpy(temp,buffer);
                        buffer = temp;
                    }
                    buffer[count] = charRead;
                    count++;
                }
                else {
                    printf("Bad syntax: Not a valid symbol %c\n", charRead);
                    texit(1);
                }
                charRead = fgetc(stdin);
            }
            ungetc(charRead, stdin);
            buffer[count] = '\0';
            count++;
            //put symbol in new array of proper size
            char *string = talloc(sizeof(char)*count);
            strcpy(string,buffer);
            //store it in token list
            Value *node = talloc(sizeof(Value));
            node->type = SYMBOL_TYPE;
            node->s = string;
            list = cons(node, list);     
        }
        // unknown character
        else {
            printf("Bad syntax: Character %c unknown\n", charRead);
            texit(1);  
        }
        charRead = fgetc(stdin);
    }
    Value *revList = reverse(list);
    return revList;
}

void displayTokens(Value *list) {   
        switch(list->type){
            case NULL_TYPE:
                printf("NULL_TYPE\n");
                break;
            case CONS_TYPE:
                printf("CONS_TYPE\n");
                displayTokens(car(list));
                displayTokens(cdr(list));
                break;
            case INT_TYPE:
                printf("%i : integer\n", list->i);
                break;
            case DOUBLE_TYPE:
                printf("%f : float\n", list->d);
                break;
            case STR_TYPE:
                printf("\"%s\" : string\n", list->s);
                break;
            case BOOL_TYPE:
                printf("%s : boolean\n", list->s);
                break;
            case OPEN_TYPE:
                printf("( : open\n");
                break;
            case CLOSE_TYPE:
                printf(") : close\n");
                break;
            case SYMBOL_TYPE:
                printf("%s : symbol\n", list->s);
                break;
            case PTR_TYPE:
                break;
            case CLOSURE_TYPE:
                printf("a CLOSURE_TYPE\n");
                break;
            case PRIMITIVE_TYPE:
                printf("a PRIMITIVE_TYPE\n");
                break;
            case VOID_TYPE:
                printf("a VOID_TYPE\n");
                break;
            default:
                printf("default\n");
                break;
    }
}
