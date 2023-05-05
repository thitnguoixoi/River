#include "ipclib.h"

#include "../revtracer/DebugPrintFlags.h"
#include "common.h"

namespace ipc {
	typedef char *va_list;
#define _ADDRESSOF(v)   ( (unsigned int *)&v )
#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#define va_start(ap,v)  ( ap = (va_list)_ADDRESSOF(v) + _INTSIZEOF(v) )
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)      ( ap = (va_list)0 )


	RingBuffer<(1 << 20)> debugLog;


	//AbstractTokenRing *ipcToken = &__ipcToken;
	DLL_IPC_PUBLIC IpcData ipcData;

	int GeneratePrefix(char *buff, int size, ...) {
		va_list va;

		va_start(va, size);
		int sz = ((_vsnprintf_sFunc)ipcImports.vsnprintf_sFunc)(
			buff, 
			size - 1, 
			"[%3s|%5s|%3s|%c] ",
			va
		);
		va_end(va);

		return sz;
	}

	char tmpBuff[512];

	char pfxBuff[] = "[___|_____|___|_]      ";
	static char lastChar = '\n';

	const char messageTypes[][4] = {
		"___",
		"ERR",
		"INF",
		"DBG"
	};

	const char executionStages[][6] = {
		"_____",
		"BRHND",
		"DIASM",
		"TRANS",
		"REASM",
		"RUNTM",
		"INSPT",
		"CNTNR"
	};

	const char codeTypes[][4] = {
		"___",
		"NAT",
		"RIV",
		"TRK",
		"SYM"
	};

	const char codeDirections[] = {
		'_', 'F', 'B'
	};

	void DebugPrint(DWORD printMask, const char *fmt, ...) {
		va_list va;


		if ('\n' == lastChar) {
			int sz = GeneratePrefix(
				pfxBuff,
				sizeof(pfxBuff),
				messageTypes[(printMask & PRINT_MESSAGE_MASK) >> PRINT_MESSAGE_SHIFT],
				executionStages[(printMask & PRINT_EXECUTION_MASK) >> PRINT_EXECUTION_SHIFT],
				codeTypes[(printMask & PRINT_CODE_TYPE_MASK) >> PRINT_CODE_TYPE_SHIFT],
				codeDirections[(printMask & PRINT_CODE_DIRECTION_MASK) >> PRINT_CODE_DIRECTION_SHIFT]
			);
			debugLog.Write(pfxBuff, sz);
		}

		va_start(va, fmt);
		int sz = ((_vsnprintf_sFunc)ipcImports.vsnprintf_sFunc)(tmpBuff, sizeof(tmpBuff)-1, fmt, va);
		va_end(va);

		if (sz) {
			debugLog.Write(tmpBuff, sz);
			lastChar = tmpBuff[sz - 1];
		}
	}


#define SECTION_MAP_WRITE            0x0002
#define SECTION_MAP_READ             0x0004
#define SECTION_MAP_EXECUTE          0x0020

