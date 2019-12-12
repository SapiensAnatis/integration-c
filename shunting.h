#ifndef SHUNTING_H_INCLUDED
#define SHUNTING_H_INCLUDED // Include guards

#include "stack.h"
#include "token.h"

int shunting_yard(struct Token *, int, struct Token *);

#endif