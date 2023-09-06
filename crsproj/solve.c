#include "solve.h"

double** coeffMatrix;		// Матрица для хранения кэффициентов
double* constVector;		// Вектор для хранения столбца свободных членов
double* solutionVector;		// Вектор для хранения решения системы	

double** lu_matrix;			// Матрица для хранения LU разложения
int* permutVector;			// Вектор для хранения перестановок строк при LU разложении


BOOL solveSystem(HWND **coeffEdtCtrls, HWND *constEdtCtrls, HWND *solutionEdtCtrls, int size, int precision) {
	static int realPrecision;		// Получаемая точность решения 

	int changesCount;				// Число элементов новой матрицы, которые отличны от элементов предыдущей 
	if (!fillSystemMatrix(coeffEdtCtrls, constEdtCtrls, solutionEdtCtrls, size, &changesCount)) {
		return FALSE;
	}

	if (changesCount > 0) {											// Если элементы матрицы изменились
		if (lu_matrix == NULL) {
			lu_matrix = createEmptyMatrix(size, sizeof(double));
			if (lu_matrix == NULL) {
				showErrMsgBox((LPCWSTR)L"Ошибка выделения памяти.");
				return FALSE;
			}
		}
		copySqrMatrixD(lu_matrix, coeffMatrix, size);

		double matrCondNum;											// LUdcmp вернет число обусловденности матрицы в эту переменную
		if (!LUdcmp(&matrCondNum, size)) {							// Производим LU разложение
			deleteAllFilledArrays(size - 1);
			return FALSE;
		}

		// Если 10^(-d) - порядок машинной точности, а число обусловленности матрицы
		// приблизительно равто 10^u, то точность решения = d - u цифр после точки
		realPrecision = (int)round(DBL_EPS_EXP - log10(matrCondNum));
		realPrecision = realPrecision > 0 ? realPrecision : 0;		// Если точность решения слишком мала, выводим только целую часть
	}

	if (realPrecision < precision) {								// Если требуемая точность больше полученной
		precision = realPrecision;									// то выводим решение с меньшей точностью
		showWarningMsgBox(L"Из-за числа обусловленности матрицы точность решения может быть снижена.");
	}

	solve(constVector, solutionVector, size);						// Решаем систему используя полученное LU разложение							
	
	char chTxt[SLTN_NUM_MAXLEN] = "";
	for (int i = 0; i < size; ++i) {								// Заполняем edit controls решения системы
		snprintf(chTxt, SLTN_NUM_MAXLEN, "%.*f", precision, solutionVector[i]);
		size_t outsize;
		WCHAR wchTxt[SLTN_NUM_MAXLEN] = L"";
		mbstowcs_s(&outsize, wchTxt, SLTN_NUM_MAXLEN, chTxt, SLTN_NUM_MAXLEN - 1);	// char в WCHAR
		SetWindowText(solutionEdtCtrls[i], wchTxt);
	}
	return TRUE;
}

BOOL fillSystemMatrix(HWND** coeffEdtCtrls, HWND* constEdtCtrls, HWND* solutionEdtCtrls, int size, int* retChanges) {
	if (!allocMemForSystem(size)) {
		showErrMsgBox((LPCWSTR)L"Ошибка выделения памяти.");
		return FALSE;
	}
	
	WCHAR textBuff[MAX_NUM_LEN] = L"";			// Буффер для чтения числа из edit control в виде строки
	double numFromString = 0;				
	int changesCount = 0;						// Количество элементов отличающихся от элементов предыдущей матрицы 
	double rowSum;

	for (int i = 0; i < size; ++i) {
		if (coeffMatrix[i] == NULL) {
			coeffMatrix[i] = lssalloc(size, sizeof(double));
			if (coeffMatrix[i] == NULL) {
				deleteAllFilledArrays(i - 1);
				return FALSE;
			}
		}

		rowSum = 0.0;
		for (int j = 0; j < size; ++j) {
			GetWindowText(coeffEdtCtrls[i][j], textBuff, MAX_NUM_LEN);
			numFromString = _wtof(textBuff);
			rowSum += fabs(numFromString);		// Подсчет суммы элементов строки для проверки матрицы на вырожденность
			if (numFromString != coeffMatrix[i][j]) {
				coeffMatrix[i][j] = numFromString;
				++changesCount;
			}
		}
		if (rowSum == 0.0) {
			deleteAllFilledArrays(i);
			showWarningMsgBox((LPCWSTR)L"Вырожденная матрица системы.");
			return FALSE;
		}

		GetWindowText(constEdtCtrls[i], textBuff, MAX_NUM_LEN);
		numFromString = _wtof(textBuff);
		if (numFromString != constVector[i]) {
			constVector[i] = numFromString;
		}
	}

	*retChanges = changesCount;
	return TRUE;
}

