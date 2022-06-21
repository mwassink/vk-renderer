#ifndef UTILS_H
#define UTILS_H
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
  
};

extern Allocator allocator;


template <typename Type>
struct Vector {
    Type* data;
    u32 sz, cap;
    u32 freeToken;
    
    Type& operator[](int index) {
        return data[index];
    }
    
    Vector(int sz_in) {
        data = (Type*) allocator.bAlloc(sz_in*sizeof(Type), &freeToken, 0);
        sz = sz_in;
        cap = sz_in;
        
    }
    
    void resize(int sz_in) {
        sz = sz_in;
    }
    
    Vector() {
        data = (Type* ) allocator.bAlloc(20*sizeof(Type), &freeToken, 0);
        sz = 0;
        cap = 20;
    }
    
    u64 size() {
        return sz;
    }
    
    ~Vector() {
        allocator.bFree(freeToken);
    }
};

#endif
