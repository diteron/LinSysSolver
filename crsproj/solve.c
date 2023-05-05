#include "solve.h"

double** coeffMatrix;
double** lMatrix;
double** uMatrix;

double* constColumn;
double* solutionColumn;
int currentVariablesNum;

BOOL fillCoeffMatrix(HWND **editCtrls, int size) {
	if (coeffMatrix == NULL) {
		coeffMatrix = calloc(size, sizeof(double*));
		if (coeffMatrix == NULL) return FALSE;
	}
	
	WCHAR textBuff[17] = L"";
	for (int i = 0; i < size; ++i) {
		coeffMatrix[i] = calloc(size, sizeof(double));
		if (coeffMatrix[i] == NULL) {
			for (i = i - 1; i >= 0; --i) {
				free(coeffMatrix[i]);
			}
			return FALSE;
		}
		for (int j = 0; j < size; ++j) {
			GetWindowText(editCtrls[i][j], textBuff, 17);
			coeffMatrix[i][j] = _wtof(textBuff);
		}
	}

	currentVariablesNum = size;
	for (int i = 0; i < size; ++i) {
		free(coeffMatrix[i]);
	}
	free(coeffMatrix);
	coeffMatrix = NULL;

	return TRUE;
}