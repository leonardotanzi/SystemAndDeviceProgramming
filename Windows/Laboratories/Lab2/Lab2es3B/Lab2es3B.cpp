// Lab2es3B.cpp: definisce il punto di ingresso dell'applicazione console.
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
	DWORD				FType, i, l, nIn, sizeFile;

	SetCurrentDirectory(SourcePathName);

	SearchHandle = FindFirstFile(_T("*"), &FindData);

	do {
		FType = FileType(&FindData);


		if (FType == TYPE_FILE) {

			_tprintf(_T("TH %d: level=%d FILE: %s\n"), GetCurrentThreadId(), level, FindData.cFileName);

		}if (FType == TYPE_DIR) {
			for (i = 0; i < level; i++) {
				_tprintf(_T("  "));
			}_tprintf(_T("TH %d: level=%d DIR : %s\n"), GetCurrentThreadId(), level, FindData.cFileName);

			Search(FindData.cFileName, level + 1);
			SetCurrentDirectory(_T(".."));
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


