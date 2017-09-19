#include "map.h"
#include <stdio.h>

Map createMap(int startingLength) {
    Map map;
    map.length = startingLength;
    map.entries = malloc(map.length * sizeof(MapEntry));
    for (int i = 0; i < map.length; i++) {
        map.entries[i] = NULL;
    }
    return map;
}

char * getValue(Map map, char * key) {
    for (int i = 0; i < map.length; i++) {
        if (map.entries[i] == NULL) {
            continue;
        }
        if (map.entries[i]->key == key) {
            return map.entries[i]->value;
        }
    }

    return NULL;
}

int _getFreeIndex(Map map) {
    for (int i = 0; i < map.length; i++) {
        if (map.entries[i] == NULL) {
            return i;
        }
    }

    return -1;
}

void putEntry(Map map, char * key, char * value) {
    if (containsEntry(map, key)) {
        for (int i = 0; i < map.length; i++) {
            if (map.entries[i] == NULL) {
                continue;
            }
            if (map.entries[i]->key == key) {
                map.entries[i]->value = value;
                return;
            }
        }
    } else {
        int freeIndex = _getFreeIndex(map);
        if (freeIndex < 0) {
            int oldLength = map.length;
            map.length = map.length << 1;
            map.entries = realloc(map.entries, map.length * sizeof(MapEntry));

            for (int i = oldLength; i < map.length; i++) {
                map.entries[i] = NULL;
            }

            MapEntry entry = malloc(sizeof(MapEntry));
            entry->key = key;
            entry->value = value;

            map.entries[map.length - 1] = entry;
        } else {
            MapEntry entry = malloc(sizeof(MapEntry));
            entry->key = key;
            entry->value = value;

            map.entries[freeIndex] = entry;
        }
    }
}

bool removeEntry(Map map, char * key) {
    for (int i = 0; i < map.length; i++) {
        if (map.entries[i] == NULL) {
            continue;
        }
        if (map.entries[i]->key == key) {
            map.entries[i]->value = NULL;
            return true;
        }
    }

    return false;
}

bool containsEntry(Map map, char * key) {
    return getValue(map, key) != NULL;
}

void cleanupMap(Map map) {
    free(map.entries);
}