#define FILE_MAP_WRITE      SECTION_MAP_WRITE
#define FILE_MAP_READ       SECTION_MAP_READ
#define FILE_MAP_EXECUTE	SECTION_MAP_EXECUTE

	void *MemoryAlloc(DWORD dwSize) {
		ipcData.type = REQUEST_MEMORY_ALLOC;
		ipcData.data.asMemoryAllocRequest = dwSize;
		ipcImports.ipcToken->Release(DEBUGGED_PROCESS_TOKENID);

		// remote execution here

		ipcImports.ipcToken->Wait(DEBUGGED_PROCESS_TOKENID);
		if (ipcData.type != REPLY_MEMORY_ALLOC) {
			DEBUG_BREAK;
		}

		void *ptr = (ipcImports.mapMemory)(
			FILE_MAP_READ | FILE_MAP_WRITE | FILE_MAP_EXECUTE,
			ipcData.data.asMemoryAllocReply.offset,
			dwSize,
			ipcData.data.asMemoryAllocReply.pointer
		);

		if (ptr != ipcData.data.asMemoryAllocReply.pointer) {
			DEBUG_BREAK;
			return NULL;
		}

		return ptr;
	}

	void MemoryFree(void *ptr) {
		ipcData.type = REQUEST_MEMORY_FREE;
		ipcData.data.asMemoryFreeRequest = ptr;
		ipcImports.ipcToken->Release(DEBUGGED_PROCESS_TOKENID);
		// remote execution here

		ipcImports.ipcToken->Wait(DEBUGGED_PROCESS_TOKENID);
		if (ipcData.type != REPLY_MEMORY_FREE) {
			DEBUG_BREAK;
		}
	}

	QWORD TakeSnapshot() {
		ipcData.type = REQUEST_TAKE_SNAPSHOT;
		ipcImports.ipcToken->Release(DEBUGGED_PROCESS_TOKENID);
		// remote execution here
		
		ipcImports.ipcToken->Wait(DEBUGGED_PROCESS_TOKENID);
		if (ipcData.type != REPLY_TAKE_SNAPSHOT) {
			DEBUG_BREAK;
		}

		return ipcData.data.asTakeSnapshotReply;
	}

	QWORD RestoreSnapshot() {
		ipcData.type = REQUEST_RESTORE_SNAPSHOT;
		ipcImports.ipcToken->Release(DEBUGGED_PROCESS_TOKENID);
		// remote execution here

		ipcImports.ipcToken->Wait(DEBUGGED_PROCESS_TOKENID);
		if (ipcData.type != REPLY_RESTORE_SNAPSHOT) {
			DEBUG_BREAK;
		}

		return ipcData.data.asRestoreSnapshotReply;
	}

	void InitializeContext(void *context) {
		ipcData.type = REQUEST_INITIALIZE_CONTEXT;
		ipcData.data.asInitializeContextRequest = context;
		ipcImports.ipcToken->Release(DEBUGGED_PROCESS_TOKENID);
		// remote execution here

		ipcImports.ipcToken->Wait(DEBUGGED_PROCESS_TOKENID);
		if (ipcData.type != REPLY_INITIALIZE_CONTEXT) {
			DEBUG_BREAK;
		}
	}

	void CleanupContext(void *context) {
		ipcData.type = REQUEST_CLEANUP_CONTEXT;
		ipcData.data.asCleanupContextRequest = context;
		ipcImports.ipcToken->Release(DEBUGGED_PROCESS_TOKENID);
		// remote execution here

		ipcImports.ipcToken->Wait(DEBUGGED_PROCESS_TOKENID);
		if (ipcData.type != REPLY_CLEANUP_CONTEXT) {
			DEBUG_BREAK;
		}
	}

	DWORD BranchHandler(void *context, void *userContext, ADDR_TYPE nextInstruction) {
		ipcData.type = REQUEST_BRANCH_HANDLER;
		ipcData.data.asBranchHandlerRequest.executionEnv = context;
		ipcData.data.asBranchHandlerRequest.userContext = userContext;
		ipcData.data.asBranchHandlerRequest.nextInstruction = nextInstruction;
		ipcImports.ipcToken->Release(DEBUGGED_PROCESS_TOKENID);

		ipcImports.ipcToken->Wait(DEBUGGED_PROCESS_TOKENID);
		if (ipcData.type != REPLY_BRANCH_HANDLER) {
			DEBUG_BREAK;
		}

		return ipcData.data.asBranchHandlerReply;
	}

	void SyscallControl(void *context, void *userContext) {
		ipcData.type = REQUEST_SYSCALL_CONTROL;
		ipcData.data.asSyscallControlRequest.context = context;
		ipcData.data.asSyscallControlRequest.userContext = userContext;
		ipcImports.ipcToken->Release(DEBUGGED_PROCESS_TOKENID);
		// remote execution here

		ipcImports.ipcToken->Wait(DEBUGGED_PROCESS_TOKENID);
		if (ipcData.type != REPLY_SYSCALL_CONTROL) {
			DEBUG_BREAK;
		}
	}

	void DummyFunc() {
		ipcData.type = REQUEST_DUMMY;
		ipcImports.ipcToken->Release(DEBUGGED_PROCESS_TOKENID);
		// remote execution here

		ipcImports.ipcToken->Wait(DEBUGGED_PROCESS_TOKENID);
		if (ipcData.type != REPLY_DUMMY) {
			DEBUG_BREAK;
		}
	}


	void Initialize() {
		debugLog.Init();
		//TODO handle Windows case accordingly
		//ipcToken.Use(DEBUGGED_PROCESS_TOKENID);
	}

	DLL_IPC_PUBLIC IpcExports ipcExports = {
		DebugPrint,
		MemoryAlloc,
		MemoryFree,

		TakeSnapshot,
		RestoreSnapshot,

		InitializeContext,
		CleanupContext,

		BranchHandler,
		SyscallControl,
		
		Initialize,

		&debugLog,
		&ipcData
	};

	DLL_IPC_PUBLIC IpcImports ipcImports;

