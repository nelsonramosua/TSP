// HashMap.c - Simple HashMap for string to int mapping (char* -> int).
//
// Nelson Ramos, 124921.
//
// November, 2025.
// 
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// it made no sense to use the overkill implementation taught in classes.
// this is a simple implementation with Separate Chaining. It can be improved substantially. Go ahead! :)

#include "HashMap.h"
#include <string.h>
#include <stdio.h>

// helpers at EOF.
static int rehash(HashMap* map);
static HashMapEntry* createEntry(const char* key, int value);
static void freeEntry(HashMapEntry* map);

// this is a bad hashing function... but for these purposes, good enough.
// adapted from here: http://www.cse.yorku.ca/~oz/hash.html
static unsigned int hash(const char* key, unsigned int capacity) {
    unsigned long hashValue = 5381;
    int c;

    while ((c = *key++)) hashValue = ((hashValue << 5) + hashValue) + c; 
    
    return hashValue % capacity;
}

HashMap* HashMapCreate(void) {
    HashMap* map = (HashMap*)malloc(sizeof(HashMap));
    if (!map) return NULL;
    
    map->capacity = INITIAL_CAPACITY; // tune in HashMap.h.
    map->bins = (HashMapEntry**)calloc(map->capacity, sizeof(HashMapEntry*));
    if (!map->bins) { free(map); return NULL; }
    map->size = 0;

    return map;
}

void HashMapDestroy(HashMap** map) {
    if (!map || !(*map)) return;
    
    HashMap *hashMap = *map;

    for (unsigned int i = 0; i < hashMap->capacity; i++) {
        HashMapEntry* current = hashMap->bins[i];
        while (current) {
            HashMapEntry* next = current->next;
            freeEntry(current);
            current = next;
        }
    }

    free(hashMap->bins); free(hashMap);

    *map = NULL; 
}

int HashMapPut(HashMap* map, const char* key, int value) {
    if (!map || !key) return 0;
    
    unsigned int binIndex = hash(key, map->capacity);
    HashMapEntry* current = map->bins[binIndex];

    // search (and update)
    while (current) {
        if (strcmp(current->key, key) == 0) {
            current->value = value; // update value
            return 1;
        }
        current = current->next;
    }

    // inserts at the front
    HashMapEntry* newEntry = createEntry(key, value);
    if (!newEntry) return 0;

    newEntry->next = map->bins[binIndex];
    map->bins[binIndex] = newEntry;
    
    map->size++;

    double load = (double)map->size / map->capacity;
    if (load > MAX_LOAD_FACTOR) if (!rehash(map)) fprintf(stderr, "Warning: hashmap rehash failed, performance degraded.\n");

    return 1;
}

int HashMapGet(const HashMap* map, const char* key) {
    if (!map || !key) return -1;
    
    unsigned int bucketIndex = hash(key, map->capacity);
    HashMapEntry* current = map->bins[bucketIndex];

    // search bin
    while (current) {
        if (strcmp(current->key, key) == 0) return current->value; // found key
        current = current->next;
    }

    return -1; 
}

int HashMapRemove(HashMap* map, const char* key) {
    if (!map || !key) return 0;
    
    unsigned int binIndex = hash(key, map->capacity);
    HashMapEntry* cur = map->bins[binIndex];
    HashMapEntry* prev = NULL;

    // search for key in ll (bin)
    while (cur) {
        if (strcmp(cur->key, key) == 0) { // key was found
            // unlink node
            if (prev) prev->next = cur->next;
            else map->bins[binIndex] = cur->next;

            freeEntry(cur);
            map->size--;
            return 1; // success
        }
        
        // move to next node
        prev = cur;
        cur = cur->next;
    }

    return 0; // key not found
}

int HashMapSize(const HashMap* map) {
    return map ? (int)map->size : 0;
}

static HashMapEntry* createEntry(const char* key, int value) {
    HashMapEntry* newEntry = (HashMapEntry*)malloc(sizeof(HashMapEntry));
    if (!newEntry) return NULL;

    // duplicate key to make sure map has it
    newEntry->key = strdup(key); // buffer overflow but i'm too lazy.
    if (!newEntry->key) { free(newEntry); return NULL; }
    
    newEntry->value = value;
    newEntry->next = NULL;
    return newEntry;
}

static void freeEntry(HashMapEntry* map) {
    if (!map) return;
    free(map->key); free(map);
}

static int rehash(HashMap* map) {
    unsigned int oldCapacity = map->capacity;
    unsigned int newCapacity = map->capacity * 2 + 1; // odd

    HashMapEntry** newBins = calloc(newCapacity, sizeof(HashMapEntry*));
    if (!newBins) return 0;

    // move entries...
    for (unsigned int i = 0; i < oldCapacity; i++) {
        HashMapEntry* cur = map->bins[i];
        while (cur) {
            HashMapEntry* next = cur->next;

            unsigned int index = hash(cur->key, newCapacity);
            cur->next = newBins[index];
            newBins[index] = cur;

            cur = next;
        }
    }

    free(map->bins);
    map->bins = newBins; map->capacity = newCapacity;

    return 1;
}