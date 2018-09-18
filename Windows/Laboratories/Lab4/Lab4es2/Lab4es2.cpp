// Lab4es2.cpp: definisce il punto di ingresso dell'applicazione console.
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

#define N_PRODUCTS	10
#define MAX_RAN		100


typedef struct {
	HANDLE	full, empty;
}THREAD_ARG;

LPTSTR	buffer;
DWORD	in = 0, out = 0;
DWORD	P, C, N, T;

DWORD WINAPI consumer(LPVOID parameter);
DWORD WINAPI producer(LPVOID parameter);
void enqueue(DWORD val);
DWORD dequeue();


INT _tmain(INT argc, LPTSTR argv[]) {

	PHANDLE		handlesC, handlesP;
	HANDLE		full, empty, personal1, personal2;
	THREAD_ARG	*args;
	
	if (argc < 5) perror("Not enough args.\n");
	else {
		P = _ttoi(argv[1]);
		C = _ttoi(argv[2]);
		N = _ttoi(argv[3]);
		T = _ttoi(argv[4]);
	}

	buffer = (LPTSTR)malloc(N * sizeof(TCHAR));
	handlesC = (PHANDLE)malloc(C * sizeof(HANDLE));
	handlesP = (PHANDLE)malloc(P * sizeof(HANDLE));
	args = (THREAD_ARG*)malloc((P + C) * sizeof(THREAD_ARG));
	
	full = CreateSemaphore(NULL, 0, P, NULL);
	empty = CreateSemaphore(NULL, C, C, NULL);
	
	for (int i = 0; i < P; i++) {
		args[i].empty = empty;
		args[i].full = full;
		handlesP[i] = CreateThread(NULL, 0, producer, &args[i], 0, NULL);
	}

	for (int i = 0; i < C; i++) {
		args[i + P].empty = empty;
		args[i + P].full = full;
		handlesC[i] = CreateThread(NULL, 0, consumer, &args[i + P], 0, NULL);
	}

	WaitForMultipleObjects(P, handlesP, TRUE, INFINITE);

	WaitForSingleObject(empty, INFINITE);
	enqueue(MAX_RAN);
	ReleaseSemaphore(full, 1, NULL);

	WaitForMultipleObjects(C, handlesC, TRUE, INFINITE);
	
	system("pause");

	return 0;
}


DWORD WINAPI producer(LPVOID parameter) {

	THREAD_ARG	*arg = (THREAD_ARG*)parameter;
	DWORD		rnd, rndTime;
	HANDLE		personal;

	srand(time(NULL));

	personal = CreateSemaphore(NULL, 1, P, NULL);

	for (int i = 0; i < N_PRODUCTS; i++) {
		rnd = rand() % MAX_RAN;
		rndTime = rand() % T;
		WaitForSingleObject(arg->empty, INFINITE);
		WaitForSingleObject(personal, INFINITE);
		enqueue(rnd);
		_tprintf(_T("I am thread %d and I wrote %d in the buffer.\n"), GetCurrentThreadId(), rnd);
		ReleaseSemaphore(personal, 1, NULL);
		ReleaseSemaphore(arg->full, 1, NULL);
		Sleep(rndTime);
	}
	

	return 0;
}


DWORD WINAPI consumer(LPVOID parameter) {

	THREAD_ARG	*arg = (THREAD_ARG*)parameter;
	HANDLE		personal;
	DWORD		m, rndTime;
	
	srand(time(NULL));

	personal = CreateSemaphore(NULL, 1, C, NULL);

	do{

		rndTime = rand() % T;
		WaitForSingleObject(arg->full, INFINITE);
		WaitForSingleObject(personal, INFINITE);
		m = dequeue();
		_tprintf(_T("I am thread %d and I read %d from the buffer.\n"), GetCurrentThreadId(), m);
		ReleaseSemaphore(personal, 1, NULL);
		ReleaseSemaphore(arg->empty, 1, NULL);
		Sleep(rndTime);

	} while (m != MAX_RAN);

	return 0;
}

void enqueue(DWORD val) {
	buffer[in] = val;
	in = (in + 1) % N;
	return;
}

DWORD dequeue() {
	DWORD val = buffer[out];
	out = (out + 1) % N;
	return val;
}