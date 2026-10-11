/* ============================================================================
 * LEVIATA 3 - PARSER (TOKENS -> AST)
 * ============================================================================ */
#include "leviata_ast.h"
#include "leviata_lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    Lexer *lex;
    Token current;
    Token prev;
    int had_error;
    int panic_mode;
} Parser;

static AstNode *parse_expr(Parser *p);
static AstNode *parse_stmt(Parser *p);
static AstNode *parse_block(Parser *p);

static void parser_error(Parser *p, const char *msg) {
    if (p->panic_mode) return;
    p->panic_mode = 1;
    p->had_error = 1;
    fprintf(stderr, "[LEVIATA][Parser] Linha %d: %s (token: '%s')\n",
            p->current.line, msg, p->current.lexeme);
}

static Token advance(Parser *p) {
    p->prev = p->current;
    p->current = lexer_next(p->lex);
    if (p->current.type == TOK_ERROR) {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "Símbolo não aceito na sintaxe canônica: '%.160s'",
                 p->current.lexeme);
        parser_error(p, msg);
    }
    return p->prev;
}

static int check(Parser *p, TokenType t) {
    return p->current.type == t;
}

static int match(Parser *p, TokenType t) {
    if (check(p, t)) {
        advance(p);
        return 1;
    }
    return 0;
}

static Token expect(Parser *p, TokenType t, const char *errmsg) {
    if (check(p, t)) return advance(p);
    parser_error(p, errmsg);
    return p->current;
}

typedef enum {
    PREC_NONE = 0,
    PREC_ASSIGN,
    PREC_OR,
    PREC_AND,
    PREC_EQ,
    PREC_CMP,
    PREC_ADD,
    PREC_MUL,
    PREC_POW,
    PREC_UNARY,
    PREC_CALL,
    PREC_PRIMARY
} Precedence;

static int token_precedence(TokenType t) {
    switch (t) {
        case TOK_ASSIGN:
        case TOK_PLUS_EQ:
        case TOK_MINUS_EQ:
        case TOK_STAR_EQ:
        case TOK_SLASH_EQ:
            return PREC_ASSIGN;
        case TOK_OR: return PREC_OR;
        case TOK_AND: return PREC_AND;
        case TOK_EQ:
        case TOK_NEQ:
            return PREC_EQ;
        case TOK_LT:
        case TOK_GT:
        case TOK_LTE:
        case TOK_GTE:
            return PREC_CMP;
        case TOK_PLUS:
        case TOK_MINUS:
            return PREC_ADD;
        case TOK_STAR:
        case TOK_SLASH:
        case TOK_PERCENT:
            return PREC_MUL;
        case TOK_CARET:
            return PREC_POW;
        case TOK_LPAREN:
        case TOK_LBRACKET:
        case TOK_DOT:
            return PREC_CALL;
        default:
            return PREC_NONE;
    }
}

