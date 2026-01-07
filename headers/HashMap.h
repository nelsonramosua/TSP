// HashMap.h - Simple HashMap for string to int mapping (char* -> int).
//
// Nelson Ramos, 124921.
//
// November, 2025.
// 
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// it made no sense to use the overkill implementation taught in classes.
// this is a simple implementation with Separate Chaining. It can be improved substantially. Go ahead! :)

#ifndef _HASH_MAP_H_
#define _HASH_MAP_H_

#include <stdlib.h>

#define INITIAL_CAPACITY 101 
#define MAX_LOAD_FACTOR 0.75

typedef struct _HashMapEntry {
    char* key;
    int value;
    struct _HashMapEntry* next;
} HashMapEntry;

typedef struct _HashMap {
    unsigned int capacity;  
    HashMapEntry** bins;     
    unsigned int size;
} HashMap;

HashMap* HashMapCreate(void);
void HashMapDestroy(HashMap** map);

int HashMapPut(HashMap* map, const char* key, int value);
int HashMapGet(const HashMap* map, const char* key);
int HashMapRemove(HashMap* map, const char* key);
int HashMapSize(const HashMap* map);

#endif // _HASH_MAP_H_