#pragma once
#include <stdint.h>

#ifdef _FAKE_MMU_TEST_
#include "constants_small.h"
#else
#include "constants_real.h"
#endif

// number of bytes in address space
#define MAX_MEMORY (1<<ADDRESS_NBITS)

// number of bits used to specified allowed operation on a segment adresses
#define SEGMENT_FLAGS_NBITS 3

// number of bits used to specified allowed operation on a pages adresses
#define PAGE_FLAGS_NBITS 5

// total number of segments
#define SEGMENTS_NUM (1<<SEGMENT_NBITS)

// logical adresses used by CPU
#define LOGICAL_ADDRESS_NBITS (ADDRESS_NBITS+SEGMENT_NBITS)

// number of pages
#define PAGES_NUM  (1<<PAGE_NBITS)

// bits of Pages entry
#define FRAME_NBITS (ADDRESS_NBITS-PAGE_NBITS)
		    
// size of a memory page
#define PAGE_SIZE (1<<FRAME_NBITS)


typedef enum {
  Valid=0x1,  //first bit
  Read=0x2,   //second bit
  Write=0x4   //third bit 
} SegmentFlags;

// entry of Segment Table
typedef struct SegmentDescriptor{
  uint32_t base: PAGE_NBITS;  
  uint32_t limit: PAGE_NBITS;
  SegmentFlags flags: SEGMENT_FLAGS_NBITS;
} SegmentDescriptor;

// page_number rappresent a portion of adress that become page entry after a segmentation
typedef struct LogicalAddress{
  uint32_t segment_id: SEGMENT_NBITS;
  uint32_t page_number: PAGE_NBITS;
  uint32_t offset: FRAME_NBITS;
} LogicalAddress;

typedef struct LinearAddress{
  uint32_t page_number: PAGE_NBITS;
  uint32_t offset:      FRAME_NBITS;
} LinearAddress;

// entry of Pages Table
typedef struct PageEntry {
  uint32_t frame_number: PAGE_NBITS;
  uint32_t flags: PAGE_FLAGS_NBITS;
} PageEntry;

typedef struct MMU {
  SegmentDescriptor* segments;  // Segment's Table
  uint32_t num_segments; // number of good segments
  PageEntry *pages; // Page's Table
  uint32_t num_pages; // number of pages
} MMU;

// applies segmentation to an address and returns linear address
LinearAddress getLinearAddress(MMU* mmu, LogicalAddress logical_address);

typedef uint32_t PhysicalAddress;

// applies pagination to an address and returns the physical address
uint32_t getPhysicalAddress(MMU* mmu, LinearAddress linear_address);
