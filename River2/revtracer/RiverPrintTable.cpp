#include "river.h"
#include "riverinternl.h"

#include "revtracer.h"

//#include <stdio.h>

extern const char PrintMnemonicTable00[][10];
extern const char PrintMnemonicTable0F[][10];
extern const char PrintMnemonicExt[][8][10];


extern const char RegNames[][4];
extern const char MemSizes[][6];

void PrintPrefixes(nodep::DWORD printMask, struct RiverInstruction *ri) {
	if (ri->family & RIVER_FAMILY_FLAG_IGNORE) {
		revtracerImports.dbgPrintFunc(printMask, "ignore");
	}

	if (ri->family & RIVER_FAMILY_FLAG_ORIG_xSP) {
		revtracerImports.dbgPrintFunc(printMask, "esp");
	}

	switch (RIVER_FAMILY(ri->family)) {
		case RIVER_FAMILY_RIVER:
			revtracerImports.dbgPrintFunc(printMask, "river");
			break;
		case RIVER_FAMILY_TRACK :
			revtracerImports.dbgPrintFunc(printMask, "track");
			break;
		case RIVER_FAMILY_PRETRACK :
			revtracerImports.dbgPrintFunc(printMask, "pretrack");
			break;
		case RIVER_FAMILY_PREMETA :
			revtracerImports.dbgPrintFunc(printMask, "premeta");
			break;
		case RIVER_FAMILY_POSTMETA:
			revtracerImports.dbgPrintFunc(printMask, "postmeta");
			break;
		case RIVER_FAMILY_RIVER_TRACK:
			revtracerImports.dbgPrintFunc(printMask, "rivertrack");
			break;
		case RIVER_FAMILY_REP:
			break;
	}
}

void PrintMnemonic(nodep::DWORD printMask, struct RiverInstruction *ri) {
	const char (*mTable)[10] = PrintMnemonicTable00;

	if (RIVER_MODIFIER_EXT & ri->modifiers) {
		mTable = PrintMnemonicTable0F;
	}

	if (ri->family == RIVER_FAMILY_REP) {
		switch(ri->opCode) {
			case 0xF2:
				revtracerImports.dbgPrintFunc(printMask, "repinit");
				break;
			case 0xF3:
				revtracerImports.dbgPrintFunc(printMask, "repfini");
				break;
			case 0xCC:
				break;
			default:
				DEBUG_BREAK;
		}
	} else {
		if ('a' <= mTable[ri->opCode][0]) {
			revtracerImports.dbgPrintFunc(printMask, "%s ", mTable[ri->opCode]);
		} else {
			revtracerImports.dbgPrintFunc(printMask, "%s ", PrintMnemonicExt[mTable[ri->opCode][0]][ri->subOpCode]);
		}
	}
}

void PrintRegister(nodep::DWORD printMask, struct RiverInstruction *ri, union RiverRegister *reg) {
	revtracerImports.dbgPrintFunc(printMask, "%s$%d", RegNames[reg->name], reg->versioned >> 8);
}

