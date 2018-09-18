// Exam24012014.cpp: definisce il punto di ingresso dell'applicazione console.
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

	TCHAR cF[15];
	TCHAR name[30];
	TCHAR surname[30];
	TCHAR sex;
	TCHAR time[6];
	DWORD timeToRegister;
	DWORD timeToVote;

} VOTERS;


typedef struct {

	TCHAR cF[15];
	TCHAR votingStation[20];
	TCHAR time[6];

} OUTPUT;

DWORD			queueSize, nVotingStation, in = 0, out = 0, *lastLeave;
HANDLE			hIn, hOut, full, empty, pc, maleFemale;
VOTERS			*votersQueue;

DWORD WINAPI maleVoters(void*);
DWORD WINAPI femaleVoters(void*);
DWORD WINAPI votingStation(void* arg);
void enqueue(VOTERS v);
VOTERS dequeue();
DWORD toMin(TCHAR t[]);
LPTSTR toString(DWORD n);

INT _tmain(INT argc, LPTSTR argv[]) {

	PHANDLE		handles;
	PDWORD		stationID;

	if (argc < 5) {
		perror("Not enough args.\n");
		return 1;
	}
	else {
		queueSize = _ttoi(argv[2]);
		nVotingStation = _ttoi(argv[3]);
	}

	handles = (PHANDLE)malloc((2 + nVotingStation) * sizeof(HANDLE));
	stationID = (PDWORD)malloc(nVotingStation * sizeof(DWORD));
	lastLeave = (PDWORD)malloc(nVotingStation * sizeof(DWORD));

	hIn = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	hOut = CreateFile(argv[4], GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	full = CreateSemaphore(NULL, 0, queueSize, NULL);
	empty = CreateSemaphore(NULL, queueSize, queueSize, NULL);
	pc = CreateSemaphore(NULL, 1, queueSize, NULL);
	maleFemale = CreateSemaphore(NULL, 0, 1, NULL);

	handles[0] = CreateThread(NULL, 0, maleVoters, NULL, 0, NULL);
	handles[1] = CreateThread(NULL, 0, femaleVoters, NULL, 0, NULL);

	for (int i = 0; i < nVotingStation; i++) {
		stationID[i] = i;
		handles[i + 2] = CreateThread(NULL, 0, votingStation, (void*)&stationID[i], NULL);
	}

	WaitForMultipleObjects(nVotingStation + 2, handles, TRUE, INFINITE);

    return 0;
}

DWORD WINAPI maleVoters(void*) {

	VOTERS		voter;
	DWORD		nIn;

	while (ReadFile(hIn, &voter, sizeof(voter), &nIn, 0) && nIn > 0) {

		if (voter.sex == 'M') {
			Sleep(voter.timeToRegister * 1000);

			WaitForSingleObject(empty, INFINITE);
			WaitForSingleObject(pc, INFINITE);

			enqueue(voter);

			ReleaseSemaphore(pc, 1, NULL);
			ReleaseSemaphore(full, 1, NULL);
		}
	}
	WaitForSingleObject(maleFemale, INFINITE);

	WaitForSingleObject(empty, INFINITE);
	WaitForSingleObject(pc, INFINITE);

	voter.sex = 'E';
	enqueue(voter);

	ReleaseSemaphore(pc, 1, NULL);
	ReleaseSemaphore(full, 1, NULL);


}


DWORD WINAPI femaleVoters(void*) {
	
	VOTERS		voter;
	DWORD		nIn;

	while (ReadFile(hIn, &voter, sizeof(voter), &nIn, 0) && nIn > 0) {

		if (voter.sex == 'F') {
			Sleep(voter.timeToRegister * 1000);

			WaitForSingleObject(empty, INFINITE);
			WaitForSingleObject(pc, INFINITE);

			enqueue(voter);

			ReleaseSemaphore(pc, 1, NULL);
			ReleaseSemaphore(full, 1, NULL);
		}
	}
	ReleaseSemaphore(maleFemale, 1, NULL);

	return 0;
}

DWORD WINAPI votingStation(void* arg) {

	DWORD		id = *(DWORD*)arg, minutes, nOut;
	VOTERS		voter;	
	OUTPUT		outputStr;

	while (1) {
		WaitForSingleObject(full, INFINITE);
		WaitForSingleObject(pc, INFINITE);

		voter = dequeue();

		ReleaseSemaphore(pc, 1, NULL);
		ReleaseSemaphore(empty, 1, NULL);

		if (voter.sex == 'E') {
			break;
		}

		minutes = toMin(voter.time);

		if (minutes < lastLeave[id]) {
			minutes = lastLeave[id];
		}
		minutes += voter.timeToRegister + voter.timeToVote;
		lastLeave[id] = minutes;

		_stprintf(outputStr.votingStation, _T("Voting_Station_%d"), id);
		_tcscpy(outputStr.cF, voter.cF);
		_tcscpy(outputStr.time, toString(minutes));
		
		//cs
		WriteFile(hOut, &outputStr, sizeof(outputStr), &nOut, 0);
		//cs

	}

}


DWORD toMin(TCHAR t[]) {

	TCHAR		h[3], m[3];
	DWORD		i, ho, mi;

	for (i = 0; i < 2; i++) {
		h[i] = t[i];
	}
	h[2] = '\0';

	for (i = 3; i < 5; i++) {
		m[i] = t[i];
	}
	m[2] = '\0';

	ho = _ttoi(h);
	mi = _ttoi(m);

	return (ho * 60 + mi);
}

LPTSTR toString(DWORD n) {
	
	DWORD		h, m, i;
	TCHAR		ho[3], mi[3];
	TCHAR		final[6];

	h = (n / 60);
	m = (n % 60);

	_itot(h, ho, 10);
	_itot(m, mi, 10);

	for (i = 0; i < 2; i++) {
		final[i] = ho[i];
	}
	final[i] = ':';
	for (i = 3; i < 5; i++) {
		final[i] = mi[i - 3];
	}
	
	return final;
}


void enqueue(VOTERS v){
	//no need for synch because they already are in mutual exclusion
	votersQueue[in] = v;
	in = (in + 1) % queueSize;
	return;
}

VOTERS dequeue() {
	VOTERS v;
	v = votersQueue[out];
	out = (out + 1) % queueSize;
	return v;
}

