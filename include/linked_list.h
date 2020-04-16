//
// Created by lionheart on 8/10/19.
// Updated to use "memory_t" struct by JosafatV on 9/3/20
//

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef SCHEDULERS_LINKED_LIST_H
#define SCHEDULERS_LINKED_LIST_H

// Unified memory struct needed for linked_list compatibility
typedef struct {	//  L1	L2	Mem  pN
  	int block;		//  X   X    	 0 		In which chip is the data
	int status;		//  X   X   X	 1		in which state is the data (enum status)
	int core;		//      X   X	 2		Which CPU "owns" the data
	int shared;		//	    X    	 3		??? flag, may be redundant with status
	int dir_data;	//  X   X   X	 4		address of the data in MEM
	int data;		//  X   X   X	 5		the user's data
} memory_t;

// node structure for the linked_list
typedef struct Node{
	memory_t* value;
	struct Node* next;
} Node_t;

// Type of status the data can have, based on the MSI protocol VALID is only for testing/developing purposes
enum status {Valid = 3, Modified = 2, Shared = 1, Invalid = 0};

//prints the ids of the package
void print_mem (Node_t* head, int property);
void print_l1 (Node_t* head, int property);
void print_l2 (Node_t* head, int property);
void push_back(Node_t** head, memory_t* value);
void push_front(Node_t** head, memory_t* value);

//returns the id of the popped element
memory_t* pop_front(Node_t** head);
memory_t* pop_back(Node_t* head);
memory_t* remove_at(Node_t** head, int index);

//returns the memory block at a position
memory_t* get_at(Node_t* head, int index);

//sets the package at a position
memory_t* set_at(Node_t *head,  int index, memory_t* value);

//swaps the content of two nodes
void swap(Node_t* head, int index1, int index2);

//returns the length of the list
int get_length(Node_t* head);




#endif //SCHEDULERS_LINKED_LIST_H