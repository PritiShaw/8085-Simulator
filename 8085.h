#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <time.h>

#define DEBUG  0
#define RAMSIZE 0x1000 //4096

typedef struct Flags
{
	uint8_t z : 1;
	uint8_t s : 1;
	uint8_t p : 1;
	uint8_t cy : 1;
	uint8_t ac : 1;
	uint8_t pad : 3; // 3 empty
} Flags;

typedef enum { UNASSIGNED, DATA_TRANSFER , ARITHMETIC_LOGIC, JUMP, RETURN, MACHINE_CONTROL, ERROR  } intruction_type;

typedef struct Instruction
{
	char instruction[16];
	char mnemonics[5];
	char operand[5];
    char toLabel[10];
    char address[5];
    int size;
    intruction_type type;
    int error;
} Instruction;

typedef struct Label
{
	int lineNo;
	int instNo;
	uint16_t declaration;
	char name[10];
	int linkedFrom[50];
	int linkedCount;
	int references;
} Label;

typedef struct Macro
{
	int parametersCount;
	char name[15];
	char template[10][16];	
	int lineCount;	
} Macro;

Flags CC_ZSPAC = {0, 0, 0, 0, 0};

typedef struct State8085
{
	uint8_t a;
	uint8_t b;
	uint8_t c;
	uint8_t d;
	uint8_t e;
	uint8_t h;
	uint8_t l;
	uint16_t sp;
	uint16_t pc;
	struct Flags cc;
	uint8_t int_enable;
	uint8_t *memory;
} State8085;

intruction_type getMnemonicsType(char * m){
    // UNASSIGNED, DATA_TRANSFER , ARITHMETIC_LOGIC, BRANCH, MACHINE_CONTROL, ERROR
    if( strcmp(m,"MOV")==0 || strcmp(m,"MVI")==0 || strcmp(m,"XCHG")==0 || strcmp(m,"LXI")==0 || 
        strcmp(m,"LDAX")==0 || strcmp(m,"LHLD")==0 || strcmp(m,"LDA")==0 || strcmp(m,"STAX")==0 ||
        strcmp(m,"SHLD")==0 || strcmp(m,"STA")==0
    )
        return DATA_TRANSFER;
    else if( strcmp(m,"ADD")==0 || strcmp(m,"ADC")==0 || strcmp(m,"SUB")==0 || strcmp(m,"SBB")==0 || 
        strcmp(m,"DAD")==0 || strcmp(m,"INR")==0 || strcmp(m,"INX")==0 || strcmp(m,"DCR")==0 ||
        strcmp(m,"DCX")==0 || strcmp(m,"DAA")==0 || strcmp(m,"CMA")==0 || strcmp(m,"STC")==0 ||
        strcmp(m,"CMC")==0 || strcmp(m,"RLC")==0 || strcmp(m,"RRC")==0 || strcmp(m,"RAL")==0 ||
        strcmp(m,"RAR")==0 || strcmp(m,"ANA")==0 || strcmp(m,"XRA")==0 || strcmp(m,"ORA")==0 ||
        strcmp(m,"CMP")==0 || strcmp(m,"ADI")==0 || strcmp(m,"ACI")==0 || strcmp(m,"SUI")==0 ||
        strcmp(m,"SBI")==0 || strcmp(m,"ANI")==0 || strcmp(m,"XRI")==0 || strcmp(m,"ORI")==0 ||
        strcmp(m,"CPI")==0 
    )
        return ARITHMETIC_LOGIC;
    else if(
        strcmp(m,"JMP")==0 || strcmp(m,"JNZ")==0 || strcmp(m,"JZ")==0 || strcmp(m,"JNC")==0 || 
        strcmp(m,"JC")==0 || strcmp(m,"JPO")==0 || strcmp(m,"JPE")==0 || strcmp(m,"JP")==0 ||
        strcmp(m,"JM")==0 || strcmp(m,"PCHL")==0 || strcmp(m,"CALL")==0 || strcmp(m,"CNZ")==0 ||
        strcmp(m,"CNC")==0 || strcmp(m,"CZ")==0 || strcmp(m,"CC")==0 || strcmp(m,"CPO")==0 || 
        strcmp(m,"CPE")==0 || strcmp(m,"CP")==0 || strcmp(m,"CM")==0
    )
        return JUMP;
    else if(    strcmp(m,"RET")==0 || strcmp(m,"RNZ")==0 || strcmp(m,"RZ")==0 || strcmp(m,"RNC")==0 || 
                strcmp(m,"RC")==0  || strcmp(m,"RPO")==0 || strcmp(m,"RPE")==0 || strcmp(m,"RP")==0 ||
                strcmp(m,"RM")==0  || strcmp(m,"RST")==0
    )
        return RETURN;
    else if( strcmp(m,"PUSH")==0 || strcmp(m,"POP")==0 || strcmp(m,"XTML")==0 || strcmp(m,"SPHL")==0 || 
        strcmp(m,"OUT")==0 || strcmp(m,"IN")==0 || strcmp(m,"DI")==0 || strcmp(m,"EI")==0 ||
        strcmp(m,"NOP")==0 || strcmp(m,"HLT")==0 || strcmp(m,"RIM")==0 || strcmp(m,"SIM")==0 
    )
        return MACHINE_CONTROL;
    /*
    else if( strcmp(m,"ORG")==0 || strcmp(m,"END")==0 || strcmp(m,"EQU")==0 || strcmp(m,"SET")==0 || 
        strcmp(m,"DS")==0 || strcmp(m,"DB")==0 || strcmp(m,"DW")==0 || strcmp(m,"MACRO")==0 ||
        strcmp(m,"ENDM")==0 || strcmp(m,"LOCAL")==0 || strcmp(m,"REPT")==0 || strcmp(m,"ZRP")==0 ||
        strcmp(m,"IRPC")==0 || strcmp(m,"EXITM")==0 || strcmp(m,"ASEG")==0 || strcmp(m,"DSEG")==0 || 
        strcmp(m,"CSEG")==0 || strcmp(m,"PUBLIC")==0 || strcmp(m,"EXTRN")==0  || strcmp(m,"NAME")==0 || 
        strcmp(m,"STKLN")==0 || strcmp(m,"STACK")==0 || strcmp(m,"MEMORY")==0 ||strcmp(m,"IF")==0 || 
        strcmp(m,"ELSE")==0 || strcmp(m,"ENDIF")==0
    )
        return ;
    */
    else
        return ERROR;
}

uint8_t getOpcode(char * m, int size){

	FILE* file = fopen(".mnemonics", "r");
	if(file == NULL){
		printf("Supporting file .mnemonics required\n");
		exit(0);
	}
	uint8_t i = 0x00;
	char temp[20];
	int sizeInp;
	while (!feof (file)){  
		fscanf (file, "%s %d" ,temp, &sizeInp);
		if(strcmp(temp,m)==0)
			break;
		i++;
	}
	fclose (file);
	if(strcmp(temp,m)!=0 || sizeInp!=size){
		printf("Syntax Error: %s \n",m);
		exit(0);
	}
	return i;
}

