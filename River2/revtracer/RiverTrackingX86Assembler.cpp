#include "RiverTrackingX86Assembler.h"

using namespace rev;

extern nodep::DWORD dwAddressTrackHandler;
extern nodep::DWORD dwAddressMarkHandler;

void RiverTrackingX86Assembler::AssemblePushFlg(nodep::DWORD testFlags, RelocableCodeBuffer &px86, nodep::DWORD &pFlags, nodep::DWORD &instrCounter) {
	const nodep::BYTE markFlagInstr[] = { 0xFF, 0x35, 0x00, 0x00, 0x00, 0x00 };

	for (nodep::BYTE c = 0, m = 1; m < RIVER_SPEC_FLAG_EXT; ++c, m <<= 1) {
		if (m & testFlags) {
			rev_memcpy(px86.cursor, markFlagInstr, sizeof(markFlagInstr));
			*(nodep::DWORD *)(&px86.cursor[0x02]) = (nodep::DWORD)&runtime->taintedFlags[c];
			px86.cursor += sizeof(markFlagInstr);
			instrCounter++;
		}
	}
}

void RiverTrackingX86Assembler::AssemblePushReg(const RiverRegister &reg, RelocableCodeBuffer &px86, nodep::DWORD &pFlags, nodep::DWORD &instrCounter) {
	const nodep::BYTE markRegInstr[] = { 0xFF, 0x35, 0x00, 0x00, 0x00, 0x00 };

	rev_memcpy(px86.cursor, markRegInstr, sizeof(markRegInstr));
	*(nodep::DWORD *)(&px86.cursor[0x02]) = (nodep::DWORD)&runtime->taintedRegisters[GetFundamentalRegister(reg.name)];
	px86.cursor += sizeof(markRegInstr);
	instrCounter++;
}

void RiverTrackingX86Assembler::AssemblePushMem(const RiverAddress *addr, nodep::BYTE offset, RelocableCodeBuffer &px86, nodep::DWORD &pFlags, nodep::DWORD &instrCounter) {
	const nodep::BYTE trackMemInstr[] = {
		0xFF, 0x76, 0x00,								// 0x00 - push [esi + 0x00] - effective address
		0x50											// 0x03
	};

	const nodep::BYTE trackSegInstr[] = {
		0xFF, 0x76, 0x00								// 0x00 - push [esi + 0x00] - segment index
	};

	const nodep::BYTE trackImmInstr[] = {
		0x6A, 0x00										// 0x00 - push 0 - no segment
	};

	if (addr->HasSegment()) {
		rev_memcpy(px86.cursor, trackSegInstr, sizeof(trackSegInstr));
		px86.cursor[0x02] = ~((offset - 1) << 2) + 1;
		px86.cursor += sizeof(trackSegInstr);
	}
	else {
		rev_memcpy(px86.cursor, trackImmInstr, sizeof(trackImmInstr));
		px86.cursor[0x02] = ~((offset - 1) << 2) + 1;
		px86.cursor += sizeof(trackImmInstr);
	}

	rev_memcpy(px86.cursor, trackMemInstr, sizeof(trackMemInstr));
	px86.cursor[0x02] = ~(offset << 2) + 1;
	px86.cursor += sizeof(trackMemInstr);
	instrCounter += 3;
}

/*void RiverTrackingX86Assembler::AssembleRiverPostMarkMemory(RelocableCodeBuffer &px86, DWORD &pFlags, DWORD &instrCounter) {
	const BYTE trackEax[] = { 0x50 };

	memcpy(px86.cursor, trackEax, sizeof(trackEax));
	px86.cursor += sizeof(trackEax);

	instrCounter += 1;
}*/

void RiverTrackingX86Assembler::AssemblePopFlg(nodep::DWORD testFlags, RelocableCodeBuffer &px86, nodep::DWORD &pFlags, nodep::DWORD &instrCounter) {
	const nodep::BYTE unmarkFlagInstr[] = { 0x8F, 0x05, 0x00, 0x00, 0x00, 0x00 };

	for (nodep::BYTE c = 7, m = RIVER_SPEC_FLAG_EXT; m > 0; --c, m >>= 1) {
		if (m & testFlags) {
			rev_memcpy(px86.cursor, unmarkFlagInstr, sizeof(unmarkFlagInstr));
			*(nodep::DWORD *)(&px86.cursor[0x02]) = (nodep::DWORD)&runtime->taintedFlags[c];
			px86.cursor += sizeof(unmarkFlagInstr);
			instrCounter++;
		}
	}
}

void RiverTrackingX86Assembler::AssemblePopReg(const RiverRegister &reg, RelocableCodeBuffer &px86, nodep::DWORD &pFlags, nodep::DWORD &instrCounter) {
	const nodep::BYTE unmarkRegInstr[] = { 0x8F, 0x05, 0x00, 0x00, 0x00, 0x00 };

	rev_memcpy(px86.cursor, unmarkRegInstr, sizeof(unmarkRegInstr));
	*(nodep::DWORD *)(&px86.cursor[0x02]) = (nodep::DWORD)&runtime->taintedRegisters[GetFundamentalRegister(reg.name)];
	px86.cursor += sizeof(unmarkRegInstr);
	instrCounter++;
}

void RiverTrackingX86Assembler::AssemblePopMem(const RiverAddress *addr, nodep::BYTE offset, RelocableCodeBuffer &px86, nodep::DWORD &pFlags, nodep::DWORD &instrCounter)  {
	const nodep::BYTE unmarkRegMem[] = {
		0x58,											// 0x06 - pop eax
		0x5F,											// 0x07 - pop edi
		0x5E											// 0x08 - pop esi
	};

	rev_memcpy(px86.cursor, unmarkRegMem, sizeof(unmarkRegMem));
	px86.cursor += sizeof(unmarkRegMem);
	instrCounter += 3;
}

bool RiverTrackingX86Assembler::Translate(const RiverInstruction &ri, RelocableCodeBuffer &px86, nodep::DWORD &pFlags, nodep::BYTE &currentFamily, nodep::BYTE &repReg, nodep::DWORD &instrCounter, nodep::BYTE outputType) {
	switch (ri.opCode) {
		case 0x9C : // push flags
			AssemblePushFlg(ri.operands[0].asImm8, px86, pFlags, instrCounter);
			break;

		case 0x9D : // pop flags
			AssemblePopFlg(ri.operands[0].asImm8, px86, pFlags, instrCounter);
			break;

		case 0x50 : // push reg
			AssemblePushReg(ri.operands[0].asRegister, px86, pFlags, instrCounter);
			break;

		case 0x58 : // pop reg
			AssemblePopReg(ri.operands[0].asRegister, px86, pFlags, instrCounter);
			break;

		case 0xFF : // push mem
			if (6 == ri.subOpCode) {
				AssemblePushMem(ri.operands[0].asAddress, ri.operands[1].asImm8, px86, pFlags, instrCounter);
			} else {
				DEBUG_BREAK;
			}
			break;

		case 0x8F :
			AssemblePopMem(ri.operands[0].asAddress, ri.operands[1].asImm8, px86, pFlags, instrCounter);
			break;

		/*case 0xE8 : // call
			AssembleUnmark(px86, pFlags, instrCounter);
			break;*/

		default :
			DEBUG_BREAK;

	}

	return true;
}
