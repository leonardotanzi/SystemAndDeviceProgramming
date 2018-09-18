// 210115.cpp: definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"#define _CRT_SECURE_NO_WARNINGS
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
#include <stdlib.h>

#define LSTR 32

typedef struct {
	TCHAR date[LSTR];
	FLOAT money;
}BANK_OPERATION;


typedef struct {
	TCHAR month[8];
	FLOAT *balances;
}MONTH_BALANCE;

DWORD				n, m, counter = 0;
HANDLE				hOut, sem;
MONTH_BALANCE		*months;
CRITICAL_SECTION	cs;

DWORD WINAPI thAction(void *arg);

INT _tmain(INT argc, LPTSTR argv[]) {

	PHANDLE				handles;
	PDWORD				accountNumber;
	TCHAR				monthName[8];

	if (argc < 4) return 1;
	else {
		n = _ttoi(argv[1]);
		m = _ttoi(argv[2]);
	}

	handles			= (PHANDLE)malloc(n * sizeof(HANDLE));
	accountNumber	= (PDWORD)malloc(n * sizeof(DWORD));
	months			= (MONTH_BALANCE*)malloc(m * sizeof(MONTH_BALANCE));
	
	for (int i = 0; i < m; i++) {
		_stprintf(months[i].month, _T("MONTH%d"), i);
		months[i].balances = (FLOAT*)malloc(n * sizeof(FLOAT));
	}

	InitializeCriticalSection(&cs);
	sem = CreateSemaphore(NULL, 0, n, NULL);

	hOut = CreateFile(argv[3], GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	for (int i = 0; i < n; i++) {
		accountNumber[i] = i + 1;
		handles[i] = CreateThread(NULL, 0, thAction, (void*)&accountNumber[i], 0, NULL);
	}
	WaitForMultipleObjects(n, handles, TRUE, INFINITE);

	CloseHandle(hOut);
	for (int i = 0; i < n; i++) {
		CloseHandle(handles[i]);
	}

	system("pause");

    return 0;
}

DWORD WINAPI thAction(void *arg) {

	DWORD				accountID = *(DWORD*)arg, nIn, localCounter = 0;
	TCHAR				fileName[19];
	HANDLE				hIn;
	BANK_OPERATION		operation;
	FLOAT				finalBalance = 0;

	for (int i = 0; i < m; i++) {

		_stprintf(fileName, _T("account%dmonth%d.bin"), accountID, i + 1);
		hIn = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		while (ReadFile(hIn, &operation, sizeof(operation), &nIn, NULL) && nIn > 0) {
			finalBalance += operation.money;
			_tprintf(_T("TH %d, finalBalance now %f\n"), accountID, finalBalance);
		}
		months[i].balances[accountID - 1] = finalBalance;

		EnterCriticalSection(&cs);
		localCounter = ++counter;
		LeaveCriticalSection(&cs);

		if (localCounter == n) {
			WriteFile(hOut, &months[i], sizeof(months[i]), &nIn, NULL);
			_tprintf(_T("%s "), months[i].month);
			for (int j = 0; j < n; j++) {
				_tprintf(_T("%f "), months[i].balances[j]);
			}
			EnterCriticalSection(&cs);
			counter = 0;
			LeaveCriticalSection(&cs);
			ReleaseSemaphore(sem, n - 1, NULL);
		}
		else {
			WaitForSingleObject(sem, INFINITE);
		}
		CloseHandle(hIn);
	}
	return 1;
}