#define FALSE 0
#define TRUE 1

#define PF_FLOATING_POINT_PRECISION_ERRATA   0   
#define PF_FLOATING_POINT_EMULATED           1   
#define PF_COMPARE_EXCHANGE_DOUBLE           2   
#define PF_MMX_INSTRUCTIONS_AVAILABLE        3   
#define PF_PPC_MOVEMEM_64BIT_OK              4   
#define PF_ALPHA_BYTE_INSTRUCTIONS           5   
#define PF_XMMI_INSTRUCTIONS_AVAILABLE       6   
#define PF_3DNOW_INSTRUCTIONS_AVAILABLE      7   
#define PF_RDTSC_INSTRUCTION_AVAILABLE       8   
#define PF_PAE_ENABLED                       9   
#define PF_XMMI64_INSTRUCTIONS_AVAILABLE    10   
#define PF_SSE_DAZ_MODE_AVAILABLE           11   
#define PF_NX_ENABLED                       12   
#define PF_SSE3_INSTRUCTIONS_AVAILABLE      13   
#define PF_COMPARE_EXCHANGE128              14   
#define PF_COMPARE64_EXCHANGE128            15   
#define PF_CHANNELS_ENABLED                 16   
#define PF_XSAVE_ENABLED                    17   
#define PF_ARM_VFP_32_REGISTERS_AVAILABLE   18   
#define PF_ARM_NEON_INSTRUCTIONS_AVAILABLE  19   
#define PF_SECOND_LEVEL_ADDRESS_TRANSLATION 20   
#define PF_VIRT_FIRMWARE_ENABLED            21   
#define PF_RDWRFSGSBASE_AVAILABLE           22   
#define PF_FASTFAIL_AVAILABLE               23   
#define PF_ARM_DIVIDE_INSTRUCTION_AVAILABLE 24   
#define PF_ARM_64BIT_LOADSTORE_ATOMIC       25   
#define PF_ARM_EXTERNAL_CACHE_AVAILABLE     26   
#define PF_ARM_FMAC_INSTRUCTIONS_AVAILABLE  27   
#define PF_RDRAND_INSTRUCTION_AVAILABLE     28   

	DLL_IPC_PUBLIC BOOL IsProcessorFeaturePresent(DWORD ProcessorFeature) {
		BOOL result;

		if (ProcessorFeature >= 0x40) {
			result = FALSE;
		}
		else {
			switch (ProcessorFeature) {
			case PF_MMX_INSTRUCTIONS_AVAILABLE:
			case PF_SSE3_INSTRUCTIONS_AVAILABLE:
			case PF_XMMI64_INSTRUCTIONS_AVAILABLE:
			case PF_XMMI_INSTRUCTIONS_AVAILABLE:
			case PF_COMPARE_EXCHANGE_DOUBLE:
			case PF_COMPARE_EXCHANGE128:
			case PF_COMPARE64_EXCHANGE128:
				return FALSE;
			default:
				result = ((BYTE *)0x7FFE0274)[ProcessorFeature];
			}
		}
		return result;
	}
};

unsigned int Entry() {
	return 1;
}
