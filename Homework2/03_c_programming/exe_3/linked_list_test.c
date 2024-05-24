#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "linked_list.h"

#define MAX_NUM_ITEMS 5
#define MAX_NUM_LIST_ITEMS 5

typedef struct {
  ListItem list;
  int info;
} IntListItem;

// print method (late binding)
void IntListItem_print(struct ListItem* item){
  printf("[int] %d\n",((IntListItem*)item)->info);
}

// vtable for int list (an INSTANCE)
ListItemOps int_list_item_ops={
  .dtor_fn=0,
  .print_fn=IntListItem_print
};

typedef struct {
  ListItem list;
  float f;
} FloatListItem;

// print method (late binding)
void FloatListItem_print(struct ListItem* item){
  printf("[float] %f\n",((FloatListItem*)item)->f);
}

// vtable for float list (an INSTANCE)
ListItemOps float_list_item_ops={
  .dtor_fn=0,
  .print_fn=FloatListItem_print
};

typedef struct{
  ListItem list;
  ListHead ll; //list of list item
} ListListItem;

// print method (late binding)
void ListListItem_print(struct ListItem* item){
  ListListItem* listlist = (ListListItem *) item;
  List_print(&(listlist->ll));
}

// vtable for float list (an INSTANCE)
ListItemOps list_list_item_ops={
  .dtor_fn=0,
  .print_fn=ListListItem_print
};


int main(int argc, char** argv) {
  // we populate the list, by inserting MAX_NUM_ITEMS

  ListHead* head = (ListHead*)malloc(sizeof(ListHead));
  List_init(head);

  for (int i=0; i<MAX_NUM_LIST_ITEMS;i++){
    ListListItem* listlist = (ListListItem*)malloc(sizeof(ListListItem));
    ListItem_construct((ListItem*) listlist, &list_list_item_ops);
    List_insert(head,head->last,(ListItem*)listlist);

    for (int j=0; j<MAX_NUM_ITEMS; ++j){
      IntListItem* item=(IntListItem*)malloc(sizeof(IntListItem));
      ListItem_construct((ListItem*) item, &int_list_item_ops);
      item->info=j;
      List_insert(&(listlist->ll), listlist->ll.last, item);
    }
  }

  List_print(head);

}
