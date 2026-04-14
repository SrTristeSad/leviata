/* ============================================================================
 * MOTOR LEVIATA - IMPLEMENTAÇÃO DO PARSER (TOKENS -> AST)
 * Parser descendente recursivo, produz AST tipada.
 * ============================================================================ */
#include "leviata_ast.h"
#include "leviata_lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- Estado do Parser ---- */
typedef struct {
    Lexer *lex;
    Token  current;
    Token  prev;
    int    had_error;
    int    panic_mode;
} Parser;

static AstNode *parse_expr(Parser *p);
static AstNode *parse_stmt(Parser *p);
static AstNode *parse_block(Parser *p);

/* ---- Utilitários ---- */
static void parser_error(Parser *p, const char *msg) {
    if (p->panic_mode) return;
    p->panic_mode = 1;
    p->had_error  = 1;
    fprintf(stderr, "[LEVIATA][Parser] Linha %d: %s (token: '%s')\n",
            p->current.line, msg, p->current.lexeme);
}

static Token advance(Parser *p) {
    p->prev    = p->current;
    p->current = lexer_next(p->lex);
    return p->prev;
}

static int check(Parser *p, TokenType t) {
    return p->current.type == t;
}

static int match(Parser *p, TokenType t) {
    if (check(p, t)) { advance(p); return 1; }
    return 0;
}

static Token expect(Parser *p, TokenType t, const char *errmsg) {
    if (check(p, t)) return advance(p);
    parser_error(p, errmsg);
    return p->current;
}

/* ---- Prioridades (Precedência de Pratt) ---- */
typedef enum {
    PREC_NONE = 0,
    PREC_ASSIGN,   /* = += -= *= /= */
    PREC_OR,       /* ou */
    PREC_AND,      /* e */
    PREC_EQ,       /* == != */
    PREC_CMP,      /* < > <= >= */
    PREC_ADD,      /* + - */
    PREC_MUL,      /* * / % */
    PREC_POW,      /* ^ */
    PREC_UNARY,    /* - nao ! */
    PREC_CALL,     /* () [] . */
    PREC_PRIMARY
} Precedence;

static int token_precedence(TokenType t) {
    switch (t) {
        case TOK_ASSIGN:
        case TOK_PLUS_EQ: case TOK_MINUS_EQ:
        case TOK_STAR_EQ: case TOK_SLASH_EQ: return PREC_ASSIGN;
        case TOK_OR:      return PREC_OR;
        case TOK_AND:     return PREC_AND;
        case TOK_EQ:  case TOK_NEQ:          return PREC_EQ;
        case TOK_LT:  case TOK_GT:
        case TOK_LTE: case TOK_GTE:          return PREC_CMP;
        case TOK_PLUS: case TOK_MINUS:       return PREC_ADD;
        case TOK_STAR: case TOK_SLASH:
        case TOK_PERCENT:                    return PREC_MUL;
        case TOK_CARET:                      return PREC_POW;
        case TOK_LPAREN: case TOK_LBRACKET:
        case TOK_DOT:                        return PREC_CALL;
        default: return PREC_NONE;
    }
}

/* ---- Expressão primária ---- */
static AstNode *parse_primary(Parser *p) {
    Token tok = p->current;

    /* Inteiro */
    if (match(p, TOK_INT_LIT)) {
        AstNode *n = ast_alloc(AST_INT_LIT, tok.line);
        n->as.int_lit.value = tok.i_val;
        return n;
    }
    /* Float */
    if (match(p, TOK_FLOAT_LIT)) {
        AstNode *n = ast_alloc(AST_FLOAT_LIT, tok.line);
        n->as.float_lit.value = tok.f_val;
        return n;
    }
    /* String */
    if (match(p, TOK_STR_LIT)) {
        AstNode *n = ast_alloc(AST_STR_LIT, tok.line);
        strncpy(n->as.str_lit.value, tok.lexeme, LV_MAX_STRING - 1);
        return n;
    }
    /* Booleanos */
    if (match(p, TOK_TRUE)) {
        AstNode *n = ast_alloc(AST_BOOL_LIT, tok.line);
        n->as.bool_lit.value = 1;
        return n;
    }
    if (match(p, TOK_FALSE)) {
        AstNode *n = ast_alloc(AST_BOOL_LIT, tok.line);
        n->as.bool_lit.value = 0;
        return n;
    }
    /* Nulo */
    if (match(p, TOK_NULL)) {
        return ast_alloc(AST_NULL_LIT, tok.line);
    }
    /* Lista literal [ ... ] */
    if (match(p, TOK_LBRACKET)) {
        AstNode *n = ast_alloc(AST_LIST_LIT, tok.line);
        n->as.list_lit.items    = NULL;
        n->as.list_lit.count    = 0;
        int cap = 8;
        n->as.list_lit.items = (AstNode**)malloc(cap * sizeof(AstNode*));

        while (!check(p, TOK_RBRACKET) && !check(p, TOK_EOF)) {
            if (n->as.list_lit.count >= cap) {
                cap *= 2;
                n->as.list_lit.items = (AstNode**)realloc(n->as.list_lit.items, cap * sizeof(AstNode*));
            }
            n->as.list_lit.items[n->as.list_lit.count++] = parse_expr(p);
            if (!match(p, TOK_COMMA)) break;
        }
        expect(p, TOK_RBRACKET, "Esperado ']' após lista");
        return n;
    }
    /* Grupo ( expr ) */
    if (match(p, TOK_LPAREN)) {
        AstNode *n = parse_expr(p);
        expect(p, TOK_RPAREN, "Esperado ')' após expressão");
        return n;
    }
    /* Identificador */
    if (match(p, TOK_IDENT)) {
        AstNode *n = ast_alloc(AST_IDENT, tok.line);
        strncpy(n->as.ident.name, tok.lexeme, LV_MAX_IDENTIFIER - 1);
        return n;
    }

    parser_error(p, "Expressão inesperada");
    advance(p);
    return ast_alloc(AST_NULL_LIT, tok.line);
}