static AstNode *parse_primary(Parser *p) {
    Token tok = p->current;

    if (match(p, TOK_INT_LIT)) {
        AstNode *n = ast_alloc(AST_INT_LIT, tok.line);
        n->as.int_lit.value = tok.i_val;
        return n;
    }

    if (match(p, TOK_FLOAT_LIT)) {
        AstNode *n = ast_alloc(AST_FLOAT_LIT, tok.line);
        n->as.float_lit.value = tok.f_val;
        return n;
    }

    if (match(p, TOK_STR_LIT)) {
        AstNode *n = ast_alloc(AST_STR_LIT, tok.line);
        strncpy(n->as.str_lit.value, tok.lexeme, LV_MAX_STRING - 1);
        n->as.str_lit.value[LV_MAX_STRING - 1] = '\0';
        return n;
    }

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

    if (match(p, TOK_NULL))
        return ast_alloc(AST_NULL_LIT, tok.line);

    if (match(p, TOK_ESTE)) {
        AstNode *n = ast_alloc(AST_IDENT, tok.line);
        strcpy(n->as.ident.name, "este");
        return n;
    }

    if (match(p, TOK_NOVO)) {
        AstNode *n = ast_alloc(AST_NEW, tok.line);
        Token classe = expect(p, TOK_IDENT, "Esperado nome da classe após 'novo'");
        strncpy(n->as.new_expr.class_name, classe.lexeme, LV_MAX_IDENTIFIER - 1);
        n->as.new_expr.class_name[LV_MAX_IDENTIFIER - 1] = '\0';

        expect(p, TOK_LPAREN, "Esperado '(' após nome da classe");
        if (!check(p, TOK_RPAREN)) {
            parser_error(p,
                "Construtores com argumentos ainda não fazem parte do Leviata 3.2");
            while (!check(p, TOK_RPAREN) && !check(p, TOK_EOF))
                advance(p);
        }
        expect(p, TOK_RPAREN, "Esperado ')' após 'novo Classe('");
        return n;
    }

    if (match(p, TOK_LBRACKET)) {
        AstNode *n = ast_alloc(AST_LIST_LIT, tok.line);
        int cap = 8;
        n->as.list_lit.items = (AstNode**)malloc((size_t)cap * sizeof(AstNode*));
        n->as.list_lit.count = 0;

        while (!check(p, TOK_RBRACKET) && !check(p, TOK_EOF)) {
            if (n->as.list_lit.count >= cap) {
                cap *= 2;
                n->as.list_lit.items = (AstNode**)realloc(
                    n->as.list_lit.items, (size_t)cap * sizeof(AstNode*));
            }
            n->as.list_lit.items[n->as.list_lit.count++] = parse_expr(p);
            if (!match(p, TOK_COMMA)) break;
        }
        expect(p, TOK_RBRACKET, "Esperado ']' após lista");
        return n;
    }

    if (match(p, TOK_LPAREN)) {
        AstNode *n = parse_expr(p);
        expect(p, TOK_RPAREN, "Esperado ')' após expressão");
        return n;
    }

    if (match(p, TOK_IDENT)) {
        AstNode *n = ast_alloc(AST_IDENT, tok.line);
        strncpy(n->as.ident.name, tok.lexeme, LV_MAX_IDENTIFIER - 1);
        n->as.ident.name[LV_MAX_IDENTIFIER - 1] = '\0';
        return n;
    }

    parser_error(p, "Expressão inesperada");
    if (!check(p, TOK_EOF)) advance(p);
    return ast_alloc(AST_NULL_LIT, tok.line);
}

static AstNode *parse_postfix(Parser *p, AstNode *left) {
    for (;;) {
        if (match(p, TOK_LPAREN)) {
            AstNode *call = ast_alloc(AST_CALL, left->line);
            call->as.call.callee = left;
            call->as.call.argc = 0;
            int cap = 8;
            call->as.call.args = (AstNode**)malloc((size_t)cap * sizeof(AstNode*));

            while (!check(p, TOK_RPAREN) && !check(p, TOK_EOF)) {
                if (call->as.call.argc >= cap) {
                    cap *= 2;
                    call->as.call.args = (AstNode**)realloc(
                        call->as.call.args, (size_t)cap * sizeof(AstNode*));
                }
                call->as.call.args[call->as.call.argc++] = parse_expr(p);
                if (!match(p, TOK_COMMA)) break;
            }
            expect(p, TOK_RPAREN, "Esperado ')' após argumentos");
            left = call;
            continue;
        }

        if (match(p, TOK_LBRACKET)) {
            AstNode *idx = ast_alloc(AST_INDEX, left->line);
            idx->as.index_access.object = left;
            idx->as.index_access.index = parse_expr(p);
            expect(p, TOK_RBRACKET, "Esperado ']' após índice");
            left = idx;
            continue;
        }

        if (match(p, TOK_DOT)) {
            Token field = expect(p, TOK_IDENT, "Esperado nome após '.'");
            AstNode *fa = ast_alloc(AST_FIELD, left->line);
            fa->as.field_access.object = left;
            strncpy(fa->as.field_access.field, field.lexeme, LV_MAX_IDENTIFIER - 1);
            fa->as.field_access.field[LV_MAX_IDENTIFIER - 1] = '\0';
            left = fa;
            continue;
        }

        break;
    }
    return left;
}