void PrintOperand(nodep::DWORD printMask, struct RiverInstruction *ri, nodep::DWORD idx) {
	bool bWr = false;

	if (RIVER_OPFLAG_IMPLICIT & ri->opTypes[idx]) {
		revtracerImports.dbgPrintFunc(printMask, "{");
	}

	switch (RIVER_OPTYPE(ri->opTypes[idx])) {
		case RIVER_OPTYPE_IMM :
			switch (ri->opTypes[idx] & 0x03) {
				case RIVER_OPSIZE_8 :
					revtracerImports.dbgPrintFunc(printMask, "0x%02x", ri->operands[idx].asImm8);
					break;
				case RIVER_OPSIZE_16:
					revtracerImports.dbgPrintFunc(printMask, "0x%04x", ri->operands[idx].asImm16);
					break;
				case RIVER_OPSIZE_32:
					revtracerImports.dbgPrintFunc(printMask, "0x%08x", ri->operands[idx].asImm32);
					break;
			};
			break;

		case RIVER_OPTYPE_REG :
			revtracerImports.dbgPrintFunc(printMask, "%s$%d", RegNames[ri->operands[idx].asRegister.name], ri->operands[idx].asRegister.versioned >> 8);
			break;

		case RIVER_OPTYPE_MEM :
			if (0 == ri->operands[idx].asAddress->type) {
				revtracerImports.dbgPrintFunc(printMask, "%s$%d", RegNames[ri->operands[idx].asAddress->base.name], ri->operands[idx].asAddress->base.versioned >> 8);
				break;
			}

			revtracerImports.dbgPrintFunc(printMask, "%s ptr ", MemSizes[ri->opTypes[idx] & 0x03]);

			if (ri->operands[idx].asAddress->HasSegment()) {
				revtracerImports.dbgPrintFunc(printMask, "%s:", RegNames[RIVER_REG_SEGMENT | (ri->operands[idx].asAddress->GetSegment() - 1)]);
			}

			revtracerImports.dbgPrintFunc(printMask, "[");
			if (ri->operands[idx].asAddress->type & RIVER_ADDR_BASE) {
				revtracerImports.dbgPrintFunc(printMask, "%s$%d", RegNames[ri->operands[idx].asAddress->base.name], ri->operands[idx].asAddress->base.versioned >> 8);
				bWr = true;
			}

			if (ri->operands[idx].asAddress->type & RIVER_ADDR_INDEX) {
				if (bWr) {
					revtracerImports.dbgPrintFunc(printMask, "+");
				}

				if (ri->operands[idx].asAddress->type & RIVER_ADDR_SCALE) {
					revtracerImports.dbgPrintFunc(printMask, "%d*", ri->operands[idx].asAddress->GetScale());
				}

				revtracerImports.dbgPrintFunc(printMask, "%s$%d", RegNames[ri->operands[idx].asAddress->index.name], ri->operands[idx].asAddress->index.versioned >> 8);
				bWr = true;
			}

			if (ri->operands[idx].asAddress->type & (RIVER_ADDR_DISP8 | RIVER_ADDR_DISP)) {
				if (bWr) {
					revtracerImports.dbgPrintFunc(printMask, "+");
				}

				switch (ri->operands[idx].asAddress->type & (RIVER_ADDR_DISP8 | RIVER_ADDR_DISP)) {
					case RIVER_ADDR_DISP8 :
						revtracerImports.dbgPrintFunc(printMask, "0x%02x", ri->operands[idx].asAddress->disp.d8);
						break;
					/*case RIVER_ADDR_DISP16:
						DbgPrint("0x%04x", ri->operands[idx].asAddress->disp.d16);
						break;*/
					case RIVER_ADDR_DISP:
						revtracerImports.dbgPrintFunc(printMask, "0x%08x", ri->operands[idx].asAddress->disp.d32);
						break;
				}
			}
				 

			revtracerImports.dbgPrintFunc(printMask, "]");
			break;
	}

	if (RIVER_OPFLAG_IMPLICIT & ri->opTypes[idx]) {
		revtracerImports.dbgPrintFunc(printMask, "}");
	}
}

void PrintOperands(nodep::DWORD printMask, struct RiverInstruction *ri) {
	PrintOperand(printMask, ri, 0);

	for (int i = 1; i < 4; ++i) {
		if (ri->opTypes[i] != RIVER_OPTYPE_NONE) {
			revtracerImports.dbgPrintFunc(printMask, ", ");
			PrintOperand(printMask, ri, i);
		}
	}
}

void RiverPrintInstruction(nodep::DWORD printMask, struct RiverInstruction *ri) {
	if (ri->family & RIVER_FAMILY_FLAG_IGNORE) {
		return;
	}
	PrintPrefixes(printMask, ri);
	PrintMnemonic(printMask, ri);
	PrintOperands(printMask, ri);
	revtracerImports.dbgPrintFunc(printMask, "\n");
	//printf("%s", )
}

const char MemSizes[][6] = {
	"dword", "word", "byte"
};

const char RegNames[][4] = {
	"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi",
	 "ax",  "cx",  "dx",  "bx",  "sp",  "bp",  "si",  "di",
	 "al",  "cl",  "dl",  "bl",  "--",  "--",  "--",  "--",
	 "ah",  "ch",  "dh",  "bh",  "--",  "--",  "--",  "--",

	 "es",  "cs",  "ss",  "ds",  "fs",  "gs",  "--",  "--",
	"cr0",  "--", "cr2", "cr3", "cr4",  "--",  "--",  "--",
	"dr0", "dr1", "dr2", "dr3", "dr4", "dr5", "dr6", "dr7"
};