int parity(int x, int size)
{
	int i;
	int p = 0;
	x = (x & ((1 << size) - 1));
	for (i = 0; i < size; i++)
	{
		if (x & 0x1)
			p++;
		x = x >> 1;
	}
	return (0 == (p & 0x1));
}

int UnimplementedErrorDisp(unsigned char *codebuffer, int pc)
{
	unsigned char *code = &codebuffer[pc];
	int opbytes = 1;
	printf("%04x ", pc);
	switch (*code)
	{
	case 0x20:
		printf("RIM");
		break;
	case 0x30:
		printf("SIM");
		break;		
	case 0xdb:
		printf("IN");
		opbytes = 2;
		break;
	case 0xf3:
		printf("DI");
		break;
	case 0xc7:
		printf("RST    0");
		break;
	case 0xcf:
		printf("RST    1");
		break;
	case 0xd7:
		printf("RST    2");
		break;
	case 0xdf:
		printf("RST    3");
		break;
	case 0xe7:
		printf("RST    4");
		break;
	case 0xef:
		printf("RST    5");
		break;
	case 0xf7:
		printf("RST    6");
		break;
	case 0xff:
		printf("RST    7");
		break;
	}

	return opbytes;
}

typedef enum { PRESERVE_CARRY, UPDATE_CARRY } should_preserve_carry;

void LogicFlagsA(State8085 *state, uint8_t ac)
{
	state->cc.cy = 0;
	state->cc.ac = 0;
	state->cc.z = (state->a == 0);
	state->cc.s = (0x80 == (state->a & 0x80));
	state->cc.p = parity(state->a, 8);
}

void ArithFlagsA(State8085 *state, uint16_t res, should_preserve_carry preserveCarry)
{
	if (preserveCarry == UPDATE_CARRY)
		state->cc.cy = (res > 0xff);
	state->cc.z = ((res & 0xff) == 0);
	state->cc.s = (0x80 == (res & 0x80));
	state->cc.p = parity(res & 0xff, 8);
}

void UnimplementedInstruction(State8085 *state)
{
	//pc will have advanced one, so undo that
	if(DEBUG) printf("Error: Unimplemented instruction\n");
	state->pc--;
	UnimplementedErrorDisp(state->memory, state->pc);
	if(DEBUG) printf("\n");
	exit(1);
}

void InvalidInstruction(State8085 *state)
{
	//pc will have advanced one, so undo that
	if(DEBUG) printf("Error: Invalid instruction\n");
	if(DEBUG) printf("PC: %x\n", state->pc);
	if(DEBUG) printf("Memory at PC: %x\n", state->memory[state->pc]);
	state->pc--;
	exit(1);
}

uint8_t addByte(State8085 *state, uint8_t lhs, uint8_t rhs, should_preserve_carry preserveCarry)
{
	uint16_t res = lhs + rhs;
	ArithFlagsA(state, res, preserveCarry);
	if((lhs & 0xf) + (rhs & 0xf) > 0xf)
		state->cc.ac = 1;
	return (uint8_t)res;
}

uint8_t addByteWithCarry(State8085 *state, uint8_t lhs, uint8_t rhs, should_preserve_carry preserveCarry)
{
	uint16_t res = lhs + rhs + (state->cc.cy ? 1 : 0);
	ArithFlagsA(state, res, preserveCarry);
	return (uint8_t)res;
}

uint8_t subtractByte(State8085 *state, uint8_t lhs, uint8_t rhs, should_preserve_carry preserveCarry)
{
	uint16_t res = lhs - rhs;
	ArithFlagsA(state, res, preserveCarry);
	if((lhs & 0xf) + (~rhs & 0xf) + 1 > 0xf)
		state->cc.ac = 1;
	return (uint8_t)res;
}

uint8_t subtractByteWithBorrow(State8085 *state, uint8_t lhs, uint8_t rhs, should_preserve_carry preserveCarry)
{
	uint16_t res = lhs - rhs - (state->cc.cy ? 1 : 0);
	ArithFlagsA(state, res, preserveCarry);
	return (uint8_t)res;
}

void call(State8085 *state, uint16_t offset, uint16_t addr)
{
	uint16_t pc = state->pc + 2;
	state->memory[state->sp - 1] = (pc >> 8) & 0xff;
	state->memory[state->sp - 2] = (pc & 0xff);
	state->sp = state->sp - 2;
	state->pc = offset + addr;
}

void returnToCaller(State8085 *state, uint16_t offset)
{
	state->pc = (state->memory[state->sp] | (state->memory[state->sp + 1] << 8));
	state->sp += 2;
}

