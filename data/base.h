#pragma once

// Code below is adapted from @modexpblog. Read linked article for more details.
// https://www.mdsec.co.uk/2020/12/bypassing-user-mode-hooks-and-direct-invocation-of-system-calls-for-red-teams

#ifndef SW3_HEADER_H_
#define SW3_HEADER_H_

#include <windows.h>
#include <stdio.h>

#define SW3_SEED <SEED_VALUE>
#define SW3_ROL8(v) (v << 8 | v >> 24)
#define SW3_ROR8(v) (v >> 8 | v << 24)
#define SW3_ROX8(v) ((SW3_SEED % 2) ? SW3_ROL8(v) : SW3_ROR8(v))
#define SW3_MAX_ENTRIES 500
#define SW3_RVA2VA(Type, DllBase, Rva) (Type)((ULONG_PTR) DllBase + Rva)

#define SW3_LAG1 ((UINT16)24)
#define SW3_LAG2 ((UINT16)55)
#define SW3_RAND_SSIZE ((UINT16)1<<6)
#define SW3_RAND_SMASK (SW3_RAND_SSIZE-1)
#define SW3_RAND_EXHAUST_LIMIT SW3_LAG2
// 10x is a heuristic, it just needs to be large enough to remove correlation
#define SW3_RAND_REFILL_COUNT ((SW3_LAG2*10)-SW3_RAND_EXHAUST_LIMIT)

#define NtCurrentProcess() ( (HANDLE)(LONG_PTR) -1 )

// Typedefs are prefixed to avoid pollution.

typedef struct _SW3_RNG_RAND
{
	DWORD64 s[SW3_RAND_SSIZE]; // Lags
	UINT32 i; // Location of the current lag
	UINT32 c; // Exhaustion count
} SW3_RNG_RAND, *PSW3_RNG_RAND;

typedef struct _SW3_SYSCALL_ENTRY
{
    DWORD Hash;
    DWORD Address;
	PVOID SyscallAddress;
} SW3_SYSCALL_ENTRY, *PSW3_SYSCALL_ENTRY;

typedef struct _SW3_SYSCALL_LIST
{
    DWORD Count;
    SW3_SYSCALL_ENTRY Entries[SW3_MAX_ENTRIES];
} SW3_SYSCALL_LIST, *PSW3_SYSCALL_LIST;

typedef struct _SW3_PEB_LDR_DATA {
	BYTE Reserved1[8];
	PVOID Reserved2[3];
	LIST_ENTRY InMemoryOrderModuleList;
} SW3_PEB_LDR_DATA, *PSW3_PEB_LDR_DATA;

typedef struct _SW3_LDR_DATA_TABLE_ENTRY {
	PVOID Reserved1[2];
	LIST_ENTRY InMemoryOrderLinks;
	PVOID Reserved2[2];
	PVOID DllBase;
} SW3_LDR_DATA_TABLE_ENTRY, *PSW3_LDR_DATA_TABLE_ENTRY;

typedef struct _SW3_PEB {
	BYTE Reserved1[2];
	BYTE BeingDebugged;
	BYTE Reserved2[1];
	PVOID Reserved3[2];
	PSW3_PEB_LDR_DATA Ldr;
} SW3_PEB, *PSW3_PEB;

DWORD SW3_HashSyscall(PCSTR FunctionName);
BOOL SW3_PopulateSyscallList();
EXTERN_C DWORD SW3_GetSyscallNumber(DWORD FunctionHash);
EXTERN_C PVOID SW3_GetSyscallAddress(DWORD FunctionHash);
EXTERN_C PVOID internal_cleancall_wow64_gate(VOID);