/* ---- Sufixos: chamadas, índices, campos ---- */
static AstNode *parse_postfix(Parser *p, AstNode *left) {
    for (;;) {
        if (match(p, TOK_LPAREN)) {
            /* Chamada de função */
            AstNode *call = ast_alloc(AST_CALL, left->line);
            call->as.call.callee = left;
            call->as.call.args   = NULL;
            call->as.call.argc   = 0;
            int cap = 8;
            call->as.call.args = (AstNode**)malloc(cap * sizeof(AstNode*));

            while (!check(p, TOK_RPAREN) && !check(p, TOK_EOF)) {
                if (call->as.call.argc >= cap) {
                    cap *= 2;
                    call->as.call.args = (AstNode**)realloc(call->as.call.args, cap * sizeof(AstNode*));
                }
                call->as.call.args[call->as.call.argc++] = parse_expr(p);
                if (!match(p, TOK_COMMA)) break;
            }
            expect(p, TOK_RPAREN, "Esperado ')' após argumentos");
            left = call;

        } else if (match(p, TOK_LBRACKET)) {
            /* Acesso a índice */
            AstNode *idx = ast_alloc(AST_INDEX, left->line);
            idx->as.index_access.object = left;
            idx->as.index_access.index  = parse_expr(p);
            expect(p, TOK_RBRACKET, "Esperado ']' após índice");
            left = idx;

        } else if (match(p, TOK_DOT)) {
            /* Acesso a campo */
            Token field = expect(p, TOK_IDENT, "Esperado nome de campo após '.'");
            AstNode *fa = ast_alloc(AST_FIELD, left->line);
            fa->as.field_access.object = left;
            strncpy(fa->as.field_access.field, field.lexeme, LV_MAX_IDENTIFIER - 1);
            left = fa;

        } else {
            break;
        }
    }
    return left;
}

/* ---- Expressão com precedência de Pratt ---- */
static AstNode *parse_expr_prec(Parser *p, int min_prec) {
    /* Prefixos unários */
    AstNode *left = NULL;
    Token tok = p->current;

    if (match(p, TOK_MINUS)) {
        AstNode *n = ast_alloc(AST_UNOP, tok.line);
        n->as.unop.op      = TOK_MINUS;
        n->as.unop.operand = parse_expr_prec(p, PREC_UNARY);
        left = n;
    } else if (match(p, TOK_NOT)) {
        AstNode *n = ast_alloc(AST_UNOP, tok.line);
        n->as.unop.op      = TOK_NOT;
        n->as.unop.operand = parse_expr_prec(p, PREC_UNARY);
        left = n;
    } else {
        left = parse_primary(p);
        left = parse_postfix(p, left);
    }

    /* Infixos binários */
    while (!check(p, TOK_EOF)) {
        int prec = token_precedence(p->current.type);
        if (prec <= min_prec) break;

        Token op_tok = advance(p);

        /* Atribuição (direita-associativa) */
        if (op_tok.type == TOK_ASSIGN ||
            op_tok.type == TOK_PLUS_EQ || op_tok.type == TOK_MINUS_EQ ||
            op_tok.type == TOK_STAR_EQ || op_tok.type == TOK_SLASH_EQ) {

            AstNode *n = ast_alloc(AST_ASSIGN, op_tok.line);
            n->as.assign.target = left;
            n->as.assign.value  = parse_expr_prec(p, PREC_ASSIGN - 1);
            n->as.assign.op     = op_tok.type;

            /* Expande compound: a += b  =>  a = a + b */
            if (op_tok.type != TOK_ASSIGN) {
                TokenType arith;
                if      (op_tok.type == TOK_PLUS_EQ)  arith = TOK_PLUS;
                else if (op_tok.type == TOK_MINUS_EQ) arith = TOK_MINUS;
                else if (op_tok.type == TOK_STAR_EQ)  arith = TOK_STAR;
                else                                   arith = TOK_SLASH;

                AstNode *binop = ast_alloc(AST_BINOP, op_tok.line);
                binop->as.binop.op    = arith;
                binop->as.binop.left  = left;
                binop->as.binop.right = n->as.assign.value;
                n->as.assign.value    = binop;
                n->as.assign.op       = TOK_ASSIGN;
            }
            left = n;
            break;
        }

        AstNode *right = parse_expr_prec(p, prec);

        AstNode *binop = ast_alloc(AST_BINOP, op_tok.line);
        binop->as.binop.op    = op_tok.type;
        binop->as.binop.left  = left;
        binop->as.binop.right = right;
        left = binop;
    }

    return left;
}

