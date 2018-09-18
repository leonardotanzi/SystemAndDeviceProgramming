// Lab2es3A.cpp: definisce il punto di ingresso dell'applicazione console.
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

static DWORD FileType(LPWIN32_FIND_DATA pFileData);
DWORD WINAPI TraverseRecursive(LPVOID Parameter);
static void Search(LPTSTR SourcePathName, DWORD level);


INT _tmain(INT argc, LPTSTR argv[]) {

	DWORD			i;
	PDWORD			threadId = (PDWORD)malloc((argc - 1) * sizeof(DWORD));
	PHANDLE			handles = (PHANDLE)malloc((argc - 1) * sizeof(HANDLE));

	for (int i = 1; i < argc; i++) {
		handles[i] = CreateThread(NULL, 0, TraverseRecursive, argv[i], 0, &threadId[i]);
		Sleep(5);
	}

	WaitForMultipleObjects(argc - 1, handles, TRUE, INFINITE);

	system("pause");

	return 0;
}

DWORD WINAPI TraverseRecursive(LPVOID Parameter) {

	LPTSTR Path = (LPTSTR)Parameter;
	Search(Path, 0);
	return 0;

}

static void Search(LPTSTR SourcePathName, DWORD level) {

	HANDLE				SearchHandle, fileHandle;
	WIN32_FIND_DATA		FindData;
	DWORD				FType, i, sizeFile, j;
	DWORD				SourceLen, lenPlus;
	TCHAR				*AbsolutPath, *PathName;
	
	//SourceLen è la lunghezza del path iniziale a cui devo aggiungere \* per poter fare la ricerca con
	//absolut path
	SourceLen = _tcslen(SourcePathName);
	AbsolutPath = (TCHAR*)malloc((SourceLen + 2) * sizeof(TCHAR));
	
	for (i = 0; i < SourceLen; i++) {
		AbsolutPath[i] = SourcePathName[i];
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
			PathName = (TCHAR*)malloc((lenPlus + SourceLen + 2) * sizeof(TCHAR));
			for (i = 0; i < SourceLen; i++) {
				PathName[i] = SourcePathName[i];
			}
			PathName[SourceLen] = '\\';
			
			j = 0;
			for (i = SourceLen + 1; i < lenPlus + SourceLen + 1; i++) {
				PathName[i] = FindData.cFileName[j];
				j++;
			}
			PathName[i] = '\0';

			Search(PathName, level + 1);
		}
	} while (FindNextFile(SearchHandle, &FindData));

	FindClose(SearchHandle);

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