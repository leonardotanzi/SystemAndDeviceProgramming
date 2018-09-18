// 04052013.cpp: definisce il punto di ingresso dell'applicazione console.
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

#define LENSTR 30

typedef struct {
	TCHAR itWord[LENSTR];
	TCHAR enWord[LENSTR];
}INPUT_DATA;


typedef struct {
	TCHAR itWord[LENSTR];
	TCHAR nTranslation;
	LPTSTR *enWords;
}OUTPUT_DATA;


TCHAR			dest[LENSTR], sharedFile[LENSTR];
HANDLE			s1, s2;



DWORD WINAPI ordering(void*);
DWORD WINAPI merging(void* arg);


INT _tmain(INT argc, LPTSTR argv[]) {
	
	PHANDLE				handles;
	DWORD				i, nTh;
	HANDLE				fileIn;
	WIN32_FIND_DATA		findData;

	if (argc < 5) return 1;
	else {
		nTh = _ttoi(argv[1]);
		_tcscpy(dest, argv[3]);
	}

	handles = (PHANDLE)malloc((nTh + 1) * sizeof(HANDLE));
	s1 = CreateSemaphore(NULL, 0, nTh, NULL);
	s2 = CreateSemaphore(NULL, 0, nTh, NULL);

	for (i = 0; i < nTh - 1; i++) {
		handles[i] = CreateThread(NULL, 0, ordering, NULL, 0, NULL);
	}
	handles[i] = CreateThread(NULL, 0, merging, (void*)&argv[4], 0, NULL);

	SetCurrentDirectory(argv[2]);
	fileIn = FindFirstFile(_T("*.*"), &findData);

	do {

		_tcscpy(sharedFile, findData.cFileName);
		ReleaseSemaphore(s1, 1, NULL);
		WaitForSingleObject(s2, INFINITE);

	} while (FindNextFile(fileIn, &findData));

	//send stop signal

    return 0;
}

DWORD WINAPI ordering(void*) {

	TCHAR				file[LENSTR];
	HANDLE				hIn;
	DWORD				nElements, nIn, i, j, found, outputElements; 
	INPUT_DATA			*fileContent = (INPUT_DATA*)malloc(sizeof(INPUT_DATA));
	OUTPUT_DATA			*outputContent = (OUTPUT_DATA*)malloc(sizeof(OUTPUT_DATA));

	WaitForSingleObject(s1, INFINITE);
	_tcscpy(file, sharedFile);
	ReleaseSemaphore(s2, 1, NULL); 

	hIn = CreateFile(file, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	ReadFile(hIn, &nElements, sizeof(nElements), &nIn, NULL);

	fileContent = (INPUT_DATA*)realloc(fileContent, nElements * sizeof(INPUT_DATA));

	for (i = 0; i < nElements; i++) {
		ReadFile(hIn, &fileContent[i], sizeof(fileContent[i], &nIn, NULL);
	}

	for (i = 0; i < nElements; i++) {
		
		found = 0;
		
		for (j = 0; j < outputElements + 1; j++) {
			if ((_tcscmp(outputContent[j].itWord, fileContent[i].itWord)) == 0) {
				outputContent[j].nTranslation++;
				outputContent[j].enWords = (LPTSTR*)realloc(outputContent[j].enWords, 
					outputContent[j].nTranslation * sizeof(LPTSTR));
				outputContent[j].enWords[outputContent[j].nTranslation - 1] =
					(LPTSTR)malloc(LENSTR * sizeof(TCHAR));
				_tcscpy(outputContent[j].enWords[outputContent[j].nTranslation - 1], fileContent[i].enWord);
				found = 1;
				break;
			}
		}
		if (found == 0) {
			_tcscpy(outputContent[i].itWord, fileContent[i].itWord);
			outputContent[i].nTranslation = 1;
			outputContent[i].enWords = (LPTSTR*)malloc(sizeof(LPTSTR));
			outputContent[i].enWords[0] = (LPTSTR)malloc(LENSTR * sizeof(TCHAR));
			_tcscpy(outputContent[i].enWords[0], fileContent[i].enWord);
			outputElements++;

		}
	}//bubbleSort(outputContent, outputElements)


}


DWORD WINAPI merging(void* arg) {

}


void bubbleSort(OUTPUT_DATA *array, DWORD len) {
	
	DWORD n1 = 0, n2 = 0;
	OUTPUT_DATA *temp;

	DWORD maxEnWords = 0; 
	for (int i = 0; i < len; i++) {
		if (array[i].nTranslation > maxEnWords) {
			maxEnWords = array[i].nTranslation;
		}
	}

	temp->enWords = (LPTSTR*)malloc(maxEnWords * sizeof(LPTSTR));
	for (int i = 0; i < maxEnWords; i++) {
		temp->enWords[i] = (LPTSTR)malloc(LENSTR * sizeof(TCHAR));
	}

	for (n1 = 0; n1 < len; n1++) {
		for (n2 = 0; n2 < len - n1 - 1; n2++) {
			if (_tcscmp(array[n2].itWord, array[n2 + 1].itWord) > 0) {  //Scambio valori
				
				for (int i = 0; i < nEnWords; i++) {
					_tcscpy(temp->enWords[i], array[n2].enWords[i]);
				}
				_tcscpy(temp->itWord, array[n2].itWord);
				temp->nTranslation = array[n2].nTranslation;


				for (int i = 0; i < nEnWords; i++) {
					_tcscpy(array[n2].enWords[i], array[n2 + 1].enWords[i]);
				}
				_tcscpy(array[n2].itWord, array[n2 + 1].itWord);
				array[n2].nTranslation = array[n2 + 1].nTranslation;


				for (int i = 0; i < nEnWords; i++) {
					_tcscpy(array[n2 + 1].enWords[i], temp->enWords[i]);
				}
				_tcscpy(array[n2 + 1].itWord, temp->itWord);
				array[n2 + 1].nTranslation = temp->nTranslation;

			}
		}
	}
}