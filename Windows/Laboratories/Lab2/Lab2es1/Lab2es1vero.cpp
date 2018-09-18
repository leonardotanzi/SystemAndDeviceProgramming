// Lab2es1vero.cpp: definisce il punto di ingresso dell'applicazione console.
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

typedef struct {
	LPTSTR name;
	DWORD id;
}newtype;


PDWORD *Array;

DWORD WINAPI MergeSort(LPVOID n);
void PrintVector(DWORD n, PDWORD v);
void BubbleSort(PDWORD array, DWORD elemN);

INT _tmain(INT argc, LPTSTR argv[]) {

	FILE				*fIn;
	HANDLE				hOut, fileHandle;
	DWORD				nIn, nOut, rnd, limit, tmp, tId, len[3], totLen = 0, j;
	PHANDLE				vHandle;
	newtype				*parameter;
	PDWORD				finalVec;

	srand(time(NULL));
	
	for (int i = 1; i < 4; i++) {
		/*
		printf("FIle %s.\n", argv[i]);

		if ((hOut = CreateFile(argv[i], GENERIC_WRITE | GENERIC_READ, 0, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) {

			fprintf(stderr, "Cannot open output file. Error: %x\n", GetLastError());
			return 3;
		}

		limit = rand() % 10;
		WriteFile(hOut, &limit, sizeof(limit), &nOut, NULL);

		for (int j = 0; j < limit; j++) {
			rnd = rand() % 100;
			WriteFile(hOut, &rnd, sizeof(rnd), &nOut, NULL);
		}


		CloseHandle(hOut);
		*/

		HANDLE hIn;
		hIn = CreateFile(argv[i], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		while ((ReadFile(hIn, &tmp, sizeof(tmp), &nOut, NULL)) && nOut > 0) {
			//_tprintf(_T("%d\n"), tmp);
		}
	}

	vHandle = (PHANDLE)malloc(3 * sizeof(HANDLE));
	parameter = (newtype*)malloc(3 * sizeof(newtype));
	Array = (PDWORD*)malloc(3 * sizeof(PDWORD));

	for (int i = 1; i < 4; i++) {
		parameter[i - 1].name = argv[i];
		parameter[i - 1].id = i - 1;
		vHandle[i - 1] = CreateThread(NULL, 0, MergeSort, &parameter[i - 1], 0, &tId);
		Sleep(5);
	}

	WaitForMultipleObjects(3, vHandle, TRUE, INFINITE);
	for (int i = 0; i < 3; i++) {
		GetExitCodeThread(vHandle[i], &len[i]);
		//PrintVector(len[i], Array[i]);
		totLen += len[i];
	}

	finalVec = (PDWORD)malloc(totLen * sizeof(DWORD));

	for (int i = 0; i < len[0]; i++){
		finalVec[i] = Array[0][i];
	}
	j = 0;
	for (int i = len[0]; i < len[0] + len[1]; i++) {
		finalVec[i] = Array[1][j];
		j++;
	}
	j = 0;
	for (int i = len[0] + len[1]; i < totLen; i++) {
		finalVec[i] = Array[2][j];
		j++;
	}

	//PrintVector(totLen, finalVec);

	BubbleSort(finalVec, totLen);

	//PrintVector(totLen, finalVec);
	
	fileHandle = CreateFile(argv[4], GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	WriteFile(fileHandle, &totLen, sizeof(totLen), &nOut, NULL);

	for (int i = 0; i < totLen; i++) {
		WriteFile(fileHandle, &finalVec[i], sizeof(DWORD), &nOut, NULL);
	}

	CloseHandle(fileHandle);

	fileHandle = CreateFile(argv[4], GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);

	while (ReadFile(fileHandle, &tmp, sizeof(tmp), &nOut, NULL) && nOut > 0) {
		_tprintf(_T("%d\n"), tmp);
	}

	system("pause");

    return 0;
}

DWORD WINAPI MergeSort(LPVOID n) {
	
	newtype	*data = (newtype*)n;
	HANDLE	fileHandle; 
	DWORD	lenArray, nIn, tmpValue, i;
	
	_tprintf(_T("Thread %d: my name is %s.\n"), GetCurrentThreadId(), data->name);

	fileHandle = CreateFile(data->name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	ReadFile(fileHandle, &lenArray, sizeof(lenArray), &nIn, NULL);

	Array[data->id] = (PDWORD)malloc(lenArray * sizeof(DWORD));

	i = 0;
	while (ReadFile(fileHandle, &tmpValue, sizeof(tmpValue), &nIn, NULL) && nIn > 0) {
		Array[data->id][i] = tmpValue;
		i++;
	}
	//PrintVector(lenArray, Array[data->id]);

	BubbleSort(Array[data->id], lenArray);

	//PrintVector(lenArray, Array[data->id]);

	ExitThread(lenArray);
}


void PrintVector(DWORD n, PDWORD v) {

	for (int i = 0; i < n; i++) {
		_tprintf(_T("%d "), v[i]);
	}
	_tprintf(_T("\n"));
}

void BubbleSort(PDWORD array, DWORD elemN)
{
	int alto;
	for (alto = elemN - 1; alto > 0; alto--)
	{
		for (int i = 0; i<alto; i++)
		{
			if (array[i]>array[i + 1]) /* sostituire ">" con "<" per avere un ordinamento decrescente */
			{
				int tmp = array[i];
				array[i] = array[i + 1];
				array[i + 1] = tmp;
			}
		}
	}
}