int Emulate8085(State8085 *state, uint16_t offset)
{
	int cycles = 4;
	unsigned char *opcode = &state->memory[state->pc];
	if(DEBUG) printf("%04x---%02x\n", state->pc,state->memory[state->pc]);
	if(offset == state->pc)
		state->sp = 0xFFFF;

	state->pc += 1;

	switch (state->memory[state->pc - 1])
	{
	case 0x00:
		break; //NOP
	case 0x01: //LXI	B,word
		state->c = opcode[1];
		state->b = opcode[2];
		state->pc += 2;
		break;
	case 0x02: //STAX B
		state->memory[(state->b << 8) | state->c] = state->a;
		break;
	case 0x03: //INX B
		state->c++;
		if (state->c == 0)
			state->b++;
		break;
	case 0x04: //INR B
		state->b = addByte(state, state->b, 1, PRESERVE_CARRY);
		break;
	case 0x05: //DCR B
		state->b = subtractByte(state, state->b, 1, PRESERVE_CARRY);
		break;
	case 0x06: // MVI B, byte
		state->b = opcode[1];
		state->pc++;
		break;
	case 0x07: //RLC
	{
		uint8_t x = state->a;
		state->a = ((x & 0x80) >> 7) | (x << 1);
		state->cc.cy = (1 == ((x & 0x80) >> 7));
	}
	break;
	case 0x08:
		InvalidInstruction(state);
		break;
	case 0x09: // DAD B
	{
		uint32_t hl = (state->h << 8) | state->l;
		uint32_t bc = (state->b << 8) | state->c;
		uint32_t res = hl + bc;
		state->h = (res & 0xff00) >> 8;
		state->l = res & 0xff;
		state->cc.cy = ((res & 0xffff0000) > 0);
	}
	break;
	case 0x0a: //LDAX B
	{
		uint16_t offset = (state->b << 8) | state->c;
		state->a = state->memory[offset];
	}
	break;
	case 0x0b: //DCX B
		state->c--;
		if (state->c == 0xFF)
			state->b--;
		break;
	case 0x0c: //INR C
	{
		state->c = addByte(state, state->c, 1, PRESERVE_CARRY);
	}
	break;
	case 0x0d: //DCR    C
		state->c = subtractByte(state, state->c, 1, PRESERVE_CARRY);
		break;
	case 0x0e: // MVI C, byte
		state->c = opcode[1];
		state->pc++;
		break;
	case 0x0f: //RRC
	{
		uint8_t x = state->a;
		state->a = ((x & 1) << 7) | (x >> 1);
		state->cc.cy = (1 == (x & 1));
	}
	break;
	case 0x10:
		InvalidInstruction(state);
		break;
	case 0x11: //LXI	D,word
		state->e = opcode[1];
		state->d = opcode[2];
		state->pc += 2;
		break;
	case 0x12:  // STAX D
		state->memory[(state->d << 8) + state->e] = state->a;
		break;
	case 0x13: //INX    D
		state->e++;
		if (state->e == 0)
			state->d++;
		break;
	case 0x14: //INR D
		state->d = addByte(state, state->d, 1, PRESERVE_CARRY);
		break;
	case 0x15: //DCR D
		state->d = subtractByte(state, state->d, 1, PRESERVE_CARRY);
		break;
	case 0x16: // MVI D, byte
		state->d = opcode[1];
		state->pc++;
		break;
	case 0x17: // RAL
	{
		uint8_t x = state->a;
		state->a = state->cc.cy | (x << 1);
		state->cc.cy = (1 == ((x & 0x80) >> 7));
	}
	break;
	case 0x18:
		InvalidInstruction(state);
		break;
	case 0x19: //DAD D
	{
		uint32_t hl = (state->h << 8) | state->l;
		uint32_t de = (state->d << 8) | state->e;
		uint32_t res = hl + de;
		state->h = (res & 0xff00) >> 8;
		state->l = res & 0xff;
		state->cc.cy = ((res & 0xffff0000) != 0);
	}
	break;
	case 0x1a: //LDAX D
	{
		uint16_t offset = (state->d << 8) | state->e;
		state->a = state->memory[offset];
	}
	break;
	case 0x1b: //DCX D
		state->e--;
		if (state->e == 0xFF)
			state->d--;
		break;
	case 0x1c: //INR E
		state->e = addByte(state, state->e, 1, PRESERVE_CARRY);
		break;
	case 0x1d: //DCR E
		state->e = subtractByte(state, state->e, 1, PRESERVE_CARRY);
		break;
	case 0x1e: //MVI E, byte
		state->e = opcode[1];
		state->pc++;
		break;
	case 0x1f: // RAR
	{
		uint8_t x = state->a;
		state->a = (x >> 1) | (state->cc.cy << 7); /* From a number with higest bit as carry value */
		state->cc.cy = (1 == (x & 1));
	}
	break;
	case 0x20:
		UnimplementedInstruction(state);
		break; //RIM
	case 0x21: //LXI	H,word
		state->l = opcode[1];
		state->h = opcode[2];
		state->pc += 2;
		break;
	case 0x22: //SHLD word
	{
		uint16_t offset = (opcode[2] << 8) | opcode[1];
		state->memory[offset] = state->l;
		state->memory[offset + 1] = state->h;
		state->pc += 2;
	}
	break;
	case 0x23: //INX H
		state->l++;
		if (state->l == 0)
			state->h++;
		break;
	case 0x24: //INR H
		state->h = addByte(state, state->h, 1, PRESERVE_CARRY);
		break;
	break;
	case 0x25: //DCR H
		state->h = subtractByte(state, state->h, 1, PRESERVE_CARRY);
		break;
	case 0x26: //MVI H, byte
		state->h = opcode[1];
		state->pc++;
		break;
	case 0x27: // DAA
	{
		uint16_t res = state->a;

		if (state->cc.ac == 1 || (state->a & 0x0f) > 9)
			res = state->a + 6;

		ArithFlagsA(state, res, PRESERVE_CARRY);
		if ((uint8_t)res > 0xf)
			state->cc.ac = 1;
		state->a = (uint8_t)res;

		if (state->cc.cy == 1 || ((state->a >> 4) & 0x0f) > 9)
			res = state->a + 96;

		ArithFlagsA(state, res, UPDATE_CARRY);
		state->a = (uint8_t)res;
	}
	break;
	case 0x28:
		InvalidInstruction(state);
		break;
	case 0x29: // DAD H
	{
		uint32_t hl = (state->h << 8) | state->l;
		uint32_t res = hl + hl;
		state->h = (res & 0xff00) >> 8;
		state->l = res & 0xff;
		state->cc.cy = ((res & 0xffff0000) != 0);
	}
	break;
	case 0x2a: // LHLD Addr
	{
		uint16_t offset = (opcode[2] << 8) | (opcode[1]);
		uint8_t l = state->memory[offset];
		uint8_t h = state->memory[offset + 1];
		uint16_t v = h << 8 | l;
		state->h = v >> 8 & 0xFF;
		state->l = v & 0xFF;
		state->pc += 2;
	}
	break;
	case 0x2b: //DCX H
		state->l--;
		if (state->l == 0xFF)
			state->h--;
		break;
	case 0x2c: //INR L
		state->l = addByte(state, state->l, 1, PRESERVE_CARRY);
		break;
	break;
	case 0x2d: //DCR L
		state->l = subtractByte(state, state->l, 1, PRESERVE_CARRY);
		break;
	case 0x2e: // MVI L,byte
		state->l = opcode[1];
		state->pc++;
		break;
	case 0x2f: // CMA
		state->a ^= 0xFF;
		break;
	case 0x30:
		UnimplementedInstruction(state);
		break; // RIM
	case 0x31: // LXI SP, word
		state->sp = (opcode[2] << 8) | opcode[1];
		state->pc += 2;
		break;
	case 0x32: // STA word
	{
		uint16_t offset = (opcode[2] << 8) | (opcode[1]);
		state->memory[offset] = state->a;
		state->pc += 2;
	}
	break;
	case 0x33: // INX SP
		state->sp++;
		break;
	case 0x34: // INR M
	{
		uint16_t offset = (state->h << 8) | state->l;
		state->memory[offset] = addByte(state, state->memory[offset], 1, PRESERVE_CARRY);
	}
	break;
	case 0x35: // DCR M
	{
		uint16_t offset = (state->h << 8) | state->l;
		state->memory[offset] = subtractByte(state, state->memory[offset], 1, PRESERVE_CARRY);
	}
	break;
	case 0x36: // MVI M, byte
	{
		//AC set if lower nibble of h was zero prior to dec
		uint16_t offset = (state->h << 8) | state->l;
		state->memory[offset] = opcode[1];
		state->pc++;
	}
	break;
	case 0x37:
		state->cc.cy = 1;
		break; // STC
	case 0x38:
		InvalidInstruction(state);
		break;
	case 0x39: // DAD SP
	{
		uint16_t hl = (state->h << 8) | state->l;
		uint16_t sp = state->sp;
		uint32_t res = hl + sp;
		state->h = (res & 0xff00) >> 8;
		state->l = res & 0xff;
		state->cc.cy = ((res & 0xffff0000) > 0);
	}
	break;
		break;
	case 0x3a: // LDA word
	{
		uint16_t offset = (opcode[2] << 8) | (opcode[1]);
		state->a = state->memory[offset];
		state->pc += 2;
	}
	break;
	case 0x3b: // DCX SP
		state->sp--;
		break;
	case 0x3c: // INR A
		state->a = addByte(state, state->a, 1, PRESERVE_CARRY);
		break;
	case 0x3d: // DCR A
		state->a = subtractByte(state, state->a, 1, PRESERVE_CARRY);
		break;
	case 0x3e: // MVI A, byte
		state->a = opcode[1];
		state->pc++;
		break;
	case 0x3f: // CMC
		if (0 == state->cc.cy)
			state->cc.cy = 1;
		else
			state->cc.cy = 0;
		break;
	case 0x40:
		state->b = state->b;
		break; // MOV B, B
	case 0x41:
		state->b = state->c;
		break; // MOV B, C
	case 0x42:
		state->b = state->d;
		break; // MOV B, D
	case 0x43:
		state->b = state->e;
		break; // MOV B, E
	case 0x44:
		state->b = state->h;
		break; // MOV B, H
	case 0x45:
		state->b = state->l;
		break; // MOV B, L
	case 0x46: // MOV B, M
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->b = state->memory[offset];
	}
	break;
	case 0x47:
		state->b = state->a;
		break; // MOV B, A
	case 0x48:
		state->c = state->b;
		break; // MOV C, B
	case 0x49:
		state->c = state->c;
		break; // MOV C, C
	case 0x4a:
		state->c = state->d;
		break; // MOV C, D
	case 0x4b:
		state->c = state->e;
		break; // MOV C, E
	case 0x4c:
		state->c = state->h;
		break; // MOV C, H
	case 0x4d:
		state->c = state->l;
		break; // MOV C, L
	case 0x4e: // MOV C, M
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->c = state->memory[offset];
	}
	break;
	case 0x4f:
		state->c = state->a;
		break; // MOV C, A
	case 0x50:
		state->d = state->b;
		break; // MOV D, B
	case 0x51:
		state->d = state->c;
		break; // MOV D, B
	case 0x52:
		state->d = state->d;
		break; // MOV D, B
	case 0x53:
		state->d = state->e;
		break; // MOV D, B
	case 0x54:
		state->d = state->h;
		break; // MOV D, B
	case 0x55:
		state->d = state->l;
		break; // MOV D, B
	case 0x56: // MOV D, M
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->d = state->memory[offset];
	}
	break;
	case 0x57:
		state->d = state->a;
		break; // MOV D, A
	case 0x58:
		state->e = state->b;
		break; // MOV E, B
	case 0x59:
		state->e = state->c;
		break; // MOV E, C
	case 0x5a:
		state->e = state->d;
		break; // MOV E, D
	case 0x5b:
		state->e = state->e;
		break; // MOV E, E
	case 0x5c:
		state->e = state->h;
		break; // MOV E, H
	case 0x5d:
		state->e = state->l;
		break; // MOV E, L
	case 0x5e: // MOV E, M
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->e = state->memory[offset];
	}
	break;
	case 0x5f:
		state->e = state->a;
		break; // MOV E, A
	case 0x60:
		state->h = state->b;
		break; // MOV H, B
	case 0x61:
		state->h = state->c;
		break; // MOV H, C
	case 0x62:
		state->h = state->d;
		break; // MOV H, D
	case 0x63:
		state->h = state->e;
		break; // MOV H, E
	case 0x64:
		state->h = state->h;
		break; // MOV H, H
	case 0x65:
		state->h = state->l;
		break; // MOV H, L
	case 0x66: // MOV H, M
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->h = state->memory[offset];
	}
	break;
	case 0x67:
		state->h = state->a;
		break; // MOV H, A
	case 0x68:
		state->l = state->b;
		break; // MOV L, B
	case 0x69:
		state->l = state->c;
		break; // MOV L, C
	case 0x6a:
		state->l = state->d;
		break; // MOV L, D
	case 0x6b:
		state->l = state->e;
		break; // MOV L, E
	case 0x6c:
		state->l = state->h;
		break; // MOV L, H
	case 0x6d:
		state->l = state->l;
		break; // MOV L, L
	case 0x6e: // MOV L, M
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->l = state->memory[offset];
	}
	break;
	case 0x6f:
		state->l = state->a;
		break; // MOV L, A
	case 0x70: // MOV M, B
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->memory[offset] = state->b;
	}
	break;
	case 0x71: // MOV M, C
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->memory[offset] = state->c;
	}
	break;
	case 0x72: // MOV M, D
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->memory[offset] = state->d;
	}
	break;
	case 0x73: // MOV M, E
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->memory[offset] = state->e;
	}
	break;
	case 0x74: // MOV M, H
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->memory[offset] = state->h;
	}
	break;
	case 0x75: // MOV M, L
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->memory[offset] = state->l;
	}
	break;
	case 0x76:
		return 1;
		break; // HLT
	case 0x77: // MOV M, A
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->memory[offset] = state->a;
	}
	break;
	case 0x78:
		state->a = state->b;
		break; // MOV A, B
	case 0x79:
		state->a = state->c;
		break; // MOV A, C
	case 0x7a:
		state->a = state->d;
		break; // MOV A, D
	case 0x7b:
		state->a = state->e;
		break; // MOV A, E
	case 0x7c:
		state->a = state->h;
		break; // MOV A, H
	case 0x7d:
		state->a = state->l;
		break; // MOV A, L
	case 0x7e: // MOV A, M
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->a = state->memory[offset];
	}
	break;
	case 0x7f:
		state->a = state->a;
		break; // MOV A, A
	case 0x80: // ADD B
		state->a = addByte(state, state->a, state->b, UPDATE_CARRY);
		break;
	case 0x81: // ADD C
		state->a = addByte(state, state->a, state->c, UPDATE_CARRY);
		break;
	case 0x82: // ADD D
		state->a = addByte(state, state->a, state->d, UPDATE_CARRY);
		break;
	case 0x83: // ADD E
		state->a = addByte(state, state->a, state->e, UPDATE_CARRY);
		break;
	case 0x84: // ADD H
		state->a = addByte(state, state->a, state->h, UPDATE_CARRY);
		break;
	case 0x85: // ADD L
		state->a = addByte(state, state->a, state->l, UPDATE_CARRY);
		break;
	case 0x86: // ADD M
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->a = addByte(state, state->a, state->memory[offset], UPDATE_CARRY);
	}
	break;
	case 0x87: // ADD A
		state->a = addByte(state, state->a, state->a, UPDATE_CARRY);
		break;
	case 0x88: // ADC B
		state->a = addByteWithCarry(state, state->a, state->b, UPDATE_CARRY);
		break;
	case 0x89: // ADC C
		state->a = addByteWithCarry(state, state->a, state->c, UPDATE_CARRY);
		break;
	break;
	case 0x8a: // ADC D
		state->a = addByteWithCarry(state, state->a, state->d, UPDATE_CARRY);
		break;
	case 0x8b: // ADC E
		state->a = addByteWithCarry(state, state->a, state->e, UPDATE_CARRY);
		break;
	case 0x8c: // ADC H
		state->a = addByteWithCarry(state, state->a, state->h, UPDATE_CARRY);
		break;
	case 0x8d: // ADC L
		state->a = addByteWithCarry(state, state->a, state->l, UPDATE_CARRY);
		break;
	case 0x8e: // ADC M
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->a = addByteWithCarry(state, state->a, state->memory[offset], UPDATE_CARRY);
	}
	break;
	case 0x8f: // ADC A
		state->a = addByteWithCarry(state, state->a, state->a, UPDATE_CARRY);
		break;
	case 0x90: // SUB B
		state->a = subtractByte(state, state->a, state->b, UPDATE_CARRY);
		break;
	case 0x91: // SUB C
		state->a = subtractByte(state, state->a, state->c, UPDATE_CARRY);
		break;
	case 0x92: // SUB D
		state->a = subtractByte(state, state->a, state->d, UPDATE_CARRY);
		break;
	case 0x93: // SUB E
		state->a = subtractByte(state, state->a, state->e, UPDATE_CARRY);
		break;
	case 0x94: // SUB H
		state->a = subtractByte(state, state->a, state->h, UPDATE_CARRY);
		break;
	case 0x95: // SUB L
		state->a = subtractByte(state, state->a, state->l, UPDATE_CARRY);
		break;
	case 0x96: // SUB M
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->a = subtractByte(state, state->a, state->memory[offset], UPDATE_CARRY);
	}
	break;
	case 0x97: // SUB A
		state->a = subtractByte(state, state->a, state->a, UPDATE_CARRY);
		break;
	case 0x98: // SBB B
		state->a = subtractByteWithBorrow(state, state->a, state->b, UPDATE_CARRY);
		break;
	case 0x99: // SBB C
		state->a = subtractByteWithBorrow(state, state->a, state->c, UPDATE_CARRY);
		break;
	case 0x9a: // SBB D
		state->a = subtractByteWithBorrow(state, state->a, state->d, UPDATE_CARRY);
		break;
	case 0x9b: // SBB E
		state->a = subtractByteWithBorrow(state, state->a, state->e, UPDATE_CARRY);
		break;
	case 0x9c: // SBB H
		state->a = subtractByteWithBorrow(state, state->a, state->h, UPDATE_CARRY);
		break;
	case 0x9d: // SBB L
		state->a = subtractByteWithBorrow(state, state->a, state->l, UPDATE_CARRY);
		break;
	case 0x9e: // SBB M
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->a = subtractByteWithBorrow(state, state->a, state->memory[offset], UPDATE_CARRY);
	}
	break;
	case 0x9f: // SBB A
		state->a = subtractByteWithBorrow(state, state->a, state->a, UPDATE_CARRY);
		break;
	case 0xa0: // ANA B
		state->a = state->a & state->b;
		LogicFlagsA(state, 1);
		break;
	case 0xa1: // ANA C
		state->a = state->a & state->c;
		LogicFlagsA(state, 1);
		break;
	case 0xa2: // ANA D
		state->a = state->a & state->d;
		LogicFlagsA(state, 1);
		break;
	case 0xa3: // ANA E
		state->a = state->a & state->e;
		LogicFlagsA(state, 1);
		break;
	case 0xa4: // ANA H
		state->a = state->a & state->h;
		LogicFlagsA(state, 1);
		break;
	case 0xa5: // ANA L
		state->a = state->a & state->l;
		LogicFlagsA(state, 1);
		break;
	case 0xa6: // ANA M
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->a = state->a & state->memory[offset];
		LogicFlagsA(state, 1);
	}
	break;
	case 0xa7: // ANA A
		state->a = state->a & state->a;
		LogicFlagsA(state, 1);
		break;
	case 0xa8:
		state->a = state->a ^ state->b;
		LogicFlagsA(state, 0);
		break; // XRA B
	case 0xa9:
		state->a = state->a ^ state->c;
		LogicFlagsA(state, 0);
		break; // XRA C
	case 0xaa:
		state->a = state->a ^ state->d;
		LogicFlagsA(state, 0);
		break; // XRA D
	case 0xab:
		state->a = state->a ^ state->e;
		LogicFlagsA(state, 0);
		break; // XRA E
	case 0xac:
		state->a = state->a ^ state->h;
		LogicFlagsA(state, 0);
		break; // XRA H
	case 0xad:
		state->a = state->a ^ state->l;
		LogicFlagsA(state, 0);
		break; // XRA L
	case 0xae: // XRA M
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->a = state->a ^ state->memory[offset];
		LogicFlagsA(state, 0);
	}
	break;
	case 0xaf:
		state->a = state->a ^ state->a;
		LogicFlagsA(state, 0);
		break; // XRA A
	case 0xb0:
		state->a = state->a | state->b;
		LogicFlagsA(state, 0);
		break; // ORA B
	case 0xb1:
		state->a = state->a | state->c;
		LogicFlagsA(state, 0);
		break; // ORA C
	case 0xb2:
		state->a = state->a | state->d;
		LogicFlagsA(state, 0);
		break; // ORA D
	case 0xb3:
		state->a = state->a | state->e;
		LogicFlagsA(state, 0);
		break; // ORA E
	case 0xb4:
		state->a = state->a | state->h;
		LogicFlagsA(state, 0);
		break; // ORA H
	case 0xb5:
		state->a = state->a | state->l;
		LogicFlagsA(state, 0);
		break; // ORA L
	case 0xb6: // ORA M
	{
		uint16_t offset = (state->h << 8) | (state->l);
		state->a = state->a | state->memory[offset];
		LogicFlagsA(state, 0);
	}
	break;
	case 0xb7:
		state->a = state->a | state->a;
		LogicFlagsA(state, 0);
		break; // ORA A
	case 0xb8: // CMP B
		subtractByte(state, state->a, state->b, UPDATE_CARRY);
		break;
	case 0xb9: // CMP C
		subtractByte(state, state->a, state->c, UPDATE_CARRY);
		break;
	case 0xba: // CMP D
		subtractByte(state, state->a, state->d, UPDATE_CARRY);
		break;
	case 0xbb: // CMP E
		subtractByte(state, state->a, state->e, UPDATE_CARRY);
		break;
	case 0xbc: // CMP H
		subtractByte(state, state->a, state->h, UPDATE_CARRY);
		break;
	case 0xbd: // CMP L
		subtractByte(state, state->a, state->l, UPDATE_CARRY);
		break;
	case 0xbe: // CMP M
	{
		uint16_t offset = (state->h << 8) | (state->l);
		subtractByte(state, state->a, state->memory[offset], UPDATE_CARRY);
	}
	break;
	case 0xbf: // CMP A
		subtractByte(state, state->a, state->a, UPDATE_CARRY);
		break;
	case 0xc0: // RNZ
		if (0 == state->cc.z)
			returnToCaller(state, offset);
		break;
	case 0xc1: // POP B
	{
		state->c = state->memory[state->sp];
		state->b = state->memory[state->sp + 1];
		state->sp += 2;
	}
	break;
	case 0xc2: // JNZ Addr
		if (0 == state->cc.z)
			state->pc = offset + ((opcode[2] << 8) | opcode[1]);
		else
			state->pc += 2;
		break;
	case 0xc3: // JMP Addr
		state->pc = offset + ((opcode[2] << 8) | opcode[1]);
		break;
	case 0xc4: // CNZ Addr
		if (0 == state->cc.z)
		{
			uint16_t pc = state->pc;
			state->memory[state->sp - 1] = (pc >> 8) & 0xff;
			state->memory[state->sp - 2] = (pc & 0xff);
			state->sp = state->sp - 2;
			state->pc = (opcode[2] << 8) | opcode[1];
		}
		else
			state->pc += 2;
		break;
	case 0xc5: // PUSH   B
	{
		state->memory[state->sp - 1] = state->b;
		state->memory[state->sp - 2] = state->c;
		state->sp = state->sp - 2;
	}
	break;
	case 0xc6: // ADI byte
	{
		uint16_t x = (uint16_t)state->a + (uint16_t)opcode[1];
		state->cc.z = ((x & 0xff) == 0);
		state->cc.s = (0x80 == (x & 0x80));
		state->cc.p = parity((x & 0xff), 8);
		state->cc.cy = (x > 0xff);
		state->a = (uint8_t)x;
		state->pc++;
	}
	break;
	case 0xc7: // RST 0
		return 1; //Unimplemented Instruction
		break;
	case 0xc8: // RZ
		if (1 == state->cc.z)
			returnToCaller(state, offset);
		break;
	case 0xc9: // RET
		returnToCaller(state, offset);
		break;
	case 0xca: // JZ Addr
		if (1 == state->cc.z)
			state->pc = offset + ((opcode[2] << 8) | opcode[1]);
		else
			state->pc += 2;
		break;
	case 0xcb:
		InvalidInstruction(state);
		break;
	case 0xcc: // CZ Addr
		if (1 == state->cc.z)
			call(state, offset, (opcode[2] << 8) | opcode[1]);
		else
			state->pc += 2;
		break;
	case 0xcd: // CALL Addr
		call(state, offset, (opcode[2] << 8) | opcode[1]);
		break;
	case 0xce: // ACI d8
		state->a = addByteWithCarry(state, state->a, opcode[1], UPDATE_CARRY);
		state->pc++;
		break;
	case 0xcf: // RST 1
		return 1; //Unimplemented Instruction
		break;
	case 0xd0: // RNC
		if (0 == state->cc.cy)
			returnToCaller(state, offset);
		break;
	case 0xd1: // POP D
	{
		state->e = state->memory[state->sp];
		state->d = state->memory[state->sp + 1];
		state->sp += 2;
	}
	break;
	case 0xd2: // JNC Addr
		if (0 == state->cc.cy)
			state->pc = offset + ((opcode[2] << 8) | opcode[1]);
		else
			state->pc += 2;
		break;
	case 0xd3:
		//Don't know what to do here (yet)
		state->pc++;
		break;
	case 0xd4: // CNC Addr
		if (0 == state->cc.cy)
			call(state, offset, (opcode[2] << 8) | opcode[1]);
		else
			state->pc += 2;
		break;
	case 0xd5: //PUSH   D
	{
		state->memory[state->sp - 1] = state->d;
		state->memory[state->sp - 2] = state->e;
		state->sp = state->sp - 2;
	}
	break;
	case 0xd6: // SUI d8
		state->a = subtractByte(state, state->a, opcode[1], UPDATE_CARRY);
		state->pc++;
		break;
	case 0xd7: // RST 2
		return 1; //Unimplemented Instruction
		break;
	case 0xd8: // RC
		if (1 == state->cc.cy)
			returnToCaller(state, offset);
		break;
	case 0xd9:
		InvalidInstruction(state);
		break;
	case 0xda: // JC Addr
		if (1 == state->cc.cy)
			state->pc = offset + ((opcode[2] << 8) | opcode[1]);
		else
			state->pc += 2;
		break;
	case 0xdb:
		UnimplementedInstruction(state);
		break; // IN d8
	case 0xdc: // CC Addr
		if (1 == state->cc.cy)
			call(state, offset, (opcode[2] << 8) | opcode[1]);
		else
			state->pc += 2;
		break;
	case 0xdd:
		InvalidInstruction(state);
		break;
	case 0xde: // SBI d8
		state->a = subtractByteWithBorrow(state, state->a, opcode[1], UPDATE_CARRY);
		state->pc++;
		break;
	case 0xdf: // RST 3
		return 1; //Unimplemented Instruction
		break;
	case 0xe0: // RPO
		if (0 == state->cc.cy)
			returnToCaller(state, offset);
		break;
	case 0xe1: // POP H
	{
		state->l = state->memory[state->sp];
		state->h = state->memory[state->sp + 1];
		state->sp += 2;
	}
	break;
	case 0xe2: // JPO Addr
		if (0 == state->cc.p)
			state->pc = offset + ((opcode[2] << 8) | opcode[1]);
		else
			state->pc += 2;
		break;
	case 0xe3: // XTHL
	{
		uint16_t spL = state->memory[state->sp];
		uint16_t spH = state->memory[state->sp + 1];
		state->memory[state->sp] = state->l;
		state->memory[state->sp + 1] = state->h;
		state->h = spH;
		state->l = spL;
	}
	break;
	case 0xe4: // CPO Addr
		if (0 == state->cc.p)
			call(state, offset, (opcode[2] << 8) | opcode[1]);
		else
			state->pc += 2;
		break;
	case 0xe5: // PUSH H
	{
		state->memory[state->sp - 1] = state->h;
		state->memory[state->sp - 2] = state->l;
		state->sp = state->sp - 2;
	}
	break;
	case 0xe6: // ANI byte
	{
		state->a = state->a & opcode[1];
		LogicFlagsA(state, 1);
		state->pc++;
	}
	break;
	case 0xe7: // RST 4
		return 1; //Unimplemented Instruction
		break;
	case 0xe8: // RPE
		if (0 == state->cc.cy)
			returnToCaller(state, offset);
		break;
	case 0xe9: // PCHL
		state->pc = (state->h << 8) | state->l;
		break;
	case 0xea: // JPE Addr
		if (1 == state->cc.p)
			state->pc = offset + ((opcode[2] << 8) | opcode[1]);
		else
			state->pc += 2;
		break;
	case 0xeb: // XCHG
	{
		uint8_t save1 = state->d;
		uint8_t save2 = state->e;
		state->d = state->h;
		state->e = state->l;
		state->h = save1;
		state->l = save2;
	}
	break;
	case 0xec: // CPE Addr
		if (1 == state->cc.p)
			call(state, offset, (opcode[2] << 8) | opcode[1]);
		else
			state->pc += 2;
		break;
	case 0xed:
		InvalidInstruction(state);
		break;
	case 0xee: // XRI d8
		state->a = state->a ^ opcode[1];
		LogicFlagsA(state, 0);
		state->pc++;
		break;
	case 0xef: // RST 5
		return 1; //Unimplemented Instruction
		break;
	case 0xf0: // RP
		if (0 == state->cc.s)
			returnToCaller(state, offset);
		break;
	case 0xf1: //POP PSW
	{
		state->a = state->memory[state->sp + 1];
		uint8_t psw = state->memory[state->sp];
		state->cc.z = (0x01 == (psw & 0x01));
		state->cc.s = (0x02 == (psw & 0x02));
		state->cc.p = (0x04 == (psw & 0x04));
		state->cc.cy = (0x05 == (psw & 0x08));
		state->cc.ac = (0x10 == (psw & 0x10));
		state->sp += 2;
	}
	break;
	case 0xf2: // JP Addr
		if (0 == state->cc.s)
			state->pc = offset + ((opcode[2] << 8) | opcode[1]);
		else
			state->pc += 2;
		break;
	case 0xf3: // DI
		UnimplementedInstruction(state);
		break;
	case 0xf4: // CP Addr
		if (0 == state->cc.s)
			call(state, offset, (opcode[2] << 8) | opcode[1]);
		else
			state->pc += 2;
		break;
	case 0xf5: // PUSH PSW
	{
		state->memory[state->sp - 1] = state->a;
		uint8_t psw = (state->cc.z |
					   state->cc.s << 1 |
					   state->cc.p << 2 |
					   state->cc.cy << 3 |
					   state->cc.ac << 4);
		state->memory[state->sp - 2] = psw;
		state->sp = state->sp - 2;
	}
	break;
	case 0xf6: // ORI d8
		state->a = state->a | opcode[1];
		LogicFlagsA(state, 0);
		state->pc++;
		break;
	case 0xf7: // RST 6
		return 1; //Unimplemented Instruction
		break;
	case 0xf8: // RM
		if (1 == state->cc.s)
			returnToCaller(state, offset);
		break;
	case 0xf9: // SPHL
		state->sp = (state->h << 8) | state->l;
		break;
	case 0xfa: // JM Addr
		if (1 == state->cc.s)
			state->pc = offset + ((opcode[2] << 8) | opcode[1]);
		else
			state->pc += 2;
		break;

	case 0xfb:
		state->int_enable = 1;
		break; // EI
	case 0xfc: // CM Addr
		if (1 == state->cc.s)
			call(state, offset, (opcode[2] << 8) | opcode[1]);
		else
			state->pc += 2;
		break;
	case 0xfd:
		InvalidInstruction(state);
		break;
	case 0xfe: // CPI d8
	{
		uint8_t x = state->a - opcode[1];
		state->cc.z = (x == 0);
		state->cc.s = (0x80 == (x & 0x80));
		state->cc.p = parity(x, 8);
		state->cc.cy = (state->a < opcode[1]);
		state->pc++;
	}
	break;
	case 0xff: // RST 7
		return 1; //Unimplemented Instruction
		break;
	}
	return 0;
}

