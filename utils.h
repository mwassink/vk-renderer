#ifndef UTILS_H
#define UTILS_H
#include "types.h"
#include "platform.h"


#define MAX_HEAPS 40
extern Platform* pform;

#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__clang__)
#include <x86intrin.h>
#else 
#error No valid compiler available for build!
#end
#endif

#include <intrin.h>

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

inline void cpy(void* dst, void* src, u32 count) {
    for (int i = 0; i < count; i++) ((char*)dst)[i] = ((char*) src)[i];
}

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
    Vector() {
        data = (Type* ) allocator.bAlloc(20*sizeof(Type), &freeToken, 0);
        sz = 0;
        cap = 20;
    }

    void push(Type dataIn) {
        if (sz == cap) {
            auto oldData = data;
            auto oldToken = freeToken;
            data = (Type*) allocator.bAlloc(sz * 1.5 * sizeof(Type), &freeToken, 0);
            cpy(data, oldData, sz * sizeof(Type));
            allocator.bFree(oldToken);

            cap = sz * 1.5;
        }
        data[sz++] = dataIn;
    }
    
    u64 size() {
        return sz;
    }
    
    void release() {
        allocator.bFree(freeToken);
    }
};


struct TripleKeyVal {
    int32 arr[4];
    TripleKeyVal() {}
    TripleKeyVal(int32 a, int32 b, int32 c, int32 d ){ 
        arr[0] = a;
        arr[1] = b;
        arr[2] = c;
        arr[3] = d; 
    }
};

inline u32 hash4to1(u32 a, u32 b, u32 c, u32 d) {
    unsigned char seed[16] {
        0x0b , 0x6a , 0xf8 , 0xf4 , 0x7b , 0x85 , 0x5b , 0xc1 , 0x38 , 0x6b , 0x97 , 0x07 , 0x09 , 0x00 , 0x58 , 0x7b
    };
    
    __m128i val = _mm_set_epi32(a, b, c, d);
    __m128i seedmm = _mm_loadu_si128((__m128i*) seed);
    __m128i res = _mm_aesdec_si128(val, seedmm);
    res = _mm_aesdec_si128(res, seedmm);
    res = _mm_aesdec_si128(res, seedmm);
    
    // Take the least significant bits
    return (_mm_extract_epi32(res, 0));
    
    
    
}

inline bool  equal(const TripleKeyVal& l, const TripleKeyVal& r) {
    for (int i = 0; i < 3; i++) if (l.arr[i] != r.arr[i]) return false;
    return true;
    
}

// Meant for hashing indices for the meshes
struct HashTable {
    TripleKeyVal*  arr;
    int ctr;
    int sz;
    u32 freeToken;
    HashTable(int sz) {
        TripleKeyVal tkv(0, 0, 0, -1);
        ctr = 0;
        arr = (TripleKeyVal*)allocator.bAlloc(sizeof(TripleKeyVal)*sz, &freeToken, 0 ); // this will be aligned fine
        for (int i = 0; i < sz; ++i) {
            arr[i] = tkv;
        }
        this->sz = sz;
    }

    
    int32 insert(u32 a, u32 b, u32 c, u32 empty) {
        TripleKeyVal tkv(a, b, c, empty);
        u32 bucketIndex = hash4to1(a, b, c, empty) % sz;
        while (arr[bucketIndex].arr[3] != -1) {
            bucketIndex = ((bucketIndex + 1) % sz);
        }
        arr[bucketIndex].arr[0] = a;
        arr[bucketIndex].arr[1] = b;
        arr[bucketIndex].arr[2] = c;
        arr[bucketIndex].arr[3] = ctr;
        return ctr++;
    
    
    }
    
    int32 at(u32 a, u32 b, u32 c, u32 empty ) {
        TripleKeyVal tkv(a, b, c, empty );
        u32 bucketIndex = hash4to1(a, b, c, empty) % sz;
        // If we find an empty index or our bucket we stop
        while ((!equal(arr[bucketIndex], tkv) ) && arr[bucketIndex].arr[3] != -1) {
            bucketIndex = ((bucketIndex + 1) % sz);
        }
        return arr[bucketIndex].arr[3];
    }
    void release() {
        allocator.bFree(freeToken);
    }
    
};

int CountOccurrences(const char* s, char ch, char delim);








// BasicModel LoadModelVertexData(const char* objFile);


#endif
