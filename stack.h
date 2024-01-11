/*
* api for handle for action related file
* feature: search, original_path
*/
#ifndef __STACK_H__
#define __STACK_H__


struct Stack { 
	int top; 
	unsigned capacity; 
	char **array; 
}; 

struct Stack* createStack(unsigned capacity);

int isFull(struct Stack *stack);

int isEmpty(struct Stack *stack);

void push(struct Stack *stack, char *item);

char* pop(struct Stack *stack);

char* peek(struct Stack *stack);

#endif