void loadMemory(State8085 * state){
	FILE *fptr;
	fptr = fopen(".memory", "r+");
	if(fptr==NULL)
		return;
	int i = 0;
	uint8_t temp8;
	while(i<RAMSIZE && !feof (fptr)){
		fscanf(fptr,"%hhx",&temp8);
		state->memory[i++]= temp8;
	}
    fclose(fptr);
	return;	
}
void dumpMemory(State8085 * state){
	FILE *fptr;
	fptr = fopen(".memory", "w");
	int i = 0x0;
	while(i<RAMSIZE)
		fprintf(fptr,"%02x ", state->memory[i++]);
    fclose(fptr);
	return;
}
State8085 *Init8085(void)
{
	State8085 *state = (State8085 *)calloc(1, sizeof(State8085));
	state->memory = (uint8_t * )malloc(RAMSIZE); //4096
	loadMemory(state);
	if(DEBUG) printf("State Ptr: %p\n", state);
	return state;
}

State8085 *LoadProgram(State8085 *state, uint8_t *lines, int len, uint16_t offset)
{
	int i = 0;
	while (i < len)
	{
		if(DEBUG) printf("line %d %x\n", i, lines[i]);
		state->memory[offset + i] = lines[i];
		i++;
	}
	if(DEBUG) printf("Offset %x\n", offset);
	if(DEBUG) printf("Memory at offset %x\n", state->memory[offset]);
	return state;
}

