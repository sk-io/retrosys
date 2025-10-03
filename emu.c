#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define LOBYTE(x) ((x) & 0xFF)
#define HIBYTE(x) ((x) >> 8 & 0xFF)

#define REG_A(x) ((x) >> 4 & 0xF)
#define REG_B(x) ((x) & 0xF)

/*
register operations opcode format: 1iii iioo
i: 32 reg/mem instructions
oo: 4 operand formats
	0: unused
	1: regs byte
	2: regs byte + byte
	3: regs byte + word
*/

#define REGOP_FORMAT_UNUSED    0
#define REGOP_FORMAT_REGS      1
#define REGOP_FORMAT_REGS_BYTE 2
#define REGOP_FORMAT_REGS_WORD 3

/* unique instructions */
#define OPC_HALT    0x00
#define OPC_NOP     0x01
#define OPC_J       0x02
#define OPC_JT      0x03
#define OPC_JF      0x04
#define OPC_INC     0x05
#define OPC_DEC     0x06
#define OPC_LOAD    0x07
#define OPC_LOADB   0x08
#define OPC_WRITE   0x09
#define OPC_WRITEB  0x0a
#define OPC_PUSH    0x0b
#define OPC_POP     0x0c

/* register operations */
#define REGOP_SET     0
#define REGOP_ADD     1
#define REGOP_ADDC    2
#define REGOP_SUB     3
#define REGOP_SUBC    4
#define REGOP_AND     5
#define REGOP_OR      6
#define REGOP_XOR     7
#define REGOP_CMP_E   8
#define REGOP_CMP_NE  9
#define REGOP_CMP_L   10
#define REGOP_CMP_LE  11
#define REGOP_CMP_G   12
#define REGOP_CMP_GE  13

#define CPU_HALTED (1 << 0)
#define CPU_COND   (1 << 1)

typedef struct {
	u16 regs[16];
	u16 pc;
	u16 flags;
} cpu_t;

cpu_t cpu = {0};

u8 mem[0x10000];

u8 readb(u16 addr) {
	return mem[addr];
}

void writeb(u16 addr, u8 byte) {
	mem[addr] = byte;
}

u16 read(u16 addr) {
	return readb(addr) | readb(addr + 1) << 8;
}

void write(u16 addr, u16 word) {
	writeb(addr, word & 0xFF);
	writeb(addr + 1, word >> 8);
}

void reset() {
	memset(&cpu, 0, sizeof(cpu_t));
	memset(mem, 0, 0x10000);
}

void error() {
	exit(1);
}

void dump_cpu() {
	printf("pc=%04x r0=%04x r1=%04x\n", cpu.pc, cpu.regs[0], cpu.regs[1]);
}

void instr() {
	u8 opc;
	u8 regs;
	u16 imm;
	u8 format;

    dump_cpu();
	opc = readb(cpu.pc++);
	printf("instr opc=%02x\n", opc);

	/* register operation? */
	if (opc & 128) {
		format = opc & 0b11;
		switch (format) {
		case REGOP_FORMAT_UNUSED:
			error();
			break;
		case REGOP_FORMAT_REGS:
			regs = readb(cpu.pc++);
			imm = cpu.regs[REG_B(regs)];
			break;
		case REGOP_FORMAT_REGS_BYTE:
			regs = readb(cpu.pc++);
			imm = readb(cpu.pc++);
			break;
		case REGOP_FORMAT_REGS_WORD:
			regs = readb(cpu.pc++);
			imm = read(cpu.pc);
			cpu.pc += 2;
			break;
		}
		
		switch ((opc & 127) >> 2) {
		case REGOP_SET:
			cpu.regs[REG_A(regs)] = imm;
			break;
		case REGOP_ADD:
			cpu.regs[REG_A(regs)] += imm;
			break;
		case REGOP_CMP_E:
			printf("COMPARE %u and %u\n", cpu.regs[REG_A(regs)], imm);
			cpu.flags &= ~CPU_COND;
			if (cpu.regs[REG_A(regs)] == imm) {
				cpu.flags |= CPU_COND;
			}
			printf("cpu flags: %u\n", cpu.flags);
			break;
		}

		return;
	}

	switch (opc) {
	case OPC_HALT:
		cpu.flags |= CPU_HALTED;
		printf("halted!\n");
		exit(0);
		break;
	case OPC_NOP:
		break;
	case OPC_J:
		imm = read(cpu.pc);
		cpu.pc = imm;
		break;
	case OPC_JT:
	case OPC_JF:
		imm = read(cpu.pc);
		cpu.pc += 2;
		if (((cpu.flags & CPU_COND) != 0) ^ (opc == OPC_JF))
			cpu.pc = imm;
		break;
	default:
		printf("error: unknown opcode %02x\n", opc);
		error();
	}
}

void dump_mem(u16 addr, u16 num) {
	int i;
	const int per_line = 8;

	for (i = 0; i < num; i++) {
		if (i % 8 == 0)
			printf("%04x: ", addr + i);

		printf("%02x ", mem[addr + i]);

		if (i % 8 == 7)
			printf("\n");
	}

	printf("\n");
}

#define MAKE_REGOP(b, f) (128 | ((b) << 2) | (f))

int main(int argc, char** argv) {
	reset();

	writeb(0, MAKE_REGOP(REGOP_SET, REGOP_FORMAT_REGS_BYTE));
	writeb(1, 0x00);
	writeb(2, 0);

	writeb(4, MAKE_REGOP(REGOP_SET, REGOP_FORMAT_REGS_BYTE));
	writeb(5, 0x10);
	writeb(6, 1);

	writeb(8, MAKE_REGOP(REGOP_ADD, REGOP_FORMAT_REGS));
	writeb(9, 0x01);

	writeb(10, MAKE_REGOP(REGOP_CMP_E, REGOP_FORMAT_REGS_WORD));
	writeb(11, 0x00);
	write(12, 10);

	writeb(14, OPC_JF);
	write(15, 8);

	write(17, OPC_HALT);

	dump_mem(0, 32);

	while (~cpu.flags & CPU_HALTED) {
		instr();
	}
	return 0;
}
