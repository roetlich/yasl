#include <interpreter/VM.h>
#include "yasl-std-collections.h"
#include "set.h"

#define YASL_SET (-4)

static struct YASL_HashTable *set_mt = NULL;


static int YASL_collections_set_new(struct YASL_State *S) {
	struct YASL_Object *obj = YASL_UserData(set_new(), YASL_SET, set_mt, set_del);
	vm_push((struct VM *)S, *obj);
	free(obj);
	return YASL_SUCCESS;
}

static int YASL_collections_set_tostr(struct YASL_State *S) {
	struct YASL_Object *set_obj = YASL_popobject(S);

	struct Set *set;
	if (YASL_isuserdata(set_obj, YASL_SET) == YASL_SUCCESS) {
		set = (struct Set *)YASL_UserData_getdata(set_obj);
	} else {
		return -1;
	}

	size_t string_count = 0;
	size_t string_size = 8;
	char *string = (char *)malloc(string_size);
	string_count += strlen("set(");
	memcpy(string, "set(", strlen("set("));
	if (set_length(set) == 0) {
		string[string_count++] = ')';
		YASL_pushstring(S, string, string_count);
		return YASL_SUCCESS;
	}
	FOR_SET(i, item, set) {
		vm_push((struct VM *)S, *item);
		vm_stringify_top((struct VM *) S);
		struct YASL_String *str = vm_popstr((struct VM *) S);
		while (string_count + yasl_string_len(str) >= string_size) {
			string_size *= 2;
			string = (char *) realloc(string, string_size);
		}

		memcpy(string + string_count, str->str + str->start, yasl_string_len(str));
		string_count += yasl_string_len(str);

		if (string_count + 2 >= string_size) {
			string_size *= 2;
			string = (char *) realloc(string, string_size);
		}

		string[string_count++] = ',';
		string[string_count++] = ' ';
	}

	string_count -= 2;
	string[string_count++] = ')';
	vm_pushstr((struct VM *)S, str_new_sized_heap(0, string_count, string));
	return YASL_SUCCESS;
}

#define YASL_COLLECTIONS_SET_BINOP(name, fn) \
static int YASL_collections_set_##name(struct YASL_State *S) {\
	struct YASL_Object *right_obj = YASL_popobject(S);\
\
	struct Set *right;\
	if (YASL_isuserdata(right_obj, YASL_SET) == YASL_SUCCESS) {\
		right = (struct Set *)YASL_UserData_getdata(right_obj);\
	} else {\
		return -1;\
	}\
\
	struct YASL_Object *left_obj = YASL_popobject(S);\
\
	struct Set *left;\
	if (YASL_isuserdata(left_obj, YASL_SET) == YASL_SUCCESS) {\
		left = (struct Set *)YASL_UserData_getdata(left_obj);\
	} else {\
		return -1;\
	}\
\
	struct Set *tmp = fn(left, right);\
\
	YASL_pushobject(S, YASL_UserData(tmp, YASL_SET, set_mt, set_del));\
	return YASL_SUCCESS;\
}

YASL_COLLECTIONS_SET_BINOP(__band, set_intersection)
YASL_COLLECTIONS_SET_BINOP(__bor, set_union)
YASL_COLLECTIONS_SET_BINOP(__bxor, set_symmetric_difference)
YASL_COLLECTIONS_SET_BINOP(__sub, set_difference)

static int YASL_collections_set___len(struct YASL_State *S) {
	struct YASL_Object *set_obj = YASL_popobject(S);

	struct Set *set;
	if (YASL_isuserdata(set_obj, YASL_SET) == YASL_SUCCESS) {
		set = (struct Set *)YASL_UserData_getdata(set_obj);
	} else {
		return -1;
	}

	YASL_pushinteger(S, set_length(set));
	return YASL_SUCCESS;
}

static int YASL_collections_set_add(struct YASL_State *S) {
	struct YASL_Object *right_obj = YASL_popobject(S);


	if (YASL_ISLIST(*right_obj) || YASL_ISTABLE(*right_obj) || YASL_ISUSERDATA(*right_obj)) {
		printf("Error: unable to insert mutable object of type %x into set.\n", right_obj->type);
		return -1;
	}
	struct YASL_Object left_obj = vm_peek((struct VM *)S);

	struct Set *left;
	if (YASL_isuserdata(&left_obj, YASL_SET) == YASL_SUCCESS) {
		left = (struct Set *)YASL_UserData_getdata(&left_obj);
	} else {
		return -1;
	}

	set_insert(left, *right_obj);

	return YASL_SUCCESS;
}

