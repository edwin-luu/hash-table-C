#define HT_PRIME_1 53
#define HT_PRIME_2 101
#define HT_INITIAL_BASE_SIZE 47

#include <stdlib.h>
#include <string.h>

#include "hash_table.h"
#include "prime.h"

static void ht_resize(ht_hash_table* ht, const int new_base_size);
static void ht_resize_up(ht_hash_table* ht);
static void ht_resize_down(ht_hash_table* ht);

// Tombstone for item deletion
static ht_item HT_DELETED_ITEM = {NULL, NULL};

// Method to create a new item; STATIC method since it's only used by ht
static ht_item* ht_new_item(const char* key, const char* value) {
    // Create a new item within hash table with malloc()
    ht_item* item = malloc(sizeof(ht_item));
    // Create a new pointer to the key string
    item->key = strdup(key);
    // Create a new pointer to the value string
    item->value = strdup(value);
    // Return the item
    return item;
}

// Method to create a new hash table using specified size; called by ht_new_table()
static ht_hash_table* ht_new_sized(const int base_size) {
    // Allocate space for ht
    ht_hash_table* ht = malloc(sizeof(ht_hash_table));
    // Note down base_size; doesn't have to be prime
    ht->base_size = base_size;
    // Compute the next valid prime size based on base_size
    ht->size = next_prime(base_size);
    // Initialize count of elements to be 0
    ht->count = 0;
    // Allocate contiguous space for items with calloc(number of items, size of each item pointer)
    ht->items = calloc((size_t)ht->size, sizeof(ht_item*));
    // Return the hash table
    return ht;
}

// Method to create a new hash table
ht_hash_table* ht_new_table() {
    return ht_new_sized(HT_INITIAL_BASE_SIZE);
}

// Method to delete an item; STATIC method since it's only used by ht
static void ht_del_item(ht_item* item) {
    // Since we malloc() to create the item and strdup() to create new pointers to key and value, we must use free()
    free(item->key);
    free(item->value);
    free(item);
}

// Method to delete the hash table
void ht_del_hash_table(ht_hash_table* ht) {
    // Iter. thru the entire size of the table
    for (int i = 0; i < ht->size; i++) {
        // Delete the item if it isn't NULL and isn't a TOMBSTONE
        ht_item* item = ht->items[i];
        if (item != NULL && item != &HT_DELETED_ITEM) {
            ht_del_item(item);
        }
    }
    // ht->items was allocated with calloc(), so use free()
    free(ht->items);
    // ht was allocated with malloc(), so use free()
    free(ht);
}

// Method to hash a given string via Horner's rule; STATIC method since it's only used by ht
    // a: turns a string into a positional number instead of a character sum; prevents anagrams from hashing to the same index; prime number is preferred
    // ht_size should be a prime number to distribute the hashing; should be prime (ex. 53, 101, 211, 431, 1009, 2003, 5003, 10007, 20011); start small and resize to these numbers as you go.
static size_t ht_hash(const char* s, const size_t a, const size_t ht_size) {
    // Store hash index
    size_t hash_index = 0;
    // Loop thru each char until null terminator is reached
    for (int i = 0; s[i] != '\0'; i++) {
        // Horner's rule: s[0]·a^(n-1) + s[1]·a^(n-2) + ... + s[n-2]·a^1 + s[n-1]·a^0
        // hash_index is the accumulation of the Horner's polynomial series -> (hash_index * a + ord(character)) % ht_size
        hash_index = (hash_index * a + s[i]) % ht_size;
    }
    return (hash_index);
}

// Method to double-hash to handle collisions; STATIC method since it's only used by ht
    // We are not using Linear Probing (index = (hash + attempt) % num_buckets) since it causes clustering
    // HT_PRIME_1 and HT_PRIME_2 allow for independent series when computing hash index
static int ht_get_hash(const char* s, const int num_buckets, const int attempt) {
    // Compute the initial hash index -> hash_a
    const int hash_a = ht_hash(s, HT_PRIME_1, num_buckets);
    // Compute the step size in case of a collision -> hash_b
    const int hash_b = ht_hash(s, HT_PRIME_2, num_buckets);
    // Return (hash_a + (attempts * steps)) % bucket size
        // The step size (hash_b) is 0, hash index is stuck at hash_a despite infinite attempts, so add 1 to prevent infinite loop
    return (hash_a + (attempt * (hash_b + 1))) % num_buckets;
}

