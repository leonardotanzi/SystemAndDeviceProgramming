// Lab4es1A.cpp: definisce il punto di ingresso dell'applicazione console.
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
	OVERLAPPED ov;
	LPTSTR name;
}THREAD_ARG;

LPTSTR			*newNames;
HANDLE			hBalance;

DWORD WINAPI operationsThread(LPVOID Parameter);


INT _tmain(INT argc, LPTSTR argv[]) {

	BALANCE_TYPE			tmp;
	HANDLE					hIn, hOut;
	DWORD					nIn, nOut, nThreads = argc - 2;
	PDWORD					threadId = (PDWORD)malloc((nThreads) * sizeof(DWORD));
	PHANDLE					handles = (PHANDLE)malloc((nThreads) * sizeof(HANDLE));
	THREAD_ARG				*args = (THREAD_ARG*)malloc((nThreads) * sizeof(THREAD_ARG));
	OVERLAPPED				ov = { 0, 0, 0, 0, NULL };

	
	//Here I create the array containing the names of the output file, who then become input file.
	newNames = (LPTSTR*)malloc((argc - 1) * sizeof(LPTSTR));
	for (int i = 0; i < argc - 1; i++) {
		newNames[i] = (LPTSTR)malloc(10 * sizeof(TCHAR));
	}
	_tcscpy(newNames[0], _T("file1.txt"));
	_tcscpy(newNames[1], _T("file2.txt"));
	_tcscpy(newNames[2], _T("file3.txt"));

	hBalance = CreateFile(newNames[0], GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
		OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

	for (int i = 0; i < nThreads; i++) {
		args[i].name = (LPTSTR)malloc(10 * sizeof(TCHAR));
		_tcscpy(args[i].name, newNames[i + 1]);
		args[i].ov = ov;
		handles[i] = CreateThread(NULL, 0, operationsThread, &args[i], 0, &threadId[i]);
	}

	WaitForMultipleObjects(nThreads, handles, TRUE, INFINITE);

	CloseHandle(hBalance);
	hBalance = CreateFile(newNames[0], GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	_tprintf(_T("\n\nFINAL RESULT:\n"));
	while (ReadFile(hBalance, &tmp, sizeof(tmp), &nOut, NULL) && nOut > 0) {
		_tprintf(_T("%d %ld %s %s %d\n"), tmp.row, tmp.bankAccountNumber, tmp.surname, tmp.name, tmp.money);
	}

/*	
	for (int i = 0; i < argc - 1; i++) {
		FILE *fp = _tfopen(argv[i + 1], _T("r+"));
		
		hIn = CreateFile(newNames[i], GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL, NULL);
		while (_ftscanf(fp, _T("%d %ld %s %s %d"), &tmp.row, &tmp.bankAccountNumber,
												   tmp.surname, tmp.name, &tmp.money) == 5){
			WriteFile(hIn, &tmp, sizeof(tmp), &nOut, NULL);
			_tprintf(_T("%d %ld %s %s %d\n"), tmp.row, tmp.bankAccountNumber, tmp.surname, tmp.name, tmp.money);
			
		}
		_tprintf(_T("\n"));
	}


	files = (PHANDLE)malloc((argc - 1) * sizeof(HANDLE));

	for (int i = 0; i < argc - 1; i++) {
		files[i] = CreateFile(newNames[i], GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		while (ReadFile(files[i], &tmp, sizeof(tmp), &nOut, NULL) && nOut > 0) {
			_tprintf(_T("%d %ld %s %s %d\n"), tmp.row, tmp.bankAccountNumber, tmp.surname, tmp.name, tmp.money);
		}
	}

*/

	system("pause");

	return 0;
}

DWORD WINAPI operationsThread(LPVOID Parameter) {

	THREAD_ARG		*arg = (THREAD_ARG*)Parameter;
	HANDLE			hIn;
	DWORD			nOut;
	BALANCE_TYPE	tmpBalance, tmp;
	LARGE_INTEGER   filePos, fileReserved;
	
	hIn = CreateFile(arg->name, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);


	while (ReadFile(hIn, &tmp, sizeof(tmp), &nOut, NULL) && nOut > 0) {

		filePos.QuadPart = (tmp.row - 1) * sizeof(BALANCE_TYPE);
		fileReserved.QuadPart = 1 * sizeof(BALANCE_TYPE);
		arg->ov.Offset = filePos.LowPart;
		arg->ov.OffsetHigh = filePos.HighPart;
		arg->ov.hEvent = (HANDLE)0;

		LockFileEx(hBalance, LOCKFILE_EXCLUSIVE_LOCK, 0, fileReserved.LowPart, fileReserved.HighPart, &arg->ov);

		ReadFile(hBalance, &tmpBalance, sizeof(tmpBalance), &nOut, &arg->ov);
		_tprintf(_T("\n\nI am thread %d and i read\n"), GetCurrentThreadId(), arg->name);
		_tprintf(_T("%d %ld %s %s %d\n"), tmpBalance.row, tmpBalance.bankAccountNumber, 
			tmpBalance.surname, tmpBalance.name, tmpBalance.money);
		tmpBalance.money += tmp.money;
		_tprintf(_T("After modif it:\n"));
		_tprintf(_T("%d %ld %s %s %d\n"), tmpBalance.row, tmpBalance.bankAccountNumber,
			tmpBalance.surname, tmpBalance.name, tmpBalance.money);
		
		WriteFile(hBalance, &tmpBalance, sizeof(tmpBalance), &nOut, &arg->ov);
		UnlockFileEx(hBalance, 0, fileReserved.LowPart, fileReserved.HighPart, &arg->ov);
	}
	
	return 0;
}
