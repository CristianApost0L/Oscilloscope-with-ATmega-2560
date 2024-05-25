#include <stdio.h>
#include "disastrOS_syscalls.h"


//Reverse the order of ready list and return the new running process
void internal_revertAndPreempt() {
  assert(running && "internal_revertAndPreempt|ERROR, no running process\n");
  disastrOS_debug("internal_revertAndPreempt|current running [ %d ] -> ", running->pid);

  //ia check if ready list contains something -- TODO return an error
  if (!ready_list.size || !ready_list.first) {
    //ia nothing to do here
    running->syscall_retvalue = DSOS_EREVERT_AND_PREEMPT;
    return;
  }

  disastrOS_debug("reverting ready list -> ");

  //ia cache the begin and the end of the list
  ListItem* prev_first = ready_list.first;
  ListItem* prev_last = ready_list.last;

  //ia auxiliary variables
  ListItem* current = ready_list.first; //ia this is required to scan the list
  ListItem* prev = 0;
  ListItem* next = 0;

  while (current) {
    //ia cache the next to go forward
    next = current->next;

    //ia reverse current node
    current->next = prev;
    current->prev = next;

    //ia go forward
    prev = current;
    current = next;
  }

  //ia update the begin and the end of the list
  ready_list.first = prev_last;
  ready_list.last = prev_first;
  /* PCBList_print(&ready_list); */

  //ia preempt and take the first of the new ready list
  PCB* next_process=(PCB*) List_detach(&ready_list, ready_list.first);
  running->status=Ready;
  List_insert(&ready_list, ready_list.last, (ListItem*) running);
  next_process->status=Running;
  running=next_process;

  disastrOS_debug("new running [ %d ]\n", running->pid);
  //ia return 0 is ok
  running->syscall_retvalue = 0;
}
