//
// Created by lionheart on 8/10/19.
// Updated to use "memory_t" struct by JosafatV on 9/3/20
//

#include "../include/linked_list.h"

void print_list(Node_t *head, int property) {
  Node_t *current = head;
  printf("[");
  while(current != NULL){
    if(property == 1)
      printf("%d ", current->value->status);
    else if(property == 2)
      printf("%d ", current->value->core);
    else if(property == 4)
      printf("%d ", current->value->dir_data);
    else if(property == 5)
      printf("%d ", current->value->data);
    current = current->next;
  }
  printf("]\n");
}

/** Prints the memory block of the according level
 * \param head the linked_list to print
 * \param level the memory level to print. Use 1 for all attributes
 */
void print_mem(Node_t *head, int level) {
	memory_t* current;
	int len = get_length(head);

	if (level == 0) {
		// Print L1 cache attributes
		for (int i = 0; i<len; i++){
			current = get_at(head, i);
			printf("0x0%d [", i);
			printf(" %d |", current->block);
			printf(" %d |", current->status);
			printf(" 0x%d |", current->dir_data);
			printf(" %d ]\n", current->data);
		}
	} else if (level == 2) { 
		// Print MEM chace attributes
		for (int i = 0; i<len; i++){
			current = get_at(head, i);
			if (i<10) {
				printf("0x0%d [", i);
			} else {
				printf("0x%d [", i);
			}
			printf(" %d |", current->status);
			printf(" %d |", current->core);
			//printf(" 0x%d |", current->dir_data);
			printf(" %d ]\n", current->data);
		}
	} else {
		// Print L2 cache attributes (all)
		for (int i = 0; i<len; i++){
			current = get_at(head, i);
			printf("0x0%d [", i);
			printf(" %d |", current->block);
			printf(" %d |", current->status);
			printf(" %d |", current->core);
			printf(" %d |", current->shared);
			printf(" 0x%d |", current->dir_data);
			printf(" %d ]\n", current->data);
		}
	}
}

void print_l2(Node_t *head, int property) {
  Node_t * current = head;
  printf("[");
  while(current != NULL){
    if(property == 0)
      printf("%d ", current->value->block);
    else if(property == 1)
      printf("%d ", current->value->status);
    else if(property == 2)
      printf("%d ", current->value->core);
    else if(property == 3)
      printf("%d ", current->value->shared);
    else if(property == 4)
      printf("%d ", current->value->dir_data);
    else if(property == 5)
      printf("%d ", current->value->data);
    current = current->next;
  }
  printf("]\n");
}

void print_l1(Node_t *head, int property) {
  Node_t * current = head;
  printf("[");
  while(current != NULL){
    if(property == 0)
      printf("%d ", current->value->block);
    else if(property == 1)
      printf("%d ", current->value->status);
    else if(property == 4)
      printf("%d ", current->value->dir_data);
    else if(property == 5)
      printf("%d ", current->value->data);
    current = current->next;
  }
  printf("]\n");
}

void push_back(Node_t **head, memory_t* value) {
  //if list is empty
  Node_t * current = *head;
  if (current == NULL){
    push_front(head, value);
    return;
  }
  //go to end of list
  while(current->next != NULL){
    current = current->next;
  }
  //add new element
  current->next = malloc(sizeof(Node_t));
  current->next->value =  value;
  current->next->next = NULL;
}

void push_front(Node_t **head, memory_t *value) {
  Node_t *new_node;
  //allocates memory for new node
  new_node = malloc(sizeof(Node_t));
  new_node->value =  value;
  new_node->next = *head;
  //assigns new head
  *head = new_node;
}

memory_t * pop_front(Node_t **head) {
  memory_t *retval = NULL;
  Node_t *new_head = NULL;

  //if list is empty
  if(*head == NULL){
    return retval;
  }

  new_head = (*head)->next;
  retval = (*head)->value;
  free(*head);
  *head = new_head;

  return retval;
}

memory_t* pop_back(Node_t *head) {
  memory_t * retval = NULL;

  if(get_length(head) == 0){
    printf("List is already empty. Cannot pop-back.\n");
    return retval;
  }

  //If there is only one item in the list
  if(head->next == NULL){
    retval = head->value;
    free(head);
    return retval;
  }

  //Go to the second last node
  Node_t * current = head;
  while(current->next->next != NULL){
    current = current->next;
  }

  retval = current->next->value;
  free(current->next);
  current->next = NULL;
  return retval;
}

memory_t *remove_at(Node_t **head, int index) {
  if(get_length(*head) <= index || index < 0){
    printf("Cannot remove, index out of range\n");
    return NULL;
  }
  int i = 0;
  memory_t * retval = NULL;
  Node_t * current = *head;
  Node_t * temp = NULL;

  //first element
  if(index == 0){
    return pop_front(head);
  }

  for(i = 0; i < index - 1; ++i){
    //the index is out of range
    if(current->next == NULL){
      printf("Index out of range");
      return NULL;
    }
    current = current->next;
  }

  temp = current->next;
  retval = temp->value;
  current->next = temp->next;
  free(temp);

  return retval;
}

int get_length(Node_t *head) {
  Node_t * current = head;
  int retval = 0;
  while(current != NULL){
    current = current->next;
    retval ++;
  }
  return retval;
}

memory_t* get_at(Node_t *head, int index) {
  memory_t * retval = NULL;

  if(get_length(head) <= index || index < 0){
    printf("Index out of range. Cannot get at %d.\n", index);
    return retval;
  }

  Node_t * current = head;

  for(int i = 0; i < index; ++i){
    current = current->next;
  }
  retval = current->value;

  return retval;
}

memory_t *set_at(Node_t *head,  int index, memory_t *value) {
  memory_t * retval = NULL;

  if(get_length(head) <= index || index < 0){
    printf("Index out of range. Cannot set at %d.\n", index);
    return retval;
  }

  Node_t * current = head;

  for(int i = 0; i < index; ++i){
    current = current->next;
  }
  current->value = value;
  retval = current->value;

  return retval;
}

void swap(Node_t *head, int index1, int index2) {
  int length = get_length(head);
  if(index1 >= length || index1 < 0 || index2 >= length || index2 < 0){
    printf("Index out of range. Cannot swap %d and %d.\n", index1, index2);
    return;
  }

  memory_t* temp;
  temp = get_at(head, index1);
  set_at(head, index1, get_at(head,index2));
  set_at(head,index2, temp);

}