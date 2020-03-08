//
// Created by lionheart on 8/10/19.
//

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef SCHEDULERS_LINKED_LIST_H
#define SCHEDULERS_LINKED_LIST_H

typedef struct Node{
  int value;
  struct Node * next;
} Node_t;

//prints the ids of the package
void print_list(Node_t* head);
void push_back(Node_t** head, int value);
void push_front(Node_t** head, int value);

//returns the id of the popped element
int pop_front(Node_t** head);
int pop_back(Node_t* head);
int remove_at(Node_t** head, int index);

//returns the package at a position
int get_at(Node_t* head, int index);

//sets the package at a position
int set_at(Node_t *head,  int index, int value);

//swaps the content of two nodes
void swap(Node_t* head, int index1, int index2);

//returns the length of the list
int get_length(Node_t* head);





#endif //SCHEDULERS_LINKED_LIST_H