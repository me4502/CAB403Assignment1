#include "map.h"

#include <stdlib.h>
#include <string.h>

Map createMap(int startingLength) {
    Map map = malloc(sizeof(struct map));
    map->length = startingLength;
    map->entries = calloc((size_t) map->length, sizeof(MapEntry));
    return map;
}

bool _areKeysEqual(char * a, char * b) {
    return strcmp(a, b) == 0;
}

void * getValue(Map map, char * key) {
    for (int i = 0; i < map->length; i++) {
        if (map->entries[i] == NULL) {
            continue;
        }
        if (_areKeysEqual(map->entries[i]->key, key)) {
            return map->entries[i]->value;
        }
    }

    return NULL;
}

int _getFreeIndex(Map map) {
    for (int i = 0; i < map->length; i++) {
        if (map->entries[i] == NULL) {
            return i;
        }
    }

    return -1;
}

void putEntry(Map map, char * key, void * value) {
    if (containsEntry(map, key)) {
        for (int i = 0; i < map->length; i++) {
            if (map->entries[i] == NULL) {
                continue;
            }
            if (map->entries[i]->key == key) {
                map->entries[i]->value = value;
                return;
            }
        }
    } else {
        int freeIndex = _getFreeIndex(map);
        if (freeIndex < 0) {
            size_t oldLength = (size_t) map->length;
            map->length <<= 1;
            map->entries = realloc(map->entries, map->length * sizeof(MapEntry));

            for (size_t i = oldLength; i < map->length; i++) {
                map->entries[i] = NULL;
            }

            MapEntry entry = malloc(sizeof(MapEntry));
            entry->key = key;
            entry->value = value;

            map->entries[map->length - 1] = entry;
        } else {
            MapEntry entry = malloc(sizeof(MapEntry));
            entry->key = key;
            entry->value = value;

            map->entries[freeIndex] = entry;
        }
    }
}

bool removeEntry(Map map, char * key) {
    for (int i = 0; i < map->length; i++) {
        if (map->entries[i] == NULL) {
            continue;
        }
        if (map->entries[i]->key == key) {
            map->entries[i]->value = NULL;
            return true;
        }
    }

    return false;
}

bool containsEntry(Map map, char * key) {
    return getValue(map, key) != NULL;
}

void ** getValues(Map map, size_t size, int * length) {
    void ** values = malloc(map->length * size);
    int index = 0;
    for (int i = 0; i < map->length; i++) {
        if (map->entries[i] == NULL) {
            continue;
        }
        values[index++] = map->entries[i]->value;
    }
    (*length) = index;

    return values;
}

void freeMap(Map map) {
    free(map->entries);
    free(map);
}