static AstNode *parse_expr(Parser *p) {
    return parse_expr_prec(p, PREC_NONE);
}

/* ============================================================
 * PARSE STATEMENTS
 * ============================================================ */
static AstNode *parse_var_decl(Parser *p, int is_const) {
    int line = p->prev.line;
    Token name = expect(p, TOK_IDENT, "Esperado nome de variável");
    AstNode *n = ast_alloc(is_const ? AST_CONST_DECL : AST_VAR_DECL, line);
    strncpy(n->as.var_decl.name, name.lexeme, LV_MAX_IDENTIFIER - 1);
    n->as.var_decl.init = NULL;

    if (match(p, TOK_ASSIGN))
        n->as.var_decl.init = parse_expr(p);

    match(p, TOK_SEMICOLON);
    return n;
}

static AstNode *parse_func_decl(Parser *p) {
    int line = p->prev.line;
    Token name = expect(p, TOK_IDENT, "Esperado nome de função");
    AstNode *n = ast_alloc(AST_FUNC_DECL, line);
    strncpy(n->as.func_decl.name, name.lexeme, LV_MAX_IDENTIFIER - 1);

    expect(p, TOK_LPAREN, "Esperado '(' após nome da função");

    int param_cap = 8;
    n->as.func_decl.params      = (char(*)[LV_MAX_IDENTIFIER])malloc(param_cap * LV_MAX_IDENTIFIER);
    n->as.func_decl.param_count = 0;

    while (!check(p, TOK_RPAREN) && !check(p, TOK_EOF)) {
        Token param = expect(p, TOK_IDENT, "Esperado nome de parâmetro");
        if (n->as.func_decl.param_count >= param_cap) {
            param_cap *= 2;
            n->as.func_decl.params = (char(*)[LV_MAX_IDENTIFIER])realloc(
                n->as.func_decl.params, param_cap * LV_MAX_IDENTIFIER);
        }
        strncpy(n->as.func_decl.params[n->as.func_decl.param_count],
                param.lexeme, LV_MAX_IDENTIFIER - 1);
        n->as.func_decl.param_count++;
        if (!match(p, TOK_COMMA)) break;
    }
    expect(p, TOK_RPAREN, "Esperado ')' após parâmetros");

    n->as.func_decl.body = parse_block(p);
    return n;
}

static AstNode *parse_if(Parser *p) {
    int line = p->prev.line;
    AstNode *n = ast_alloc(AST_IF, line);

    /* Aceita com ou sem parênteses: "se (x > 0)" ou "se x > 0" */
    int has_paren = match(p, TOK_LPAREN);
    n->as.if_stmt.condition = parse_expr(p);
    if (has_paren) expect(p, TOK_RPAREN, "Esperado ')' após condição do 'se'");

    n->as.if_stmt.then_branch  = parse_block(p);
    n->as.if_stmt.else_branch  = NULL;

    if (match(p, TOK_SENAO)) {
        if (check(p, TOK_SE)) {
            advance(p);
            n->as.if_stmt.else_branch = parse_if(p);
        } else {
            n->as.if_stmt.else_branch = parse_block(p);
        }
    }
    return n;
}

static AstNode *parse_while(Parser *p) {
    int line = p->prev.line;
    AstNode *n = ast_alloc(AST_WHILE, line);
    int has_paren = match(p, TOK_LPAREN);
    n->as.while_stmt.condition = parse_expr(p);
    if (has_paren) expect(p, TOK_RPAREN, "Esperado ')' após condição do 'enquanto'");
    n->as.while_stmt.body = parse_block(p);
    return n;
}

