
#include "utils.h"

Platform* pform;


#define KILOBYTE 1024
#define ALIGNMENT 16
void* Allocator::bump(u32 bucket, u32 size) {
  void* mem = data.heaps[bucket];
  uintptr_t ptr = (uintptr_t)(mem);
  ptr += (size + (ptr % ALIGNMENT)) ;
  data.space[bucket] -= (size + (ptr % ALIGNMENT));
  data.heaps[bucket] = (void*)ptr;
  data.ctrs[bucket]++;
  return mem;
}


u32 Allocator::heuristic(u32 size) {
  if (size > 10000 * KILOBYTE) {
    return 2;
  }
  if (size > 1000 * KILOBYTE) {
    return 4;
  }

  if (size > 100 * KILOBYTE) {
    return 10;
  }

  return 50;
}

void* Allocator::bAlloc(u32 sz, u32* token, u32 hint = 0) {

  // start at the ctr and check if a bucket is 1) empty or 2) used and has space
  u32 initialSize = sz;
  sz = heuristic(sz) * sz;
  void* ptr  = 0;
  for (u32 i = 0; i < MAX_HEAPS; i++) {
    u32 bucket = (data.currHeap) % MAX_HEAPS;
    u32 space;
    if (data.ctrs[bucket] == 0) {
      
      data.heaps[bucket] = pform->GetMemory(sz, &space);
      data.space[bucket] = sz;
      data.freePtrs[bucket] = data.heaps[bucket];
      ptr = bump(bucket, initialSize);
      *token = bucket;
      return ptr;
    } else if (data.ctrs[bucket] > 0 && (initialSize < (data.space[bucket] + ALIGNMENT))) {
      ptr = bump(bucket, initialSize);
      *token = bucket;
      return ptr;
    }
    data.currHeap++;
  }
  *token = 0;
  return 0;
}

void Allocator::bFree(u32 token) {
  data.ctrs[token]--;
  if (data.ctrs[token] == 0) {
    pform->FreeMemory(data.freePtrs[token]);
  }
}

Allocator allocator;

int CountOccurrences(const char* s, char ch, char delim = 0)  {
    int ctr = 0;
    while (*s && *s != delim) ctr += (*s++ == ch);
    return ctr;
}

//BasicModel LoadModelVertexData(const char* objFile) {  
//}
