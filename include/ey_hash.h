#ifndef EY_HASH_H
#define EY_HASH_H 1

struct memory_handler;
typedef void* ey_hash_t;

typedef unsigned int (*hash_line)(void* key);
typedef int (*hash_compare)(void *key, void *value);
typedef void (*hash_destroy)(void *value);
typedef int (*hash_foreach)(void *value, void *arg);

extern ey_hash_t ey_hash_create(char *name, size_t power2_size, size_t limit,
	hash_line gen_key, hash_compare compare_key, hash_destroy destroy_value, 
	struct memory_handler *memory_handler);
extern void ey_hash_init(ey_hash_t hash);
extern void ey_hash_destroy(ey_hash_t hash);

#define EY_HASH_BAD_PARAM	(-1)
#define EY_HASH_NO_MEM		(-2)
#define EY_HASH_CONFLICT	(-3)
#define EY_HASH_LIMIT		(-4)
extern int ey_hash_insert(ey_hash_t hash, void *key, void *value);
#define EY_HASH_NO_KEY		(-5)
extern int ey_hash_remove(ey_hash_t hash, void *key, void **value);
extern int ey_hash_remove_all(ey_hash_t hash, void *key, hash_compare compare);
extern void* ey_hash_find(ey_hash_t hash, void *key);
extern void* ey_hash_find_ex(ey_hash_t hash, void *key, hash_compare compare);
extern int ey_hash_foreach(ey_hash_t hash, void *key, hash_compare compare, hash_foreach foreach, void *arg);
#endif
