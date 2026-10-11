/* ============================================================================
 * LEVIATA 3.8 - COMPOSICAO DO RUNTIME
 *
 * O runtime de modulos 3.7 permanece responsavel por carregamento/isolamento.
 * Apenas suas entradas de parser e compilador sao redirecionadas para as
 * camadas 3.8, que depois delegam para as implementacoes anteriores.
 * ============================================================================ */
#define leviata_parse leviata_parse_v38
#define compiler_compile_program compiler_compile_program_v38
#include "leviata_runtime_v37.c"
#undef compiler_compile_program
#undef leviata_parse
