#ifndef _LOADER_H
#define _LOADER_H

#include "../BinLoader/LoaderAPI.h"
#include "../CommonCrossPlatform/LibraryLayout.h"

namespace ldr {
#if defined _WIN32 || defined __CYGWIN__
	#ifdef _BUILDING_LOADER_DLL
		#ifdef __GNUC__
			#define DLL_LOADER_PUBLIC __attribute__ ((dllexport))
		#else
			#define DLL_LOADER_PUBLIC __declspec(dllexport)
		#endif
	#else
		#ifdef __GNUC__
			#define DLL_LOADER_PUBLIC __attribute__ ((dllimport))
		#else
			#define DLL_LOADER_PUBLIC __declspec(dllimport)
		#endif
	#endif
	#define DLL_LOADER_LOCAL
#else
	#if __GNUC__ >= 4
		#define DLL_LOADER_PUBLIC __attribute__ ((visibility ("default")))
		#define DLL_LOADER_LOCAL  __attribute__ ((visibility ("hidden")))
	#else
		#define DLL_LOADER_PUBLIC
		#define DLL_LOADER_LOCAL
	#endif
#endif

/* Define NULL pointer value */
#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else  /* __cplusplus */
#define NULL    ((void *)0)
#endif  /* __cplusplus */
#endif  /* NULL */

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86) || defined(_ARM_) || defined(_M_ARM)) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif
#endif

#if defined(_WIN64)
	typedef __int64 INT_PTR, *PINT_PTR;
	typedef unsigned __int64 UINT_PTR, *PUINT_PTR;

	typedef __int64 LONG_PTR, *PLONG_PTR;
	typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;

#define __int3264   __int64

#else
	typedef _W64 int INT_PTR, *PINT_PTR;
	typedef _W64 unsigned int UINT_PTR, *PUINT_PTR;

	typedef _W64 long LONG_PTR, *PLONG_PTR;
	typedef _W64 unsigned long ULONG_PTR, *PULONG_PTR;

#define __int3264   __int32

#endif

	typedef unsigned long long QWORD;
	typedef unsigned long DWORD;
	typedef unsigned short WORD;
	typedef unsigned char BYTE;
	//typedef unsigned int SIZE_T;

	typedef void *ADDR_TYPE;

#if defined _WIN32 || defined __CYGWIN__
	typedef void *SHM_HANDLE;
#else
	typedef unsigned long SHM_HANDLE;
#endif

#define MAX_LIBS 0x10
#define MAX_SEGMENTS 0x100

	struct PESections {
		ADDR_TYPE mappingAddress;
		DWORD     mappingSize;
		DWORD     sectionOffset;
		DWORD     desiredAccess;
	};

	struct LoaderConfig {
		SHM_HANDLE sharedMemory;
		ADDR_TYPE shmBase;
		ADDR_TYPE entryPoint;

		union {
			struct {
				PESections sections[32];
				DWORD sectionCount;
			} windows;

			struct {
				ADDR_TYPE retAddr;
				MappedObject mos[MAX_LIBS];
				DWORD segments[MAX_SEGMENTS];
			} linux;
		} osData;
	};

#define MAX_LIBS 0x10
#define MAX_SEGMENTS 0x100

	struct LoaderImports {
		ext::LibraryLayout *libraries;

		union {
			struct {
				struct {
					unsigned int dummy;
				} libc;
			} linFunc;

			struct {
				struct {
					unsigned int _ntMapViewOfSection;
					unsigned int _ntFlushInstructionCache;
					unsigned int _ntFreeVirtualMemory;
				} ntdll;
			} winFunc;
		} functions;
		

		//unsigned long sharedMemoryAddress;
		//MappedObject mos[MAX_LIBS];
		//DWORD segments[MAX_SEGMENTS];
	};

	typedef void *(*MapMemoryFunc)(unsigned long access, unsigned long offset, unsigned long size, void *address);
	typedef void (*PerformFunc)();

	struct LoaderExports {
		MapMemoryFunc mapMemory;
		PerformFunc perform;
	};

	extern "C" {
		DLL_LOADER_PUBLIC extern LoaderConfig loaderConfig;
		DLL_LOADER_PUBLIC extern LoaderImports loaderImports;
		DLL_LOADER_PUBLIC extern LoaderExports loaderExports;
	};

};


#endif