BOOL allocMemForSystem(int size) {
	if (coeffMatrix == NULL) {
		coeffMatrix = calloc(size, sizeof(double*));
		if (coeffMatrix == NULL) {
			return FALSE;
		}

		constVector = calloc(size, sizeof(double));
		if (constVector == NULL) {
			free(coeffMatrix);
			return FALSE;
		}

		solutionVector = calloc(size, sizeof(double));
		if (solutionVector == NULL) {
			free(coeffMatrix);
			free(constVector);
			return FALSE;
		}
	}

	return TRUE;
}

BOOL LUdcmp(double *retMatrixCondNum, int size) {
	int i, imax, j, k;
	double big, temp;
	BOOL result = FALSE;

	// Вектор для хранения масштабирования строк
	// (используется для выбора главного элемента матрицы "implicit pivoting")
	double* scalingVector = lssalloc(size, sizeof(double));		
	if (scalingVector == NULL) {
		return result;
	}
	
	double rowSum;
	double infNorm = 0.0;
	for (i = 0; i < size; ++i) {
		big = 0.0;
		rowSum = 0.0;
		for (j = 0; j < size; ++j) {
			rowSum += fabs(lu_matrix[i][j]);			// Подсчет суммы элементов строки для вычисления нормы матрицы
			temp = fabs(lu_matrix[i][j]);
			if (temp > big) {
				big = temp;
			}
		}

		if (rowSum > infNorm) {
			infNorm = rowSum;
		}
		scalingVector[i] = 1.0 / big;					// Сохраняем в вектор масштабирование данной строки		
	}

	permutVector = lssalloc(size, sizeof(int));
	if (permutVector == NULL) {
		free(scalingVector);
		return result;
	}
	for (k = 0; k < size; ++k) {
		big = 0.0;
		imax = k;
		for (i = k; i < size; ++i) {					// Выбор ведущего элемента матрицы "implicit pivoting"
			temp = scalingVector[i] * fabs(lu_matrix[i][k]);
			if (temp > big) {
				big = temp;
				imax = i;
			}
		}
		if (k != imax) {								// Если ведущий элемент найден в другой строке
			for (j = 0; j < size; ++j) {
				temp = lu_matrix[imax][j];				// Переставляем строки
				lu_matrix[imax][j] = lu_matrix[k][j];
				lu_matrix[k][j] = temp;
			}
			scalingVector[imax] = scalingVector[k];		// Меняем значения в векторе масштабирования строк
		}
		permutVector[k] = imax;							// Сохраняем перестановку в соответствующий вектор

		if (fabs(lu_matrix[k][k]) < DBL_EPSILON) {		// Если нет ненулевого ведущего элемента, то матрица вырождена
			showWarningMsgBox((LPCWSTR)L"Вырожденная матрица системы.");
			free(scalingVector);
			return result;
		}

		for (i = k + 1; i < size; ++i) {
			lu_matrix[i][k] /= lu_matrix[k][k];			// Получаем элемент L матрицы
			temp = lu_matrix[i][k];
			for (j = k + 1; j < size; ++j) {			// Получаем элементы U матрицы
				lu_matrix[i][j] -= temp * lu_matrix[k][j];
			}
		}
	}

	// Вычисляем норму обратной матрицы и помещаем её значение в переменную invMatrNorm
	double invMatrInfNorm = 0.0;	
	if (getInvMatrixInfNorm(&invMatrInfNorm, lu_matrix, size)) {		
		*retMatrixCondNum = infNorm * invMatrInfNorm;
		result = TRUE;
	}

	free(scalingVector);
	return result;
}