static AstNode *parse_for(Parser *p) {
    int line = p->prev.line;
    AstNode *n = ast_alloc(AST_FOR_RANGE, line);

    /* para i de 0 ate 10 { ... } */
    Token var = expect(p, TOK_IDENT, "Esperado variável de iteração após 'para'");
    strncpy(n->as.for_range.var_name, var.lexeme, LV_MAX_IDENTIFIER - 1);
    expect(p, TOK_DE, "Esperado 'de' após nome da variável");
    n->as.for_range.from = parse_expr(p);
    expect(p, TOK_ATE, "Esperado 'ate' após expressão inicial");
    n->as.for_range.to   = parse_expr(p);
    n->as.for_range.body = parse_block(p);
    return n;
}

static AstNode *parse_return(Parser *p) {
    int line = p->prev.line;
    AstNode *n = ast_alloc(AST_RETURN, line);
    n->as.ret.value = NULL;

    if (!check(p, TOK_SEMICOLON) && !check(p, TOK_RBRACE) && !check(p, TOK_EOF))
        n->as.ret.value = parse_expr(p);

    match(p, TOK_SEMICOLON);
    return n;
}

static AstNode *parse_block(Parser *p) {
    int line = p->current.line;
    expect(p, TOK_LBRACE, "Esperado '{' para início de bloco");
    AstNode *n = ast_alloc(AST_BLOCK, line);

    int cap = 16;
    n->as.block.stmts = (AstNode**)malloc(cap * sizeof(AstNode*));
    n->as.block.count = 0;

    while (!check(p, TOK_RBRACE) && !check(p, TOK_EOF)) {
        if (n->as.block.count >= cap) {
            cap *= 2;
            n->as.block.stmts = (AstNode**)realloc(n->as.block.stmts, cap * sizeof(AstNode*));
        }
        n->as.block.stmts[n->as.block.count++] = parse_stmt(p);
    }
    expect(p, TOK_RBRACE, "Esperado '}' para fechar bloco");
    return n;
}

static AstNode *parse_stmt(Parser *p) {
    if (p->had_error && p->panic_mode) {
        /* Modo pânico: avança até ponto seguro */
        while (!check(p, TOK_SEMICOLON) && !check(p, TOK_RBRACE) && !check(p, TOK_EOF))
            advance(p);
        match(p, TOK_SEMICOLON);
        p->panic_mode = 0;
        return ast_alloc(AST_NULL_LIT, p->current.line);
    }

    if (match(p, TOK_VAR))      return parse_var_decl(p, 0);
    if (match(p, TOK_CONST_KW)) return parse_var_decl(p, 1);
    if (match(p, TOK_FUNC))     return parse_func_decl(p);
    if (match(p, TOK_SE))       return parse_if(p);
    if (match(p, TOK_ENQUANTO)) return parse_while(p);
    if (match(p, TOK_PARA))     return parse_for(p);
    if (match(p, TOK_RETORNAR)) return parse_return(p);
    if (match(p, TOK_QUEBRAR)) {
        AstNode *n = ast_alloc(AST_BREAK, p->prev.line);
        match(p, TOK_SEMICOLON);
        return n;
    }
    if (match(p, TOK_CONTINUAR)) {
        AstNode *n = ast_alloc(AST_CONTINUE, p->prev.line);
        match(p, TOK_SEMICOLON);
        return n;
    }
    if (check(p, TOK_LBRACE)) return parse_block(p);

    /* Statement de expressão */
    AstNode *expr = parse_expr(p);
    match(p, TOK_SEMICOLON);
    return expr;
}

/* ============================================================
 * API PÚBLICA DO PARSER
 * ============================================================ */
AstNode *leviata_parse(const char *source, int *had_error) {
    Lexer lex;
    lexer_init(&lex, source);

    Parser p;
    p.lex        = &lex;
    p.had_error  = 0;
    p.panic_mode = 0;
    p.current    = lexer_next(&lex);

    int cap = 32;
    AstNode *prog = ast_alloc(AST_PROGRAM, 1);
    prog->as.program.stmts = (AstNode**)malloc(cap * sizeof(AstNode*));
    prog->as.program.count = 0;

    while (!check(&p, TOK_EOF)) {
        if (prog->as.program.count >= cap) {
            cap *= 2;
            prog->as.program.stmts = (AstNode**)realloc(prog->as.program.stmts, cap * sizeof(AstNode*));
        }
        prog->as.program.stmts[prog->as.program.count++] = parse_stmt(&p);
    }

    if (had_error) *had_error = p.had_error;
    return prog;
}