void getMemory(State8085 *state, uint16_t i){
	if(i<0 || i > RAMSIZE)
		printf("Memory out of bound\n");
	else
		printf("[%04x] = %02x\n",i,state->memory[i]);
}

void setMemory(State8085 *state, uint16_t i, uint8_t newVal){
	if(i<0 || i > RAMSIZE)
		printf("Memory out of bound\n");
	else
		state->memory[i] = newVal;
}

void updateRegister(State8085 *state, int i){
	uint8_t t8;
	uint16_t t16;
	if(i<7)
		printf("Enter 8 Bit number:\t");
	else
		printf("Enter 16 Bit number:\t");	
	if(i<7)
		scanf("%hhx",&t8);
	else
		scanf("%hx",&t16);
	switch (i)
	{
	case 0:
		state->a = t8;
		break;
	case 1:
		state->b = t8;
		break;
	case 2:
		state->c = t8;
		break;
	case 3:
		state->d = t8;
		break;
	case 4:
		state->e = t8;
		break;
	case 5:
		state->h = t8;
		break;
	case 6:
		state->l = t8;
		break;
	case 7:
		state->sp = t16;
		break;
	case 8:
		state->pc = t16;
		break;
	}
}

void showRegisters(State8085 *state){
	printf("General Purpose Registers\n");
	printf("REGISTER| CONTENT\n");
	printf("--------+--------\n");
	printf("   A\t|   %02x\n",state->a);
	printf("   B\t|   %02x\n",state->b);
	printf("   C\t|   %02x\n",state->c);
	printf("   D\t|   %02x\n",state->d);
	printf("   E\t|   %02x\n",state->e);
	printf("   H\t|   %02x\n",state->h);
	printf("   L\t|   %02x\n",state->l);
	printf("   SP\t|  %04x\n",state->sp);
	printf("   PC\t|  %04x\n\n",state->pc);
}

