#include <stdlib.h>
#include <memory.h>
#include "VM.h"
#include "interpreter/builtins/builtins.h"

VM* newVM(char* code,    // pointer to bytecode
    int pc0,             // address of instruction to be executed first -- entrypoint
    int datasize) {      // total locals size required to perform a program operations
    VM* vm = malloc(sizeof(VM));
    vm->code = code;
    vm->pc = pc0;
    vm->pc0 = vm->pc;
    vm->fp = 0;
    vm->sp = -1;
    vm->globals = malloc(sizeof(YASL_Object) * datasize);
    vm->globals[0] = (YASL_Object) {FILEH, (int64_t)stdin};
    vm->globals[1] = (YASL_Object) {FILEH, (int64_t)stdout};
    vm->globals[2] = (YASL_Object) {FILEH, (int64_t)stderr};
    vm->stack = malloc(sizeof(YASL_Object) * STACK_SIZE);
    vm->builtins_vtable = malloc(sizeof(VTable_t*) * NUM_TYPES);
    vm->builtins_vtable[1] = float64_builtins();
    vm->builtins_vtable[2] = int64_builtins();
    vm->builtins_vtable[3] = bool_builtins();
    vm->builtins_vtable[4] = str8_builtins();
    vm->builtins_vtable[5] = list_builtins();
    vm->builtins_vtable[6] = map_builtins();
    vm->builtins_vtable[7] = file_builtins();
    return vm;
}

void delVM(VM* vm){
    free(vm->globals);                   // TODO: free these properly
    free(vm->stack);                     // TODO: free these properly
    //del_vtable(vm->builtins_vtable[0]);
    del_vtable(vm->builtins_vtable[1]);
    del_vtable(vm->builtins_vtable[2]);
    del_vtable(vm->builtins_vtable[3]);
    del_vtable(vm->builtins_vtable[4]);
    del_vtable(vm->builtins_vtable[5]);
    del_vtable(vm->builtins_vtable[6]);
    del_vtable(vm->builtins_vtable[7]);
    free(vm->builtins_vtable);
        free(vm);
}


