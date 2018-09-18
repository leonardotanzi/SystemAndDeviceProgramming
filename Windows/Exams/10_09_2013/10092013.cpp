// 10092013.cpp: definisce il punto di ingresso dell'applicazione console.
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

#define MAX_CHAR 30

typedef struct {
	TCHAR output[MAX_CHAR];
	DWORD id;
}THREAD_ARG;

typedef struct {
	HANDLE s;
	DWORD offset;
}OFFSET;

HANDLE		hIn;
OFFSET		*offsets;

INT _tmain(INT argc, LPTSTR argv[]) {

	PHANDLE					handles;
	DWORD					nIn, nThreads = _ttoi(argv[2]), x, nVertex;
	THREAD_ARG				*args;

	hIn = CreateFile(argv[1], );
	ReadFile(hIn, &nVertex, sizeof(nVertex), &nIn, 0);

	handles = (PHANDLE)malloc(nThreads * sizeof(HANDLE));
	args = (THREAD_ARG*)malloc(nThreads * sizeof(THREAD_ARG));
	offsets = (OFFSET*)malloc(nVertex * sizeof(OFFSET));
	

	for (int i = 0; i < nThreads; i++) {
		i = 0 ? x = 1 : x = 0;
		offsets[i].s = CreateSemaphore(NULL, x, nThreads, NULL);
		offsets[i].offset = 1;
		args[i].id = i;
		_tcscpy(args[i].output, argv[3]);
		handles[i] = CreateThread(NULL, 0, thAction, (void*)&args[i], 0, NULL);
	}

	WaitForMultipleObjects(nThreads, handles, TRUE, INFINITE);

	system("pause");

    return 0;
}

DWORD WINAPI thAction(void *arg) {

	THREAD_ARG		*data = (THREAD_ARG*)arg;
	DWORD			*list;

	//cs
	localc = counter++;
	//cs
	list = RecursiveSearch(localc);
	

}


DWORD *RecursiveSearch(DWORD VertexId) {

	DWORD		nVertex, nIn, linked, *list;

	while (1) {
		//cs

		WaitForSingleObject(offsets[VertexId].s, INFINITE);

		ReadFile(hIn, &nVertex, sizeof(nVertex), &nIn, NULL);
		ReadFile(hIn, &linked, sizeof(linked), &nIn, NULL);

		list = (DWORD*)malloc(linked * sizeof(DWORD));

		for (int i = 0; i < linked; i++) {
			ReadFile(hIn, &list[i], sizeof(list[i]), &nIn, NULL);
		}
		//cs
		if (nVertex != 0) {
			offsets[nVertex].offset = offsets[nVertex - 1].offset + linked + 3;
		}
		else {
			offsets[nVertex].offset = linked + 3;
		}
		ReleaseSemaphore(offsets[nVertex + 1].s, 1, NULL);
	}
		for (int i = 0; i < linked; i++) {
			RecursiveSearch(list[i]);
		}
	}
}