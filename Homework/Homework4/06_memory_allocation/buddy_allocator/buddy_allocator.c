#include <stdio.h>
#include <assert.h>
#include <math.h> // for floor and log2
#include "buddy_allocator.h"

// these are trivial helpers to support you in case you want
// to do a bitmap implementation
int levelIdx(size_t idx){
  return (int)floor(log2(idx));
};

int buddyIdx(int idx){
  if (idx&0x1){
    return idx-1;
  }
  return idx+1;
}

int parentIdx(int idx){
  return idx/2;
}

int startIdx(int idx){
  return (idx-(1<<levelIdx(idx)));
}


// computes the size in bytes for the allocator
int BuddyAllocator_calcSize(int num_levels) {
  int list_items=1<<(num_levels+1); // maximum number of allocations, used to determine the max list items
  int list_alloc_size=(sizeof(BuddyListItem)+sizeof(int))*list_items;
  return list_alloc_size;
}

// Creazione del item dato un indice e suo padre, mettendolo nella lista del suo livello
BuddyListItem* BuddyAllocator_createListItem(BuddyAllocator* alloc,
                                             int idx,
                                             BuddyListItem* parent_ptr){
  BuddyListItem* item=(BuddyListItem*)PoolAllocator_getBlock(&alloc->list_allocator);
  item->idx=idx;
  item->level=levelIdx(idx);

  // size of an item = to the min_size +  2^the number of levels from the bottom
  item->size=(1<<(alloc->num_levels-item->level))*alloc->min_bucket_size;

  // start of an item
  item->start= alloc->memory + startIdx(idx)*item->size; //indirizzo gestito dal Buddy

  item->parent_ptr=parent_ptr;
  item->buddy_ptr=0;
  //mette in coda alla lista del livello corrispondente al Buddy appena creato
  List_pushBack(&alloc->free[item->level],(ListItem*)item);
  printf("Creating Item. idx:%d, level:%d, start:%p, size:%d\n", 
         item->idx, item->level, item->start, item->size);
  return item;
};

// detaches and destroys an item in the free lists 
void BuddyAllocator_destroyListItem(BuddyAllocator* alloc, BuddyListItem* item){
  int level=item->level;
  List_detach(&alloc->free[level], (ListItem*)item);
  printf("Destroying Item. level:%d, idx:%d, start:%p, size:%d\n",
         item->level, item->idx, item->start, item->size);
  //viene rilasciato anche la memoria occupata dal buddy anche nell'poolallocator oltre che nella free_list del suo livello       
  PoolAllocatorResult release_result=PoolAllocator_releaseBlock(&alloc->list_allocator, item);
  assert(release_result==Success);
};


//Inizializzazione del BuddyAllocator con buffer corrispondente allo spazio di memoria in cui salvare le strutture 
//dell'allocatore mentre memory è la memoria che dovrà gestire. In alloc->list_allocator vivranno tutti i buddy 
//che potranno popolare l'albero binario completo, mentre nelle alloc->free_list ci saranno solo dei loro
//riferimenti.
void BuddyAllocator_init(BuddyAllocator* alloc,
                         int num_levels,
                         char* buffer,
                         int buffer_size,
                         char* memory,
                         int min_bucket_size){

  // we need room also for level 0
  alloc->num_levels=num_levels;   //livelli dell'albero
  alloc->memory=memory;           //memoria da gestire
  alloc->min_bucket_size=min_bucket_size;     //dimensione minima delle foglie
  assert (num_levels<MAX_LEVELS);
  // we need enough memory to handle internal structures
  assert (buffer_size>=BuddyAllocator_calcSize(num_levels));

  int list_items=1<<(num_levels+1); // maximum number of allocations, used to size the list (numero di nodi)
  int list_alloc_size=(sizeof(BuddyListItem)+sizeof(int))*list_items; // nodi == BuddyListItem + intero

  printf("BUDDY INITIALIZING\n");
  printf("\tlevels: %d", num_levels);
  printf("\tmax list entries %d bytes\n", list_alloc_size);
  printf("\tbucket size:%d\n", min_bucket_size);
  printf("\tmanaged memory %d bytes\n", (1<<num_levels)*min_bucket_size);
  
  // the buffer for the list starts where the bitmap ends
  char *list_start=buffer;  // memoria per allocare l'albero

  //viene inizializzato il poolAllocator su buffer con un numero di oggetti pari a list_items ovvero i numeri dei Buddy
  PoolAllocatorResult init_result=PoolAllocator_init(&alloc->list_allocator,
						     sizeof(BuddyListItem),
						     list_items,
						     list_start,
						     list_alloc_size);
  printf("%s\n",PoolAllocator_strerror(init_result));

  // we initialize all lists, le free sono delle ListHead che devono essere inizializzate
  for (int i=0; i<MAX_LEVELS; ++i) {
    List_init(alloc->free+i);
  }

  // we allocate a list_item to mark that there is one "materialized" list
  // in the first block, il primo blocco corrisponde al primo nodo radice che non ha genitori (lo 0 serve a questo)
  BuddyAllocator_createListItem(alloc, 1, 0);
};