void run(VM* vm){
    while (1) {
        unsigned char opcode = NCODE(vm);        // fetch
        int argc, rval;
        signed char offset;
        uint64_t big_offset, size;
        int64_t addr;
        int i;
        YASL_Object a, b, v;
        int64_t c;
        double d;
        void* ptr;
        /*printf("\nopcode: %x\n", opcode);
        printf("tpye is: %s\n", YASL_TYPE_NAMES[PEEK(vm).type]);
        print(PEEK(vm));

        //print(vm->stack[vm->sp-1]);
        //print(vm->stack[vm->sp-2]);
        //print(vm->stack[vm->sp-3]);
        //print(vm->globals[0]);
        //print(vm->globals[1]);
        //print(vm->globals[2]);
        //print(vm->globals[3]);
        //print(vm->globals[4]);
        //print(vm->globals[5]); //*/

        //printf("pc: %d\n", vm->pc);
        //printf("vm->sp: %d, vm->pc: %d\n", vm->sp, vm->pc);
        /*printf("stack[0, 1, 2, 3] are %d:%x, %d:%x, %d:%x, %d:%x\n",
               (int)vm->stack[vm->sp].value,     (int)vm->stack[vm->sp].type,
               (int)vm->stack[vm->sp - 1].value, (int)vm->stack[vm->sp - 1].type,
               (int)vm->stack[vm->sp - 2].value, (int)vm->stack[vm->sp - 2].type,
               (int)vm->stack[vm->sp - 3].value, (int)vm->stack[vm->sp - 3].type); //
        /*printf("globals[0, 1, 2, 3] are %d:%x, %d:%x, %d:%x, %d:%x\n",
               (int)vm->globals[0].value, (int)vm->globals[0].type,
               (int)vm->globals[1].value, (int)vm->globals[1].type,
               (int)vm->globals[2].value, (int)vm->globals[2].type,
               (int)vm->globals[3].value, (int)vm->globals[3].type); //*/
        switch (opcode) {   // decode
            case HALT: return;  // stop the program
            case NOP: break;    // pass
            case ICONST_M1:     // TODO: make sure no changes to opcodes ruin this
            case ICONST_0:
            case ICONST_1:
            case ICONST_2:
            case ICONST_3:
            case ICONST_4:
            case ICONST_5:
                vm->stack[++vm->sp].type = INT64;
                vm->stack[vm->sp].value  = opcode - 0x04;
                break;
            case DCONST_0:    // TODO: make sure no changes to opcodes ruin this
            case DCONST_1:
            case DCONST_2:
                vm->stack[++vm->sp].type = FLOAT64;
                d = opcode - 0x0B;
                memcpy(&vm->stack[vm->sp].value, &d, sizeof(double));
                break;
            case DCONST:        // constants have native endianness
                memcpy(&c, vm->code + vm->pc, sizeof c);
                vm->pc += sizeof c;
                PUSH(vm, ((YASL_Object) {FLOAT64, c}));
                break;
            case ICONST:        // constants have native endianness
                memcpy(&c, vm->code + vm->pc, sizeof c);
                vm->pc += sizeof c;
                PUSH(vm, ((YASL_Object) {INT64, c}));
                break;
            case BCONST_F:
            case BCONST_T:
                vm->stack[++vm->sp].type = BOOL;
                vm->stack[vm->sp].value  = opcode & 0x01;
                break;
            case NCONST:
                vm->stack[++vm->sp].type = UNDEF;
                vm->stack[vm->sp].value  = 0x00;
                break;
            case BOR:
                b = POP(vm);
                a = PEEK(vm);
                if (a.type == INT64 && b.type == INT64) {
                    PEEK(vm).value = a.value | b.value;
                    break;
                } else {
                    printf("TypeError: | not supported for operands of types %s and %s.\n",
                           YASL_TYPE_NAMES[a.type],
                           YASL_TYPE_NAMES[b.type]);
                    return;
                }
            case BXOR:
                b = POP(vm);
                a = PEEK(vm);
                if (a.type == INT64 && b.type == INT64) {
                    PEEK(vm).value = a.value ^ b.value;
                    break;
                } else {
                    printf("TypeError: binary ~ not supported for operands of types %s and %s.\n",
                           YASL_TYPE_NAMES[a.type],
                           YASL_TYPE_NAMES[b.type]);
                    return;
                }
            case BAND:
                b = POP(vm);
                a = PEEK(vm);
                if (a.type == INT64 && b.type == INT64) {
                    PEEK(vm).value = a.value & b.value;
                    break;
                } else {
                    printf("TypeError: & not supported for operands of types %s and %s.\n",
                           YASL_TYPE_NAMES[a.type],
                           YASL_TYPE_NAMES[b.type]);
                    return;
                }
            case BNOT:
                a = PEEK(vm);
                if (a.type == INT64) {
                    PEEK(vm).value = ~a.value;
                    break;
                } else {
                    printf("TypeError: unary ~ not supported for operand of type %s.\n",
                           YASL_TYPE_NAMES[a.type]);
                    return;
                }
            case BLSHIFT:
                b = POP(vm);
                a = PEEK(vm);
                if (a.type == INT64 && b.type == INT64) {
                    PEEK(vm).value = a.value << b.value;
                    break;
                } else {
                    printf("TypeError: << not supported for operands of types %s and %s.\n",
                           YASL_TYPE_NAMES[a.type],
                           YASL_TYPE_NAMES[b.type]);
                    return;
                }
            case BRSHIFT:
                b = POP(vm);
                a = PEEK(vm);
                if (a.type == INT64 && b.type == INT64) {
                    PEEK(vm).value = (uint64_t)a.value >> (uint64_t)b.value;
                    break;
                } else {
                    printf("TypeError: >> not supported for operands of types %s and %s.\n",
                           YASL_TYPE_NAMES[a.type],
                           YASL_TYPE_NAMES[b.type]);
                    return;
                }
            case ADD:
                b = vm->stack[vm->sp--];
                a = vm->stack[vm->sp--];
                BINOP(vm, a, b, ADD, "+");
                break;
            case MUL:
                b = vm->stack[vm->sp--];
                a = vm->stack[vm->sp--];
                BINOP(vm, a, b, MUL, "*");
                break;
            case SUB:
                b = vm->stack[vm->sp--];
                a = vm->stack[vm->sp--];
                BINOP(vm, a, b, SUB, "binary -");
                break;
            case FDIV:    // handled differently because we always convert to float
                b = vm->stack[vm->sp--];
                a = vm->stack[vm->sp--];
                if (a.type == INT64 && b.type == INT64) {
                    d = (double)a.value / (double)b.value;
                }
                else if (a.type == FLOAT64 && b.type == INT64) {
                    d = DVAL(a) / (double)b.value;
                }
                else if (a.type == INT64 && b.type == FLOAT64) {
                    d = (double)a.value / DVAL(b);
                }
                else if (a.type == FLOAT64 && b.type == FLOAT64) {
                    d = DVAL(a) / DVAL(b);
                }
                else {
                    printf("TypeError: / not supported for operands of types %s and %s.\n",
                           YASL_TYPE_NAMES[a.type],
                           YASL_TYPE_NAMES[b.type]);
                    return;
                }
                vm->stack[++vm->sp].type = FLOAT64;
                memcpy(&vm->stack[vm->sp].value, &d, sizeof(d));
                break;
            case IDIV:
                b = POP(vm);
                a = PEEK(vm);
                if (a.type == INT64 && b.type == INT64) {
                    //printf("a = %d, b = %d, a/b = %d\n", a.value, b.value, a.value / b.value);
                    PEEK(vm).value = a.value / b.value;
                    break;
                } else {
                    printf("TypeError: // not supported for operands of types %s and %s.\n",
                           YASL_TYPE_NAMES[a.type],
                           YASL_TYPE_NAMES[b.type]);
                    return;
                }
            case MOD:
                b = vm->stack[vm->sp--];
                a = vm->stack[vm->sp];
                if (a.type != INT64 || b.type != INT64) {
                    printf("TypeError: %% not supported for operands of types %s and %s.\n",
                           YASL_TYPE_NAMES[a.type],
                           YASL_TYPE_NAMES[b.type]);
                    return;
                }
                vm->stack[vm->sp].value = a.value % b.value;
                break;
            case EXP:
                b = POP(vm);
                a = POP(vm);
                BINOP(vm, a, b, EXP, "^");
                break;
            case NEG:
                a = vm->stack[vm->sp];
                if (a.type == INT64) {
                    vm->stack[vm->sp].value = -a.value;
                    break;
                }
                else if (a.type == FLOAT64) {
                    memcpy(&d, &vm->stack[vm->sp].value, sizeof(double));
                    d = -d;
                    memcpy(&vm->stack[vm->sp].value, &d, sizeof(double));
                    break;
                }
                else {
                    printf("TypeError: unary - not supported for operand of type %s.\n",
                           YASL_TYPE_NAMES[a.type]);
                    return;
                }
            case NOT:
                a = vm->stack[vm->sp];
                if (a.type == BOOL) {
                    vm->stack[vm->sp].value ^= 1;    // flip the last bit
                    break;
                }
                else if (a.type == UNDEF) {
                    break;
                }
                else {
                    printf("TypeError: ! not supported for operand of type %s.\n",
                           YASL_TYPE_NAMES[a.type]);
                    return;
                }
            case LEN:
                v = vm->stack[vm->sp];
                if (v.type == STR8) {
                    vm->stack[vm->sp].value = ((String_t*)v.value)->length; // (int64_t*)v.value;
                } else if (v.type == MAP) {
                    vm->stack[vm->sp].value = ((Hash_t*)v.value)->count;
                } else if (v.type == LIST) {
                    vm->stack[vm->sp].value = ((List_t*)v.value)->count;
                } else {
                    printf("TypeError: # not supported for operand of type %s.\n",
                           YASL_TYPE_NAMES[v.type]);
                    return;
                }
                vm->stack[vm->sp].type = INT64;
                break;
            case CNCT:
                b = vm->stack[vm->sp--];
                a = vm->stack[vm->sp];
                if (a.type == STR8 && b.type == STR8) {
                    size = ((String_t*)a.value)->length + ((String_t*)b.value)->length;
                    ptr = new_sized_string8(size);
                    vm->stack[vm->sp].value = (int64_t)ptr;
                    ((String_t*)vm->stack[vm->sp].value)->length = size;
                    memcpy(((String_t*)ptr)->str, ((String_t*)a.value)->str, ((String_t*)a.value)->length);
                    memcpy(((String_t*)ptr)->str + ((String_t*)a.value)->length, ((String_t*)b.value)->str, ((String_t*)b.value)->length);
                    break;
                }
                printf("TypeError: || not supported for operands of types %s and %s.\n",
                       YASL_TYPE_NAMES[a.type],
                       YASL_TYPE_NAMES[b.type]);
                return;
            case HARD_CNCT:
                b = POP(vm);
                a = PEEK(vm);
                if (a.type != STR8 || b.type != STR8) {
                    puts("||| should have coerced to strings, aborting.");
                    return;
                }
                if (((String_t*)b.value)->length == 0) break;
                if (((String_t*)a.value)->length == 0) {
                    PEEK(vm) = b;
                    break;
                }
                size = ((String_t*)a.value)->length + ((String_t*)b.value)->length + 1;
                ptr = new_sized_string8(size);
                vm->stack[vm->sp].value = (int64_t)ptr;
                ((String_t*)vm->stack[vm->sp].value)->length = size;
                memcpy(((String_t*)ptr)->str, ((String_t*)a.value)->str, ((String_t*)a.value)->length);
                ((String_t*)PEEK(vm).value)->str[((String_t*)a.value)->length] = ' ';
                memcpy(((String_t*)ptr)->str + ((String_t*)a.value)->length + 1, ((String_t*)b.value)->str, ((String_t*)b.value)->length);
                break;
            case GT:
                b = POP(vm);
                a = POP(vm);
                if ((a.type != INT64 && a.type != FLOAT64) ||
                    (b.type != INT64 && b.type != FLOAT64)) {
                    printf("TypeError: < and > not supported for operand of types %s and %s.\n",
                           YASL_TYPE_NAMES[a.type],
                           YASL_TYPE_NAMES[b.type]);
                    return;
                }
                COMP(vm, a, b, GT, ">");
                break;
            case GE:
                b = POP(vm);
                a = POP(vm);
                if ((a.type != INT64 && a.type != FLOAT64) ||
                    (b.type != INT64 && b.type != FLOAT64)) {
                    printf("TypeError: <= and >= not supported for operand of types %s and %s.\n",
                           YASL_TYPE_NAMES[a.type],
                           YASL_TYPE_NAMES[b.type]);
                    return;
                }
                COMP(vm, a, b, GE, ">=");
                break;
            case EQ:
                b = POP(vm);
                a = PEEK(vm);
                vm->stack[vm->sp] = (vm, isequal(a, b));
                break;
            case ID:
                b = POP(vm);
                a = PEEK(vm);
                vm->stack[vm->sp].value = a.type == b.type && a.value == b.value;
                vm->stack[vm->sp].type = BOOL;
                break;
            case NEWSTR8:
                vm->stack[++vm->sp].type = STR8;

                memcpy(&size, vm->code+vm->pc, sizeof(int64_t));
                vm->pc += sizeof(int64_t);

                memcpy(&addr, vm->code+vm->pc, sizeof(int64_t));
                vm->pc += sizeof(int64_t);

                vm->stack[vm->sp].value  = (int64_t)new_sized_string8(size);


                ((String_t*)(vm->stack[vm->sp].value))->length = size;

                memcpy(((String_t*)vm->stack[vm->sp].value)->str, vm->code+addr, size);
                //vm->pc += size;
                break;
            case NEWMAP:
                vm->stack[++vm->sp].type = MAP;
                vm->stack[vm->sp].value  = (int64_t)new_hash();
                break;
            case NEWLIST:
                vm->stack[++vm->sp].type = LIST;
                vm->stack[vm->sp].value  = (int64_t)new_list();
                break;
            case DUP:
                vm->stack[vm->sp+1] = vm->stack[vm->sp];
                vm->sp++;
                break;
            case SWAP:
                a = vm->stack[vm->sp];
                b = vm->stack[vm->sp-1];
                vm->stack[vm->sp-1] = a;
                vm->stack[vm->sp] = b;
                break;
            case GOTO:
                memcpy(&c, vm->code + vm->pc, sizeof c);
                vm->pc = c + vm->pc0;
                break;
            case BR_8:
                memcpy(&c, vm->code + vm->pc, sizeof c);
                vm->pc += sizeof c;
                vm->pc += c;
                break;
            case BRF_8:
                memcpy(&c, vm->code + vm->pc, sizeof c);
                vm->pc += sizeof c;
                v = vm->stack[vm->sp--];
                if (FALSEY(v)) vm->pc += c;
                break;
            case BRT_8:
                memcpy(&c, vm->code + vm->pc, sizeof c);
                vm->pc += sizeof c;
                v = vm->stack[vm->sp--];
                if (!(FALSEY(v))) vm->pc += c;
                break;
            case BRN_8:
                memcpy(&c, vm->code + vm->pc, sizeof(c));
                vm->pc += sizeof(c);
                v = vm->stack[vm->sp--];
                if (v.type != UNDEF) vm->pc += c;
                break;
            case GLOAD_1:
                addr = vm->code[vm->pc++];               // get addr of var in locals
                vm->stack[++vm->sp] = vm->globals[addr];  // load value from memory of the provided addr
                break;
            case GSTORE_1:
                addr = vm->code[vm->pc++];
                vm->globals[addr] = vm->stack[vm->sp--];
                break;
            case LLOAD_1:
                offset = NCODE(vm);
                vm->stack[++vm->sp] = vm->stack[vm->fp+offset];
                break;
            case LSTORE_1:
                offset = NCODE(vm);
                vm->stack[vm->fp+offset] = vm->stack[vm->sp--];
                break;
            case CALL_8:
                offset = NCODE(vm);
                memcpy(&addr, vm->code + vm->pc, sizeof addr);
                vm->pc += sizeof addr;
                PUSH(vm, ((YASL_Object) {offset, vm->fp}));  // store previous frame ptr;
                PUSH(vm, ((YASL_Object) {offset, vm->pc}));  // store pc addr
                vm->fp = vm->sp;
                offset = NCODE(vm);
                vm->sp += offset + 1; // + 2
                vm->pc = addr;
                break;
            case BCALL_8:
                memcpy(&addr, vm->code + vm->pc, sizeof(addr));
                vm->pc += sizeof(addr);
                if (builtins[addr](vm)) {
                    printf("ERROR: invalid argument type(s) to builtin function.\n");
                    return;
                };
                break;
            case MCALL_8:
                /*
                 * 1 -> float64_builtins();
                 * 2 -> int64_builtins();
                 * 3 -> bool_builtins();
                 * 4 -> str8_builtins();
                 * 5 -> list_builtins();
                 * 6 -> map_builtins();
                 * 7 -> file_builtins();
                 */
                memcpy(&addr, vm->code + vm->pc, sizeof(addr));
                vm->pc += sizeof(addr);
                if (PEEK(vm).type == FLOAT64) {
                    addr = vt_search(vm->builtins_vtable[1], addr);
                } else if (PEEK(vm).type == INT64) {
                    addr = vt_search(vm->builtins_vtable[2], addr);
                } else if (PEEK(vm).type == BOOL) {
                    addr = vt_search(vm->builtins_vtable[3], addr);
                } else if (PEEK(vm).type == STR8) {
                    addr = vt_search(vm->builtins_vtable[4], addr);
                } else if (PEEK(vm).type == LIST) {
                    addr = vt_search(vm->builtins_vtable[5], addr);
                } else if (PEEK(vm).type == MAP) {
                    addr = vt_search(vm->builtins_vtable[6], addr);
                } else if (PEEK(vm).type == FILEH) {
                    addr = vt_search(vm->builtins_vtable[7], addr);
                } else {
                    printf("ERROR: No methods implemented for this type: %s.\n",
                           YASL_TYPE_NAMES[PEEK(vm).type]);
                    return;
                }
                if (addr != -1) {
                    if (((int (*)(VM*))addr)(vm)) {
                        printf("ERROR: invalid argument type(s) to builtin function.\n");
                        return;
                    }
                }
                else {
                    printf("ERROR: No method implemented by this name.\n");
                    return;
                }
                break;
            case RCALL_8:
                offset = NCODE(vm);
                int i;
                for (i = 0; i < offset; i++) {
                    vm->stack[vm->fp - 2 - i] = vm->stack[vm->sp - i];
                }
                memcpy(&addr, vm->code + vm->pc, sizeof addr);
                vm->pc += sizeof addr;
                offset = NCODE(vm);
                vm->sp = vm->fp + offset;
                vm->pc = addr;
                break;
            case RET:
                v = POP(vm);
                a = vm->stack[vm->fp];
                b = vm->stack[vm->fp-1];
                vm->pc = a.value;
                vm->pc++;
                vm->sp = vm->fp - a.type;
                vm->sp--;
                vm->fp = b.value;
                PUSH(vm, v);
                break;
            case POP:
                --vm->sp;      // throw away value at top of the stack
                break;
            case MLC_8:
                vm->stack[++vm->sp].type = vm->code[vm->pc++];
                memcpy(&size, vm->code + vm->pc, sizeof(size));
                vm->pc += sizeof(size);
                ptr = malloc(size);
                vm->stack[vm->sp].value = (int64_t)ptr;
                break;
            case MCP_8:
                memcpy(&big_offset, vm->code + vm->pc, sizeof(big_offset));
                vm->pc += sizeof(big_offset);
                memcpy(&size, vm->code + vm->pc, sizeof(size));
                vm->pc += sizeof(size);
                memcpy((char*)vm->stack[vm->sp].value + big_offset, vm->code + vm->pc, size);
                vm->pc += size;
                break;
            default:
                printf("ERROR UNKNOWN OPCODE: %x\n", opcode);
                return;
        }
    }
}