/*
 * CS 261 PA4: Mini-ELF interpreter
 *
 * Name: 
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "p4-interp.h"

void usage_p4 ()
{
    printf("Usage: y86 <option(s)> mini-elf-file\n");
    printf(" Options are:\n");
    printf("  -h      Display usage\n");
    printf("  -H      Show the Mini-ELF header\n");
    printf("  -a      Show all with brief memory\n");
    printf("  -f      Show all with full memory\n");
    printf("  -s      Show the program headers\n");
    printf("  -m      Show the memory contents (brief)\n");
    printf("  -M      Show the memory contents (full)\n");
    printf("  -d      Disassemble code contents\n");
    printf("  -D      Disassemble data contents\n");
    printf("  -e      Execute program\n");
    printf("  -E      Execute program (debug trace mode)\n");
}

//parse command line p4
bool parse_command_line_p4 (int argc, char **argv,
        bool *header, bool *segments, bool *membrief, bool *memfull,
        bool *disas_code, bool *disas_data,
        bool *exec_normal, bool *exec_debug, char **file)
{
    int option = 0;
	opterr = 0;

	if (header == NULL || segments == NULL || membrief == NULL || memfull == NULL || 
		disas_code == NULL || disas_data == NULL || exec_normal == NULL || 
		exec_debug == NULL || file == NULL)
	{
		printf("%s", "Failed to read file\n");
		return false;
	}

	while ((option = getopt(argc, argv, "hHafsmMdDeE")) != -1)
	{
		switch (option)
		{
			case 'H' :              //H option
				*header = true;
				break;
			case 'h' :              //h option
				usage_p4();
				*header = false;
				*segments = false;
				*membrief = false;
				*memfull = false;
				*disas_code = false;
				*disas_data = false;
				*exec_normal = false;
				*exec_debug = false;
				*file = NULL;
				return true;
			case 'a' :              //a option
				*header = true;
				*segments = true;
				if (*memfull)
					return false;
				*membrief = true;
				break;
			case 'f' :              //f option
				*header = true;
				*segments = true;
				if (*membrief)
					return false;
				*memfull = true;
				break;
			case 's' :              //s option
				*segments = true;
				break;
			case 'm' :              //m option
				if (*memfull)
				{
					usage_p4();
					return false;
				}
				*membrief = true;
				break;
			case 'M' :              //M option
				if (*membrief)
				{
					usage_p4();
					return false;
				}
				*memfull = true;
				break;
			case 'd' :              //d option
				*disas_code = true;
				break;
			case 'D' :              //D option
				*disas_data = true;
				break;
			case 'e' :              //e option
				if (*exec_debug)
				{
					usage_p4();
					return false;
				}
				*exec_normal = true;
				break;
			case 'E' :              //E option
				if (*exec_normal)
				{
					usage_p4();
					return false;
				}
				*exec_debug = true;
				break;
			default :               //bad option
				usage_p4();
				return false;
		}
	}
	if (argv[optind] == NULL)   //no arguments
	{
		if (*header)
			usage_p4();
		return false;
	}
	else if (optind + 1 < argc) //multiple arguments
	{
		usage_p4();
		return false;
	}
	else                        //one argument (correct)
	{
		*file = argv[optind];
		return true;
	}
}

//print cpu state
void cpu_state (y86_t cpu)
{
	printf("Y86 CPU state:\n");
	printf("  %%rip: %016jx   flags:", cpu.pc);

	if (cpu.zf)
		printf(" Z1");
	else
		printf(" Z0");

	if (cpu.sf)
		printf(" S1");
	else
		printf(" S0");

	if (cpu.of)
		printf(" O1");
	else
		printf(" O0");

	switch (cpu.stat)
	{
		case AOK :
			printf("     AOK\n");
			break;
		case HLT :
			printf("     HLT\n");
			break;
		case ADR :
			printf("     ADR\n");
			break;
		case INS :
			printf("     INS\n");
			break;
		default :
			break;
	}

	printf("  %%rax: %016jx    %%rcx: %016jx\n", cpu.rax, cpu.rcx);
	printf("  %%rdx: %016jx    %%rbx: %016jx\n", cpu.rdx, cpu.rbx);
	printf("  %%rsp: %016jx    %%rbp: %016jx\n", cpu.rsp, cpu.rbp);
	printf("  %%rsi: %016jx    %%rdi: %016jx\n", cpu.rsi, cpu.rdi);
}

//get register helper
y86_register_t get_reg (y86_t cpu, y86_rnum_t r)
{
	y86_register_t reg = 0;

	switch (r)
	{
		case 0 :
			reg = cpu.rax;
			break;
		case 1 :
			reg = cpu.rcx;
			break;
		case 2 :
			reg = cpu.rdx;
			break;
		case 3 :
			reg = cpu.rbx;
			break;
		case 4 :
			reg = cpu.rsp;
			break;
		case 5 :
			reg = cpu.rbp;
			break;
		case 6 :
			reg = cpu.rsi;
			break;
		case 7 :
			reg = cpu.rdi;
			break;
		default :
			break;
	}
	return reg;
}

//xor helper
bool xor (bool a, bool b)
{
    return ((a && !b) || (!a && b));
}

//cond cmov helper
bool cond_cmov (y86_t *cpu, y86_cmov_t cmov)
{
	bool cond = false;

	switch (cmov)
	{
		case RRMOVQ :
			cond = true;
			break;
		case CMOVLE :
			if ((xor(cpu->sf, cpu->of)) || cpu->zf)
				cond = true;
			break;
		case CMOVL :
			if (xor(cpu->sf, cpu->of))
				cond = true;
			break;
		case CMOVE :
			if (cpu->zf)
				cond = true;
			break;
		case CMOVNE :
			if (!cpu->zf)
				cond = true;
			break;
		case CMOVGE :
			if (!xor(cpu->sf, cpu->of))
				cond = true;
			break;
		case CMOVG :
			if ((!xor(cpu->sf, cpu->of)) && !cpu->zf) 
				cond = true;
			break;
		default :
			cpu->stat = INS;
			cpu->pc = 0xffffffffffffffff;
			break;		
	}
	return cond;
}

//cond jump helper
bool cond_jump (y86_t *cpu, y86_jump_t jump)
{
	bool cond = false;

	switch (jump)
	{
		case JMP :
			cond = true;
			break;
		case JLE :
			if (xor(cpu->sf, cpu->of) || cpu->zf)
				cond = true;
			break;
		case JL :
			if (xor(cpu->sf, cpu->of))
				cond = true;
			break;
		case JE :
			if (cpu->zf)
				cond = true;
			break;
		case JNE :
			if (!cpu->zf)
				cond = true;
			break;
		case JGE :
			if (!xor(cpu->sf, cpu->of))
				cond = true;
			break;
		case JG :
			if (!xor(cpu->sf, cpu->of) && !cpu->zf)
				cond = true;
				break;
		default :
			cpu->stat = INS;
			cpu->pc = 0xffffffffffffffff;
			break;
	}
	return cond;
}

//opq cc helper
y86_register_t opq_cc (y86_t *cpu, y86_register_t valA, y86_register_t valB,
					   y86_op_t op)
{
	y86_register_t valE = 0;
	int64_t s_valE, s_valA = valA, s_valB = valB;

	switch (op)
	{
		case ADD :
			s_valE = s_valB + s_valA;
			if (cpu->stat == AOK)
			{
				if ((s_valB > 0 && s_valA > 0 && s_valE < 0) || //overflow
				    (s_valB < 0 && s_valA < 0 && s_valE > 0))
				{
					cpu->of = true;
				}
				else
				{
					cpu->of = false;
				}

				if (s_valE == 0) //zero
				{
					cpu->zf = true;
					cpu->sf = false;
				}
				else if (s_valE < 0) //sign
				{
					cpu->zf = false;
					cpu->sf = true;
				}
				else
				{
					cpu->zf = false;
					cpu->sf = false;
				}
			}
			break;
		case SUB :
			s_valE = s_valB - s_valA;
			if (cpu->stat == AOK)
			{
				if ((s_valA > 0 && s_valE > s_valB) || //overflow
				    (s_valA < 0 && s_valE < s_valB))
				{
					cpu->of = true;
				}
				else
				{
					cpu->of = false;
				}

				if (s_valE == 0) //zero
				{
					cpu->zf = true;
					cpu->sf = false;
				}
				else if (s_valE < 0) //sign
				{
					cpu->zf = false;
					cpu->sf = true;
				}
				else
				{
					cpu->zf = false;
					cpu->sf = false;
				}
			}
			break;
		case AND :
			s_valE = s_valB & s_valA;
			if (cpu->stat == AOK)
			{
				cpu->of = false; //reset overflow

				if (s_valE == 0) //zero
				{
					cpu->zf = true;
					cpu->sf = false;
				}
				else if (s_valE < 0) //sign
				{
					cpu->zf = false;
					cpu->sf = true;
				}
				else
				{
					cpu->zf = false;
					cpu->sf = false;
				}
			}
			break;
		case XOR :
			s_valE = s_valB ^ s_valA;
			if (cpu->stat == AOK)
			{
				cpu->of = false; //reset overflow

				if (s_valE == 0) //zero
				{
					cpu->zf = true;
					cpu->sf = false;
				}
				else if (s_valE < 0) //sign
				{
					cpu->zf = false;
					cpu->sf = true;
				}
				else
				{
					cpu->zf = false;
					cpu->sf = false;
				}
			}
			break;
		default :
			cpu->stat = INS;
			cpu->pc = 0xffffffffffffffff;
			break;
	}

	valE = s_valE;
	return valE;
}

//decode execute
y86_register_t decode_execute (y86_t *cpu, bool *cond, y86_inst_t inst,
        y86_register_t *valA)
{
	y86_register_t valB, valE;

    if (cpu != NULL && cond != NULL && valA != NULL)
	{
		switch (inst.type)
		{
			case HALT :
				cpu->stat = HLT;
				cpu->zf = false;
				cpu->sf = false;
				cpu->of = false;
				valE = 0;
				break;
			case NOP :
				valE = 0;
				break;
			case CMOV :
				*valA = get_reg (*cpu, inst.ra);
				valE  = *valA;
				*cond = cond_cmov (cpu, inst.cmov);
				break;
			case IRMOVQ :
				valE = inst.value;
				break;
			case RMMOVQ :
				*valA = get_reg (*cpu, inst.ra);
				valB  = get_reg (*cpu, inst.rb);
				valE  = valB + inst.d;
				break;
			case MRMOVQ :
				valB = get_reg (*cpu, inst.rb);
				valE = valB + inst.d;
				break;
			case OPQ :
				*valA = get_reg (*cpu, inst.ra);
				valB  = get_reg (*cpu, inst.rb);
				valE  = opq_cc (cpu, *valA, valB, inst.op);
				break;
			case JUMP :
				valE = 0;
				*cond = cond_jump (cpu, inst.jump);
				break;
			case CALL :
				valB = get_reg (*cpu, 4);
				valE = valB - 8;
				break;
			case RET :
				*valA = get_reg (*cpu, 4);
				valB  = get_reg (*cpu, 4);
				valE  = valB + 8;
				break;
			case PUSHQ :
				*valA = get_reg (*cpu, inst.ra);
				valB  = get_reg (*cpu, 4);
				valE  = valB - 8;
				break;
			case POPQ :
				*valA = get_reg (*cpu, 4);
				valB  = get_reg (*cpu, 4);
				valE  = valB + 8;
				break;
			default :
				cpu->stat = INS;
				cpu->pc = 0xffffffffffffffff;
				valE = 0;
				break;
		}
	}
	else
	{
		valE = 0;
		cpu->stat = INS;
	}
	return valE;
}

//set reg helper
void set_reg (y86_t *cpu, y86_rnum_t r, y86_register_t reg)
{
	switch (r)
	{
		case 0 :
			cpu->rax = reg;
			break;
		case 1 :
			cpu->rcx = reg;
			break;
		case 2 :
			cpu->rdx = reg;
			break;
		case 3 :
			cpu->rbx = reg;
			break;
		case 4 :
			cpu->rsp = reg;
			break;
		case 5 :
			cpu->rbp = reg;
			break;
		case 6 :
			cpu->rsi = reg;
			break;
		case 7 :
			cpu->rdi = reg;
			break;
	}
}

//memory wb pc
void memory_wb_pc (y86_t *cpu, memory_t memory, bool cond, y86_inst_t inst,
        y86_register_t valE, y86_register_t valA)
{
	uint64_t *buff;
	if (cpu != NULL && memory != NULL)
	{
		switch (inst.type)
		{
			case HALT :
				cpu->pc = 0;
				break;
			case NOP :
				cpu->pc += inst.size;
				break;
			case CMOV :
				if (cond)
					set_reg (cpu, inst.rb, valE);
				cpu->pc += inst.size;
				break;
			case IRMOVQ :
				set_reg (cpu, inst.rb, valE);
				cpu->pc += inst.size;
				break;
			case RMMOVQ :
				if (valE > 4096)
				{
					cpu->stat = ADR;
					cpu->pc = 0xffffffffffffffff;
					break;
				}
				buff = (uint64_t*) &memory[valE];
				*buff = valA;
				cpu->pc += inst.size;
				printf("Memory write to 0x%04jx: 0x%jx\n", valE, valA);
				break;
			case MRMOVQ :
				if (valE < 4096)
				{
					buff = (uint64_t*) &memory[valE];
					set_reg (cpu, inst.ra, *buff);
					cpu->pc += inst.size;
				}
				else
				{
					cpu->stat = ADR;
					cpu->pc = 0xffffffffffffffff;
				}
				break;
			case OPQ :
				set_reg (cpu, inst.rb, valE);
				cpu->pc += inst.size;
				break;
			case JUMP :
				if (cond)
				{
					if (inst.dest < 4096)
					{
						cpu->pc = inst.dest;
					}
					else
					{
						cpu->stat = ADR;
						cpu->pc = 0xffffffffffffffff;
						break;
					}
				}
				else
				{
					cpu->pc += inst.size;
				}
				break;
			case CALL :
				if (valE > 4096)
				{
					cpu->stat = ADR;
					cpu->pc = 0xffffffffffffffff;
					break;
				}
				buff = (uint64_t*) &memory[valE];
				*buff = cpu->pc + inst.size;
				cpu->rsp = valE;
				if (inst.dest < 4096)
				{
					cpu->pc = inst.dest;
				}
				else
				{
					cpu->stat = ADR;
					cpu->pc = 0xffffffffffffffff;
					break;
				}
				printf("Memory write to 0x%04jx: 0x%jx\n", valE, *buff);
				break;
			case RET :
				buff = (uint64_t*) &memory[valA];
				if (*buff < 4096 && *buff > 0)
				{
					cpu->pc = *buff;
					cpu->rsp = valE;
				}
				else
				{
					cpu->stat = ADR;
					cpu->pc = 0xffffffffffffffff;
				}
				break;
			case PUSHQ :
				if (valE > 4096)
				{
					cpu->stat = ADR;
					cpu->pc = 0xffffffffffffffff;
					break;
				}
				buff = (uint64_t*) &memory[valE];
				*buff = valA;
				cpu->rsp = valE;
				cpu->pc += inst.size;
				printf("Memory write to 0x%04jx: 0x%jx\n", valE, valA);
				break;
			case POPQ :
				buff = (uint64_t*) &memory[valA];
				set_reg (cpu, inst.ra, *buff);
				cpu->rsp = valE;
				cpu->pc += inst.size;
				break;
			default :
				break;
		}
	}
}

