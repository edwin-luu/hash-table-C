#include "hash_table.h"
#include <stdio.h>

int main() {
    ht_hash_table* ht = ht_new_table();

    // Insert some key-value pairs
    ht_insert(ht, "apple", "red");
    ht_insert(ht, "banana", "yellow");
    ht_insert(ht, "grape", "purple");

    // Search
    printf("apple -> %s\n", ht_search(ht, "apple"));
    printf("banana -> %s\n", ht_search(ht, "banana"));
    printf("grape -> %s\n", ht_search(ht, "grape"));
    printf("orange -> %s\n", ht_search(ht, "orange")); // should be NULL

    // Delete
    ht_delete(ht, "banana");
    printf("banana -> %s\n", ht_search(ht, "banana")); // should be NULL

    // Clean up
    ht_del_hash_table(ht);

    return 0;
}
