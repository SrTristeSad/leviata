/* ============================================================================
 * MOTOR LEVIATA v2.0
 * Linguagem de Programação 'PT' — Arquitetura Completa
 *
 * Componentes:
 *   - Lexer:     Tokenizador com enum de TokenTypes
 *   - Parser:    Gerador de AST (Árvore de Sintaxe Abstrata)
 *   - Compiler:  Compilador AST → Bytecode
 *   - VM:        Máquina Virtual baseada em Bytecode
 *   - GC:        Garbage Collector Mark-and-Sweep
 *   - HashMap:   Tabela Hash para variáveis (O(1) lookup)
 *   - STD:       Biblioteca padrão (I/O, Math, String, Lista, Sistema)
 *   - FFI:       Bindings de Interface Gráfica e Motor de Jogos
 *
 * Compilação:
 *   gcc -o leviata src/*.c -Iinclude -lm -Wall -Wextra -std=c99
 *
 * Com Raylib:
 *   gcc -o leviata src/*.c -Iinclude -lm -lraylib -DLVA_USE_RAYLIB -Wall -std=c99
 *
 * Com SDL2:
 *   gcc -o leviata src/*.c -Iinclude -lm -lSDL2 -DLVA_USE_SDL2 -Wall -std=c99
 *
 * Uso:
 *   ./leviata programa.pt        — Executa arquivo .pt
 *   ./leviata                    — REPL interativo
 *   ./leviata --ast arquivo.pt   — Mostra AST sem executar
 *   ./leviata --demo             — Executa demonstração interna
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

/* ---- Declaração do parser público ---- */
AstNode *leviata_parse(const char *source, int *had_error);

/* ============================================================
 * BANNER
 * ============================================================ */
static void print_banner(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║         MOTOR LEVIATA v2.0 — Linguagem PT            ║\n");
    printf("║  Lexer | Parser | AST | Bytecode VM | GC | FFI       ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");
}

/* ============================================================
 * LER ARQUIVO FONTE
 * ============================================================ */
static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "[LEVIATA] Não foi possível abrir: %s\n", path);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *buf = (char*)malloc(size + 1);
    if (!buf) {
        fclose(f);
        fprintf(stderr, "[LEVIATA] Sem memória para carregar arquivo\n");
        return NULL;
    }
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);
    return buf;
}

/* ============================================================
 * EXECUTAR CÓDIGO FONTE
 * ============================================================ */
static int execute_source(LvVM *vm, const char *source, int show_ast) {
    /* 1. Parse → AST */
    int parse_error = 0;
    AstNode *ast = leviata_parse(source, &parse_error);
    if (parse_error) {
        fprintf(stderr, "[LEVIATA] Erros de parse. Execução cancelada.\n");
        ast_free(ast);
        return 0;
    }

    if (show_ast) {
        printf("\n=== AST GERADA ===\n");
        ast_print(ast, 0);
        printf("==================\n\n");
    }

    /* 2. Compile AST → Bytecode */
    Compiler compiler;
    compiler_init(&compiler, vm, NULL);
    Chunk chunk;
    int ok = compiler_compile_program(&compiler, ast, &chunk);
    ast_free(ast);
    compiler_free(&compiler);

    if (!ok) {
        fprintf(stderr, "[LEVIATA] Erros de compilação.\n");
        chunk_free(&chunk);
        return 0;
    }

    /* 3. Executar na VM */
    int result = vm_run_chunk(vm, &chunk);
    chunk_free(&chunk);
    return result;
}

/* ============================================================
 * REPL INTERATIVO
 * ============================================================ */
