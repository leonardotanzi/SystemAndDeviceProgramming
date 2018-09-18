// Lab4es1D.cpp: definisce il punto di ingresso dell'applicazione console.
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

typedef struct {
	HANDLE sem;
	LPTSTR name;
}THREAD_ARG;

HANDLE	hBalance;

DWORD WINAPI operationsThread(LPVOID Parameter);


INT _tmain(INT argc, LPTSTR argv[]) {

	BALANCE_TYPE			tmp;
	HANDLE					hIn, hOut, sem;
	DWORD					nIn, nOut, nThreads = argc - 2;
	PDWORD					threadId = (PDWORD)malloc((nThreads) * sizeof(DWORD));
	PHANDLE					handles = (PHANDLE)malloc((nThreads) * sizeof(HANDLE));
	THREAD_ARG				*args = (THREAD_ARG*)malloc((nThreads) * sizeof(THREAD_ARG));

	sem = CreateSemaphore(NULL, 1, nThreads, NULL);

	hBalance = CreateFile(argv[1], GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	for (int i = 0; i < nThreads; i++) {
		args[i].sem = sem;
		args[i].name = (LPTSTR)malloc(10 * sizeof(TCHAR));
		_tcscpy(args[i].name, argv[i + 2]);
		handles[i] = CreateThread(NULL, 0, operationsThread, &args[i], 0, &threadId[i]);
	}

	WaitForMultipleObjects(nThreads, handles, TRUE, INFINITE);

	CloseHandle(hBalance);
	hBalance = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	_tprintf(_T("\n\nFINAL RESULT:\n"));
	while (ReadFile(hBalance, &tmp, sizeof(tmp), &nOut, NULL) && nOut > 0) {
		_tprintf(_T("%d %ld %s %s %d\n"), tmp.row, tmp.bankAccountNumber, tmp.surname, tmp.name, tmp.money);
	}

	CloseHandle(hBalance);
	CloseHandle(sem);
	for (int i = 0; i < 2; i++) {
		CloseHandle(handles[i]);
	}

	system("pause");

	return 0;
}

DWORD WINAPI operationsThread(LPVOID Parameter) {

	THREAD_ARG *arg = (THREAD_ARG*)Parameter;
	HANDLE			hIn;
	DWORD			nOut;
	BALANCE_TYPE	tmpBalance, tmp;
	LARGE_INTEGER   filePos, fileReserved;

	hIn = CreateFile(arg->name, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);


	while (ReadFile(hIn, &tmp, sizeof(tmp), &nOut, NULL) && nOut > 0) {

		WaitForSingleObject(arg->sem, INFINITE);
		SetFilePointer(hBalance, (tmp.row - 1) * sizeof(BALANCE_TYPE), NULL, FILE_BEGIN);
		ReadFile(hBalance, &tmpBalance, sizeof(tmpBalance), &nOut, NULL);
		_tprintf(_T("\n\nI am thread %d and i read\n"), GetCurrentThreadId(), arg->name);
		_tprintf(_T("%d %ld %s %s %d\n"), tmpBalance.row, tmpBalance.bankAccountNumber,
			tmpBalance.surname, tmpBalance.name, tmpBalance.money);
		tmpBalance.money += tmp.money;
		_tprintf(_T("After modif it:\n"));
		_tprintf(_T("%d %ld %s %s %d\n"), tmpBalance.row, tmpBalance.bankAccountNumber,
			tmpBalance.surname, tmpBalance.name, tmpBalance.money);

		SetFilePointer(hBalance, (tmp.row - 1) * sizeof(BALANCE_TYPE), NULL, FILE_BEGIN);
		WriteFile(hBalance, &tmpBalance, sizeof(tmpBalance), &nOut, NULL);
		ReleaseSemaphore(arg->sem, 1, NULL);
	}
	CloseHandle(hIn);

	return 0;
}

