#ifndef _GENERIC_X86_ASSEMBLER_H
#define _GENERIC_X86_ASSEMBLER_H

#include "revtracer.h"
#include "river.h"
#include "Runtime.h"

#include "RelocableCodeBuffer.h"

#define ASSEMBLER_DIR_FORWARD		0x00
#define ASSEMBLER_DIR_BACKWARD		0x01

#define ASSEMBLER_CODE_NATIVE		0x00
#define ASSEMBLER_CODE_TRACKING		0x02

class UnusedRegister {
	//virtual nodep::Acquire()
};

class GenericX86Assembler {
protected :
	RiverRuntime *runtime;

public:
	virtual bool Init(RiverRuntime *rt);
	virtual bool Translate(const RiverInstruction &ri, RelocableCodeBuffer &px86, nodep::DWORD &pFlags, nodep::BYTE &currentFamily, nodep::BYTE &repReg, nodep::DWORD &instrCounter, nodep::BYTE outputType) = 0;
};

bool GeneratePrefixes(const RiverInstruction &ri, nodep::BYTE *&px86);
bool ClearPrefixes(const RiverInstruction &ri, nodep::BYTE *&px86);

#endif