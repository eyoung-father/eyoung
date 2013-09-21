#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "ey_queue.h"
#include "ey_lock.h"
#include "ey_memory.h"
#include "ey_hash.h"

typedef struct ey_hash_entry
{
	TAILQ_ENTRY(ey_hash_entry) link;
	void *value;
}ey_hash_entry_t;
typedef TAILQ_HEAD(ey_hash_entry_list, ey_hash_entry) ey_hash_entry_list_t;

typedef struct ey_hash_line
{
	ey_rwlock_t line_lock;
	ey_hash_entry_list_t entry_head;
}ey_hash_line_t;

typedef struct ey_hash
{
	char name[64];
	size_t power2_size;
	size_t mask;
	size_t limit;
	size_t count;
	ey_spinlock_t hash_lock;
	ey_slab_t slab;
	hash_line gen_fn;
	hash_compare compare_fn;
	hash_destroy destroy_fn;
	memory_handler_t memory_fn;
	ey_hash_line_t hash_lines[0];
}ey_hash;

ey_hash_t ey_hash_create(char *name, size_t power2_size, size_t limit,
	hash_line gen_key, hash_compare compare_key, hash_destroy destroy_value, 
	memory_handler_t *memory_handler)
{
	ey_hash *ret = NULL;
	memory_handler_t default_handler = {ey_malloc, ey_realloc, ey_free, ey_calloc};
	int lines = 0;

	if(!gen_key || !compare_key)
		return NULL;
	
	if(!name)
		name = "";

	if(power2_size < 10)
		power2_size = 10;	/*2^10 = 1K*/
	if(power2_size > 20)
		power2_size = 20;	/*2^20 = 1M*/
	lines = 2^power2_size;

	if(limit < 8192)
		limit = 8192;
	
	if(!memory_handler || !memory_handler->malloc || !memory_handler->free)
		memory_handler = &default_handler;
	
	ret = (ey_hash_t)memory_handler->malloc(sizeof(ey_hash) + sizeof(ey_hash_line_t)*lines);
	if(!ret)
		return NULL;
	
	ret->slab = ey_zinit(name, sizeof(ey_hash_entry_t), memory_handler);
	if(!ret->slab)
	{
		memory_handler->free(ret);
		return NULL;
	}

	strncpy(ret->name, name, sizeof(ret->name)-1);
	ret->power2_size = power2_size;
	ret->mask = lines-1;
	ret->limit = limit;
	ret->count = 0;
	ey_spinlock_init(&ret->hash_lock);
	ret->gen_fn = gen_key;
	ret->compare_fn = compare_key;
	ret->destroy_fn = destroy_value;
	ret->memory_fn = *memory_handler;

	ey_hash_init(ret);
	return (ey_hash_t)ret;
}

void ey_hash_init(ey_hash_t hash)
{
	ey_hash *tbl = (ey_hash*)hash;
	if(!tbl)
		return;
	
	int index = 0, lines = tbl->mask+1;
	ey_hash_line_t *line = NULL;
	for(index=0, line=tbl->hash_lines; index<lines; index++, line++)
	{
		TAILQ_INIT(&line->entry_head);
		ey_rwlock_init(&line->line_lock);
	}
}

void ey_hash_destroy(ey_hash_t hash)
{
	ey_hash *tbl = (ey_hash*)hash;
	if(!tbl)
		return;
	
	/*NOTE: NO LOCK HERE, so NOT safe in multi-thread*/
	int index = 0, lines = tbl->mask+1;
	ey_hash_line_t *line = NULL;
	for(index=0, line=tbl->hash_lines; index<lines; index++, line++)
	{
		ey_hash_entry_t *to_del = NULL, *next = NULL;
		TAILQ_FOREACH_SAFE(to_del, &line->entry_head, link, next)
		{
			if(tbl->destroy_fn)
				tbl->destroy_fn(to_del->value);
			ey_zfree(tbl->slab, to_del);
		}
	}
	ey_zfinit(tbl->slab);
	tbl->memory_fn.free(tbl);
}

void* ey_hash_find(ey_hash_t hash, void *key)
{
	if(!hash || !key)
		return NULL;
	
	ey_hash *tbl = (ey_hash*)hash;
	ey_hash_entry_t *entry = NULL;
	unsigned int index = tbl->mask & tbl->gen_fn(key);
	ey_hash_line_t *line = tbl->hash_lines + index;
	ey_rwlock_rdlock(&line->line_lock);
	TAILQ_FOREACH(entry, &line->entry_head, link)
	{
		if(!tbl->compare_fn(key, entry->value))
		{
			ey_rwlock_rdunlock(&line->line_lock);
			return entry->value;
		}
	}
	ey_rwlock_rdunlock(&line->line_lock);
	return NULL;
}

void* ey_hash_find_ex(ey_hash_t hash, void *key, hash_compare compare)
{
	if(!hash || !key || !compare)
		return NULL;
	
	ey_hash *tbl = (ey_hash*)hash;
	ey_hash_entry_t *entry = NULL;
	unsigned int index = tbl->mask & tbl->gen_fn(key);
	ey_hash_line_t *line = tbl->hash_lines + index;
	ey_rwlock_rdlock(&line->line_lock);
	TAILQ_FOREACH(entry, &line->entry_head, link)
	{
		if(!compare(key, entry->value))
		{
			ey_rwlock_rdunlock(&line->line_lock);
			return entry->value;
		}
	}
	ey_rwlock_rdunlock(&line->line_lock);
	return NULL;
}

