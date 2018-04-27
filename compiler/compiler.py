from .opcode import *
from .visitor import *
from .environment import Env
from .ast import *

BINRESERVED = {
        "^":   [EXP],
        "*":   [MUL],
        "/":   [FDIV],
        "//":  [IDIV],
        "%":   [MOD],
        "+":   [ADD],
        "-":   [SUB],
        "<<":  [BLSHIFT],
        ">>":  [BRSHIFT],
        "||":  [CNCT],
        "<":   [GE, NOT],
        "<=":  [GT, NOT],
        ">":   [GT],
        ">=":  [GE],
        "==":  [EQ],
        "!=":  [EQ, NOT],
        "===": [ID],
        "!==": [ID, NOT],
        "&":   [BAND],
        "~":   [BXOR],
        "|":   [BOR],
        }
UNRESERVED = {
        "-": [NEG],
        "!": [NOT],
        "+": [NOP],
        "#": [LEN],
        "~": [BNOT],
        }
ICONSTANTS = {
        -1: [ICONST_M1],
        0:  [ICONST_0],
        1:  [ICONST_1],
        2:  [ICONST_2],
        3:  [ICONST_3],
        4:  [ICONST_4],
        5:  [ICONST_5],
        }
DCONSTANTS = {
        0.0: [DCONST_0],
        1.0: [DCONST_1],
        2.0: [DCONST_2],
        }

BUILTINS = {
        "print":        0x00,
        "input":        0x01,
        "open":         0x02,
        "popen":        0x03,
        #"typeof":       0x04,
}

METHODS = {
        "tofloat64":    0x0A,
        "toint64":      0x0B,
        "tobool":       0x0C,
        "tostr":        0x0D,
        "tolist":       0x0E,
        "tomap":        0x0F,

        "upcase":       0x10,
        "downcase":     0x11,
        "isalnum":      0x12,
        "isal":         0x13,
        "isnum":        0x14,
        "isspace":      0x15,
        "startswith":   0x16,
        "endswith":     0x17,
        "search":       0x18,
        "split":        0x19,
        "ltrim":        0x1A,
        "rtrim":        0x1B,
        "trim":         0x1C,

        "append":       0x20,

        "keys":         0x30,
        "values":       0x31,

        "close":        0x40,
        "pclose":       0x41,
        "read":         0x42,
        "write":        0x43,
        "readline":     0x44,

        "__get":        0xF0,
        "__set":        0xF1,
}

###############################################################################
#                                                                             #
#  COMPILER                                                                   #
#                                                                             #
###############################################################################

