// Struct definition for a hash table item
typedef struct {
    char* key;
    char* value;
} ht_item;

// Struct definition for the hash table
typedef struct {
    int base_size;
    int size;
    int count;
    // Pointer to array of pointers of items
    ht_item** items;
} ht_hash_table;

ht_hash_table* ht_new_table(void);
void ht_del_hash_table(ht_hash_table* ht);

void ht_insert(ht_hash_table* ht, const char* key, const char* value);
char* ht_search(ht_hash_table* ht, const char* key);
void ht_delete(ht_hash_table* ht, const char* key);
