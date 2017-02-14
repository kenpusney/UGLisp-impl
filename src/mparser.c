
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "mparser.h"
#include "mhash.h"
#include "mobject.h"

#define storebuf()                       \
    buf[boffset] = '\0';                 \
    if (strlen(buf))                     \
    {                                    \
        this->next = make_atom_tok(buf); \
        this = this->next;               \
        boffset = 0;                     \
    }

static int advance(LexState state)
{
    state->index++;
    return state->index < state->size;
}

static char current(LexState state)
{
    return state->buf[state->index];
}

static int eof(LexState state)
{
    return state->index >= state->size;
}

TokenList lex(LexState state)
{
    TokenList list = malloc(sizeof(struct tokenlist_t));
    token_t *this = malloc(sizeof(struct token_t));
    list->head = this;
    this->next = NULL;
    while (1)
    {
        switch (current(state))
        {
        case '(':
            this->t = TOK_LPAR;
            this->v.symbol = '(';
            break;
        case ')':
            this->t = TOK_RPAR;
            this->v.symbol = ')';
            break;
        case '\'':
            this->t = TOK_QUOTE;
            this->v.symbol = '\'';
            break;
        case '\"':
            this->v.symbol = '\"';
            break;
        case '#':
            this->t = TOK_SHARP;
            this->v.symbol = '#';
            break;
        case '`':
            this->t = TOK_QUASHIQUOTE;
            this->v.symbol = '`';
            break;
        case ',':
            this->t = TOK_UNQUOTE;
            this->v.symbol = ',';
            break;
        case ';':
            this->t = TOK_SEMICOLOUN;
            this->v.symbol = ';';
            break;
        case '&':
            this->t = TOK_REF;
            this->v.symbol = '&';
            break;
        case ':':
            this->t = TOK_KEYWORDS;
            this->v.symbol = ':';
            break;
        default:
            this->t = TOK_SYMBOL;
            break;
        }
        advance(state);
        if (eof(state))
        {
            break;
        }
        else
        {
            this->next = malloc(sizeof(struct token_t));
            this = this->next;
            this->next = NULL;
        }
    }
    return list;
}