static AstNode *parse_expr_prec(Parser *p, int min_prec) {
    AstNode *left = NULL;
    Token tok = p->current;

    if (match(p, TOK_MINUS)) {
        AstNode *n = ast_alloc(AST_UNOP, tok.line);
        n->as.unop.op = TOK_MINUS;
        n->as.unop.operand = parse_expr_prec(p, PREC_UNARY);
        left = n;
    } else if (match(p, TOK_NOT)) {
        AstNode *n = ast_alloc(AST_UNOP, tok.line);
        n->as.unop.op = TOK_NOT;
        n->as.unop.operand = parse_expr_prec(p, PREC_UNARY);
        left = n;
    } else {
        left = parse_primary(p);
        left = parse_postfix(p, left);
    }

    while (!check(p, TOK_EOF)) {
        int prec = token_precedence(p->current.type);
        if (prec <= min_prec) break;

        Token op_tok = advance(p);

        if (op_tok.type == TOK_ASSIGN ||
            op_tok.type == TOK_PLUS_EQ || op_tok.type == TOK_MINUS_EQ ||
            op_tok.type == TOK_STAR_EQ || op_tok.type == TOK_SLASH_EQ) {
            AstNode *n = ast_alloc(AST_ASSIGN, op_tok.line);
            n->as.assign.target = left;
            n->as.assign.value = parse_expr_prec(p, PREC_ASSIGN - 1);
            n->as.assign.op = op_tok.type;

            if (op_tok.type != TOK_ASSIGN) {
                TokenType arith = TOK_PLUS;
                if (op_tok.type == TOK_MINUS_EQ) arith = TOK_MINUS;
                else if (op_tok.type == TOK_STAR_EQ) arith = TOK_STAR;
                else if (op_tok.type == TOK_SLASH_EQ) arith = TOK_SLASH;

                AstNode *binop = ast_alloc(AST_BINOP, op_tok.line);
                binop->as.binop.op = arith;
                binop->as.binop.left = left;
                binop->as.binop.right = n->as.assign.value;
                n->as.assign.value = binop;
                n->as.assign.op = TOK_ASSIGN;
            }
            left = n;
            break;
        }

        AstNode *right = parse_expr_prec(p, prec);
        AstNode *binop = ast_alloc(AST_BINOP, op_tok.line);
        binop->as.binop.op = op_tok.type;
        binop->as.binop.left = left;
        binop->as.binop.right = right;
        left = binop;
    }

    return left;
}

static AstNode *parse_expr(Parser *p) {
    return parse_expr_prec(p, PREC_NONE);
}

static AstNode *parse_var_decl(Parser *p, int is_const) {
    int line = p->prev.line;
    Token name = expect(p, TOK_IDENT, "Esperado nome de variável");
    AstNode *n = ast_alloc(is_const ? AST_CONST_DECL : AST_VAR_DECL, line);
    strncpy(n->as.var_decl.name, name.lexeme, LV_MAX_IDENTIFIER - 1);
    n->as.var_decl.name[LV_MAX_IDENTIFIER - 1] = '\0';
    n->as.var_decl.init = NULL;

    if (match(p, TOK_ASSIGN))
        n->as.var_decl.init = parse_expr(p);

    match(p, TOK_SEMICOLON);
    return n;
}

static AstNode *parse_func_decl(Parser *p) {
    int line = p->prev.line;
    Token name = expect(p, TOK_IDENT, "Esperado nome da função");
    AstNode *n = ast_alloc(AST_FUNC_DECL, line);
    strncpy(n->as.func_decl.name, name.lexeme, LV_MAX_IDENTIFIER - 1);
    n->as.func_decl.name[LV_MAX_IDENTIFIER - 1] = '\0';

    expect(p, TOK_LPAREN, "Esperado '(' após nome da função");

    int cap = 8;
    n->as.func_decl.params = (char(*)[LV_MAX_IDENTIFIER])malloc(
        (size_t)cap * LV_MAX_IDENTIFIER);
    n->as.func_decl.param_count = 0;

    while (!check(p, TOK_RPAREN) && !check(p, TOK_EOF)) {
        Token param = expect(p, TOK_IDENT, "Esperado nome de parâmetro");
        if (n->as.func_decl.param_count >= cap) {
            cap *= 2;
            n->as.func_decl.params = (char(*)[LV_MAX_IDENTIFIER])realloc(
                n->as.func_decl.params, (size_t)cap * LV_MAX_IDENTIFIER);
        }
        strncpy(n->as.func_decl.params[n->as.func_decl.param_count],
                param.lexeme, LV_MAX_IDENTIFIER - 1);
        n->as.func_decl.params[n->as.func_decl.param_count][LV_MAX_IDENTIFIER - 1] = '\0';
        n->as.func_decl.param_count++;
        if (!match(p, TOK_COMMA)) break;
    }

    expect(p, TOK_RPAREN, "Esperado ')' após parâmetros");
    n->as.func_decl.body = parse_block(p);
    return n;
}

