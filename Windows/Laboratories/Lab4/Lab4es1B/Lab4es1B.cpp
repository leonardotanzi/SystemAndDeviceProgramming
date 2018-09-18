// Lab4es1B.cpp: definisce il punto di ingresso dell'applicazione console.
//

#define _CRT_SECURE_NO_WARNINGS
#define UNICODE
#define _UNICODE

#ifdef UNICODE
#define TCHAR WCHAR
#else
#define TCHAR CHAR
#endif

#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <time.h>

#define MAX_CHAR 30

typedef struct {
	DWORD row;
	LONG bankAccountNumber;
	TCHAR surname[MAX_CHAR];
	TCHAR name[MAX_CHAR];
	DWORD money;
}BALANCE_TYPE;

HANDLE					hBalance;
CRITICAL_SECTION		cs;

DWORD WINAPI operationsThread(LPVOID Parameter);


INT _tmain(INT argc, LPTSTR argv[]) {

	BALANCE_TYPE			tmp;
	HANDLE					hIn, hOut;
	DWORD					nIn, nOut, nThreads = argc - 2;
	PDWORD					threadId = (PDWORD)malloc((nThreads) * sizeof(DWORD));
	PHANDLE					handles = (PHANDLE)malloc((nThreads) * sizeof(HANDLE));
	

	InitializeCriticalSection(&cs);

	hBalance = CreateFile(argv[1], GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	for (int i = 0; i < nThreads; i++) {
		handles[i] = CreateThread(NULL, 0, operationsThread, argv[i + 2], 0, &threadId[i]);
	}

	WaitForMultipleObjects(nThreads, handles, TRUE, INFINITE);

	CloseHandle(hBalance);
	hBalance = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	_tprintf(_T("\n\nFINAL RESULT:\n"));
	while (ReadFile(hBalance, &tmp, sizeof(tmp), &nOut, NULL) && nOut > 0) {
		_tprintf(_T("%d %ld %s %s %d\n"), tmp.row, tmp.bankAccountNumber, tmp.surname, tmp.name, tmp.money);
	}
	
	system("pause");

	return 0;
}

DWORD WINAPI operationsThread(LPVOID Parameter) {

	LPTSTR name = (LPTSTR)Parameter;
	HANDLE			hIn;
	DWORD			nOut;
	BALANCE_TYPE	tmpBalance, tmp;
	LARGE_INTEGER   filePos, fileReserved;

	hIn = CreateFile(name, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);


	while (ReadFile(hIn, &tmp, sizeof(tmp), &nOut, NULL) && nOut > 0) {	
		
		EnterCriticalSection(&cs);
		SetFilePointer(hBalance, (tmp.row - 1) * sizeof(BALANCE_TYPE), NULL, FILE_BEGIN);
		ReadFile(hBalance, &tmpBalance, sizeof(tmpBalance), &nOut, NULL);
		_tprintf(_T("\n\nI am thread %d and i read\n"), GetCurrentThreadId(), name);
		_tprintf(_T("%d %ld %s %s %d\n"), tmpBalance.row, tmpBalance.bankAccountNumber,
			tmpBalance.surname, tmpBalance.name, tmpBalance.money);
		tmpBalance.money += tmp.money;
		_tprintf(_T("After modif it:\n"));
		_tprintf(_T("%d %ld %s %s %d\n"), tmpBalance.row, tmpBalance.bankAccountNumber,
			tmpBalance.surname, tmpBalance.name, tmpBalance.money);

		SetFilePointer(hBalance, (tmp.row - 1) * sizeof(BALANCE_TYPE), NULL, FILE_BEGIN);
		WriteFile(hBalance, &tmpBalance, sizeof(tmpBalance), &nOut, NULL);
		LeaveCriticalSection(&cs);
		
	}

	return 0;
}