void solve(double *b, double *x, int size) {
	int i, ii = 0, ip, j;
	double sum;
	for (i = 0; i < size; ++i) {
		x[i] = b[i];
	}
	
	for (i = 0; i < size; ++i) {
		ip = permutVector[i];					// Получаем индекс свободного члена с учетом перестановки в LUdcmp
		sum = x[ip];
		x[ip] = x[i];
		if (ii != 0) {
			for (j = ii - 1; j < i; ++j) {		// и проводим прямую подстановку (решение L*y = b)
				sum -= lu_matrix[i][j] * x[j];
			}
		}
		else if (sum != 0.0) {					// Если значение текущей переменной решения системы не равно нулю,
			ii = i + 1;							// то на следующей итерации используем её для прямого хода 
		}

		x[i] = sum;
	}
	for (i = size - 1; i >= 0; --i) {
		sum = x[i];
		for (j = i + 1; j < size; ++j) {		// Обратная подстановка (решение U*x = y)
			sum -= lu_matrix[i][j] * x[j];
		}
		x[i] = sum / lu_matrix[i][i];
	}
}

BOOL matrixSolve(double **b, double **x, int size) {
	int i, j;

	double* currSolutionColumn = lssalloc(size, sizeof(double));
	if (currSolutionColumn == NULL) {
		return FALSE;
	}

	for (j = 0; j < size; ++j) {
		for (i = 0; i < size; ++i) {	// Решаем систему с текущим вектором свободных членов
			currSolutionColumn[i] = b[i][j];
		}
		solve(currSolutionColumn, currSolutionColumn, size); 
		for (i = 0; i < size; ++i) {	// Помещаем полученный вектор с решенем в столбец матрицы 
			x[i][j] = currSolutionColumn[i];
		}
	}

	free(currSolutionColumn);
	return TRUE;
}

BOOL getInvMatrixInfNorm(double* normReturn, double** matrix, int size) {
	BOOL result = FALSE;
	double** matrixCopy = createEmptyMatrix(size, sizeof(double));
	if (matrixCopy == NULL) {
		return result;
	}

	int i, j;

	// Создаем единичную матрицу
	for (i = 0; i < size; ++i) {
		for (j = 0; j < size; ++j) {
			matrixCopy[i][j] = 0.0;
		}
		matrixCopy[i][i] = 1.0;
	}

	// Решаем матричное уравнение для единичной матрицы и получаем обратную матрицу
	if (matrixSolve(matrixCopy, matrixCopy, size)) {
		double matrixInfNorm = 0.0;
		double sum;
		for (i = 0; i < size; ++i) {
			sum = 0.0;
			for (j = 0; j < size; ++j) {
				sum += fabs(matrixCopy[i][j]);		// Получаем сумму по строке матрицы
			}
			if (sum > matrixInfNorm) {				// Выбираем макимальную сумму (inf-норма) 
				matrixInfNorm = sum;
			}
		}
		*normReturn = matrixInfNorm;
		result = TRUE;
	}

	for (i = 0; i < size; ++i) {
		free(matrixCopy[i]);
	}
	free(matrixCopy);
	return result;
}

void* lssalloc(size_t count, size_t size) {
	void* ptr = calloc(count, size);
	if (ptr == NULL) {
		showErrMsgBox((LPCWSTR)L"Ошибка выделения памяти.");
	}
	return ptr;
}

void* createEmptyMatrix(size_t count, size_t size) {	
	int i = 5;
	int b = 3;
	int s = b++ + ++i;
	
	void** matrix = calloc(count, sizeof(void*));
	if (matrix == NULL) {
		return NULL;
	}

	for (int i = 0; i < count; ++i) {
		matrix[i] = calloc(count, size);

		if (matrix[i] == NULL) {
			for (i = i - 1; i >= 0; --i) {
				free(matrix[i]);
			}
			free(matrix);
			return NULL;
		}
	}

	return matrix;
}

void copySqrMatrixD(double **destMatr, double **srcMatr, int size) {
	for (int i = 0; i < size; ++i) {
		memcpy_s(
			destMatr[i], size * sizeof(double),
			srcMatr[i], size * sizeof(double));
	}
}

void deleteAllFilledArrays(int toIndex) {
	if (coeffMatrix != NULL) {
		for (int i = toIndex; i >= 0; --i) {
			free(coeffMatrix[i]);
		}
		free(coeffMatrix);
		free(constVector);
		free(solutionVector);
		coeffMatrix = NULL, constVector = NULL, solutionVector = NULL;
	}

	if (lu_matrix != NULL) {
		for (int i = toIndex; i >= 0; --i) {
			free(lu_matrix[i]);
		}
		free(lu_matrix);
		lu_matrix = NULL;
	}

	if (permutVector != NULL) {
		free(permutVector);
		permutVector = NULL;
	}
}