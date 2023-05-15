#pragma once

#include <stdio.h>
#include <math.h>
#include <float.h>
#include "gui.h"

#define SLTN_NUM_MAXLEN 120		// Максимальная длина числа получаемого при решении СЛАУ
#define DBL_EPS_EXP 16			// Порядок точности double в экспоненциальной записи ( 2.2204460492503131e-16 )

BOOL solveSystem(HWND** coeffEdtCtrls, HWND* constEdtCtrls, HWND* solutionEdtCtrls, int size, int precision);
BOOL fillSystemMatrix(HWND** coeffEdtCtrls, HWND* constEdtCtrls, HWND* solutionEdtCtrls, int size, int* retChanges);
BOOL allocMemForSystem(int size);
void deleteAllFilledArrays(int fromIndex);

// Тестовые функции
BOOL LUdcmp(double* retMatrixCondNum, int size);
void copySqrMatrixD(double** destMatr, double** srcMatr, int size);
void solve(double* b, double* x, int size);
int numOfDigitsInIntPart(double number);
BOOL improvePrec(double** coeffMatrCpy, int size);
//////////////

BOOL matrixSolve(double** b, double** x, int size);
BOOL getInvMatrixAbsNorm(double* normReturn, double** matrix, int size);

void* lssalloc(size_t num, size_t size);
void* createEmptyMatrix(size_t num, size_t size);
