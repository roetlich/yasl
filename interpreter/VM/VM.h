#pragma once

#include "hashtable/hashtable.h"
#include "yasl_conf.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define STACK_SIZE 100024
#define NUM_TYPES 13                                     // number of builtin types, each needs a vtable
#define PEEK(vm)     (vm->stack[vm->sp])                 // pop value from top of stack
#define BPUSH(vm, v) (vm_push(vm, YASL_BOOL(v)))  //push boolean v onto stack


#define BUFFER_SIZE 256
#define NCODE(vm)    (vm->code[vm->pc++])     // get next bytecode

#define GT(a, b) ((a) > (b))
#define GE(a, b) ((a) >= (b))
#define COMP(vm, a, b, f, str)  do {\
                            if (a.type == Y_INT64 && b.type == Y_INT64) {\
                                c = f(a.value.ival, b.value.ival);\
                            }\
                            else if (a.type == Y_FLOAT64 && b.type == Y_INT64) {\
                                c = f(a.value.dval, (yasl_float)b.value.ival);\
                            }\
                            else if (a.type == Y_INT64 && b.type == Y_FLOAT64) {\
                                c = f((yasl_float)a.value.ival, (b).value.dval);\
                            }\
                            else if (a.type == Y_FLOAT64 && b.type == Y_FLOAT64) {\
                                c = f(a.value.dval, (b).value.dval);\
                            }\
                            else {\
                                printf("TypeError: %s not supported for operands of types %s and %s.\n", str,\
                                        YASL_TYPE_NAMES[a.type], YASL_TYPE_NAMES[b.type]);\
                                return;\
                            }\
                            BPUSH(vm, c);} while(0);

struct VM{
	struct YASL_Object *globals;          // variables, see "constant.c" for details on YASL_Object.
    size_t num_globals;
	struct YASL_Object *stack;            // stack
	unsigned char *code;           // bytecode
	size_t pc;                    // program counter
    size_t pc0;                   // initial value for pc
	int sp;                     // stack pointer
	int fp;                     // frame pointer
    int next_fp;
    int lp;                     // foreach pointer
	struct RC_Table **builtins_htable;   // htable of builtin methods
};

struct VM* vm_new(unsigned char *code,    // pointer to bytecode
           int pc0,             // address of instruction to be executed first -- entrypoint
           int datasize);       // total params size required to perform a program operations

void vm_del(struct VM *vm);

struct YASL_Object vm_pop(struct VM *vm);
void vm_push(struct VM *vm, struct YASL_Object val);

void vm_run(struct VM *vm);

struct RC_Table* float_builtins(void);
struct RC_Table* int_builtins(void);
struct RC_Table* bool_builtins(void);
struct RC_Table* str_builtins(void);
struct RC_Table* list_builtins(void);
struct RC_Table* table_builtins(void);