// Method to insert key-value pair into hash table
void ht_insert(ht_hash_table* ht, const char* key, const char* value) {
    // If load is greater than 70%, resize
    const double load = ((double) ht->count / ht->size);
    if (load > 0.7) {
        ht_resize_up(ht);
    }
    // This pair becomes a new item
    ht_item* new_item = ht_new_item(key, value);
    // Track attempts to hash
    int attempt = 0;
    // Track index of hash
    int index;
    // Track first tombstone index
    int first_tombstone_index = -1;

    // Search ht if ht is not full yet
    while (attempt < ht->size) {
        // Compute the hash index
        index = ht_get_hash(key, ht->size, attempt);
        // Get the item at hash index
        ht_item* current_item = ht->items[index];
        
        // Case 1: bucket points to a tombstone (implemented with ht_delete())
        if (current_item == &HT_DELETED_ITEM) {
            // Remember the first tombstone encountered; only update if the first tombstone hasn't been encountered
            if (first_tombstone_index == -1) {
                first_tombstone_index = index;
            }
            attempt++;
            continue;
        }
        // Case 2: bucket is empty -> insert either at first tombstone or this index
        if (current_item == NULL) {
            int index_to_insert;
            if (first_tombstone_index != -1) {
                index_to_insert = first_tombstone_index;
            }
            else {
                index_to_insert = index;
            }
            // Insert item at this bucket and up ht's count
            ht->items[index_to_insert] = new_item;
            ht->count++;
            return;
        }
        // Case 3: bucket is not empty, but key is DUPLICATE -> so update the value
        if (strcmp(current_item->key, key) == 0) {
            // Free current_item's value
            free(current_item->value);
            // Create a new pointer to the new value and assign it to current_item's value
            current_item->value = strdup(value);
            // Free new_item's key and value, and then the new_item, since we will be reusing the existing item
            free(new_item->key);
            free(new_item->value);
            free(new_item);
            return;
        }
        // Case 4: bucket is not empty, and key is DIFFERENT -> increment attempt
        attempt++;   
    }
    // After we exceeded attempts, a tombstone could be found; insert the item there
    if (first_tombstone_index != -1) {
        ht->items[first_tombstone_index] = new_item;
        ht->count++;
        return;
    }
    
}

// Method to search for key
char* ht_search(ht_hash_table* ht, const char* target_key) {
    // Track attempts
    int attempt = 0;
    // Search up to the size of ht to prevent infinite searches
    while (attempt < ht->size) {
        // Compute the hash index
        int index = ht_get_hash(target_key, ht->size, attempt);
        // Get the item at hash index
        ht_item* current_item = ht->items[index];
        // Case 1: item is NULL
        if (current_item == NULL) {
            return NULL;
        }
        // Case 2: Since item isn't NULL, so get its value ONLY if the bucket isn't a tombstone
        if (current_item != &HT_DELETED_ITEM) {
            if (strcmp(current_item->key, target_key) == 0) {
                return current_item->value;
            }
        }
        // Otherwise, item hasn't been found yet
        attempt++;
    }
    return NULL;
}

// Method to delete a key -> marking slot as a tombstone rather than NULL
void ht_delete(ht_hash_table* ht, const char* target_key) {
    // If load is less than 10%, resize down
    const double load = ((double) ht->count / ht->size);
    if (load < 0.1) {
        ht_resize_down(ht);
    }
    // Track attempts
    int attempt = 0;
    // Search up to the size of ht to prevent infinite searches
    while (attempt < ht->size) {
        // Compute the hash index
        int index = ht_get_hash(target_key, ht->size, attempt);
        // Get the item at hash index
        ht_item* existing_item = ht->items[index];
        // Check if item is NULL first
        if (existing_item == NULL) {
            return;
        }
        // Since item isn't NULL, check if it is a tombstone
        if (existing_item != &HT_DELETED_ITEM) {
            // If the item's key matches with the target key. DELETE
            if (strcmp(existing_item->key, target_key) == 0) {
                // Delete it
                ht_del_item(existing_item);
                // Mark it as a tombstone
                ht->items[index] = &HT_DELETED_ITEM;
                // Decrement count of items
                ht->count--;
                return;
            }
        }
        // Otherwise, item hasn't been found yet
        attempt++;
    }
    return;
};

// Method to resize the table; called by ht_resize_up() and ht_resize_down()
static void ht_resize(ht_hash_table* ht, const int new_base_size) {
    // New base size cannot be smaller than ht's base size
    if (new_base_size < HT_INITIAL_BASE_SIZE) {
        return;
    }
    // Create a new temporary ht with current size; we will update old ht
    ht_hash_table* temp_ht = ht_new_sized(new_base_size);
    // Iter. thru old ht
    for (int i = 0; i < ht->size; i++) {
        // Grab item from old table and rehash it for insertion into temp ht
        ht_item* item = ht->items[i];
        if (item != NULL && item != &HT_DELETED_ITEM) {
            ht_insert(temp_ht, item->key, item->value);
        }
    }
    // Update the base_size and count of old ht, since we are keeping them
    ht->base_size = temp_ht->base_size;
    ht->count = temp_ht->count;

    // Swap actual size and items of old ht, since we are keeping them
    const int temp_size = ht->size;
    ht->size = temp_ht->size;
    temp_ht->size = temp_size;

    ht_item** temp_items = ht->items;
    ht->items = temp_ht->items;
    temp_ht->items = temp_items;

    // Delete temporary ht
    ht_del_hash_table(temp_ht);

}

// Method to resize UP the table; called by ht_insert()
static void ht_resize_up(ht_hash_table* ht) {
    // Double the size
    const int new_size = (ht->base_size * 2);
    ht_resize(ht, new_size);
}

// Method to resize DOWN the table; called by ht_delete()
static void ht_resize_down(ht_hash_table* ht) {
    // Half the size
    const int new_size = (ht->base_size / 2);
    ht_resize(ht, new_size);
}