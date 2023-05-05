#ifdef _WIN32
#include <Windows.h>
#elif defined(__linux__)
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <dlfcn.h>
#endif

#include "Shm.Mapper.h"

// WARNING: DO NOT USE ON WINDOWS, YET!

namespace ldr {
#ifdef _WIN32

	// virtual memory functions
	static const DWORD PageProtections[8] = {
		PAGE_NOACCESS,				// 0 ---
		PAGE_EXECUTE,				// 1 --X
		PAGE_READWRITE,				// 2 -W- (specified as RW)
		PAGE_EXECUTE_READWRITE,		// 3 -WX (specified as RWX)
		PAGE_READONLY,				// 4 R--
		PAGE_EXECUTE_READ,			// 5 R-X
		PAGE_READWRITE,				// 6 RW-
		PAGE_EXECUTE_READWRITE		// 7 RWX
	};
#define VIRTUAL_ALLOC(addr, size, protect, fd, offset) VirtualAlloc((addr), (size), MEM_RESERVE | MEM_COMMIT, (protect))
#define VIRTUAL_PROTECT(addr, size, newProtect, oldProtect) (TRUE == VirtualProtect((addr), (size), (newProtect), &(oldProtect)))

#elif defined(__linux__)
	static const DWORD PageProtections[8] = {
		PROT_NONE,							// 0 ---
		PROT_EXEC,							// 1 --X
		PROT_WRITE,							// 2 -W-
		PROT_EXEC | PROT_WRITE,				// 3 -WX
		PROT_READ,							// 4 R--
		PROT_EXEC | PROT_READ,				// 5 R-X
		PROT_WRITE | PROT_READ,				// 6 RW-
		PROT_EXEC | PROT_WRITE | PROT_READ,	// 7 RWX
	};
	// virtual memory functions
#define VIRTUAL_ALLOC(addr, size, protect, fd, offset) ({ int flags = ((fd) < 0) ? MAP_SHARED | MAP_ANONYMOUS : MAP_SHARED; \
		addr = mmap(addr, (size), (protect), flags, (fd), (offset)); addr; })
#define VIRTUAL_PROTECT(addr, size, newProtect, oldProtect) (0 == mprotect((addr), (size), (newProtect)))

#endif

	ShmMapper::ShmMapper(int shmFd, SIZE_T offset) {
		this->shmFd = shmFd;
		this->offset = offset;
	}

	void *ShmMapper::CreateSection(void *lpAddress, SIZE_T dwSize, DWORD flProtect) {
		return VIRTUAL_ALLOC(lpAddress, dwSize, PageProtections[flProtect], shmFd, offset);
	}

	bool ShmMapper::ChangeProtect(void *lpAddress, SIZE_T dwSize, DWORD flProtect) {
		DWORD oldProtect;
		return VIRTUAL_PROTECT(lpAddress, dwSize, PageProtections[flProtect], oldProtect);
	}

	bool ShmMapper::WriteBytes(void *lpAddress, void *lpBuffer, SIZE_T nSize) {
		memcpy(lpAddress, lpBuffer, nSize);
		return true;
	}
};
