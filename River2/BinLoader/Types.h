#ifndef _TYPES_H_
#define _TYPES_H_

namespace ldr {
	typedef unsigned short USHORT;
	typedef long LONG;

	typedef unsigned char BYTE;
	typedef unsigned short WORD;
	typedef unsigned long DWORD;
	typedef unsigned long long QWORD;

	// TODO: Make arch aware size_t
	typedef unsigned long SIZE_T;
}; // namespace ldr

#endif
