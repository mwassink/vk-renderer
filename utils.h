#include "types.h"
#include "platform.h"

#define MAX_HEAPS 400
extern Platform* pform;

struct Allocator {
  void* heaps[400];
  int ctrs[400];
  int currHeap;
  
  void* bAlloc(u32 sz, u32 pg_hint);
  void bFree(void* ptr);
};


