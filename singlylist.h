/************************************************
 *	Creates a singly linked list
 *	
 *	Each node holds a char pointer to a C-String  of
 *		and a node pointer
 *
*************************************************/
#ifndef _SINGLYLIST_H_
#define _SINGLYLIST_H_

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define ROOM_NAME_MAX 15

struct node {
	char* name;
	struct node* next;
};

void print_list(struct node*, int);
void free_list(struct node**);
void add_node(struct node**, char*);
#endif
