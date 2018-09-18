// Lab3es1.cpp: definisce il punto di ingresso dell'applicazione console.
//
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
#define N_THREADS 3

typedef struct {
	LPTSTR name;
	DWORD id;
}newtype;

typedef struct {
	PDWORD array;
	DWORD len;
}vecArg;

vecArg				*allArray;
DWORD				*startingVec;
DWORD				nT = N_THREADS;
CRITICAL_SECTION	s1, s2, s3;

DWORD WINAPI MergeSort(LPVOID n);
void PrintVector(DWORD n, PDWORD v);
void BubbleSort(PDWORD array, DWORD elemN);
void mergeArrays(DWORD arr1[], DWORD arr2[], DWORD n1, DWORD n2, DWORD arr3[]);


INT _tmain(INT argc, LPTSTR argv[]) {

	FILE				*fIn;
	HANDLE				hOut, fileHandle;
	DWORD				nIn, nOut, rnd, limit, tmp, tId, len[N_THREADS], totLen = 0, j;
	PHANDLE				vHandle, vHandleTmp;
	newtype				*parameter;
	PDWORD				finalVec;
	DWORD				id, tmpTId, numT = 0, previousLen;

	srand(time(NULL));

	//Just read and print the files' content
	for (int i = 1; i < N_THREADS + 1; i++) {
		
		HANDLE hIn;
		hIn = CreateFile(argv[i], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		while ((ReadFile(hIn, &tmp, sizeof(tmp), &nOut, NULL)) && nOut > 0) {
			_tprintf(_T("%d\n"), tmp);
		}
	}

	//Initialize
	vHandle = (PHANDLE)malloc(N_THREADS * sizeof(HANDLE));
	parameter = (newtype*)malloc(N_THREADS * sizeof(newtype));
	allArray = (vecArg*)malloc(N_THREADS * sizeof(vecArg));
	startingVec = (PDWORD)malloc(sizeof(DWORD));
	InitializeCriticalSection(&s1);

	//Create threads
	for (int i = 1; i < N_THREADS + 1; i++) {
		parameter[i - 1].name = argv[i];
		parameter[i - 1].id = i - 1;
		vHandle[i - 1] = CreateThread(NULL, 0, MergeSort, &parameter[i - 1], 0, &tId);
	}

	while(1){

		//WaitFor scorre il vettore vHandle e rimanda l'indice del thread che è finito. se più di uno ritorna il primo
		tmpTId = WaitForMultipleObjects(1, vHandle, FALSE, INFINITE) - WAIT_OBJECT_0;

		//Recupero il valore di vHandle all'indice del thread che ha finito
		GetExitCodeThread(vHandle[tmpTId], &id);
		_tprintf(_T("Exit code is %d\n"), id);

		//Creo un nuovo vettore di handle in cui non metto quello del thread appena terminato
		vHandleTmp = (PHANDLE)malloc(--nT * sizeof(HANDLE));
		j = 0;
		for (int i = 0; i < nT + 1; i++) {
			//se l'indice è quello che è appena ritornato, lo cancello dal vettore
			if (i != tmpTId) {
				vHandleTmp[j] = vHandle[i];
				j++;
			}
		}
		//Rialloco vhandle e copio da vhandletmp appena creato
		vHandle = (PHANDLE)realloc(vHandle, nT * sizeof(HANDLE));
		for (int i = 0; i < nT; i++) {
			vHandle[i] = vHandleTmp[i];
		}

		_tprintf(_T("%d threads\n"), tmpTId);
		numT++;
		if (numT > N_THREADS) {
			break;
		}
		if (numT == 1) {
			totLen = allArray[id].len;
		}

		PDWORD tmpVec = (PDWORD)malloc(totLen * sizeof(DWORD));
		for (int i = 0; i < totLen; i++) {
			tmpVec[i] = startingVec[i];
		}
		previousLen = totLen;
		totLen += allArray[id].len;
		startingVec = (PDWORD)realloc(startingVec, totLen * sizeof(DWORD));
		
		mergeArrays(allArray[id].array, tmpVec, allArray[id].len, previousLen, startingVec);
		
		//PrintVector(len[i], Array[i]);
		
	}

	_tprintf(_T("Final vec is:\n"));

	fileHandle = CreateFile(argv[N_THREADS + 1], GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	WriteFile(fileHandle, &totLen, sizeof(totLen), &nOut, NULL);

	for (int i = 0; i < totLen; i++) {
		WriteFile(fileHandle, &startingVec[i], sizeof(DWORD), &nOut, NULL);
	}

	CloseHandle(fileHandle);

	fileHandle = CreateFile(argv[N_THREADS], GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
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

	_tprintf(_T("Thread %d: my name is %s and data id is %d.\n"), GetCurrentThreadId(), data->name, data->id);

	fileHandle = CreateFile(data->name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	//The first number in the file is the length of the array
	ReadFile(fileHandle, &lenArray, sizeof(lenArray), &nIn, NULL);

	//Set in allArray at the index id the len and initilize the array
	allArray[data->id].len = lenArray;
	allArray[data->id].array = (PDWORD)malloc(lenArray * sizeof(DWORD));

	//Read number by number and set in the just initialised array
	i = 0;
	while (ReadFile(fileHandle, &tmpValue, sizeof(tmpValue), &nIn, NULL) && nIn > 0) {
		allArray[data->id].array[i] = tmpValue;
		i++;
	}

	BubbleSort(allArray[data->id].array, lenArray);

	//Return the data->id aka 0,1,2
	return (data->id);
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

void mergeArrays(DWORD arr1[], DWORD arr2[], DWORD n1, DWORD n2, DWORD arr3[])
{
	int i = 0, j = 0, k = 0;

	// Traverse both array
	while (i < n1 && j < n2)
	{
		// Check if current element of first
		// array is smaller than current element
		// of second array. If yes, store first
		// array element and increment first array
		// index. Otherwise do same with second array
		if (arr1[i] < arr2[j])
			arr3[k++] = arr1[i++];
		else
			arr3[k++] = arr2[j++];
	}

	// Store remaining elements of first array
	while (i < n1)
		arr3[k++] = arr1[i++];

	// Store remaining elements of second array
	while (j < n2)
		arr3[k++] = arr2[j++];
}