//Restituisce il primo Buddy disponibile dato un certo livello, se vuoto viene costruito partendo dai suoi antenati.
//Tutti i suoi antenati verranno tolti dalle corrispondenti free_list dei loro livelli, tra cui l'item che deve essere
//restituito come risultato.
BuddyListItem* BuddyAllocator_getBuddy(BuddyAllocator* alloc, int level){
  if (level<0)
    return 0;
  assert(level <= alloc->num_levels);

  if (! alloc->free[level].size ) { // no buddies on this level
    BuddyListItem* parent_ptr=BuddyAllocator_getBuddy(alloc, level-1);
    if (! parent_ptr)
      return 0;

    // parent already detached from free list
    int left_idx=parent_ptr->idx<<1;
    int right_idx=left_idx+1;
    
    printf("split l:%d, left_idx: %d, right_idx: %d\r", level, left_idx, right_idx);
    BuddyListItem* left_ptr=BuddyAllocator_createListItem(alloc,left_idx, parent_ptr);
    BuddyListItem* right_ptr=BuddyAllocator_createListItem(alloc,right_idx, parent_ptr);
    // we need to update the buddy ptrs
    left_ptr->buddy_ptr=right_ptr;
    right_ptr->buddy_ptr=left_ptr;
  }
  // we detach the first
  if(alloc->free[level].size) {
    BuddyListItem* item=(BuddyListItem*)List_popFront(alloc->free+level);
    return item;
  }
  assert(0);
  return 0;
}


//Rilascio dell'item con il consecutivo rilascio del buddy se libero ripetendo il tutto sul padre.
void BuddyAllocator_releaseBuddy(BuddyAllocator* alloc, BuddyListItem* item){

  BuddyListItem* parent_ptr=item->parent_ptr;
  BuddyListItem *buddy_ptr=item->buddy_ptr;
  
  // buddy back in the free list of its level
  List_pushFront(&alloc->free[item->level],(ListItem*)item);

  // if on top of the chain, do nothing
  if (! parent_ptr)
    return;
  
  // if the buddy of this item is not free, we do nothing
  if (buddy_ptr->list.prev==0 && buddy_ptr->list.next==0) 
    return;
  
  //join
  //1. we destroy the two buddies in the free list;
  printf("merge %d\n", item->level);
  BuddyAllocator_destroyListItem(alloc, item);
  BuddyAllocator_destroyListItem(alloc, buddy_ptr);
  //2. we release the parent
  BuddyAllocator_releaseBuddy(alloc, parent_ptr);

}

//Allocazione della memoria grazie all'elimininazione del corrispondente Buddy di dimensione sufficienti per contenere size
//restituendo l'indirizzo a cui fa capo grazie al campo start dell'item + 8, in start viene salvato un riferimento dell'item.
void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size) {
  // we determine the level of the page
  int mem_size=(1<<alloc->num_levels)*alloc->min_bucket_size;

  // log2(mem_size): n bits to represent the whole memory
  // log2(size): n nits to represent the requested chunk
  // bits_mem_size-bits_size = depth of the chunk = level
  int  level=floor(log2(mem_size/size));

  // if the level is too small, we pad it to max
  if (level>alloc->num_levels)
    level=alloc->num_levels;

  printf("requested: %d bytes, level %d \n",
         size, level);

  // we get a buddy of that size;
  BuddyListItem* buddy=BuddyAllocator_getBuddy(alloc, level);
  if (! buddy)
    return 0;

  // we write in the memory region managed the buddy address
  BuddyListItem** target= (BuddyListItem**)(buddy->start);
  *target=buddy;
  return buddy->start+8;
}
//Rilascio della memoria grazie alla realease_Buddy dell'item in *(mem-8)
void BuddyAllocator_free(BuddyAllocator* alloc, void* mem) {
  printf("freeing %p/n", mem);
  // we retrieve the buddy from the system
  char* p=(char*) mem;
  p=p-8;
  BuddyListItem** buddy_ptr=(BuddyListItem**)p;
  BuddyListItem* buddy=*buddy_ptr;
  //printf("level %d", buddy->level);
  // sanity check;
  assert(buddy->start==p);
  BuddyAllocator_releaseBuddy(alloc, buddy);
  
}
