/*
 * CS 261: Main driver
 *
 * Name: 
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "p1-check.h"
#include "p2-load.h"
#include "p3-disas.h"
#include "p4-interp.h"
#include "printinst.c"

int main (int argc, char **argv)
{
    bool header = false, segments = false, membrief = false, memfull = false, 
		disas_code = false, disas_data = false, exec_normal = false,
		exec_debug = false, arguments = false, cond = false;;
	char *filename;
	int i, exe_count = 0;
	FILE *file;
	elf_hdr_t hdr;
	memory_t memory;
	uint16_t offset;
	y86_register_t valA, valE = 0;
	y86_t cpu;
	y86_inst_t inst;

	arguments = parse_command_line_p4(argc, argv, &header, &segments, &membrief, &memfull, 
		&disas_code, &disas_data, &exec_normal, &exec_debug, &filename);

	//bad arguments
	if (!arguments)
		return EXIT_FAILURE;

	//h option
	if (arguments && filename == NULL)
		return EXIT_SUCCESS;

	//open file
	file = fopen(filename, "r");
	if (!file)
	{
		printf("%s", "Failed to read file\n");
		return EXIT_FAILURE;
	}

	//read in elf header
	if (!read_header(file, &hdr))
		return EXIT_FAILURE;

	//read in program headers
	elf_phdr_t phdr[hdr.e_num_phdr];
	offset = hdr.e_phdr_start;

	for (i = 0; i < hdr.e_num_phdr; i++)
	{
		if (!read_phdr(file, offset, &phdr[i]))
		{
			printf("%s", "Failed to read file\n");
			return EXIT_FAILURE;
		}
		offset = offset + sizeof(elf_phdr_t);
	}	
	
	//allocate memory
	memory = malloc(MEMSIZE);

	//bad memory allocation
	if (memory == NULL)
	{
		printf("Failed to read file");
		return EXIT_FAILURE;
	}

	//load segments
	for (i = 0; i < hdr.e_num_phdr; i++)
	{
		if (!load_segment(file, memory, phdr[i]))
		{
			printf("%s", "Failed to read file\n");	
			return EXIT_FAILURE;
		}
	}

	//H option
	if (header)
	{
		dump_header(hdr);
	}

	//s option
	if (segments)
	{
		dump_phdrs(hdr.e_num_phdr, phdr);	
	}
	
	//M option
	if (memfull)
	{
		dump_memory(memory, 0, 4096);
	}
	
	//m option
	if (membrief)
	{
		for (i = 0; i < hdr.e_num_phdr; i++)
			dump_memory(memory, phdr[i].p_vaddr, phdr[i].p_vaddr + phdr[i].p_filesz);
	}

	//d option
	if (disas_code)
	{
		printf("Disassembly of executable contents:\n");
		for (i = 0; i < hdr.e_num_phdr; i++)
		{
			if (phdr[i].p_type == CODE)
				disassemble_code (memory, &phdr[i], &hdr);
		}
	}

	//D option
	if (disas_data)
	{
		printf("Disassembly of data contents:\n");
		for (i = 0; i < hdr.e_num_phdr; i++)
		{
			if (phdr[i].p_type == DATA)
			{
				if (phdr[i].p_flag == 4)
					disassemble_rodata (memory, &phdr[i]);
				else
					disassemble_data (memory, &phdr[i]);
			}
		}
	}

	//initialize cpu
	memset (&cpu, 0, sizeof (y86_t));
	cpu.pc = hdr.e_entry;
	cpu.stat = AOK;

	//e option
	if (exec_normal)
	{
		printf("Beginning execution at 0x%04x\n", (int)cpu.pc);
		do
		{
			inst = fetch (cpu, memory);

			valE = decode_execute (&cpu, &cond, inst, &valA);

			memory_wb_pc (&cpu, memory, cond, inst, valE, valA);

			exe_count++;
		} while (cpu.stat == AOK);

		cpu_state (cpu);
		printf("Total execution count: %d\n", exe_count);
	}

	//E option
	if (exec_debug)
	{
		printf("Beginning execution at 0x%04x\n", (int)cpu.pc);
		cpu_state (cpu);
		printf("\n");
		do
		{
			inst = fetch (cpu, memory);
			printf("Executing: ");
			print_inst (inst);

			valE = decode_execute (&cpu, &cond, inst, &valA);

			memory_wb_pc (&cpu, memory, cond, inst, valE, valA);

			cpu_state (cpu);
			exe_count++;

			if (cpu.stat == AOK)
				printf("\n");
		} while (cpu.stat == AOK);

		printf("Total execution count: %d\n", exe_count);
	}

	//deallocate memory
	free(memory);

	//close file
	fclose(file);
	
	return EXIT_SUCCESS;
}

