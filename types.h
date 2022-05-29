#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef float f32;
typedef double f64;
typedef int s32;
typedef unsigned int u32;
typedef short int s16;
typedef short unsigned int u16;
typedef unsigned char u8;
typedef int32_t int32;
typedef uint64_t u64;
typedef int64_t s64;


template <typename Type>
struct Vector {
    Type* data;
    int sz, cap;
    
    void push(const Type& in) {
        if (sz == cap) {
            data = (Type*)realloc(data, cap*1.5*sizeof(Type));
            cap *= 1.5;
        }
        data[sz++] = in;
    }
    
    void pop() {
        sz--;
    }
    
    Type& operator[](int index) {
        return data[index];
    }
    
    Vector(int sz_in) {
        data = (Type*) malloc(sz_in*sizeof(Type));
        sz = sz_in;
        cap = sz_in;
        
    }
    
    void resize(int sz_in) {
        sz = sz_in;
    }
    
    Vector() {
        data = (Type* ) malloc(20*sizeof(Type));
        sz = 0;
        cap = 20;
    }
    
    u64 size() {
        return sz;
    }
    
    ~Vector() {
        free(data);
    }
};

#endif
