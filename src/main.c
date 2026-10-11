/* ============================================================================
 * LEVIATA 3 - launcher e REPL
 * ============================================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "leviata_lexer.h"
#include "leviata_vm.h"
#include "leviata_stdlib.h"
#include "leviata_ffi.h"
#include "leviata_2d.h"
#include "leviata_world.h"
#include "leviata_canon.h"
#include "leviata_resources.h"
#include "leviata_classes.h"
#include "leviata_super.h"
#include "leviata_maps.h"
#include "leviata_runtime.h"

static void print_banner(void) {
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║              LEVIATA v%d.%d.%d — Português              ║\n",
           LV_VERSION_MAJOR, LV_VERSION_MINOR, LV_VERSION_PATCH);
    printf("║ VM | Bytecode | World | Objetos | Classes | Modulos    ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");
}

static char *read_file_for_tokens(const char *path) {
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
        return NULL;
    }
    size_t count = fread(buf, 1, (size_t)size, f);
    buf[count] = '\0';
    fclose(f);
    return buf;
}

static void run_repl(LvRuntime *rt, LvVM *vm) {
    printf("REPL Leviata — 'sair()' para encerrar\n\n");
    char line[4096];
    while (1) {
        printf("LV> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) break;
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';
        if (strcmp(line, "sair") == 0 || strcmp(line, "sair()") == 0) break;
        if (line[0] == '\0') continue;
        vm->had_error = 0;
        leviata_execute_source(rt, vm, line, "<repl>", 0);
    }
    printf("\n[LEVIATA] Sessao REPL encerrada.\n");
}

static const char *DEMO_PROGRAMA =
    "var nome = \"Leviata\"\n"
    "var versao = \"3.8.0\"\n"
    "escreva(\"Sistema:\", nome, \"v\" + versao)\n"
    "var notas = [8, 9, 7, 10]\n"
    "adicionar(notas, 9)\n"
    "var soma_notas = 0\n"
    "para nota em notas { soma_notas = soma_notas + nota }\n"
    "escreva(\"Itens:\", tamanho(notas), \" soma:\", soma_notas)\n"
    "var perfil = { nome: \"Leviata\", versao: 3.8 }\n"
    "perfil.estavel = verdadeiro\n"
    "escreva(\"Mapa:\", perfil.nome, perfil[\"versao\"], tamanho(perfil))\n"
    "var config = arquivo(\"leviata_demo.tmp\")\n"
    "config.escreva(\"objetos=ok\")\n"
    "escreva(\"Recurso:\", tipo(config), config.caminho, config.leia())\n"
    "classe Base {\n"
    "  var texto = \"Oi\"\n"
    "  funcao diga(nome_alvo) { retorne este.texto + \" \" + nome_alvo }\n"
    "}\n"
    "classe Saudacao herda Base {\n"
    "  funcao diga(nome_alvo) { retorne super.diga(nome_alvo) + \"!\" }\n"
    "}\n"
    "var saudacao = novo Saudacao()\n"
    "escreva(\"Classe:\", tipo(saudacao), saudacao.diga(\"Leviata\"))\n"
    "cena.mudar(\"demo\")\n"
    "escreva(\"Cena:\", cena.atual())\n";

static void run_demo(LvRuntime *rt, LvVM *vm) {
    printf("=== DEMONSTRACAO LEVIATA 3.8 ===\n\n");
    leviata_execute_source(rt, vm, DEMO_PROGRAMA, "<demo>", 0);
    remove("leviata_demo.tmp");
}

static void demo_lexer(const char *source) {
    printf("=== TOKENS ===\n");
    Lexer lex;
    lexer_init(&lex, source);
    Token tok;
    do {
        tok = lexer_next(&lex);
        printf("[%3d:%2d] %-16s '%s'\n",
               tok.line, tok.col, token_type_name(tok.type), tok.lexeme);
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
    LvRuntime runtime;
    vm_init(&vm);
    leviata_runtime_init(&runtime);

    stdlib_register_all(&vm);
    ffi_register_runtime(&vm);
    leviata_2d_register_runtime(&vm);
    leviata_world_register_runtime(&vm);
    leviata_canon_register(&vm);
    leviata_resources_register(&vm);
    leviata_classes_register(&vm);
    leviata_super_register(&vm);
    leviata_maps_register(&vm);
    leviata_canon_finalize(&vm);

    int exit_code = 0;
    if (argc < 2) {
        run_repl(&runtime, &vm);
    } else if (strcmp(argv[1], "--demo") == 0) {
        run_demo(&runtime, &vm);
    } else if (strcmp(argv[1], "--ast") == 0 && argc >= 3) {
        exit_code = leviata_execute_file(&runtime, &vm, argv[2], 1) ? 0 : 1;
    } else if (strcmp(argv[1], "--tokens") == 0 && argc >= 3) {
        char *src = read_file_for_tokens(argv[2]);
        if (!src) exit_code = 1;
        else {
            demo_lexer(src);
            free(src);
        }
    } else if (argv[1][0] != '-') {
        exit_code = leviata_execute_file(&runtime, &vm, argv[1], 0) ? 0 : 1;
    } else {
        print_usage();
        exit_code = 1;
    }

    leviata_world_shutdown();
    leviata_2d_shutdown();
    game_close();
    leviata_runtime_free(&runtime);
    vm_free(&vm);
    return exit_code;
}