static void run_repl(LvVM *vm) {
    printf("REPL da Linguagem PT — 'sair()' para encerrar\n");
    printf("Dica: escreva código PT e pressione Enter\n\n");

    char line[4096];
    while (1) {
        printf("PT> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) break;

        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';

        if (strcmp(line, "sair") == 0 || strcmp(line, "sair()") == 0)
            break;
        if (strlen(line) == 0)
            continue;

        /* Reset do estado de erro da VM para permitir recuperação no REPL */
        vm->had_error = 0;
        execute_source(vm, line, 0);
    }
    printf("\n[LEVIATA] Sessão REPL encerrada.\n");
}

/* ============================================================
 * DEMONSTRAÇÃO INTERNA
 * ============================================================ */
static const char *DEMO_PROGRAMA =
    "/* === DEMONSTRAÇÃO DO MOTOR LEVIATA v2.0 === */\n"
    "\n"
    "/* 1. Variáveis e tipos básicos */\n"
    "var nome = \"Motor Leviata\"\n"
    "var versao = 2\n"
    "var pi_aprox = 3.14159\n"
    "escrever(\"Sistema:\", nome, \"v\" + para_texto(versao))\n"
    "\n"
    "/* 2. Aritmética e funções matemáticas */\n"
    "var r = raiz(144)\n"
    "escrever(\"Raiz de 144 =\", r)\n"
    "escrever(\"2 elevado a 10 =\", potencia(2, 10))\n"
    "escrever(\"Pi =\", pi())\n"
    "\n"
    "/* 3. Manipulação de strings */\n"
    "var msg = \"olá mundo\"\n"
    "escrever(\"Maiúsculas:\", maiusculas(msg))\n"
    "escrever(\"Contém 'mundo'?\", contem(msg, \"mundo\"))\n"
    "escrever(\"Tamanho:\", tamanho(msg))\n"
    "\n"
    "/* 4. Estrutura de controle: se/senao */\n"
    "var x = 42\n"
    "se (x > 10) {\n"
    "    escrever(\"x é maior que 10\")\n"
    "} senao {\n"
    "    escrever(\"x é menor ou igual a 10\")\n"
    "}\n"
    "\n"
    "/* 5. Laço: enquanto */\n"
    "var i = 0\n"
    "var soma = 0\n"
    "enquanto (i < 5) {\n"
    "    soma = soma + i\n"
    "    i = i + 1\n"
    "}\n"
    "escrever(\"Soma de 0+1+2+3+4 =\", soma)\n"
    "\n"
    "/* 6. Laço: para de ate */\n"
    "escrever(\"Contagem:\")\n"
    "para n de 1 ate 5 {\n"
    "    escrever(n)\n"
    "}\n"
    "\n"
    "/* 7. Listas (Arrays Dinâmicos) */\n"
    "var notas = [8, 9, 7, 10, 6]\n"
    "escrever(\"Lista criada, tamanho:\", tamanho(notas))\n"
    "escrever(\"Primeira nota:\", notas[0])\n"
    "lista_adicionar(notas, 9)\n"
    "escrever(\"Após adicionar, tamanho:\", tamanho(notas))\n"
    "\n"
    "/* 8. FFI - Interface Gráfica (stub) */\n"
    "escrever(\"\\n--- BINDING GRÁFICO ---\")\n"
    "janela_criar(\"Minha Janela PT\", 800, 600)\n"
    "desenhar_texto(\"Olá da Linguagem PT!\", 100, 100)\n"
    "\n"
    "/* 9. FFI - Motor de Jogos (stub) */\n"
    "escrever(\"\\n--- MOTOR DE JOGOS ---\")\n"
    "jogo_iniciar(\"Meu Jogo PT\", 1280, 720, 60)\n"
    "jogo_limpar_fundo(30, 30, 30)\n"
    "jogo_desenhar_sprite(\"heroi.png\", 400, 300)\n"
    "escrever(\"Delta time:\", jogo_delta())\n"
    "jogo_fechar()\n"
    "\n"
    "/* 10. Sistema */\n"
    "escrever(\"\\nTempo de execução:\", tempo_atual(), \"segundos\")\n"
    "escrever(\"\\n=== Demonstração completa! ===\")\n";

static void run_demo(LvVM *vm) {
    printf("=== EXECUTANDO DEMONSTRAÇÃO INTERNA ===\n\n");
    execute_source(vm, DEMO_PROGRAMA, 0);
}

/* ============================================================
 * DEMO DO LEXER
 * ============================================================ */
static void demo_lexer(const char *source) {
    printf("=== TOKENS GERADOS PELO LEXER ===\n");
    Lexer lex;
    lexer_init(&lex, source);
    Token tok;
    do {
        tok = lexer_next(&lex);
        printf("  [%3d:%2d] %-16s '%s'\n",
               tok.line, tok.col, token_type_name(tok.type), tok.lexeme);
    } while (tok.type != TOK_EOF && tok.type != TOK_ERROR);
    printf("=================================\n\n");
}

/* ============================================================
 * PONTO DE ENTRADA
 * ============================================================ */
int main(int argc, char *argv[]) {
    print_banner();

    /* Inicializa a VM */
    LvVM vm;
    vm_init(&vm);

    /* Registra toda a biblioteca padrão */
    stdlib_register_all(&vm);

    int exit_code = 0;

    if (argc < 2) {
        /* Sem argumentos → REPL */
        run_repl(&vm);

    } else if (strcmp(argv[1], "--demo") == 0) {
        /* Demonstração interna */
        run_demo(&vm);

    } else if (strcmp(argv[1], "--ast") == 0 && argc >= 3) {
        /* Mostrar AST de um arquivo */
        char *src = read_file(argv[2]);
        if (src) {
            execute_source(&vm, src, 1);
            free(src);
        } else {
            exit_code = 1;
        }

    } else if (strcmp(argv[1], "--tokens") == 0 && argc >= 3) {
        /* Mostrar tokens de um arquivo */
        char *src = read_file(argv[2]);
        if (src) {
            demo_lexer(src);
            free(src);
        } else {
            exit_code = 1;
        }

    } else if (argv[1][0] != '-') {
        /* Executar arquivo .pt */
        char *src = read_file(argv[1]);
        if (src) {
            int ok = execute_source(&vm, src, 0);
            free(src);
            exit_code = ok ? 0 : 1;
        } else {
            exit_code = 1;
        }

    } else {
        fprintf(stderr, "Uso:\n");
        fprintf(stderr, "  leviata                    — REPL interativo\n");
        fprintf(stderr, "  leviata programa.pt        — Executa arquivo\n");
        fprintf(stderr, "  leviata --demo             — Demonstração interna\n");
        fprintf(stderr, "  leviata --ast arquivo.pt   — Mostra AST\n");
        fprintf(stderr, "  leviata --tokens arq.pt    — Mostra tokens\n");
        exit_code = 1;
    }

    /* Finaliza: GC + libera VM */
    vm_free(&vm);

    return exit_code;
}