class Compiler(NodeVisitor):
    def __init__(self):
        self.globals = Env()
        self.globals.decl_var("stdin")
        self.globals.decl_var("stdout")
        self.globals.decl_var("stderr")
        self.locals = Env()
        self.code = []
        self.checkpoints = []
        self.header = intbytes_8(8) + intbytes_8(0)
        self.fns = {}
        self.current_fn = None
        self.offset = 0
        self.strs = {}
    def compile(self, statements):
        for statement in statements:
            self.code.extend(self.visit(statement))
        self.header[0:8] = intbytes_8(len(self.header))
        self.header[8:16] = intbytes_8(len(self.globals))  # TODO: fix so this works with locals as well
        for opcode in self.header: print(hex(opcode))
        print("entry point:")
        for opcode in self.code + [HALT]: print(hex(opcode))
        return self.header + self.code + [HALT] #TODO: fix return values once we have proper ones
    def enter_scope(self):
        if self.current_fn is not None:
            self.locals = Env(self.locals)
        else:
            self.globals = Env(self.globals)
    def exit_scope(self):
        if self.current_fn is not None:
            self.locals = self.locals.parent
        else:
            self.globals = self.globals.parent
    def visit_Print(self, node):
        expr = self.visit(node.expr)
        return expr + [BCALL_8] + intbytes_8(BUILTINS["print"])
    def visit_Block(self, node):
        result = []
        for statement in node.statements:
            result = result + self.visit(statement)
        return result
    def visit_ExprStmt(self, node):
        expr = self.visit(node.expr)
        return expr + [POP]
    def visit_If(self, node):
        cond = self.visit(node.cond)
        self.enter_scope()
        body = self.visit(node.body)
        self.exit_scope()
        return cond + [BRF_8] + intbytes_8(len(body)) + body
    def visit_IfElse(self, node):
        cond = self.visit(node.cond)
        self.enter_scope()
        left = self.visit(node.left)
        self.exit_scope()
        self.enter_scope()
        right = self.visit(node.right)
        self.exit_scope()
        left.extend([BR_8] + intbytes_8(len(right)))
        return cond + [BRF_8] + intbytes_8(len(left)) + left + right
    def visit_Break(self, node):
        if len(self.checkpoints) == 0:
            raise Exception("invalid break outside of loop")
        else:
            return [BCONST_F, GOTO] + intbytes_8(self.checkpoints[-1])
    def visit_Continue(self, node):
        if len(self.checkpoints) == 0:
            raise Exception("invalid continue outside of loop")
        else:
            return [GOTO] + intbytes_8(self.checkpoints[-2])
    def visit_While(self, node):
        cond = self.visit(node.cond)
        self.checkpoints.append(len(self.code))
        self.checkpoints.append(len(self.code)+len(cond))
        self.enter_scope()
        body = []
        for stmt in node.body:
            body.extend(self.visit(stmt))
        self.exit_scope()
        self.checkpoints.pop()
        self.checkpoints.pop()
        cond.extend([BRF_8] + intbytes_8(len(body)+9))
        body.extend([BR_8] + intbytes_8(-(len(body)+9+len(cond))))
        return cond + body
    def visit_For(self, node):
        #assert False
        self.enter_scope()
        self.globals.decl_var("for")
        result = [ICONST_0, GSTORE_1, self.globals["for"]]
        self.enter_scope()
        right = self.visit(node.ls)
        self.globals.decl_var(node.var.value)
        body = right + [GLOAD_1, self.globals["for"]] + \
                [BCALL_8] + intbytes_8(BUILTINS["find"]) + [ GSTORE_1, self.globals[node.var.value]] + \
               self.visit(node.body) + [GLOAD_1, self.globals["for"], ICONST_1, ADD, GSTORE_1, self.globals["for"]]
        self.exit_scope()
        cond = [GLOAD_1, self.globals["for"]] + right + [LEN, GE, NOT, BRF_8] + \
            intbytes_8(len(body)+9)
        body = cond + body + [BR_8] + intbytes_8(-len(body)-len(cond)-9)
        self.exit_scope()
        print(self.globals.vars)
        return result + body
    def visit_TriOp(self, node): #only 1 tri-op is possible
        cond = self.visit(node.cond)
        left = self.visit(node.left)
        right = self.visit(node.right)
        left = left + [BR_8] + intbytes_8(len(right))
        return cond + [BRF_8] + intbytes_8(len(left)) + left + right
    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)
        if node.token.value == "??":
            return left + [DUP, BRN_8] + intbytes_8(len(right)+1) + [POP] + right
        elif node.token.value == "|||":
            return left + [MCALL_8] + intbytes_8(METHODS["tostr"]) + right + [MCALL_8] + intbytes_8(METHODS["tostr"]) \
                + [HARD_CNCT]
        this = BINRESERVED.get(node.op.value)
        return left + right + this
    def visit_LogicOp(self, node):
        left = self.visit(node.left)
        right = [POP] + self.visit(node.right)
        left = left + [DUP]
        if node.op.value == "and":
            return left + [BRF_8] + intbytes_8(len(right)) + right
        elif node.op.value == "or":
            return left + [BRT_8] + intbytes_8(len(right)) + right
        else:
            assert False
    def visit_UnOp(self, node):
        expr = self.visit(node.expr)
        this = UNRESERVED.get(node.op.value)
        return expr + this
    def visit_FunctionDecl(self, node):
        if self.current_fn is not None:
            raise Exception("cannot declare function outside of global scope. (line %s)" % node.token.line)
        self.current_fn = node.token.value
        if node.token.value in self.fns:
            raise Exception("invalid redefinition of function %s (line %s)" % (node.token.value, node.token.line))
        self.fns[node.token.value] = {}
        self.fns[node.token.value]["addr"] = len(self.header)
        self.fns[node.token.value]["params"] = node.params.__len__()
        self.locals = Env()
        self.offset = self.fns[node.token.value]["params"]
        node.params.reverse()
        for i in node.params:
            self.locals[i.value] = 255 - len(self.locals.vars) - 1
        self.fns[node.token.value]["locals"] = len(self.locals.vars)
        for stmt in node.block.statements:
            self.header.extend(self.visit(stmt))
        self.header.extend([NCONST, RET])
        self.locals = Env()
        self.current_fn = None  # TODO: allow nested function definitions
        return []
    def visit_FunctionCall(self, node):
        result = []
        for expr in node.params:
            result = result + self.visit(expr)
        if node.value in BUILTINS:
            return result + [BCALL_8] + intbytes_8(BUILTINS[node.value])
        return result + [CALL_8, self.fns[node.token.value]["params"]] + \
                         intbytes_8(self.fns[node.token.value]["addr"]) + \
                         [self.fns[node.token.value]["locals"]]
    def visit_Return(self, node):
        if isinstance(node.expr, FunctionCall) and node.expr.value == self.current_fn:
            result = []
            for param in node.expr.params:
                result = result + self.visit(param)
            return result + [RCALL_8, self.fns[node.expr.value]["params"]] + \
                        intbytes_8(self.fns[node.expr.value]["addr"]) + \
                        [self.fns[node.expr.value]["locals"]]
        return self.visit(node.expr) + [RET]
    def visit_MethodCall(self, node):
        result = []
        for expr in node.params:
            result = result + self.visit(expr)
        if node.value in METHODS:
            return result + self.visit(node.left) + [MCALL_8] + intbytes_8(METHODS[node.value])
        assert False
    def visit_Decl(self, node):
        result = []
        for i in range(len(node.left)):
            var = node.left[i]
            val = node.right[i]
            if self.current_fn is not None:
                if var.value not in self.locals.vars:
                    self.locals[var.value] = len(self.locals.vars) + 1 - self.offset
                result = result + self.visit(val) + [LSTORE_1, self.locals[var.value]]
                continue
            if var.value not in self.globals.vars:
                self.globals.decl_var(var.value)
            right = self.visit(val)
            result = result + right + [GSTORE_1, self.globals[var.value]]
        return result
    def visit_Assign(self, node):
        if node.left.value not in self.locals and node.left.value not in self.globals:
            raise Exception("undeclared variable: %s in line %s" % (node.left.value, node.left.token.line))
        if isinstance(node.left, Var):
            if self.current_fn is not None:
                if node.left.value in self.locals:
                    return self.visit(node.right) + [LSTORE_1,
                           self.locals[node.left.value],
                           LLOAD_1,
                           self.locals[node.left.value]]
            return self.visit(node.right) + [GSTORE_1, self.globals[node.left.value],
                                             GLOAD_1, self.globals[node.left.value]]
        elif isinstance(node.left, Index):
            index = node.left
            return self.visit(index.left) + self.visit(index.right) + self.visit(node.right) + \
                   [BCALL_8] + intbytes_8(BUILTINS["insert"])
    def visit_Var(self, node):
        if node.value not in self.locals and node.value not in self.globals:
            raise Exception("undefined variable: %s in line %s" % (node.value, node.token.line))
        if (self.current_fn is not None) and (node.value in self.locals):
            return [LLOAD_1, self.locals[node.value]]
        return [GLOAD_1, self.globals[node.value]]
    def visit_Index(self, node):
        right = self.visit(node.right)
        return right + self.visit(node.left) + [MCALL_8] + intbytes_8(METHODS["__get"])
    def visit_Hash(self, node):
        result = [NEWHASH]
        for i in range(len(node.keys)):
            result = result + [DUP] + self.visit(node.keys[i]) + [SWAP] + self.visit(node.vals[i]) + [SWAP] + [MCALL_8] + intbytes_8(METHODS["__set"]) + [POP]
        return result  # TODO: allow declaration with a bunch of values in it
    def visit_List(self, node):
        result = [NEWLIST]
        for expr in node.params:
            #[MCALL_8] + intbytes_8(METHODS[node.value])
            result.extend([DUP] + self.visit(expr) + [SWAP] + [MCALL_8] + intbytes_8(METHODS["append"]) + [POP])
            # TODO: fix order of arguments here
            #result = result + [DUP] + self.visit(expr) + [BCALL_8] + intbytes_8(BUILTINS["append"]) + [POP]
        return result
    def visit_String(self, node):
        if node.value not in self.strs:
            string = [int(b) for b in str.encode(node.value)]
            length = intbytes_8(len(string))
            self.strs[node.value] = {}
            self.strs[node.value]["length"] = length
            self.strs[node.value]["addr"] = intbytes_8(len(self.header))
            self.header = self.header + string
        return [NEWSTR8] + self.strs[node.value]["length"] + self.strs[node.value]["addr"]
    def visit_Boolean(self, node):
        if node.value == False:
            return [BCONST_F]
        elif node.value == True:
            return [BCONST_T]
        else:
            raise Exception("invalid boolean")
    def visit_Integer(self, node):
        if node.value in ICONSTANTS:
            return ICONSTANTS[node.value]
        return [ICONST] + intbytes_8(node.value)
    def visit_Float(self, node):
        if node.value in DCONSTANTS:
            return DCONSTANTS[node.value]
        return [DCONST] + doublebytes(node.value)
    def visit_Undef(self, node):
        return [NCONST]
