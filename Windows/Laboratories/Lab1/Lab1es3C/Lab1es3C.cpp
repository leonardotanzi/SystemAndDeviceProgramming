// Lab1es3C.cpp: definisce il punto di ingresso dell'applicazione console.
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

	TCHAR		choice, tmpChar;
	HANDLE		hIn;
	typeStruct	tmp;
	DWORD		parameter, nOut, nRecords = 0;

	hIn = CreateFile(argv[1], GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	while ((ReadFile(hIn, &tmp, sizeof(tmp), &nOut, NULL)) && nOut > 0) {
		nRecords++;
	}
	SetFilePointer(hIn, 0, NULL, FILE_BEGIN);

	while (1) {
		_tprintf(_T("User Menu:\n1) Press R + n\n2) Press W + n\n3) Press E\n"));
		_tscanf(_T("%c %d"), &choice, &parameter);
		_tscanf(_T("%c"), &tmpChar);

		if (choice == 'R') {
			while ((ReadFile(hIn, &tmp, sizeof(tmp), &nOut, NULL)) && nOut > 0) {
				if (tmp.id == parameter) {
					_tprintf(_T("Student %d: Name %s, Surname %s, Registration Number %ld, Mark %d.\n"),
						tmp.id, tmp.name, tmp.surname, tmp.regNumber, tmp.exMark);
				}
			}
		}
		else if (choice == 'W') {

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
				SetFilePointer(hIn, nRecords * sizeof(typeStruct), NULL, FILE_BEGIN);
				nRecords++;
			}
			else {
				SetFilePointer(hIn, (parameter - 1) * sizeof(typeStruct), NULL, FILE_BEGIN);
			}
			WriteFile(hIn, &tmp, sizeof(tmp), &nOut, NULL);
		}
		else {
			break;
		}

		SetFilePointer(hIn, 0, NULL, FILE_BEGIN);

		while ((ReadFile(hIn, &tmp, sizeof(tmp), &nOut, NULL)) && nOut > 0) {
			_tprintf(_T("%d %ld %s %s %d\n"), tmp.id, tmp.regNumber, tmp.name, tmp.surname, tmp.exMark);
		}

		SetFilePointer(hIn, 0, NULL, FILE_BEGIN);

	}

	return 0;
}

