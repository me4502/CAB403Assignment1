#include "list.h"
#include <memory.h>

List createList(size_t startingLength, size_t dataSize) {
    List list = calloc(1, sizeof(struct list));
    list->length = 0;
    list->arrayLength = startingLength;
    list->dataSize = dataSize;
    list->values = calloc(list->arrayLength, dataSize);
    return list;
}

void * getValueAt(List list, int index) {
    if (index < 0 || index > list->length - 1) {
        return NULL;
    }

    return list->values[index];
}

void add(List list, void * value) {
    list->length++;
    if (list->length > list->arrayLength) {
        size_t oldLength = list->arrayLength;
        list->arrayLength <<= 1;
        list->values = realloc(list->values, list->arrayLength * list->dataSize);

        for (size_t i = oldLength; i < list->arrayLength; i++) {
            list->values[i] = NULL;
        }
    }

    list->values[list->length - 1] = value;
}

void removeAt(List list, int index) {
    if (index < 0 || index > list->length - 1) {
        return;
    }

    memmove(list->values + index, list->values + (index + 1),
            (size_t) (list->arrayLength - index) * sizeof(*(list->values)));
    list->length--;
}

void freeList(List list) {
    free(list->values);
    free(list);
}