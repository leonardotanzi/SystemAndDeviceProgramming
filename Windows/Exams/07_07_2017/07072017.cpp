// 07072017.cpp: definisce il punto di ingresso dell'applicazione console.
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
#include <stdlib.h>


typedef struct {

	DWORD		id;
	LPTSTR		name;
	FLOAT		v, c0, coefficent, result;
	DWORD		rowOffset;

}ARG_T;


ARG_T				*threads;
DWORD				nRow = 0, counter = 0, nTh;
CRITICAL_SECTION	cs;
HANDLE				s0, s1;

FLOAT poww(FLOAT b, DWORD esp);
DWORD WINAPI thAction(void *arg);
void filePrint(LPCTSTR name);

INT _tmain(INT argc, LPTSTR argv[]) {

	HANDLE			hIn;
	DWORD			nIn, i;
	PHANDLE			handles;


	hIn = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	ReadFile(hIn, &nTh, sizeof(nTh), &nIn, NULL);

	handles = (PHANDLE)malloc(nTh * sizeof(HANDLE));
	threads = (ARG_T*)malloc(nTh * sizeof(ARG_T));

	InitializeCriticalSection(&cs);
	s0 = CreateSemaphore(NULL, 0, nTh, NULL);
	s1 = CreateSemaphore(NULL, 0, nTh, NULL);

	for (i = 0; i < nTh; i++) {
		threads[i].id = i;
		threads[i].name = argv[1];
		threads[i].rowOffset = (3 + nTh);
		handles[i] = CreateThread(NULL, 0, thAction, (void*)&threads[i].id, 0, NULL);
	}
	WaitForMultipleObjects(nTh, handles, TRUE, INFINITE);

	filePrint(argv[1]);

	system("pause");

    return 0;
}

DWORD WINAPI thAction(void *arg) {

	DWORD			id = *(DWORD*)arg;
	HANDLE			fIn;
	DWORD			nIn, localC;
	FLOAT			resultFinal;

	fIn = CreateFile(threads[id].name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	while (1) {

		SetFilePointer(fIn, sizeof(DWORD) + ((nRow * threads[id].rowOffset) * sizeof(FLOAT)), NULL, FILE_BEGIN);

		if (ReadFile(fIn, &threads[id].v, sizeof(FLOAT), &nIn, NULL) || nIn <= 0) {
			break;
		}
		ReadFile(fIn, &threads[id].c0, sizeof(FLOAT), &nIn, NULL);
		
		SetFilePointer(fIn, id * sizeof(FLOAT), NULL, FILE_CURRENT);
		ReadFile(fIn, &threads[id].coefficent, sizeof(FLOAT), &nIn, NULL);

		threads[id].result = poww(threads[id].v, id + 1) * threads[id].coefficent;

		EnterCriticalSection(&cs);
		localC = ++counter;
		LeaveCriticalSection(&cs);

		if (localC == 1) {
			for (int j = 0; j < nTh - 1; j++) {
				WaitForSingleObject(s0, INFINITE);
			}
			resultFinal = threads[id].c0;

			for (int j = 0; j < nTh; j++) {
				resultFinal += threads[j].result;
			}
			_tprintf(_T("%d final result is %f.\n"), nRow, resultFinal);
			SetFilePointer(fIn, sizeof(DWORD) + (nRow * threads[id].rowOffset * sizeof(FLOAT)) +
				((2 + nTh) * sizeof(FLOAT)), NULL, FILE_BEGIN);
			WriteFile(fIn, &resultFinal, sizeof(FLOAT), &nIn, NULL);
			nRow++;
			ReleaseSemaphore(s1, nTh - 1, NULL);
		}
		else {
			ReleaseSemaphore(s0, 1, NULL);
			WaitForSingleObject(s1, INFINITE);
		}
	}
	return 1;
}



FLOAT poww(FLOAT b, DWORD esp) {
	FLOAT res = 1;
	for (int i = 0; i < esp; i++) {
		res *= b;
	}
	return res;
}

void filePrint(LPCTSTR name) {
	HANDLE hIn;
	FLOAT f;
	INT i, n;
	DWORD nIn;

	hIn = CreateFile(name, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIn == INVALID_HANDLE_VALUE) {
		_tprintf(_T("Cannot open input file. Error: %x\n"),
			GetLastError());
		exit(1);
	}

	ReadFile(hIn, &n, sizeof(INT), &nIn, NULL);
	_tprintf(_T("%d\n"), n);
	i = 0;
	while (ReadFile(hIn, &f, sizeof(FLOAT), &nIn, NULL) && nIn > 0) {
		_tprintf(_T("%f "), f);
		if ((++i) == 6) {
			_tprintf(_T("\n"));
			i = 0;
		}
	}
	CloseHandle(hIn);

	return;
}
