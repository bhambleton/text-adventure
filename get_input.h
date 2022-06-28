#ifndef _GET_INPUT_H_
#define _GET_INPUT_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct user_input {
    int length;
    char* input;
};

int alloc_line(char**, int*, int);
int get_user_input(struct user_input*);
int get_line(char**, int*, FILE*);

#endif