int ey_hash_insert(ey_hash_t hash, void *key, void *value)
{
	if(!hash || !key || !value)
		return EY_HASH_BAD_PARAM;
	
	ey_hash *tbl = (ey_hash*)hash;
	unsigned int index = tbl->mask & tbl->gen_fn(key);
	ey_hash_line_t *line = tbl->hash_lines + index;
	ey_hash_entry_t *entry = (ey_hash_entry_t*)ey_zalloc(tbl->slab);
	if(!entry)
		return EY_HASH_NO_MEM;
	
	entry->value = value;
	ey_hash_entry_t *tmp = NULL;
	ey_rwlock_wtlock(&line->line_lock);
	TAILQ_FOREACH(tmp, &line->entry_head, link)
	{
		if(!tbl->compare_fn(key, tmp->value))
		{
			ey_rwlock_wtunlock(&line->line_lock);
			ey_zfree(tbl->slab, entry);
			return EY_HASH_CONFLICT;
		}
	}
	ey_spinlock_lock(&tbl->hash_lock);
	if(tbl->count >= tbl->limit)
	{
		ey_spinlock_unlock(&tbl->hash_lock);
		ey_rwlock_wtunlock(&line->line_lock);
		ey_zfree(tbl->slab, entry);
		return EY_HASH_LIMIT;
	}
	tbl->count++;
	ey_spinlock_unlock(&tbl->hash_lock);
	TAILQ_INSERT_HEAD(&line->entry_head, entry, link);
	ey_rwlock_wtunlock(&line->line_lock);
	return 0;
}

int ey_hash_remove(ey_hash_t hash, void *key, void **value)
{
	if(!hash || !key)
		return EY_HASH_BAD_PARAM;
	
	if(value)
		*value = NULL;

	ey_hash *tbl = (ey_hash*)hash;
	unsigned int index = tbl->mask & tbl->gen_fn(key);
	ey_hash_line_t *line = tbl->hash_lines + index;
	ey_hash_entry_t *tmp = NULL;
	ey_rwlock_wtlock(&line->line_lock);
	TAILQ_FOREACH(tmp, &line->entry_head, link)
	{
		if(!tbl->compare_fn(key, tmp->value))
		{
			ey_spinlock_lock(&tbl->hash_lock);
			if(!tbl->count)
				*(int*)0 = 0;
			tbl->count--;
			ey_spinlock_unlock(&tbl->hash_lock);
			TAILQ_REMOVE(&line->entry_head, tmp, link);
			ey_rwlock_wtunlock(&line->line_lock);
			if(tbl->destroy_fn)
				tbl->destroy_fn(tmp->value);
			else if(value)
				*value = tmp->value;
			ey_zfree(tbl->slab, tmp);
			return 0;
		}
	}
	ey_rwlock_wtunlock(&line->line_lock);
	return EY_HASH_NO_KEY;
}

int ey_hash_remove_all(ey_hash_t hash, void *key, hash_compare compare)
{
	if(!hash || !key || !compare)
		return EY_HASH_BAD_PARAM;
	
	ey_hash *tbl = (ey_hash*)hash;
	unsigned int index = 0, lines=tbl->mask+1;
	ey_hash_line_t *line = NULL;

	for(index=0, line=tbl->hash_lines; index<lines; index++, line++)
	{
		ey_hash_entry_t *entry = NULL, *tmp = NULL;
		ey_rwlock_wtlock(&line->line_lock);
		TAILQ_FOREACH_SAFE(entry, &line->entry_head, link, tmp)
		{
			if(!compare(key, entry->value))
			{
				ey_spinlock_lock(&tbl->hash_lock);
				if(!tbl->count)
					*(int*)0 = 0;
				tbl->count--;
				ey_spinlock_unlock(&tbl->hash_lock);
				TAILQ_REMOVE(&line->entry_head, entry, link);
				if(tbl->destroy_fn)
					tbl->destroy_fn(entry->value);
				ey_zfree(tbl->slab, entry);
			}
		}
		ey_rwlock_wtunlock(&line->line_lock);
	}
	return 0;
}

int ey_hash_foreach(ey_hash_t hash, void *key, hash_compare compare, hash_foreach foreach, void *arg)
{
	if(!hash || !foreach)
		return EY_HASH_BAD_PARAM;
	
	ey_hash *tbl = (ey_hash*)hash;
	unsigned int index = 0, lines=tbl->mask+1;
	ey_hash_line_t *line = NULL;

	for(index=0, line=tbl->hash_lines; index<lines; index++, line++)
	{
		ey_hash_entry_t *entry = NULL, *tmp = NULL;
		ey_rwlock_rdlock(&line->line_lock);
		TAILQ_FOREACH_SAFE(entry, &line->entry_head, link, tmp)
		{
			if(compare && !compare(key, entry->value))
			{
				if(foreach(entry->value, arg))
				{
					ey_rwlock_rdunlock(&line->line_lock);
					goto out;
				}
			}
			else if(!compare)
			{
				if(foreach(entry->value, arg))
				{
					ey_rwlock_rdunlock(&line->line_lock);
					goto out;
				}
			}
		}
		ey_rwlock_rdunlock(&line->line_lock);
	}
out:
	return 0;
}