void updateFlag(State8085 *state, int i){
	uint8_t t8;
	printf("Enter 1 or 0:\t");
	scanf("%hhx",&t8);
	switch (i)
	{
	case 0:
		state->cc.cy = t8;
		break;
	case 1:
		state->cc.p = t8;
		break;
	case 2:
		state->cc.ac = t8;
		break;
	case 3:
		state->cc.z = t8;
		break;
	case 4:
		state->cc.s = t8;
		break;
	}
}

void showFlagRegisters(State8085 *state){
	printf("\nFlag Register\n");
	printf(" S\t| Z\t| AC\t| P\t| CY\t|\n");
	printf(" %d\t| %d\t| %d\t| %d\t| %d\t|\n\n",state->cc.s,state->cc.z,state->cc.ac,state->cc.p,state->cc.cy);
}

State8085 *ExecuteProgram(State8085 *state, uint16_t offset)
{
	int done = 0;
	int cycles = 0;

	if(DEBUG) printf("State Ptr: %p, SP Ptr: %p\n", state, &state->sp);
	if(DEBUG) printf("Offset %x\n", offset);
	state->pc = offset;
	state->sp = 0xFFFF;
	if(DEBUG) printf("Memory at offset %x\n", state->memory[offset]);
	if(DEBUG) printf("Memory at offset + 1 %x\n", state->memory[offset + 1]);

	while (done == 0)
	{
		if (cycles > 10000){
			printf("Error Timeout\n");
			exit(0);			
		}
		done = Emulate8085(state, offset);
		cycles++;
	}
	if(DEBUG) showFlagRegisters(state);
	if(DEBUG) showRegisters(state);
	return state;
}

