#include "YASL_HashTable.h"

#include <string.h>

#include "data-structures/YASL_String.h"
#include "debug.h"
#include "hash_function.h"
#include "interpreter/refcount.h"
#include "interpreter/YASL_Object.h"

#define TABLE_BASESIZE 30

struct YASL_HashTable_Item TOMBSTONE = { { Y_END, { Y_END } }, { Y_END, { Y_END } } };

static struct YASL_HashTable_Item new_item(const struct YASL_Object k, const struct YASL_Object v) {
	struct YASL_HashTable_Item item = {k, v};
	inc_ref(&item.value);
	inc_ref(&item.key);
	return item;
}

void del_item(struct YASL_HashTable_Item *const item) {
	dec_ref(&item->key);
	dec_ref(&item->value);
}

struct YASL_HashTable *table_new_sized(const size_t base_size) {
	struct YASL_HashTable *table = (struct YASL_HashTable *)malloc(sizeof(struct YASL_HashTable));
	table->base_size = base_size;
	table->size = next_prime(table->base_size);
	table->count = 0;
	table->items = (struct YASL_HashTable_Item *)calloc((size_t) table->size, sizeof(struct YASL_HashTable_Item));
	return table;
}

struct YASL_HashTable *table_new(void) {
	return table_new_sized(TABLE_BASESIZE);
}

void table_del(struct YASL_HashTable *const table) {
	if (!table) return;
	FOR_TABLE(i, item, table) {
		del_item(item);
	}
	free(table->items);
	free(table);
}

struct RC_UserData *rcht_new_sized(const size_t base_size) {
        struct RC_UserData *ht = (struct RC_UserData *)malloc(sizeof(struct RC_UserData));
        ht->data = table_new_sized(base_size);
        ht->rc = rc_new();
        ht->tag = T_TABLE;
        ht->destructor = rcht_del_data;
        return ht;
}

struct RC_UserData *rcht_new(void) {
	return rcht_new_sized(TABLE_BASESIZE);
}

void rcht_del(struct RC_UserData *const hashtable) {
	table_del((struct YASL_HashTable *)hashtable->data);
	rc_del(hashtable->rc);
	free(hashtable);
}

void rcht_del_data(void *const hashtable) {
        table_del((struct YASL_HashTable *)hashtable);
}

void rcht_del_rc(struct RC_UserData *const hashtable) {
	rc_del(hashtable->rc);
	free(hashtable);
}

void table_del_string_int(struct YASL_HashTable *const table) {
	table_del(table);
}

static void table_resize(struct YASL_HashTable *const table, const size_t base_size) {
	if (base_size < TABLE_BASESIZE) return;
	struct YASL_HashTable *new_table = table_new_sized(base_size);
	FOR_TABLE(i, item, table) {
		table_insert(new_table, item->key, item->value);
	}
	table->base_size = new_table->base_size;
	table->count = new_table->count;

	const size_t tmp_size = table->size;
	table->size = new_table->size;
	new_table->size = tmp_size;

	struct YASL_HashTable_Item *tmp_items = table->items;
	table->items = new_table->items;
	new_table->items = tmp_items;

	table_del(new_table);
}

static void table_resize_up(struct YASL_HashTable *const table) {
	const size_t new_size = table->base_size * 2;
	table_resize(table, new_size);
}

static void table_resize_down(struct YASL_HashTable *const table) {
	const size_t new_size = table->base_size / 2;
	table_resize(table, new_size);
}

void table_insert(struct YASL_HashTable *const table, const struct YASL_Object key, const struct YASL_Object value) {
	const size_t load = table->count * 100 / table->size;
	if (load > 70) table_resize_up(table);
	struct YASL_HashTable_Item item = new_item(key, value);
	size_t index = get_hash(item.key, table->size, 0);
	struct YASL_HashTable_Item curr_item = table->items[index];
	size_t i = 1;
	while (!YASL_ISUNDEF(curr_item.key)) {
		if (curr_item.key.type != Y_END) {
			if (!isfalsey(isequal(curr_item.key, item.key))) {
				del_item(&curr_item);
				table->items[index] = item;
				return;
			}
		}
		index = get_hash(item.key, table->size, i++);
		curr_item = table->items[index];
	}
	table->items[index] = item;
	table->count++;
}

void table_insert_string_int(struct YASL_HashTable *const table, const char *const key, const size_t key_len, const int64_t val) {
	struct YASL_String *string = YASL_String_new_sized_heap(0, key_len, copy_char_buffer(key_len, key));
	struct YASL_Object ko = YASL_STR(string);
	struct YASL_Object vo = YASL_INT(val);
	table_insert(table, ko, vo);
}

void table_insert_literalcstring_cfunction(struct YASL_HashTable *const ht, const char *key, int (*addr)(struct YASL_State *), const int num_args) {
	struct YASL_String *string = YASL_String_new_sized(strlen(key), key);
	struct YASL_Object f = YASL_CFN(addr, num_args);
	struct YASL_Object s = YASL_STR(string);
	table_insert(ht, s, f);
}

struct YASL_Object table_search(const struct YASL_HashTable *const table, const struct YASL_Object key) {
	YASL_ASSERT(table != NULL, "table to search should not be NULL");
	size_t index = get_hash(key, table->size, 0);
	struct YASL_HashTable_Item item = table->items[index];
	int i = 1;
	while (!YASL_ISUNDEF(item.key)) {
		if (!isfalsey(isequal(item.key, key))) {
			return item.value;
		}
		index = get_hash(key, table->size, i++);
		item = table->items[index];
	}
	return YASL_END();
}

struct YASL_Object table_search_string_int(const struct YASL_HashTable *const table, const char *const key, const size_t key_len) {
	struct YASL_String *string = YASL_String_new_sized_heap(0, key_len, copy_char_buffer(key_len, key));
	struct YASL_Object object = YASL_STR(string);

	struct YASL_Object result = table_search(table, object);
	str_del(string);
	return result;
}

void table_rm(struct YASL_HashTable *const table, const struct YASL_Object key) {
	const size_t load = table->count * 100 / table->size;
	if (load < 10) table_resize_down(table);
	size_t index = get_hash(key, table->size, 0);
	struct YASL_HashTable_Item item = table->items[index];
	size_t i = 1;
	while (!YASL_ISUNDEF(item.key)) {
		if (item.key.type != Y_END) {
			if (!isfalsey(isequal(item.key, key))) {
				del_item(&item);
				table->items[index] = TOMBSTONE;
			}
		}
		index = get_hash(key, table->size, i++);
		item = table->items[index];
	}
	table->count--;
}
