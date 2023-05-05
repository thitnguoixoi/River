#ifndef _CALL_GATES_H
#define _CALL_GATES_H

#include "environment.h"
#include "revtracer.h"

typedef nodep::DWORD (_cdecl *_fn_cdecl_0) (void);
typedef nodep::DWORD (_cdecl *_fn_cdecl_1) (void *);
typedef nodep::DWORD (_cdecl *_fn_cdecl_2) (void *, void *);
typedef nodep::DWORD (_cdecl *_fn_cdecl_3) (void *, void *, void *);
typedef nodep::DWORD (_cdecl *_fn_cdecl_4) (void *, void *, void *, void *);

typedef nodep::DWORD (_stdcall *_fn_stdcall_0) (void);
typedef nodep::DWORD (_stdcall *_fn_stdcall_1) (void *);
typedef nodep::DWORD (_stdcall *_fn_stdcall_2) (void *, void *);
typedef nodep::DWORD (_stdcall *_fn_stdcall_3) (void *, void *, void *);
typedef nodep::DWORD (_stdcall *_fn_stdcall_4) (void *, void *, void *, void *);

nodep::DWORD __declspec(noinline) call_cdecl_0(struct ExecutionEnvironment *env, _fn_cdecl_0 f);
nodep::DWORD __declspec(noinline) call_cdecl_1(struct ExecutionEnvironment *env, _fn_cdecl_1 f, void *);
nodep::DWORD __declspec(noinline) call_cdecl_2(struct ExecutionEnvironment *env, _fn_cdecl_2 f, void *, void *);
nodep::DWORD __declspec(noinline) call_cdecl_3(struct ExecutionEnvironment *env, _fn_cdecl_3 f, void *, void *, void *);
nodep::DWORD __declspec(noinline) call_cdecl_4(struct ExecutionEnvironment *env, _fn_cdecl_4 f, void *, void *, void *, void *);

nodep::DWORD __declspec(noinline) call_stdcall_0(struct ExecutionEnvironment *env, _fn_stdcall_0 f);
nodep::DWORD __declspec(noinline) call_stdcall_1(struct ExecutionEnvironment *env, _fn_stdcall_1 f, void *);
nodep::DWORD __declspec(noinline) call_stdcall_2(struct ExecutionEnvironment *env, _fn_stdcall_2 f, void *, void *);
nodep::DWORD __declspec(noinline) call_stdcall_3(struct ExecutionEnvironment *env, _fn_stdcall_3 f, void *, void *, void *);
nodep::DWORD __declspec(noinline) call_stdcall_4(struct ExecutionEnvironment *env, _fn_stdcall_4 f, void *, void *, void *, void *);



#endif
