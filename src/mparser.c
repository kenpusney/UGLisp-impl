
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "mparser.h"
#include "mhash.h"
#include "mobject.h"

#define storebuf()   \
    buf[boffset] = '\0'; \
    if(strlen(buf)){     \
        this->next = make_atom_tok(buf);    \
        this = this->next;    \
        boffset = 0;    \
    }

/* == private == */

//token symbol table
static char tokens[] = {
    'a',  ':',  '(',  ')',  '\'',
    '`',  ',',  '#',  ';',  '&',
    '@',  '"',  '\0', 
};

static token_t tok_base;
static char buf[256];
static int boffset = 0;
static token_t* ast_cursor;

static token_t* make_atom_tok(char* repr);
static token_t* make_tok(enum TOKEN tag);
static token_t* make_str_tok();
static ast_t* make_ast(token_t* this);

static int cx2int(char c){
    if(isdigit(c)){
        return c - '0';
    }else if(isxdigit(c)){
        return (c - 'A') + 10;
    }
    return -1;
}

/* == implementation == */

void parser(){
    char a;
    enum TOKEN tag;
    int brace = 0;
    token_t *this=NULL;
    tok_base.t = TOK_BEGIN;
    tok_base.next = NULL;
    this = &tok_base;
    while(1){
        a = getc(stdin);
        if(a == EOF) break;
        if(isblank(a)){
        __blank:
            storebuf();
        }else{
            switch(a){
                case '(': tag = TOK_LPAR; brace++;
                    goto __make_tok;
                case ')': tag = TOK_RPAR; brace--;
                    goto __make_tok;
                case '#': tag = TOK_SHARP;
            __make_tok:
                    storebuf();
                    this->next = make_tok(tag);
                    this = this->next;
                    break;
                case ';': while((a = getc(stdin)) != '\n'){if(a == EOF) break;};
                    break;
                case '"':
                    storebuf();
                    this->next = make_str_tok();
                    this = this->next;
                    break;
                case '\n':
                    storebuf();
                    if(brace){
                        goto __blank;
                    }
                    return;
                    // this = &tok_base;
                    // goto __eval;
                    break;
                default:
                    buf[boffset++] = a;
            }   
        }
    }
// __eval:
//     this = &tok_base;
//     while(this){
//         switch(this->t){
//             case TOK_ATOM: printf("ATOM: %s\n",this->v.repr); break;
//             case TOK_STR: printf( "STR:  %s\n",this->v.repr); break;
//             case TOK_BEGIN: break;
//             default: printf("TOK:  %c\n",this->v.symbol);
            
//         }
//         this = this->next;
//     }
}

static token_t* make_atom_tok(char* repr){
    token_t* tok = malloc(sizeof(token_t));
    tok->v.repr = malloc(strlen(repr)+1);
    strcpy(tok->v.repr,repr);
    tok->next = NULL;
    tok->t = TOK_ATOM;
    return tok;
}

static token_t* make_tok(enum TOKEN tag){
    token_t* tok = malloc(sizeof(token_t));
    tok->v.symbol = tokens[tag];
    tok->next = NULL;
    tok->t = tag;
    return tok;
}

static token_t* make_str_tok(){
    char c;
    int hexval = 0;
    token_t* tok = malloc(sizeof(token_t));
    tok->next = NULL;
    tok->t = TOK_STR;
    while( (c = getc(stdin)) ){
        if(c == '"')
            break;
        if(c == '\\'){
            switch(c = getc(stdin)){
                case 'n': c = '\n'; break;
                case 't': c = '\t'; break;
                case 'b': c = '\b'; break;
                case 'r': c = '\r'; break;
                case '0': c = '\0'; break;
                case 'x':
                    while(isxdigit(c=getc(stdin))) hexval = hexval * 16 + cx2int(c);
                    if(!isxdigit(c)) { ungetc(c,stdin);}
                    c = (char)hexval;
                    hexval = 0;
                    break;
                default: break;
            }
        }
        buf[boffset++] = c;
    }
    buf[boffset] = '\0';
    boffset = 0;
    tok->v.repr = malloc(strlen(buf)+1);
    strcpy(tok->v.repr,buf);
    return tok;
}

ast_t* gen_ast(){
	return make_ast(&tok_base)->next;
}

static ast_t* make_ast(token_t* this){
	ast_t* ast = malloc(sizeof(ast_t));
	ast_t* base = ast;
	ast->t = AST_ATOM;
    ast->next=NULL;
    ast->childs=NULL;
    if(this->t == TOK_LPAR){
    	ast->t = AST_LIST;
    	ast->childs = make_ast(this->next);
    }
    while((this=this->next)){
    	if(this->t == TOK_RPAR)
    		break;
    	ast->next = malloc(sizeof(ast_t));
    	ast = ast->next;
    	ast->next=NULL;
    	ast->childs=NULL;
    	switch(this->t){
			case TOK_ATOM: 
				ast->v = make_matom(this->v.repr);
				break;
	//		case TOK_KEYWORDS: 
	//			ast->v = make_keyword(this->v.repr);
	//			break;
		//	case TOK_QUOTE: break;
		//	case TOK_QUASHIQUOTE: break;
		//	case TOK_UNQUOTE: break;
		//	case TOK_SHARP: break;
		//	case TOK_SEMICOLOUN: break;
		//	case TOK_REF: break;
		//	case TOK_DEREF: break;
			case TOK_STR:
				ast->v = make_mstr(this->v.repr);
				break;
		//	case TOK_BEGIN: break;
			default:break;
	}
    ast_cursor = this->next;
    if(this->t == TOK_RPAR)
		return base;
    }
    return base;
}

