
#include "parser.h"
#include "object.h"
#include <stdlib.h>
#include <ctype.h>

typedef struct parse_state_t
{
    TokenList tokens;
    token_t *current;
} parse_state_t, *ParseState;

static MObject tryLispObject(ParseState state);

static ParseState makeParseState(TokenList tokens)
{
    ParseState state = malloc(sizeof(struct parse_state_t));
    state->tokens = tokens;
    state->current = tokens->head;
    return state;
}

static ParseState advance(ParseState state)
{
    if (state->current != NULL)
        state->current = state->current->next;
    return state;
}

static MObject makeLispObject(ParseState state)
{
    token_t *token = state->current;
    if (isdigit(token->v.repr[0]))
    {
        advance(state);
        return make_mnum(atoi(token->v.repr));
    }
    else
    {
        advance(state);
        return make_matom(token->v.repr);
    }
}

static MObject makeLispList(ParseState state)
{
    if (state->current->t != TOK_LPAR)
        return NULL;

    advance(state);
    MList mlist = make_mlist();
    while (state->current != NULL && state->current->t != TOK_RPAR)
    {
        append_mlist(mlist, tryLispObject(state));
    }
    advance(state);

    return wrap_mlist(mlist);
}

static MObject makeQuotedObject(ParseState token)
{
    return NULL;
}

static MObject makeLispStr(ParseState state)
{
    MObject str = make_mstr(state->current->v.repr);
    advance(state);
    return str;
}

static MObject tryLispObject(ParseState state)
{
    switch (state->current->t)
    {
    case TOK_SYMBOL:
        return makeLispObject(state);
    case TOK_LPAR:
        return makeLispList(state);
    case TOK_QUOTE:
        return makeQuotedObject(state);
    case TOK_STR:
        return makeLispStr(state);
    default:
        return NULL;
    }
}

MObject parse(TokenList tokens)
{
    if (tokens == NULL || tokens->head == NULL)
        return NULL;

    ParseState state = makeParseState(tokens);
    MObject object = tryLispObject(state);

    free(state);

    return object;
}

static void freeAstList(MList list)
{
    MListNode current = list->head;
    while (current != NULL)
    {
        MListNode next = current->next;
        freeAst(current->v);
        free(current);
        current = next;
    }
    free(list);
}

void freeAst(MObject object)
{
    switch (object->t)
    {
    case M_ATOM:
    case M_STRING:
        free(object->v.str);
        break;
    case M_LIST:
        freeAstList(object->v.l);
        break;
    default:
        break;
    }
    free(object);
}
