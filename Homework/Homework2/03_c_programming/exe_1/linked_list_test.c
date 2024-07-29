#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "linked_list.h"
#define MAX_ROWS 9
#define MAX_COLS 8

int main(int argc, char** argv) {
  ListHead* head = (ListHead*)malloc(sizeof(ListHead)*MAX_ROWS);

  for(int i = 0; i<MAX_ROWS;i++){
    List_init(&(head[i]));
  }

  for (int i=0; i<MAX_ROWS; ++i){

    for(int j=0; j<MAX_COLS; ++j){
      IntListItem* new_element= (IntListItem*)malloc(sizeof(IntListItem));
      if (! new_element) {
        printf("out of memory\n");
        break;
      }

      new_element->list.prev=0;
      new_element->list.next=0;
      new_element->rows=i;
      new_element->cols=j;

      ListItem* result = List_insert(&(head[i]), head[i].last, (ListItem*) new_element);
      assert(result);
    }
  }
  for(int i = 0; i<MAX_ROWS;i++){
    IntList_print(&(head[i])); 
  }

  ListHead head_result;
  List_init(&head_result);

  for (int i=0; i<MAX_ROWS; ++i){

    int sum = 0;

    ListItem * succ = head[i].first;

    for(int j=0; j<MAX_COLS; ++j){
      IntListItem* j_esimo = (IntListItem*) succ;
      sum += (j_esimo->rows + j_esimo->cols);
      succ = succ->next;

    }

    IntListItemRes* new_element= (IntListItemRes*)malloc(sizeof(IntListItemRes));
    if (! new_element) {
      printf("out of memory\n");
      break;
    }

    new_element->list.prev=0;
    new_element->list.next=0;
    new_element->sum = sum;

    ListItem* result = List_insert(&head_result, head_result.last, (ListItem*) new_element);
    assert(result);
    
  }

  printf("\n");

  IntList_print_Res(&head_result); 

}
