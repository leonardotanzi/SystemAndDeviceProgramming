// Lab3es2.cpp: definisce il punto di ingresso dell'applicazione console.
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

#define TYPE_FILE 1
#define TYPE_DIR  2
#define TYPE_DOT  3

typedef struct {
	LPTSTR path;
	DWORD id;
}THREAD_ARG;

static DWORD FileType(LPWIN32_FIND_DATA pFileData);
DWORD WINAPI TraverseRecursive(LPVOID Parameter);
DWORD WINAPI ConfrontingThread(LPVOID);
static void Search(THREAD_ARG *SourcePathName, DWORD level);

//CRITICAL_SECTION cs1, cs2, cs3;
THREAD_ARG *pathStruct; //questo è per passare l'id e path iniziale
LPTSTR *pathsArray;  //qua metto i vari path da confrontare
DWORD nOrderingThreads;
DWORD equal = 1;
HANDLE s1, s2, s3;

INT _tmain(INT argc, LPTSTR argv[]) {

	DWORD			i;
	PDWORD			threadId = (PDWORD)malloc((argc) * sizeof(DWORD));
	PHANDLE			handles = (PHANDLE)malloc((argc) * sizeof(HANDLE));
	
	//Inizializzo i vettori degli argomenti da passare e il vettore di stringhe contenenti
	//tutti i path da confrontare volta per volta
	pathStruct = (THREAD_ARG*)malloc((argc - 1) * sizeof(THREAD_ARG));
	for (int i = 0; i < argc - 1; i++) {
		pathStruct[i].path = (LPTSTR)malloc(256 * sizeof(TCHAR));
	}
	pathsArray = (LPTSTR*)malloc((argc - 1) * sizeof(LPTSTR));
	for (int i = 0; i < argc - 1; i++) {
		pathsArray[i] = (LPTSTR)malloc(256 * sizeof(TCHAR));
	}

	nOrderingThreads = argc - 1;
	//InitializeCriticalSection(&cs1);
	//InitializeCriticalSection(&cs2);
	s1 = CreateSemaphore(NULL, 2, argc, NULL);
	s2 = CreateSemaphore(NULL, 0, argc, NULL);
	s3 = CreateSemaphore(NULL, 0, argc, NULL);

	for (int i = 1; i < argc; i++) {
		pathStruct[i - 1].path = argv[i];
		pathStruct[i - 1].id = i - 1;
		handles[i - 1] = CreateThread(NULL, 0, TraverseRecursive, &pathStruct[i - 1], 0, &threadId[i - 1]);
	}

	handles[argc - 1] = CreateThread(NULL, 0, ConfrontingThread, NULL, 0, &threadId[argc - 1]);

	WaitForMultipleObjects(argc, handles, TRUE, INFINITE);

	if (equal == 1) {
		_tprintf(_T("The tree are equals.\n"));
	}

	system("pause");

	return 0;
}

DWORD WINAPI TraverseRecursive(LPVOID Parameter) {

	THREAD_ARG *Path = (THREAD_ARG*)Parameter;
	Search(Path, 0);
	return 0;

}

static void Search(THREAD_ARG *SourcePathName, DWORD level) {

	HANDLE				SearchHandle, fileHandle;
	WIN32_FIND_DATA		FindData;
	DWORD				FType, i, sizeFile, j;
	DWORD				SourceLen, lenPlus;
	TCHAR				*AbsolutPath;
	THREAD_ARG			*structToPass = (THREAD_ARG*)malloc(sizeof(THREAD_ARG));

	//SourceLen è la lunghezza del path iniziale a cui devo aggiungere \* per poter fare la ricerca con
	//absolut path
	SourceLen = _tcslen(SourcePathName->path);
	AbsolutPath = (TCHAR*)malloc((SourceLen + 2) * sizeof(TCHAR));

	for (i = 0; i < SourceLen; i++) {
		AbsolutPath[i] = SourcePathName->path[i];
	}

	AbsolutPath[i++] = '\\';
	AbsolutPath[i++] = '*';
	AbsolutPath[i] = '\0';

	SearchHandle = FindFirstFile(AbsolutPath, &FindData);
	
	do {
		FType = FileType(&FindData);

		if (FType == TYPE_FILE) {
			for (i = 0; i < level; i++) {
				_tprintf(_T("  "));
			}_tprintf(_T("TH %d: level=%d FILE: %s\n"), GetCurrentThreadId(), level, FindData.cFileName);

		}if (FType == TYPE_DIR) {
			for (i = 0; i < level; i++) {
				_tprintf(_T("  "));
			}_tprintf(_T("TH %d: level=%d DIR : %s\n"), GetCurrentThreadId(), level, FindData.cFileName);

			//prima di chiamare recursivamente devo aggiungere il filename al vecchio source, in modo che 
			//posso passare a fare la ricerca nella sotto-cartella

			lenPlus = _tcslen(FindData.cFileName);
			structToPass->path = (TCHAR*)malloc((lenPlus + SourceLen + 2) * sizeof(TCHAR));
			for (i = 0; i < SourceLen; i++) {
				structToPass->path[i] = SourcePathName->path[i];
			}
			structToPass->path[SourceLen] = '\\';

			j = 0;
			for (i = SourceLen + 1; i < lenPlus + SourceLen + 1; i++) {
				structToPass->path[i] = FindData.cFileName[j];
				j++;
			}
			structToPass->path[i] = '\0';

			structToPass->id = SourcePathName->id;

			Search(structToPass, level + 1);

			//EnterCriticalSection(&cs1);
			WaitForSingleObject(s1, INFINITE);
			pathsArray[SourcePathName->id] = FindData.cFileName;
			_tprintf(_T("th %d : I am writing %s to id %d\n"), GetCurrentThreadId(), FindData.cFileName, SourcePathName->id);
			ReleaseSemaphore(s2, 1, NULL);
			//LeaveCriticalSection(&cs2);

		}
	} while (FindNextFile(SearchHandle, &FindData));

	ReleaseSemaphore(s3, 2, NULL);

	FindClose(SearchHandle);

}

DWORD WINAPI ConfrontingThread(LPVOID){

	while (1) {

		if (WaitForSingleObject(s3, 0) == WAIT_OBJECT_0) {
			break;
		}

		for (int i = 0; i < nOrderingThreads; i++) {
			//EnterCriticalSection(&cs2);
			WaitForSingleObject(s2, INFINITE);
		}
		//_tprintf(_T("one is %s two is %s\n"), pathsArray[0], pathsArray[1]);

		//for (int i = 0; i < nOrderingThreads - 1; i++) {
			if (_tcsncmp(pathsArray[0], pathsArray[1], 256) != 0) {
				_tprintf(_T("%s and %s are Not Equal.\n"), pathsArray[0], pathsArray[1]);
				equal = 0;
			}
		//}
		for (int i = 0; i < nOrderingThreads; i++) {
			ReleaseSemaphore(s1, 1, NULL);
			//LeaveCriticalSection(&cs1);
		}
	}
	return 0;
}


static DWORD FileType(LPWIN32_FIND_DATA pFileData) {
	BOOL IsDir;
	DWORD FType;
	FType = TYPE_FILE;
	IsDir = (pFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	if (IsDir) {
		if (lstrcmp(pFileData->cFileName, _T(".")) == 0 || lstrcmp(pFileData->cFileName, _T("..")) == 0) {
			FType = TYPE_DOT;
		}
		else {
			FType = TYPE_DIR;
		}
	}
	return FType;
}