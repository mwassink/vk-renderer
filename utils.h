#include "types.h"
#include "platform.h"

#define MAX_HEAPS 40
extern Platform* pform;

struct AllocData {
  void* heaps[MAX_HEAPS];
  void* freePtrs[MAX_HEAPS];
  int ctrs[MAX_HEAPS];
  int space[MAX_HEAPS];
  int currHeap;
};

struct Allocator {
  
  AllocData data = {0};
  
  void* bAlloc(u32 sz, u32* token, u32 hint);
  void bFree(u32 token);
  void* bump(u32 bucket, u32 size);
  u32 heuristic(u32 sz);
  Allocator();
};