static void class_add_member(Parser *p, AstNode *classe, AstNode *member,
                             int *cap) {
    (void)p;
    if (classe->as.class_decl.member_count >= *cap) {
        *cap *= 2;
        classe->as.class_decl.members = (AstNode**)realloc(
            classe->as.class_decl.members,
            (size_t)(*cap) * sizeof(AstNode*));
    }
    classe->as.class_decl.members[classe->as.class_decl.member_count++] = member;
}

static AstNode *parse_class_decl(Parser *p) {
    int line = p->prev.line;
    Token name = expect(p, TOK_IDENT, "Esperado nome da classe");

    AstNode *n = ast_alloc(AST_CLASS_DECL, line);
    strncpy(n->as.class_decl.name, name.lexeme, LV_MAX_IDENTIFIER - 1);
    n->as.class_decl.name[LV_MAX_IDENTIFIER - 1] = '\0';
    n->as.class_decl.has_parent = 0;
    n->as.class_decl.parent[0] = '\0';

    if (match(p, TOK_HERDAR)) {
        Token parent = expect(p, TOK_IDENT, "Esperado classe pai após 'herda'");
        strncpy(n->as.class_decl.parent, parent.lexeme, LV_MAX_IDENTIFIER - 1);
        n->as.class_decl.parent[LV_MAX_IDENTIFIER - 1] = '\0';
        n->as.class_decl.has_parent = 1;
    }

    expect(p, TOK_LBRACE, "Esperado '{' após declaração da classe");

    int cap = 8;
    n->as.class_decl.members = (AstNode**)malloc((size_t)cap * sizeof(AstNode*));
    n->as.class_decl.member_count = 0;

    while (!check(p, TOK_RBRACE) && !check(p, TOK_EOF)) {
        if (match(p, TOK_VAR)) {
            class_add_member(p, n, parse_var_decl(p, 0), &cap);
            continue;
        }
        if (match(p, TOK_CONST_KW)) {
            class_add_member(p, n, parse_var_decl(p, 1), &cap);
            continue;
        }
        if (match(p, TOK_FUNC)) {
            class_add_member(p, n, parse_func_decl(p), &cap);
            continue;
        }

        parser_error(p, "Dentro de classe use 'var', 'const' ou 'funcao'");
        if (!check(p, TOK_EOF)) advance(p);
        while (!check(p, TOK_RBRACE) && !check(p, TOK_VAR) &&
               !check(p, TOK_CONST_KW) && !check(p, TOK_FUNC) &&
               !check(p, TOK_EOF)) {
            advance(p);
        }
        p->panic_mode = 0;
    }

    expect(p, TOK_RBRACE, "Esperado '}' para fechar classe");
    return n;
}

static AstNode *parse_if(Parser *p) {
    int line = p->prev.line;
    AstNode *n = ast_alloc(AST_IF, line);

    int has_paren = match(p, TOK_LPAREN);
    n->as.if_stmt.condition = parse_expr(p);
    if (has_paren)
        expect(p, TOK_RPAREN, "Esperado ')' após condição do 'se'");

    n->as.if_stmt.then_branch = parse_block(p);
    n->as.if_stmt.else_branch = NULL;

    if (match(p, TOK_SENAO)) {
        if (match(p, TOK_SE))
            n->as.if_stmt.else_branch = parse_if(p);
        else
            n->as.if_stmt.else_branch = parse_block(p);
    }
    return n;
}

static AstNode *parse_while(Parser *p) {
    int line = p->prev.line;
    AstNode *n = ast_alloc(AST_WHILE, line);
    int has_paren = match(p, TOK_LPAREN);
    n->as.while_stmt.condition = parse_expr(p);
    if (has_paren)
        expect(p, TOK_RPAREN, "Esperado ')' após condição do 'enquanto'");
    n->as.while_stmt.body = parse_block(p);
    return n;
}