static int YASL_collections_set_remove(struct YASL_State *S) {
	struct YASL_Object right_obj =  vm_pop((struct VM *)S);


	if (YASL_ISLIST(right_obj) || YASL_ISTABLE(right_obj) || YASL_ISUSERDATA(right_obj)) {
		printf("Error: unable to insert mutable object of type %x into set.\n", right_obj.type);
		return -1;
	}
	struct YASL_Object left_obj = vm_pop((struct VM *)S);

	struct Set *left;
	if (YASL_isuserdata(&left_obj, YASL_SET) == YASL_SUCCESS) {
		left = (struct Set *)YASL_UserData_getdata(&left_obj);
	} else {
		return -1;
	}

	set_rm(left, right_obj);

	vm_push((struct VM *)S, right_obj);
	return YASL_SUCCESS;
}

static int YASL_collections_set_copy(struct YASL_State *S) {
	struct YASL_Object *set_obj = YASL_popobject(S);
	struct Set *set;
	if (YASL_isuserdata(set_obj, YASL_SET) == YASL_SUCCESS) {
		set = (struct Set *)YASL_UserData_getdata(set_obj);
	} else {
		return -1;
	}

	struct Set *tmp = set_new();
	FOR_SET(i, item, set) {
		set_insert(tmp, *item);
	}

	YASL_pushobject(S, YASL_UserData(tmp, YASL_SET, set_mt, set_del));
	return YASL_SUCCESS;
}

static int YASL_collections_set_clear(struct YASL_State *S) {
	struct YASL_Object set_obj = vm_peek((struct VM *)S);
	struct Set *set;
	if (YASL_isuserdata(&set_obj, YASL_SET) == YASL_SUCCESS) {
		set = (struct Set *)YASL_UserData_getdata(&set_obj);
	} else {
		return -1;
	}

	FOR_SET(i, item, set) {
			set_rm(set, *item);
	}

	return YASL_SUCCESS;
}

static int YASL_collections_set_contains(struct YASL_State *S) {
	struct YASL_Object *object = YASL_popobject(S);
	struct YASL_Object *set_obj = YASL_popobject(S);
	struct Set *set;
	if (YASL_isuserdata(set_obj, YASL_SET) == YASL_SUCCESS) {
		set = (struct Set *)YASL_UserData_getdata(set_obj);
	} else {
		return -1;
	}

	vm_push((struct VM *)S, set_search(set, *object));
	return YASL_SUCCESS;
}

int YASL_load_collections(struct YASL_State *S) {

	if (!set_mt) {
		set_mt = table_new();
		table_insert_literalcstring_cfunction(set_mt, "tostr", YASL_collections_set_tostr, 1);
		table_insert_literalcstring_cfunction(set_mt, "__band", YASL_collections_set___band, 2);
		table_insert_literalcstring_cfunction(set_mt, "__bor", YASL_collections_set___bor, 2);
		table_insert_literalcstring_cfunction(set_mt, "__bxor", YASL_collections_set___bxor, 2);
		table_insert_literalcstring_cfunction(set_mt, "__sub", YASL_collections_set___sub, 2);
		table_insert_literalcstring_cfunction(set_mt, "__len", YASL_collections_set___len, 1);
		table_insert_literalcstring_cfunction(set_mt, "add", YASL_collections_set_add, 2);
		table_insert_literalcstring_cfunction(set_mt, "remove", YASL_collections_set_remove, 2);
		table_insert_literalcstring_cfunction(set_mt, "copy", YASL_collections_set_copy, 1);
		table_insert_literalcstring_cfunction(set_mt, "clear", YASL_collections_set_clear, 1);
		table_insert_literalcstring_cfunction(set_mt, "contains", YASL_collections_set_contains, 2);
	}

	struct YASL_Object *collections = YASL_Table();

	struct YASL_Object *set_new_str = YASL_LiteralString("set");
	struct YASL_Object *set_new_fn = YASL_CFunction(YASL_collections_set_new, 0);

	YASL_Table_set(collections, set_new_str, set_new_fn);

	free(set_new_str);
	free(set_new_fn);

	YASL_declglobal(S, "collections");
	YASL_pushobject(S, collections);
	YASL_setglobal(S, "collections");

	// free(collections);
	return YASL_SUCCESS;
}

