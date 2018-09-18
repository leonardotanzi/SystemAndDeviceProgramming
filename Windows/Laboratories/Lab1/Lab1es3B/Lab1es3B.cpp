// Lab1es3B.cpp: definisce il punto di ingresso dell'applicazione console.
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

#define BUF_SIZE 30

typedef struct {
	LPTSTR						name[BUF_SIZE];
	LPTSTR						surname[BUF_SIZE];
	DWORD						id;
	LONG						regNumber;
	DWORD						exMark;
} typeStruct;



INT _tmain(INT argc, LPTSTR argv[]) {

	TCHAR			choice, tmpChar;
	HANDLE			hIn;
	typeStruct		tmp;
	DWORD			parameter, nOut, nRecords = 0;
	OVERLAPPED		ov = {0, 0, 0, 0, NULL};
	LARGE_INTEGER	FilePos;

	hIn = CreateFile(argv[1], GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);


	do{
		ReadFile(hIn, &tmp, sizeof(tmp), &nOut, &ov);
		_tprintf(_T("Student %d: Name %s, Surname %s, Registration Number %ld, Mark %d.\n"),
			tmp.id, tmp.name, tmp.surname, tmp.regNumber, tmp.exMark);
		nRecords++;
		FilePos.QuadPart = nRecords * sizeof(tmp);
		ov.Offset = FilePos.LowPart;
		ov.OffsetHigh = FilePos.HighPart;
	} while (nOut > 0);

	_tprintf(_T("NREC IS %d.\n"), nRecords );

	FilePos.QuadPart = 0;
	ov.Offset = FilePos.LowPart;
	ov.OffsetHigh = FilePos.HighPart;

	while (1) {
		_tprintf(_T("User Menu:\n1) Press R + n\n2) Press W + n\n3) Press E\n"));
		_tscanf(_T("%c %d"), &choice, &parameter);
		_tscanf(_T("%c"), &tmpChar);

		FilePos.QuadPart = (parameter - 1) * sizeof(tmp); //We change the position of the pointer.
		ov.Offset = FilePos.LowPart;
		ov.OffsetHigh = FilePos.HighPart;
	
		switch (choice){
		case 'R':
			ReadFile(hIn, &tmp, sizeof(tmp), &nOut, &ov);
			_tprintf(_T("Student %d: Name %s, Surname %s, Registration Number %ld, Mark %d.\n"),
						tmp.id, tmp.name, tmp.surname, tmp.regNumber, tmp.exMark);
			break;

		case 'W':

			tmp.id = parameter;
			_tprintf(_T("Insert Name of student %d.\n"), parameter);
			_tscanf(_T("%s"), tmp.name);
			_tprintf(_T("Insert Surname of student %d.\n"), parameter);
			_tscanf(_T("%s"), tmp.surname);
			_tprintf(_T("Insert RegNumb of student %d.\n"), parameter);
			_tscanf(_T("%ld"), &tmp.regNumber);
			_tprintf(_T("Insert Mark of student %d.\n"), parameter);
			_tscanf(_T("%d"), &tmp.exMark);
			_tscanf(_T("%c"), &tmpChar);

			if (parameter > nRecords) {
				FilePos.QuadPart = nRecords * sizeof(tmp);
				ov.Offset = FilePos.LowPart;
				ov.OffsetHigh = FilePos.HighPart;
				nRecords++;
			}
			WriteFile(hIn, &tmp, sizeof(tmp), &nOut, &ov);
			break;

		case 'E':
			break;
		}
	}
	FilePos.QuadPart = 0;
	ov.Offset = FilePos.LowPart;
	ov.OffsetHigh = FilePos.HighPart;
	while ((ReadFile(hIn, &tmp, sizeof(tmp), &nOut, &ov)) && nOut > 0) {
		_tprintf(_T("%d %ld %s %s %d\n"), tmp.id, tmp.regNumber, tmp.name, tmp.surname, tmp.exMark);
	}

	return 0;
}
