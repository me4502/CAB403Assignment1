#include "../common/list.h"
#include "../common/map.h"

#include <stdio.h>
#include <memory.h>

void testLists() {
    // Test Lists
    List list = createList((size_t) 1, sizeof(char *));
    add(list, "test");
    add(list, "this");
    add(list, "code");
    add(list, "please");

    for (int i = 0; i < list->length; i++) {
        printf("%s\n", (char *) getValueAt(list, i));
    }

    freeList(list);
}

void testMaps() {
    // Test Maps
    Map map = createMap(1);
    putEntry(map, "key", "value");
    putEntry(map, "abc", "def");

    printf("key=%s\n", (char *) getValue(map, "key"));
    printf("abc=%s\n", (char *) getValue(map, "abc"));

    char test[16];
    strcpy(test, "abc");

    printf("abc=%s\n", (char *) getValue(map, test));

    for (int i = 0; i < map->length; i++) {
        if (map->entries[i] == NULL) {
            continue;
        }
        printf("%s=%s\n", (char *) map->entries[i]->key, (char *) map->entries[i]->value);
    }

    freeMap(map);
}

int main() {
    testLists();
    testMaps();
}