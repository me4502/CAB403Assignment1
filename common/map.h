#pragma once

#include <stdbool.h>

typedef struct map_entry {
    char * key;
    void * value;
} * MapEntry;

typedef struct map {
    int length;
    MapEntry * entries;
} * Map;

/**
 * Creates a map of the given length.
 *
 * @param startingLength The starting length
 * @return The new map
 */
Map createMap(int startingLength);

/**
 * Gets a value from the Map, or NULL if not present.
 *
 * @param map The map
 * @param key The key
 * @return The value, or NULL
 */
void * getValue(Map map, char * key);

/**
 * Put an entry in the Map.
 *
 * @param map The map
 * @param key The key
 * @param value The value
 */
void putEntry(Map map, char * key, void * value);

/**
 * Removes an entry from the Map.
 *
 * @param map The map
 * @param key The key to remove
 * @return If an entry was removed
 */
bool removeEntry(Map map, char * key);

/**
 * Gets if a Map contains a key.
 *
 * @param map The map
 * @param key The key
 * @return If the map contains the key
 */
bool containsEntry(Map map, char * key);

/**
 * Frees the memory allocated to this map.
 *
 * @param map The map
 */
void freeMap(Map map);