static AstNode *parse_for(Parser *p) {
    int line = p->prev.line;
    Token var = expect(p, TOK_IDENT, "Esperado variável após 'para'");

    if (match(p, TOK_DE)) {
        AstNode *n = ast_alloc(AST_FOR_RANGE, line);
        strncpy(n->as.for_range.var_name, var.lexeme, LV_MAX_IDENTIFIER - 1);
        n->as.for_range.var_name[LV_MAX_IDENTIFIER - 1] = '\0';
        n->as.for_range.from = parse_expr(p);
        expect(p, TOK_ATE, "Esperado 'ate' na faixa do 'para'");
        n->as.for_range.to = parse_expr(p);
        n->as.for_range.body = parse_block(p);
        return n;
    }

    if (match(p, TOK_EM)) {
        AstNode *n = ast_alloc(AST_FOR_EACH, line);
        strncpy(n->as.for_each.var_name, var.lexeme, LV_MAX_IDENTIFIER - 1);
        n->as.for_each.var_name[LV_MAX_IDENTIFIER - 1] = '\0';
        n->as.for_each.collection = parse_expr(p);
        n->as.for_each.body = parse_block(p);
        return n;
    }

    parser_error(p, "Esperado 'de' ou 'em' após variável do 'para'");
    return ast_alloc(AST_NULL_LIT, line);
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
    n->as.block.stmts = (AstNode**)malloc((size_t)cap * sizeof(AstNode*));
    n->as.block.count = 0;

    while (!check(p, TOK_RBRACE) && !check(p, TOK_EOF)) {
        if (n->as.block.count >= cap) {
            cap *= 2;
            n->as.block.stmts = (AstNode**)realloc(
                n->as.block.stmts, (size_t)cap * sizeof(AstNode*));
        }
        n->as.block.stmts[n->as.block.count++] = parse_stmt(p);
    }

    expect(p, TOK_RBRACE, "Esperado '}' para fechar bloco");
    return n;
}

static AstNode *parse_stmt(Parser *p) {
    if (p->had_error && p->panic_mode) {
        while (!check(p, TOK_SEMICOLON) && !check(p, TOK_RBRACE) && !check(p, TOK_EOF))
            advance(p);
        match(p, TOK_SEMICOLON);
        p->panic_mode = 0;
        return ast_alloc(AST_NULL_LIT, p->current.line);
    }

    if (match(p, TOK_VAR)) return parse_var_decl(p, 0);
    if (match(p, TOK_CONST_KW)) return parse_var_decl(p, 1);
    if (match(p, TOK_CLASSE)) return parse_class_decl(p);
    if (match(p, TOK_FUNC)) return parse_func_decl(p);
    if (match(p, TOK_SE)) return parse_if(p);
    if (match(p, TOK_ENQUANTO)) return parse_while(p);
    if (match(p, TOK_PARA)) return parse_for(p);
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

    AstNode *expr = parse_expr(p);
    match(p, TOK_SEMICOLON);
    return expr;
}

AstNode *leviata_parse(const char *source, int *had_error) {
    Lexer lex;
    lexer_init(&lex, source);

    Parser p;
    memset(&p, 0, sizeof(p));
    p.lex = &lex;
    p.current = lexer_next(&lex);
    if (p.current.type == TOK_ERROR) {
        p.had_error = 1;
        p.panic_mode = 1;
        fprintf(stderr, "[LEVIATA][Parser] Linha %d: símbolo inválido '%s'\n",
                p.current.line, p.current.lexeme);
    }

    int cap = 32;
    AstNode *prog = ast_alloc(AST_PROGRAM, 1);
    prog->as.program.stmts = (AstNode**)malloc((size_t)cap * sizeof(AstNode*));
    prog->as.program.count = 0;

    while (!check(&p, TOK_EOF)) {
        if (prog->as.program.count >= cap) {
            cap *= 2;
            prog->as.program.stmts = (AstNode**)realloc(
                prog->as.program.stmts, (size_t)cap * sizeof(AstNode*));
        }
        prog->as.program.stmts[prog->as.program.count++] = parse_stmt(&p);
    }

    if (had_error) *had_error = p.had_error;
    return prog;
}