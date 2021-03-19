/************************************************
 *
 *	Singly Linked list implementation for
 *	adventure game player route
 *
*************************************************/
#ifndef _SINGLYLIST_C_
#define _SINGLYLIST_C_

#include "singlylist.h"

void
free_list (struct node** head) {
    struct node* temp = (*head);

    while(temp != NULL) {
        struct node* next = temp->next;

        if (temp->name != NULL) {
            free(temp->name);
            temp->name = NULL;
        }

        free(temp);
        temp = next;
    }

    (*head) = NULL;
}

void
print_list (struct node* head, int step_count) {
    struct node* curr = head;

    printf("\e[10A\e[0J");
    printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
    printf("YOU TOOK %d STEPS.\nYOUR PATH TO VICTORY WAS:\n", step_count);

    while (curr) {
        printf("\t%s\n", curr->name);
        curr = curr->next;
    }
}

void
add_node (struct node** head, char* name) {
    struct node* temp = (*head);

    // allocate new node
    struct node* new_node = (struct node*) malloc(sizeof(struct node));
    assert(new_node);
    new_node->next = NULL;

    //copy string contents
    new_node->name = (char*) calloc(ROOM_NAME_MAX, sizeof(char));
    assert(new_node->name);
    memcpy(new_node->name, name, ROOM_NAME_MAX);

    if (!(*head)) {
        (*head) = new_node;
        return;
    }
    else {
        // search for end of list
        while (temp->next) {
            temp = temp->next;
        }
        temp->next = new_node;
    }
}
#endif
