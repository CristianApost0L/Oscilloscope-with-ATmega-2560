#pragma once

typedef struct ListItem {
  struct ListItem* prev;
  struct ListItem* next;
} ListItem;

typedef struct ListHead {
  ListItem* first;
  ListItem* last;
  int size;
} ListHead;

typedef struct IntListItem{
  ListItem list;
  float rows;
  float cols;
} IntListItem;

typedef struct IntListItemRes{
  ListItem list;
  float sum;
} IntListItemRes;

void List_init(ListHead* head);

ListItem* List_find(ListHead* head, ListItem* item);

ListItem* List_insert(ListHead* head, ListItem* previous, ListItem* item);

ListItem* List_detach(ListHead* head, ListItem* item);

void IntList_print(ListHead* head);

void IntList_print_Res(ListHead* head);
