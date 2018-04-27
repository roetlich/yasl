#pragma once

#include "token.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define EATWHITESPACE() while (lastchar == ' ' || lastchar == '\t') { lastchar = getchar(); }

typedef struct {
    FILE *file;
    int type;
    char *value;
    int val_len;
    int line;
} Lexer;

int YASLToken_FourChars(char c1, char c2, char c3, char c4);
int YASLToken_ThreeChars(char c1, char c2, char c3);
int YASLToken_TwoChars(char c1, char c2);
int YASLToken_OneChar(char c1);

Lexer *lex_new(FILE *file);
void lex_del(Lexer *lex);
void gettok(Lexer *lex);

const char *YASL_TOKEN_NAMES[];