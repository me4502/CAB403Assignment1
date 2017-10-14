#pragma once

#include <stdlib.h>

typedef struct list {
    int length;
    size_t arrayLength;
    size_t dataSize;
    void ** values;
} * List;

/**
 * Creates a list of the given length.
 *
 * @param startingLength The starting length
 * @param dataSize The size of the data
 * @return The new list
 */
List createList(size_t startingLength, size_t dataSize);

/**
 * Gets the value at the given index
 *
 * @param list The list
 * @param index The index
 * @return The value at the index
 */
void * getValueAt(List list, int index);

/**
 * Adds a value to the list, resizing if necessary
 *
 * @param list The list
 * @param value The value
 */
void add(List list, void * value);

/**
 * Adds a value to the list at the index, resizing if necessary
 *
 * @param list The list
 * @param value The value
 * @param index The index
 */
//void addAt(List list, void * value, int index);

/**
 * Removes a value from the list by the index.
 *
 * @param list The list
 * @param index The index
 */
void removeAt(List list, int index);

/**
 * Frees the list.
 *
 * @param list The list
 */
void freeList(List list);