const char PrintMnemonicTable00[][10] = {
	/*0x00*/"add", "add", "add", "add", "add", "add", "", "", "or",  "or",  "or",  "or",  "or",  "or", "", "",
	/*0x10*/"adc", "adc", "adc", "adc", "adc", "adc", "", "", "sbb", "sbb", "sbb", "sbb", "sbb", "sbb", "", "",
	/*0x20*/"and", "and", "and", "and", "and", "and", "", "", "sub", "sub", "sub", "sub", "sub", "sub", "", "",
	/*0x30*/"xor", "xor", "xor", "xor", "xor", "xor", "", "", "cmp", "cmp", "cmp", "cmp", "cmp", "cmp", "", "",
	/*0x40*/"inc", "inc", "inc", "inc", "inc", "inc", "inc", "inc", "dec", "dec", "dec", "dec", "dec", "dec", "dec", "dec",
	/*0x50*/"push", "push", "push", "push", "push", "push", "push", "push", "pop", "pop", "pop", "pop", "pop", "pop", "pop", "pop",
	/*0x60*/"", "", "", "", "", "", "", "", "push", "imul", "push", "imul", "", "", "", "",
	/*0x70*/"jo", "jno", "jc", "jnc", "jz", "jnz", "jbe", "jnbe", "js", "jns", "jp", "jnp", "jl", "jnl", "jle", "jnle",
	/*0x80*/"\1", "\1", "\1", "\1", "test", "test", "xchg", "xchg", "mov", "mov", "mov", "mov", "", "lea", "", "pop",
	/*0x90*/"nop", "xchg", "xchg", "xchg", "xchg", "xchg", "xchg", "xchg", "", "cdq", "", "", "pushf", "popf", "", "",
	/*0xA0*/"mov", "mov", "mov", "mov", "movs", "movs", "cmps", "cmps", "test", "test", "stos", "stos", "", "", "scas", "scas",
	/*0xB0*/"mov", "mov", "mov", "mov", "mov", "mov", "mov", "mov", "mov", "mov", "mov", "mov", "mov", "mov", "mov", "mov",
	/*0xC0*/"\2", "\2", "retn", "retn", "", "", "mov", "mov", "", "leave", "", "", "", "", "", "",
	/*0xD0*/"\2", "\2", "\2", "\2", "", "", "", "", "", "", "", "", "", "", "", "",
	/*0xE0*/"loopnz", "loopz", "loop", "jecx", "", "", "", "", "call", "jmp", "jmpf", "jmp", "", "", "", "",
	/*0xF0*/"", "", "", "", "", "", "\4", "\4", "", "", "", "", "cld", "std", "\6", "\3"
};

const char PrintMnemonicTable0F[][10] = {
	/*0x00*/"", "", "lar", "lsl", "", "syscall", "", "", "", "", "", "", "", "", "", "",
	/*0x10*/"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	/*0x20*/"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	/*0x30*/"", "rdtsc", "", "", "sysenter", "", "", "", "", "", "", "", "", "", "", "",
	/*0x40*/"cmovo", "cmovno", "cmovc", "cmovnc", "cmovz", "cmovnz", "cmovbe", "cmovnbe", "cmovs", "cmovns", "cmovp", "cmovnp", "cmovl", "cmovnl", "cmovle", "cmovnle",
	/*0x50*/"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	/*0x60*/"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	/*0x70*/"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	/*0x80*/"jo", "jno", "jc", "jnc", "jz", "jnz", "jbe", "jnbe", "js", "jns", "jp", "jnp", "jl", "jnl", "jle", "jnle",
	/*0x90*/"seto", "setno", "setc", "setnc", "setz", "setnz", "setbe", "setnbe", "sets", "setns", "setp", "setnp", "setl", "setnl", "setle", "setnle",
	/*0xA0*/"", "", "cpuid", "", "shld", "shld", "", "", "", "", "", "", "shrd", "shrd", "", "imul",
	/*0xB0*/"cmpxchg", "cmpxchg", "", "", "", "", "movzx", "movzx", "", "", "\5", "", "bsf", "bsr", "movsx", "movsx",
	/*0xC0*/"xadd", "xadd", "", "", "", "", "", "\7", "bswap", "bswap", "bswap", "bswap", "bswap", "bswap", "bswap", "bswap",
	/*0xD0*/"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	/*0xE0*/"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	/*0xF0*/"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""
};

const char PrintMnemonicExt[][8][10] = {
	/*for unknown */{ "___", "___", "___", "___", "___", "___", "___", "___" },
	/*for 0x83    */{ "add",  "or", "adc", "sbb", "and", "sub", "xor", "cmp" },
	/*for 0xC0/C1 */{ "rol", "ror", "rcl", "rcr", "shl", "shr", "sal", "sar"},
	/*for 0xFF    */{ "inc", "dec", "call", "call", "jmp", "jmp", "push", "___" },
	/*for 0xF6/F7 */{ "test", "test", "not", "neg", "mul", "imul", "div", "idiv" },
	/*for 0x0FBA  */{ "___", "___", "___", "___", "bt", "bts", "btr", "btc" },
	/*for 0xFE    */{ "inc", "dec", "___", "___", "___", "___", "___", "___" },
	/*for 0x0FC7  */{ "___", "cmpxchg8b", "___", "___", "___", "___", "___", "___" }
};
