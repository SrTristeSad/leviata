/* ============================================================================
 * MOTOR LEVIATA v2.3
 * Linguagem PT - launcher, REPL e execucao de arquivos .pt
 * ============================================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "leviata_lexer.h"
#include "leviata_ast.h"
#include "leviata_vm.h"
#include "leviata_compiler.h"
#include "leviata_stdlib.h"
#include "leviata_ffi.h"
#include "leviata_2d.h"
#include "leviata_world.h"

AstNode *leviata_parse(const char *source, int *had_error);

static void print_banner(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║       MOTOR LEVIATA v2.3 — Linguagem PT             ║\n");
    printf("║ VM | Raylib | Camera | Tilemap | Entidades | Fisica║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");
}

static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "[LEVIATA] Nao foi possivel abrir: %s\n", path);
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long size = ftell(f);
    if (size < 0) { fclose(f); return NULL; }
    rewind(f);

    char *buf = (char*)malloc((size_t)size + 1);
    if (!buf) {
        fclose(f);
        fprintf(stderr, "[LEVIATA] Sem memoria para carregar arquivo.\n");
        return NULL;
    }
    size_t read_count = fread(buf, 1, (size_t)size, f);
    buf[read_count] = '\0';
    fclose(f);
    return buf;
}

static int execute_source(LvVM *vm, const char *source, int show_ast) {
    int parse_error = 0;
    AstNode *ast = leviata_parse(source, &parse_error);
    if (!ast || parse_error) {
        fprintf(stderr, "[LEVIATA] Erros de parse. Execucao cancelada.\n");
        ast_free(ast);
        return 0;
    }

    if (show_ast) {
        printf("\n=== AST GERADA ===\n");
        ast_print(ast, 0);
        printf("==================\n\n");
    }

    Compiler compiler;
    compiler_init(&compiler, vm, NULL);
    Chunk chunk;
    int ok = compiler_compile_program(&compiler, ast, &chunk);
    ast_free(ast);
    compiler_free(&compiler);

    if (!ok) {
        fprintf(stderr, "[LEVIATA] Erros de compilacao.\n");
        chunk_free(&chunk);
        return 0;
    }

    int result = vm_run_chunk(vm, &chunk);
    chunk_free(&chunk);
    return result;
}

static void run_repl(LvVM *vm) {
    printf("REPL da Linguagem PT — 'sair()' para encerrar\n\n");
    char line[4096];
    while (1) {
        printf("PT> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) break;
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';
        if (strcmp(line, "sair") == 0 || strcmp(line, "sair()") == 0) break;
        if (line[0] == '\0') continue;
        vm->had_error = 0;
        execute_source(vm, line, 0);
    }
    printf("\n[LEVIATA] Sessao REPL encerrada.\n");
}

static const char *DEMO_PROGRAMA =
    "var nome = \"Motor Leviata\"\n"
    "var versao = 2.3\n"
    "escrever(\"Sistema:\", nome, \"v\" + para_texto(versao))\n"
    "escrever(\"Raiz de 144 =\", raiz(144))\n"
    "var notas = [8, 9, 7, 10]\n"
    "lista_adicionar(notas, 9)\n"
    "escrever(\"Itens na lista:\", tamanho(notas))\n"
    "cena_mudar(\"demo\")\n"
    "var entidade_demo = entidade_criar(\"demo\", 10, 20, 32, 32)\n"
    "entidade_fisica(entidade_demo, 980, 2)\n"
    "escrever(\"Entidade:\", entidade_nome(entidade_demo), \" componentes fisica=\", entidade_tem_componente(entidade_demo, \"fisica\"))\n"
    "escrever(\"Motor 2D/World ativo. Cena:\", cena_atual())\n";

static void run_demo(LvVM *vm) {
    printf("=== DEMONSTRACAO LEVIATA ===\n\n");
    execute_source(vm, DEMO_PROGRAMA, 0);
}

static void demo_lexer(const char *source) {
    printf("=== TOKENS ===\n");
    Lexer lex;
    lexer_init(&lex, source);
    Token tok;
    do {
        tok = lexer_next(&lex);
        printf("[%3d:%2d] %-16s '%s'\n", tok.line, tok.col, token_type_name(tok.type), tok.lexeme);
    } while (tok.type != TOK_EOF && tok.type != TOK_ERROR);
}

static void print_usage(void) {
    fprintf(stderr, "Uso:\n");
    fprintf(stderr, "  leviata                    REPL interativo\n");
    fprintf(stderr, "  leviata programa.pt        Executa arquivo\n");
    fprintf(stderr, "  leviata --demo             Demonstra a linguagem\n");
    fprintf(stderr, "  leviata --ast arquivo.pt   Mostra AST e executa\n");
    fprintf(stderr, "  leviata --tokens arq.pt    Mostra tokens\n");
}

int main(int argc, char *argv[]) {
    print_banner();

    LvVM vm;
    vm_init(&vm);
    stdlib_register_all(&vm);
    ffi_register_runtime(&vm);
    leviata_2d_register_runtime(&vm);
    leviata_world_register_runtime(&vm);

    int exit_code = 0;
    if (argc < 2) {
        run_repl(&vm);
    } else if (strcmp(argv[1], "--demo") == 0) {
        run_demo(&vm);
    } else if (strcmp(argv[1], "--ast") == 0 && argc >= 3) {
        char *src = read_file(argv[2]);
        if (!src) exit_code = 1;
        else { exit_code = execute_source(&vm, src, 1) ? 0 : 1; free(src); }
    } else if (strcmp(argv[1], "--tokens") == 0 && argc >= 3) {
        char *src = read_file(argv[2]);
        if (!src) exit_code = 1;
        else { demo_lexer(src); free(src); }
    } else if (argv[1][0] != '-') {
        char *src = read_file(argv[1]);
        if (!src) exit_code = 1;
        else { exit_code = execute_source(&vm, src, 0) ? 0 : 1; free(src); }
    } else {
        print_usage();
        exit_code = 1;
    }

    leviata_world_shutdown();
    leviata_2d_shutdown();
    game_close();
    vm_free(&vm);
    return exit_code;
}
