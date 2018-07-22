//printing helper functions
void print_reg (y86_rnum_t reg)
{
	switch (reg)
	{
		case 0 :
			printf("%%rax");
			break;
		case 1 :
			printf("%%rcx");
			break;
		case 2 :
			printf("%%rdx");
			break;
		case 3 :
			printf("%%rbx");
			break;
		case 4 :
			printf("%%rsp");
			break;
		case 5 :
			printf("%%rbp");
			break;
		case 6 :
			printf("%%rsi");
			break;
		case 7 :
			printf("%%rdi");
			break;
		default :
			break;
	}
}

void print_CMOV (y86_inst_t inst)
{
	switch (inst.cmov)
	{
		case RRMOVQ :
			printf("rrmovq ");
			print_reg(inst.ra);
			printf(", ");
			print_reg(inst.rb);
			break;
		case CMOVLE :
			printf("cmovle ");
			print_reg(inst.ra);
			printf(", ");
			print_reg(inst.rb);
			break;
		case CMOVL :
			printf("cmovl ");
			print_reg(inst.ra);
			printf(", ");
			print_reg(inst.rb);
			break;
		case CMOVE :
			printf("cmove ");
			print_reg(inst.ra);
			printf(", ");
			print_reg(inst.rb);
			break;
		case CMOVNE :
			printf("cmovne ");
			print_reg(inst.ra);
			printf(", ");
			print_reg(inst.rb);
			break;
		case CMOVGE :
			printf("cmovge ");
			print_reg(inst.ra);
			printf(", ");
			print_reg(inst.rb);
			break;
		case CMOVG :
			printf("cmovg ");
			print_reg(inst.ra);
			printf(", ");
			print_reg(inst.rb);
			break;
		default :
			break;
	}
}

void print_opq (y86_inst_t inst)
{
	switch (inst.op)
	{
		case ADD :
			printf("add ");
			print_reg(inst.ra);
			printf(", ");
			print_reg(inst.rb);
			break;
		case SUB :
			printf("sub ");
			print_reg(inst.ra);
			printf(", ");
			print_reg(inst.rb);
			break;
		case AND :
			printf("and ");
			print_reg(inst.ra);
			printf(", ");
			print_reg(inst.rb);
			break;
		case XOR :
			printf("xor ");
			print_reg(inst.ra);
			printf(", ");
			print_reg(inst.rb);
			break;
		default :
			break;			
	}
}

void print_jump (y86_inst_t inst)
{
	switch (inst.jump)
	{
		case JMP :
			printf("jmp 0x%jx", inst.dest);
			break;
		case JLE :
			printf("jle 0x%jx", inst.dest);
			break;
		case JL :
			printf("jl 0x%jx", inst.dest);
			break;
		case JE :
			printf("je 0x%jx", inst.dest);
			break;
		case JNE :
			printf("jne 0x%jx", inst.dest);
			break;
		case JGE :
			printf("jge 0x%jx", inst.dest);
			break;
		case JG :
			printf("jg 0x%jx", inst.dest);
			break;
		default :
			break;
	}
}

void print_inst (y86_inst_t inst)
{
	switch (inst.type)
	{
		case HALT :
			printf("halt");
			break;
		case NOP :
			printf("nop");
			break;
		case CMOV :
			print_CMOV(inst);
			break;
		case IRMOVQ :
			printf("irmovq 0x%jx, ", (inst.value));
			print_reg(inst.rb);
			break;
		case RMMOVQ :
			printf("rmmovq ");
			print_reg(inst.ra);
			printf(", 0x%jx(", inst.d);
			print_reg(inst.rb);
			printf(")");
			break;
		case MRMOVQ :
			printf("mrmovq 0x%jx(", inst.d);
			print_reg(inst.rb);
			printf("), ");
			print_reg(inst.ra);
			break;
		case OPQ :
			print_opq(inst);
			break;
		case JUMP :
			print_jump(inst);
			break;
		case CALL :
			printf("call 0x%jx", inst.dest);
			break;
		case RET :
			printf("ret");
			break;
		case PUSHQ :
			printf("pushq ");
			print_reg(inst.ra);
			break;
		case POPQ :
			printf("popq ");
			print_reg(inst.ra);
			break;
		default :
			break;
	}
	printf("\n");
}
