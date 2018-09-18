// Lab2Es2.cpp: definisce il punto di ingresso dell'applicazione console.
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
static void TraverseRecursive(LPTSTR, LPTSTR);

INT _tmain(INT argc, LPTSTR argv[]) {

	TraverseRecursive(argv[1], argv[2]);

	system("pause");

	return 0;
}

static void TraverseRecursive(LPTSTR SourcePathName, LPTSTR DestPathName) {

	HANDLE				SearchHandle, fileHandle;
	WIN32_FIND_DATA		FindData;
	DWORD				FType, i, l, nIn, sizeFile;
	TCHAR				NewPath[MAX_PATH], dirBack[MAX_PATH], textRead[MAX_PATH];

	_tprintf(_T("--> Create Dir : %s\n"), DestPathName);
	CreateDirectory(DestPathName, NULL); 
	SetCurrentDirectory(SourcePathName);

	SearchHandle = FindFirstFile(_T("*"), &FindData);

	do {
		FType = FileType(&FindData);
		
		//aggiunge il \ in fondo se non c'è
		l = _tcslen(DestPathName);
		if (DestPathName[l - 1] == '\\') {
			_stprintf(NewPath, _T("%s%s"), DestPathName, FindData.cFileName);
		}
		else {
			_stprintf(NewPath, _T("%s\\%s"), DestPathName, FindData.cFileName);
		}
		
		if (FType == TYPE_FILE) {
			
			GetCurrentDirectory(MAX_PATH, dirBack);
			sizeFile = FindData.nFileSizeLow;
			//serve per togliere il /file.txt da NewPath
			int i = _tcslen(NewPath) ;
			do {
				NewPath[i] = '\0';
				i--;
			} while (NewPath[i] != '\\');
			NewPath[i] = '\0';
				
			_tprintf(_T("I am setting the cur dir to %s from %s.\n"), NewPath, dirBack);
			
			FILE *fp1 = _tfopen(FindData.cFileName, _T("r"));
			
			_ftscanf(fp1, _T("%s"), textRead);
			SetCurrentDirectory(NewPath);
			
			FILE *fp = _tfopen(FindData.cFileName, _T("w"));
			_ftprintf(fp, _T("%s %d %s\n"), FindData.cFileName, sizeFile, textRead);
			SetCurrentDirectory(dirBack);

		}if (FType == TYPE_DIR) {
			TraverseRecursive(FindData.cFileName, NewPath),
			SetCurrentDirectory(_T(".."));
		}
	} while (FindNextFile(SearchHandle, &FindData));

	FindClose(SearchHandle);

	return;
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
