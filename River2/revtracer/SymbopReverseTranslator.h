#ifndef _SYMBOP_REVERSE_TRANSLATOR_H_
#define _SYMBOP_REVERSE_TRANSLATOR_H_

#include "revtracer.h"
#include "river.h"

using namespace rev;

class RiverCodeGen;

class SymbopReverseTranslator {
private :
	RiverCodeGen *codegen;

public :
	bool Init(RiverCodeGen *cg);
	bool Translate(const RiverInstruction &rIn, RiverInstruction &rOut);

private :
	void TranslatePushReg(RiverInstruction &rOut, const RiverInstruction &rIn);
	void TranslatePushFlg(RiverInstruction &rOut, const RiverInstruction &rIn);
	void TranslatePushMem(RiverInstruction &rOut, const RiverInstruction &rIn);
};

#endif
