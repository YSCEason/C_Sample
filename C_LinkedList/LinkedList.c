#include <stdio.h>
#include <stdlib.h>
// #include "linkedlist.h"

typedef struct node
{
    int data;
    struct node *next;
}Node;

typedef struct list
{
    Node *head;
}List;

Node *createnode(int data);

Node *createnode(int data){
    Node *newNode = malloc(sizeof(Node));

    if (!newNode)
    {
        return NULL;
    }
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

List *makelist(){
    List *list = malloc(sizeof(List));

    if (!list)
    {
        return NULL;
    }
    list->head = NULL;
    return list;
}

void display(List *list) {
    Node *current = list->head;

    if(list->head == NULL)
        return;

    for(; current != NULL; current = current->next)
    {
        printf("%d, ", current->data);
    }
}

void add(int data, List *list){
    Node *current = NULL;

    if(list->head == NULL)
    {
        list->head = createnode(data);
    }
    else
    {
        current = list->head;
        while (current->next!=NULL)
        {
          current = current->next;
        }
        current->next = createnode(data);
    }
}

void delete(int data, List *list){
    Node *current = list->head;
    Node *previous = current;

    while(current != NULL)
    {
        if(current->data == data)
        {
            previous->next = current->next;
            if(current == list->head)
                list->head = current->next;
            free(current);
            return;
        }
        previous = current;
        current = current->next;
    }
}

void reverse(List *list){
    Node *reversed = NULL;
    Node *current = list->head;
    Node *temp = NULL;

    while(current != NULL)
    {
        temp = current;
        current = current->next;
        temp->next = reversed;
        reversed = temp;
    }
    list->head = reversed;
}

void destroy(List *list){
    Node *current = list->head;
    Node *next = current;

    while(current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }
    free(list);
}






int main(void){
    List *list = makelist();

    add(1, list);
    add(20, list);
    add(2, list);
    add(5, list);
    add(8, list);
    add(9, list);
    add(13, list);
    printf("Create list: ");
    display(list);

    printf("\nAdd '2': ");
    delete(2, list);
    display(list);

    printf("\ndelete '1': ");
    delete(1, list);
    display(list);

    printf("\ndelete '20': ");
    delete(20, list);
    display(list);

    reverse(list);
    printf("\nReversed: ");
    display(list);

    destroy(list);

    return 0;
}
