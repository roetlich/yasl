#include "ast.h"


/*
 * Node {
 *   AST nodetype;
 *   Token type;
 *   Node *children;
 *   char* name;
 *   int len;
 *   int line;
 * }
 */

Node *new_Node_0(AST nodetype, Token type, char *name, int64_t name_len, int line) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = nodetype;
    node->type = type;
    node->children = NULL;
    node->name_len = name_len;
    if (name == NULL) {
        node->name = NULL;
    } else {
        node->name = malloc(node->name_len);
        memcpy(node->name, name, node->name_len);
    }
    node->line = line;
    return node;
}

Node *new_Node_1(AST nodetype, Token type, Node *child, char *name, int64_t name_len, int line) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = nodetype;
    node->type = type;
    node->children = malloc(sizeof(Node*));
    node->children[0] = child;
    node->name_len = name_len;
    if (name == NULL) {
        node->name = NULL;
    } else {
        node->name = malloc(node->name_len);
        memcpy(node->name, name, node->name_len);
    }
    node->line = line;
    return node;
}

Node *new_Node_2(AST nodetype, Token type, Node *child1, Node *child2, char *name, int64_t name_len, int line) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = nodetype;
    node->type = type;
    node->children = malloc(sizeof(Node*));
    node->children[0] = child1;
    node->children[1] = child2;
    node->name_len = name_len;
    if (name == NULL) {
        node->name = NULL;
    } else {
        node->name = malloc(node->name_len);
        memcpy(node->name, name, node->name_len);
    }
    node->line = line;
    return node;
}

Node *new_Node_3(AST nodetype, Token type, Node *child1, Node *child2, Node *child3, char *name, int64_t name_len, int line) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = nodetype;
    node->type = type;
    node->children = malloc(sizeof(Node*));
    node->children[0] = child1;
    node->children[1] = child2;
    node->children[2] = child3;
    node->name_len = name_len;
    if (name == NULL) {
        node->name = NULL;
    } else {
        node->name = malloc(node->name_len);
        memcpy(node->name, name, node->name_len);
    }
    node->line = line;
    return node;
}


Node *new_ExprStmt(Node *child, int line) {
    return new_Node_1(NODE_EXPRSTMT, T_UNKNOWN, child, NULL, 0, line);
}

Node *new_Block(int line) {
    return new_Node_0(NODE_BLOCK, T_UNKNOWN, NULL, 0, line);
}

void block_append(Node *node, Node *child) {
    node->children = realloc(node->children, (++node->children_len)*sizeof(Node*));  //TODO: make better implementation
    node->children[node->children_len-1] = child;
}

Node *new_FunctionCall(Node *params, char *name, int64_t name_len, int line) {
    return new_Node_1(NODE_FN, T_UNKNOWN, params, name, name_len, line);
}

Node *new_Index(Node *collection, Node *value, int line) {
    return new_Node_2(NODE_INDEX, T_UNKNOWN, collection, value, NULL, 0, line);
}

Node *new_While(Node *cond, Node *body, int line) {
    return new_Node_2(NODE_WHILE, T_UNKNOWN, cond, body, NULL, 0, line);
}

Node *new_Break(int line) {
    return new_Node_0(NODE_BREAK, T_UNKNOWN, NULL, 0, line);
}

Node *new_Continue(int line) {
    return new_Node_0(NODE_CONT, T_UNKNOWN, NULL, 0, line);
}

Node *new_If(Node *cond, Node *then_node, Node *else_node, int line) {
    return new_Node_3(NODE_IF, T_UNKNOWN, cond, then_node, else_node, NULL, 0, line);
}

Node *new_Print(Node *child, int line) {
    return new_Node_1(NODE_PRINT, T_UNKNOWN, child, NULL, 0, line);
}

Node *new_Let(char *name, int64_t name_len, Node *child, int line) {
    return new_Node_1(NODE_LET, T_UNKNOWN, child, name, name_len, line);
}

Node *new_TriOp(Token op, Node *left, Node *middle, Node *right, int line) {
    return new_Node_3(NODE_TRIOP, op, left, middle, right, NULL, 0, line);
}

Node *new_BinOp(Token op, Node *left, Node *right, int line) {
    return new_Node_2(NODE_BINOP, op, left, right, NULL, 0, line);
}

Node *new_UnOp(Token op, Node *child, int line) {
    return new_Node_1(NODE_UNOP, op, child, NULL, 0, line);
}

Node *new_Assign(char *name, int64_t name_len, Node *child, int line) {
    return new_Node_1(NODE_ASSIGN, T_UNKNOWN, child, name, name_len, line);
}

Node *new_Var(char *name, int64_t name_len, int line) {
    return new_Node_0(NODE_VAR, T_UNKNOWN, name, name_len, line);
}

Node *new_Undef(int line) {
    return new_Node_0(NODE_UNDEF, T_UNKNOWN, NULL, 0, line);
}

Node *new_Float(char *value, int len, int line) {
    return new_Node_0(NODE_FLOAT64, T_UNKNOWN, value, len, line);
}

Node *new_Integer(char *value, int len, int line) {
    return new_Node_0(NODE_INT64, T_UNKNOWN, value, len, line);
}

Node *new_Boolean(char *value, int len, int line) {
    return new_Node_0(NODE_BOOL, T_UNKNOWN, value, len, line);
}

Node *new_String(char *value, int len, int line) {
    return new_Node_0(NODE_STR, T_UNKNOWN, value, len, line);
}

Node *new_List(Node *values, int line) {
    return new_Node_1(NODE_LIST, T_UNKNOWN, values, NULL, 0, line);
}
Node *new_Map(Node *keys, Node *values, int line) {
    return new_Node_2(NODE_MAP, T_UNKNOWN, keys, values, NULL, 0, line);
}

void node_del(Node *node) {
    while(node->children_len-- > 0)
        if (node->children[node->children_len] != NULL)
            node_del(node->children[node->children_len]);
    free(node->name);
    free(node->children);
    free(node);
}