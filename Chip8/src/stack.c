#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/stack.h"

#define STACK_BUFFER_SIZE 0x10
#define stack_size(stack_ptr) ( (stack_ptr)->top - (stack_ptr)->base )

int n = 1;

void init_stack(Stack *stack_ptr) {
    stack_ptr->base = malloc(sizeof(void *) * STACK_BUFFER_SIZE * n);
    stack_ptr->top = stack_ptr->base;
}

int is_empty_stack(Stack *stack_ptr) {
    return stack_ptr->top == stack_ptr->base;
}

int stack_push(Stack *stack_ptr, void *data_ptr) {
    if (stack_size(stack_ptr) >= STACK_BUFFER_SIZE) {
        Stack *new_ptr = realloc(stack_ptr, sizeof(void *) * STACK_BUFFER_SIZE * ++n);
        if (new_ptr == NULL) {
            perror("Stack Reallocation Failed, Out of Memory");
            return 0;
        }
        stack_ptr = new_ptr;
    }

    void *ptr = malloc(sizeof(ptr));
    memcpy(ptr, data_ptr, sizeof(data_ptr));
    *stack_ptr->top = ptr;

    stack_ptr->top++;
    return 1;
}

void* stack_pop(Stack *stack_ptr) {
    if (is_empty_stack(stack_ptr))
        return NULL;

    stack_ptr->top--;
    return *stack_ptr->top;
}

void free_stack(Stack *stack_ptr) {
    free(stack_ptr->top);
    free(stack_ptr->base);
    free(stack_ptr);
    n = 1;
}
