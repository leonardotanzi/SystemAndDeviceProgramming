// Lab4es3.cpp: definisce il punto di ingresso dell'applicazione console.
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
	HANDLE sem;
	DWORD numOfCars, TimeArrival, timePassage;
}THREAD_ARG;

DWORD n1 = 0, n2 = 0;

DWORD WINAPI writer(LPVOID);
DWORD WINAPI reader(LPVOID);

INT _tmain(INT argc, LPTSTR argv[]) {

	DWORD		timeReader, timeWriter, timeARRIVAL, numOfCarsReader, numOfCarsWriter;
	HANDLE		threads[2], sem;
	THREAD_ARG  args[2];

	srand(time(NULL));

	if (argc < 6) perror("Not enough arguments.\n");
	else {
		timeReader = _ttoi(argv[1]);
		timeWriter = _ttoi(argv[2]);
		timeARRIVAL = _ttoi(argv[3]);
		numOfCarsReader = _ttoi(argv[4]);
		numOfCarsWriter = _ttoi(argv[5]);
	}

	sem = CreateSemaphore(NULL, 1, numOfCarsReader + numOfCarsWriter, NULL);
	args[0].sem = sem;
	args[0].numOfCars = numOfCarsReader;
	args[0].TimeArrival = timeARRIVAL;
	args[0].timePassage = timeWriter;
	args[1].sem = sem;
	args[1].numOfCars = numOfCarsWriter;
	args[1].TimeArrival = timeARRIVAL;
	args[1].timePassage = timeReader;

	threads[0] = CreateThread(NULL, 0, writer, &args[0], 0, NULL);
	threads[1] = CreateThread(NULL, 0, reader, &args[1], 0, NULL);

	WaitForMultipleObjects(2, threads, TRUE, INFINITE);

	system("pause");

	return 0;
}


DWORD WINAPI writer(LPVOID argument) {

	HANDLE			s;
	THREAD_ARG		*arg = (THREAD_ARG*)argument;
	DWORD			rndSleep;

	s = CreateSemaphore(NULL, 1, arg->numOfCars, NULL);
	
	srand(time(NULL));

	for (int i = 0; i < arg->numOfCars; i++) {
		Sleep(rand() % arg->TimeArrival);
		WaitForSingleObject(s, INFINITE);
		_tprintf(_T("THREAD %d, cycle number %d.\n"), GetCurrentThreadId(), i);
		n1++;
		if (n1 == 1) {
			WaitForSingleObject(arg->sem, INFINITE);
		}
		ReleaseSemaphore(s, 1, NULL);

		rndSleep = rand() % arg->timePassage;
		_tprintf(_T("THREAD %d, cycle number %d: took %d seconds to traverse the bridge.\n"), GetCurrentThreadId(), i, rndSleep);
		Sleep(rndSleep);


		WaitForSingleObject(s, INFINITE);
		n1--;
		if (n1 == 0) {
			ReleaseSemaphore(arg->sem, 1, NULL);
		}
		ReleaseSemaphore(s, 1, NULL);
	}
	return 0;
}


DWORD WINAPI reader(LPVOID argument) {

	HANDLE			s;
	THREAD_ARG		*arg = (THREAD_ARG*)argument;
	DWORD			rndSleep;

	s = CreateSemaphore(NULL, 1, arg->numOfCars, NULL);

	srand(time(NULL));

	for (int i = 0; i < arg->numOfCars; i++) {
		Sleep(rand() % arg->TimeArrival);
		WaitForSingleObject(s, INFINITE);
		_tprintf(_T("THREAD %d, cycle number %d.\n"), GetCurrentThreadId(), i);
		n2++;
		if (n2 == 1) {
			WaitForSingleObject(arg->sem, INFINITE);
		}
		ReleaseSemaphore(s, 1, NULL);

		rndSleep = rand() % arg->timePassage;
		_tprintf(_T("THREAD %d, cycle number %d: took %d seconds to traverse the bridge.\n"), GetCurrentThreadId(), i, rndSleep);
		Sleep(rndSleep);


		WaitForSingleObject(s, INFINITE);
		n2--;
		if (n2 == 0) {
			ReleaseSemaphore(arg->sem, 1, NULL);
		}
		ReleaseSemaphore(s, 1, NULL);
	}
	return 0;
}