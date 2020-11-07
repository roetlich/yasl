#include "str_methods.h"

#include "yasl.h"
#include "yasl_aux.h"
#include "yasl_include.h"
#include "yasl_types.h"
#include "yasl_state.h"

static struct YASL_String *YASLX_checkstr(struct YASL_State *S, const char *name, int pos) {
	if (!YASL_isstr(S)) {
		YASLX_print_err_bad_arg_type(S, name, pos, "str", YASL_peektypestr(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	return vm_popstr((struct VM *)S);
}


void str___get(struct YASL_State *S) {
	yasl_int index = YASLX_checkint(S, "str.__get", 1);
	struct YASL_String *str = YASLX_checkstr(S, "str.__get", 0);
	if (index < -(yasl_int) YASL_String_len(str) ||
		   index >= (yasl_int) YASL_String_len(
			   str)) {
		vm_print_err_value(&S->vm, "unable to index str of length %" PRI_SIZET " with index %" PRI_SIZET ".", YASL_String_len(str), index);
		YASL_throw_err(S, YASL_VALUE_ERROR);
	} else {
		if (index >= 0)
			vm_push((struct VM *) S, YASL_STR(
				YASL_String_new_substring(str->start + index,
							  str->start + index + 1,
							  str)));
		else
			vm_push((struct VM *) S,
				YASL_STR(YASL_String_new_substring(
					str->start + index + YASL_String_len(str),
					str->start + index + YASL_String_len(str) + 1,
					str)));
	}
}

void str_tofloat(struct YASL_State *S) {
	struct YASL_String *str = YASLX_checkstr(S, "str.tofloat", 0);
	YASL_pushfloat(S, YASL_String_tofloat(str));
}

void str_toint(struct YASL_State *S) {
	struct YASL_String *str = YASLX_checkstr(S, "str.toint", 0);
	YASL_pushint(S, YASL_String_toint(str));
}

void str_tobool(struct YASL_State* S) {
	struct YASL_String *a = YASLX_checkstr(S, "str.tobool", 0);
	YASL_pushbool(S, YASL_String_len(a) != 0);
}

void str_tostr(struct YASL_State *S) {
	if (!YASL_isstr(S)) {
		YASLX_print_err_bad_arg_type(S, "str.tostr", 0, "str", YASL_peektypestr(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
}

void str_toupper(struct YASL_State *S) {
	struct YASL_String *a = YASLX_checkstr(S, "str.toupper", 0);
	vm_push((struct VM *) S, YASL_STR(YASL_String_toupper(a)));
}

void str_tolower(struct YASL_State *S) {
	struct YASL_String *a = YASLX_checkstr(S, "str.tolower", 0);
	vm_push((struct VM *) S, YASL_STR(YASL_String_tolower(a)));
}

void str_isalnum(struct YASL_State *S) {
	struct YASL_String *a = YASLX_checkstr(S, "str.isalnum", 0);
	YASL_pushbool(S, YASL_String_isalnum(a));
}

void str_isal(struct YASL_State *S) {
	struct YASL_String *a = YASLX_checkstr(S, "str.isal", 0);
	YASL_pushbool(S, YASL_String_isal(a));
}

void str_isnum(struct YASL_State *S) {
	struct YASL_String *a = YASLX_checkstr(S, "str.isnum", 0);
	YASL_pushbool(S, YASL_String_isnum(a));
}

void str_isspace(struct YASL_State *S) {
	struct YASL_String *a = YASLX_checkstr(S, "str.isspace", 0);
	YASL_pushbool(S, YASL_String_isspace(a));
}

void str_startswith(struct YASL_State *S) {
	struct YASL_String *needle = YASLX_checkstr(S, "str.startswith", 1);
	struct YASL_String *haystack = YASLX_checkstr(S, "str.startswith", 0);
	YASL_pushbool(S, YASL_String_startswith(haystack, needle));
}

void str_endswith(struct YASL_State *S) {
	struct YASL_String *needle = YASLX_checkstr(S, "str.endswith", 1);
	struct YASL_String *haystack = YASLX_checkstr(S, "str.endswith", 0);
	YASL_pushbool(S, YASL_String_endswith(haystack, needle));
}

static void str_replace_default(struct YASL_State *S) {
	struct YASL_String *replace_str = YASLX_checkstr(S, "str.replace", 2);
	struct YASL_String *search_str = YASLX_checkstr(S, "str.replace", 1);
	struct YASL_String *str = YASLX_checkstr(S, "str.replace", 0);

	if (YASL_String_len(search_str) < 1) {
		vm_print_err((struct VM *)S,
			"ValueError: %s expected a nonempty str as arg 1.",
			"str.replace");
		YASL_throw_err(S, YASL_VALUE_ERROR);
	}

	vm_push((struct VM *) S, YASL_STR(YASL_String_replace_fast_default(str, search_str, replace_str)));
}

void str_replace(struct YASL_State *S) {
	if (YASL_isundef(S)) {
		YASL_pop(S);
		str_replace_default(S);
		return;
	}

	yasl_int max = YASLX_checkint(S, "str.replace", 3);
	struct YASL_String *replace_str = YASLX_checkstr(S, "str.replace", 2);
	struct YASL_String *search_str = YASLX_checkstr(S, "str.replace", 1);
	struct YASL_String *str = YASLX_checkstr(S, "str.replace", 0);

	if (YASL_String_len(search_str) < 1) {
		vm_print_err((struct VM *)S,
			     "ValueError: %s expected a nonempty str as arg 1.",
			     "str.replace");
		YASL_throw_err(S, YASL_VALUE_ERROR);
	}

	vm_push((struct VM *) S, YASL_STR(YASL_String_replace_fast(str, search_str, replace_str, max)));
}

void str_search(struct YASL_State *S) {
	struct YASL_String *needle = YASLX_checkstr(S, "str.search", 1);
	struct YASL_String *haystack = YASLX_checkstr(S, "str.search", 0);

	int64_t index = str_find_index(haystack, needle);
	if (index != -1) YASL_pushint(S, index);
	else YASL_pushundef(S);
}

void str_count(struct YASL_State *S) {
	struct YASL_String *needle = YASLX_checkstr(S, "str.count", 1);
	struct YASL_String *haystack = YASLX_checkstr(S, "str.count", 0);
	YASL_pushint(S, YASL_String_count(haystack, needle));
}

static void str_split_default(struct YASL_State *S) {
	struct YASL_String *haystack = YASLX_checkstr(S, "str.split", 0);
	vm_push((struct VM *) S, YASL_LIST(string_split_default(haystack)));
}

void str_split(struct YASL_State *S) {
	if (YASL_isundef(S)) {
		YASL_pop(S);
		str_split_default(S);
		return;
	}
	struct YASL_String *needle = YASLX_checkstr(S, "str.split", 1);
	struct YASL_String *haystack = YASLX_checkstr(S, "str.split", 0);

	if (YASL_String_len(needle) == 0) {
		vm_print_err((struct VM *)S, "ValueError: %s expected a nonempty str as arg 1.", "str.split");
		YASL_throw_err(S, YASL_VALUE_ERROR);
	}

	vm_push((struct VM *) S, YASL_LIST(YASL_String_split_fast(haystack, needle)));
}

static void str_ltrim_default(struct YASL_State *S) {
	if (!YASL_isstr(S)) {
		YASLX_print_err_bad_arg_type(S, "str.ltrim", 0, "str", YASL_peektypestr(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	struct YASL_Object top = vm_peek((struct VM *) S);
	inc_ref(&top);
	struct YASL_String *haystack = vm_popstr(&S->vm);

	vm_push((struct VM *) S, YASL_STR(YASL_String_ltrim_default(haystack)));
	dec_ref(&top);
}

void str_ltrim(struct YASL_State *S) {
	if (YASL_isundef(S)) {
		YASL_pop(S);
		str_ltrim_default(S);
		return;
	}
	struct YASL_String *needle = YASLX_checkstr(S, "str.ltrim", 1);
	struct YASL_String *haystack = YASLX_checkstr(S, "str.ltrim", 0);

	vm_push((struct VM *) S,
		YASL_STR(YASL_String_ltrim(haystack, needle)));
}

static void str_rtrim_default(struct YASL_State *S) {
	if (!YASL_isstr(S)) {
		YASLX_print_err_bad_arg_type(S, "str.rtrim", 0, "str", YASL_peektypestr(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	struct YASL_Object top = vm_peek((struct VM *) S);
	inc_ref(&top);
	struct YASL_String *haystack = vm_popstr(&S->vm);

	vm_push((struct VM *) S, YASL_STR(YASL_String_rtrim_default(haystack)));
	dec_ref(&top);
}

void str_rtrim(struct YASL_State *S) {
	if (YASL_isundef(S)) {
		vm_pop((struct VM *) S);
		str_rtrim_default(S);
		return;
	}
	struct YASL_String *needle = YASLX_checkstr(S, "str.rtrim", 1);
	struct YASL_String *haystack = YASLX_checkstr(S, "str.rtrim", 0);

	vm_push((struct VM *) S, YASL_STR(YASL_String_rtrim(haystack, needle)));
}

static void str_trim_default(struct YASL_State *S) {
	if (!YASL_isstr(S)) {
		YASLX_print_err_bad_arg_type(S, "str.trim", 0, "str", YASL_peektypestr(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	struct YASL_Object top = vm_peek((struct VM *) S);
	inc_ref(&top);
	struct YASL_String *haystack = vm_popstr(&S->vm);

	vm_push((struct VM *) S, YASL_STR(YASL_String_trim_default(haystack)));
	dec_ref(&top);
}

void str_trim(struct YASL_State *S) {
	if (YASL_isundef(S)) {
		YASL_pop(S);
		str_trim_default(S);
		return;
	}
	struct YASL_String *needle = YASLX_checkstr(S, "str.trim", 1);
	if (!YASL_isstr(S)) {
		YASLX_print_err_bad_arg_type(S, "str.trim", 0, "str", YASL_peektypestr(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	struct YASL_Object top = vm_peek((struct VM *) S);
	inc_ref(&top);
	struct YASL_String *haystack = vm_popstr(&S->vm);

	vm_push((struct VM *) S, YASL_STR(YASL_String_trim(haystack, needle)));
	dec_ref(&top);
}

void str_repeat(struct YASL_State *S) {
	yasl_int num = YASLX_checkint(S, "str.rep", 1);
	struct YASL_String *string = YASLX_checkstr(S, "str.rep", 0);

	if (num < 0) {
		vm_print_err((struct VM *)S, "ValueError: %s expected non-negative int as arg 1.", "str.rep");
		YASL_throw_err(S, YASL_VALUE_ERROR);
	}

	vm_push((struct VM *) S, YASL_STR(YASL_String_rep_fast(string, num)));
}
