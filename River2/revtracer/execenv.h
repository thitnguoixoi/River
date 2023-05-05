#ifndef _EXEC_ENV_H
#define _EXEC_ENV_H

#include "environment.h"
#include "sync.h"
#include "river.h"
#include "mm.h"
#include "cb.h"
#include "CodeGen.h"
#include "Runtime.h"
#include "AddressContainer.h"

struct ExecutionEnvironment {
	RiverRuntime runtimeContext;

	nodep::UINT_PTR saveLog;
	
	unsigned int /*heapSize,*/ historySize /*, logHashSize*/, outBufferSize;

	unsigned char *pStack; // = NULL;

	RiverHeap heap;

	//_tbm_mutex cbLock; //  = 0;
	//struct _cb_info **hashTable; // = 0
	RiverBasicBlockCache blockCache;

	nodep::UINT_PTR lastFwBlock;
	//UINT_PTR *history;
	//unsigned long posHist, totHist; // = 0;

	nodep::UINT_PTR *executionBuffer, executionBase;

	//unsigned char *saveBuffer;

	unsigned int bForward;

	RiverCodeGen codeGen;

	nodep::DWORD exitAddr;

	bool bValid;
	void *userContext;

	AddressContainer ac;

	nodep::DWORD generationFlags;
public :
	void* operator new(size_t);
	void operator delete(void*);

	ExecutionEnvironment(nodep::DWORD flags, unsigned int heapSize, unsigned int historySize, unsigned int executionSize, unsigned int trackSize, unsigned int logHashSize, unsigned int outBufferSize);
	~ExecutionEnvironment();
};

void *AllocUserContext(struct ExecutionEnvironment *pEnv, unsigned int size);
void DeleteUserContext(struct ExecutionEnvironment *pEnv);

#endif
