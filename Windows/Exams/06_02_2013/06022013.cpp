// 06022013.cpp: definisce il punto di ingresso dell'applicazione console.
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


#define MAX_CHAR 256


typedef struct {

	HANDLE s1;
	HANDLE s2;
	HANDLE cs;

}ARG_TH;

typedef struct {

	LPTSTR line;
	DWORD nWord;

}LINE_TYPE;


TCHAR				fileName[MAX_CHAR];
LINE_TYPE			*allLines;
LPTSTR				dest;
DWORD				allLinesCounter = 0;
CRITICAL_SECTION	cs;


DWORD WINAPI copyingThreads(void * arg);
DWORD WINAPI mergingThreads(void * arg);
void BubbleSort(LINE_TYPE* v, DWORD len);


INT _tmain(INT argc, LPTSTR argv[]) {

	DWORD					n = _ttoi(argv[1]), i;
	LPTSTR					source = argv[2];
	PHANDLE					handles;
	WIN32_FIND_DATA			dataFile;
	HANDLE					file;
	ARG_TH					arg;

	dest = argv[3];

	arg.s1 = CreateSemaphore(NULL, 0, n, NULL);
	arg.s2 = CreateSemaphore(NULL, 0, n, NULL);
	arg.cs = CreateSemaphore(NULL, 1, n, NULL);
	InitializeCriticalSection(&cs);
	
	handles = (PHANDLE)malloc((n + 1) * sizeof(HANDLE));
	allLines = (LINE_TYPE*)malloc(sizeof(LINE_TYPE));

	for (i = 0; i < n; i++) {
		handles[i] = CreateThread(NULL, 0, copyingThreads, &arg, 0, NULL);
	}
	handles[i] = CreateThread(NULL, 0, mergingThreads, &arg, 0, NULL);

	CreateDirectory(dest, NULL);
	SetCurrentDirectory(source);


	file = FindFirstFile(_T("*.txt"), &dataFile);
	
	do {

		_tcscpy(fileName, dataFile.cFileName);
		ReleaseSemaphore(arg.s1, 1, NULL);
		WaitForSingleObject(arg.s2, INFINITE);

	} while (FindNextFile(file, &dataFile));

	_tcscpy(fileName, _T("STOP"));

	WaitForMultipleObjects(n + 1, handles, TRUE, INFINITE);

	system("pause");

    return 0;
}



DWORD WINAPI copyingThreads(void * arg) {

	ARG_TH			*str = (ARG_TH*)arg;
	FILE			*fp, *fpOut;
	DWORD			len, lenDest, nChar = 0, nLine = 0, nWord = 0, totWord = 0, i, totChar = 0, j, k = 0;
	LPTSTR			name = (LPTSTR)malloc(sizeof(TCHAR)), destinationFile = (LPTSTR)malloc(sizeof(TCHAR));
	LINE_TYPE		*lines = (LINE_TYPE*)malloc(sizeof(LINE_TYPE));
	TCHAR			tmp;


	while (1) {

		WaitForSingleObject(str->s1, INFINITE);
		
		if ((_tcscmp(fileName, _T("STOP"))) == 0) {
			break;
		}
		len = _tcslen(fileName);
		name = (LPTSTR)realloc(name, len * sizeof(TCHAR));
		_tcscpy(name, fileName);
		ReleaseSemaphore(str->s2, 1, NULL);

		fp = _tfopen(name, _T("r"));

		
		while (_ftscanf(fp, _T("%c"), &tmp) == 1) {
			
			if (nChar == 0) {
				lines[nLine].line = (LPTSTR)malloc(sizeof(TCHAR));
				allLines[allLinesCounter].line = (LPTSTR)malloc(sizeof(TCHAR));
			}

			if (tmp != '\n') {
				
				WaitForSingleObject(str->cs, INFINITE);
				allLines[allLinesCounter].line[nChar] = tmp;
				allLines[allLinesCounter].line = (LPTSTR)realloc(allLines[allLinesCounter].line, 
					(nChar + 2) * sizeof(TCHAR));
				
				ReleaseSemaphore(str->cs, 1, NULL);

				lines[nLine].line[nChar] = tmp;
				nChar++;
				lines[nLine].line = (LPTSTR)realloc(lines[nLine].line, (nChar + 1) * sizeof(TCHAR));
			}
			if (tmp == ' ') {
				nWord++;
			}if (tmp == '\n') {
				nWord++;
				totWord += nWord;
				totChar += nChar;
				lines[nLine].nWord = nWord;
				allLines[allLinesCounter].nWord = nWord;
				nLine++;
				allLinesCounter++;
				lines = (LINE_TYPE*)realloc(lines, (nLine + 1) * sizeof(LINE_TYPE));
				allLines = (LINE_TYPE*)realloc(allLines, (allLinesCounter + 1) * sizeof(LINE_TYPE));

				nWord = 0;
				nChar = 0;
			}
		}
		//nLine++; //ultimo a capo non c'è

		lenDest = _tcslen(dest);
		destinationFile = (LPTSTR)realloc(destinationFile, (lenDest + len + 2) * sizeof(TCHAR));
		for (i = 0; i < lenDest; i++) {
			destinationFile[i] = dest[i]; //potrei fare tcscpy?
		}destinationFile[i] = '\\';
		k = 0;
		for (j = i + 1; j < lenDest + len + 2; j++) {
			destinationFile[j] = name[k];
			k++;
		}destinationFile[j] = '\0';

		fpOut = _tfopen(destinationFile, _T("w"));
		_ftprintf(fpOut, _T("%d %d %d\n"), nLine, totWord, totChar);
		_tprintf(_T("%d %d\n"), sizeof(lines), sizeof(lines[0]));
		BubbleSort(lines, nLine);
		for (i = 0; i < nLine; i++) {
			_ftprintf(fpOut, _T("%d %s\n"), lines[i].nWord, lines[i].line);
		}

	}
	return 1;
}

DWORD WINAPI mergingThreads(void * arg) {
	return 1;
}

void BubbleSort(LINE_TYPE* v, DWORD len){

	DWORD n1, n2;
	LINE_TYPE temp;

	for (n1 = 0; n1 < len; n1++) {
		for (n2 = 0; n2 < len - n1 - 1; n2++) {
			if (v[n2].nWord > v[n2 + 1].nWord) {  //Scambio valori
				temp.line = v[n2].line;
				temp.nWord = v[n2].nWord;
				v[n2].line = v[n2 + 1].line;
				v[n2].nWord = v[n2 + 1].nWord;
				v[n2 + 1].line = temp.line;
				v[n2 + 1].nWord = temp.nWord;
			}
		}
	}
}