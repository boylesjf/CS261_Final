#ifndef __CS261_P4__
#define __CS261_P4__

#include <stdbool.h>
#include <string.h>

#include "elf.h"
#include "y86.h"

void usage_p4 ();
bool parse_command_line_p4 (int argc, char **argv,
        bool *header, bool *segments, bool *membrief, bool *memfull,
        bool *disas_code, bool *disas_data,
        bool *exec_normal, bool *exec_debug, char **file);

void cpu_state (y86_t cpu);

y86_register_t decode_execute (y86_t *cpu, bool *cond, y86_inst_t inst,
        y86_register_t *valA);

void memory_wb_pc (y86_t *cpu, memory_t memory, bool cond, y86_inst_t inst,
        y86_register_t valE, y86_register_t valA);

#endif