void stringReplace(char * line,char * find,char * replace){
	int debug = 0;
	char final[256];
	char temp1[125];
	char temp2[125];
	int findLen 	= strlen(find);
	int replaceLen 	= strlen(replace);
	char * sub;
	sub = strstr(line,find);
	if(sub == NULL)
		return;
	if(sub[findLen] == '\n'  || sub[findLen] == ' '){
		while(sub != NULL){
			int sIdx = sub - line;
			int i,j;
			for(i=0; i < sIdx ; i++)
				temp1[i] = line[i];
			temp1[i] = '\0';
			j = 0;
			for(i=sIdx + findLen; i < strlen(line) ; i++)
				temp2[j++] = line[i];
			temp2[j] = '\0';
			strcpy(final,temp1);
			strcat(final,replace);
			strcat(final,temp2);
			strcpy(line,final);
			sub = strstr(line,find);
		}
	}
}

int macroProcessor(char * filename){
	FILE * iFile;
	FILE * oFile;
	Macro macros[100];
	int macroCount = 0;
	
	iFile = fopen(filename,"r");
	oFile = fopen(".tempasm","w");
	if (iFile == NULL){
        printf("Error: Cannot open file \n");
        exit(0);
    }
	char line[256];
	int macroSection = 1;
	while (fgets(line, sizeof(line), iFile)) {
		for(char* c=line; *c=toupper(*c); ++c) ;
		char * sub;
		sub = strstr(line,"MACRO");
		if(sub != NULL){
			if(macroSection == 0){
				printf("Incorrect Macro Declaration\n");
				exit(0);
			}
			int sIdx = sub - line;
			Macro temp;
			char name[6];
			char subLine[256];
			int i;
			for(i = 0; i <sIdx; i++)
				name[i] = line[i];
			name[i-1] = '\0';
			strcpy(temp.name,name);
			i = 0;
			temp.lineCount = 0;
			while(fgets(subLine, sizeof(subLine), iFile)){
				for(char* c=subLine; *c=toupper(*c); ++c) ;
				sub = strstr(subLine,"ENDM");
				if(sub != NULL){
					break;
				}
				strcpy(temp.template[temp.lineCount++],subLine);				
			}
			char paramName[6] = " ";
			int j=1;
			for(i = sIdx + 6; i < strlen(line);i++){
				if(line[i]==' ' || line[i]=='\n'){
					paramName[j] = '\0';
					if(strlen(paramName)>1){
						//process the macro 
						for(j=0;j<temp.lineCount;j++){
							sub = strstr(temp.template[j],paramName);
							if(sub != NULL){
								// Line contains parameter
								char tempStr[10];
								sprintf(tempStr, " %%%d      ", temp.parametersCount); 
								stringReplace(temp.template[j],paramName,tempStr);
							}
						}
						temp.parametersCount++;
					}
					j = 1;
				}
				else{
					paramName[j++] = line[i];
				}
			}
			macros[macroCount++] = temp;
		}
		else{
			macroSection = 0;
		}
		if(macroSection == 0){
			char name[15];
			int i;
			for(i = 0 ; i < 15;i++){
				if(line[i] == ' ')
					break;
				name[i] = line[i];
			}
			name[i] = '\0';
			int macroIdx = -1;
			for(i = 0;i<macroCount;i++){
				if(strcmp(name,macros[i].name)==0){
					macroIdx = i;
					break;
				}
			}
			if(macroIdx!=-1){
				char tempStr[16];
				char paramArr[10][50];
				sub = strstr(line,name);
				int paramCount = 0;
				int j = 0;

				char * p = sub + strlen(name);
				while(* p && isspace(* p)) ++p;
				int length = 0;
				while(*p != ';' && *p != '\n' && *p != '\0'){
					tempStr[length++] = *p;
					p++;
				}
				tempStr[length] = '\0';
				for(j = length - 1; j>-1;j--){
					if(tempStr[j] == ' ')
						tempStr[j] = '\0';
					else
						break;
					length--;
				}
			    char* token = strtok(tempStr, " ");  
				while (token != 0) { 
					strcpy(paramArr[paramCount++],token);
					token = strtok(0, " "); 
				}
				if(macros[macroIdx].parametersCount != paramCount){
					printf("Error: MACRO %s parameter mismatch, required %d, given %d\n",macros[macroIdx].name,macros[macroIdx].parametersCount,paramCount);
					break;
				}
				for(i=0;i<macros[macroIdx].lineCount;i++){					
					if(strlen(macros[macroIdx].template[i])>1){
						strcpy(tempStr,macros[macroIdx].template[i]);
						for(j = 0;j<paramCount;j++){
							char tempParamName[5];
							sprintf(tempParamName, "%%%d", j); 
							stringReplace(tempStr,tempParamName,paramArr[j]);
						}
						fprintf(oFile,"%s",tempStr);
					}
				}
			}
			else
				if(strlen(line)>3)
					fprintf(oFile,"%s",line);
		}
    }
    fclose(iFile);
	fclose(oFile);
}