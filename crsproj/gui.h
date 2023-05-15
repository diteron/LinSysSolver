#pragma once

#include <Windows.h>
#include <stdlib.h>
#include <commctrl.h>

//#include <crtdbg.h>
//#define _CRTDBG_MAP_ALLOC

#define MWND_WIDTH 1000		// Ширина главного окна
#define MWND_HEIGHT 640		// Высота главного окна
#define MAX_NUM_LEN 17		// Максимальный размер числа для матрицы коэффициетнов и столбца свободных членов (включая '-' и '.')
#define PREC_NUM_LEN 2		// Максимальный размер числа для ввода количества чисел после десятичной точки
#define MAX_PREC 14			// Максимальное число знаков после запятой у полученного решения системы 
#define START_X_POS 10		// x координата первого элемента матрицы коэффициентов		
#define START_Y_POS 70		// y координата первого элемента матрицы коэффициентов		
#define UNSELECTED -1		// Начальная позиция variablesNumDropdown
#define ONECHAR_BUFF_SIZE 2	// Размер текстового буффера под один символ
#define YSPACE_BTWN_EC 25	// Расстояние между edit conrols по высоте
#define XSPACE_BTWN_EC 85	// Расстояние между edit conrols по ширине
#define YSPACE_AFT_ECTXT 20	// Расстояние после текста static control по высоте
#define EC_HEIGHT 20		// Высота всех edit controls
#define SYSTEM_EC_WIDTH 80	// Ширина edit control матрицы СЛАУ
#define SOLVE_EC_WIDTH 140	// Ширина edit control столбца с решением


LRESULT CALLBACK mainWinProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

void comctl32Check();
void getSysFont();

void createVarsNumDropdown(HWND parentWindow);
void createPrecUpDownCtrl(HWND parentWindow);

BOOL failedDropDwnChange(HWND drpDwnHwnd, HWND parentHwnd);
BOOL failedSolve();
int getPrecision();

BOOL createSystemEditCtrls(HWND parentWindow, int size, int xPos, int yPos);
LRESULT systemEditCtrlsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
BOOL incorrectDotPos(HWND hWnd);
BOOL incorrectMinusPos(HWND hWnd);
BOOL сharBeforeMinus(HWND hWnd, WPARAM wParam);

BOOL createConstEditCtrls(HWND parentWindow, int size, int xPos, int yPos);
BOOL createSolutionEditCtrls(HWND parentWindow, int size, int xPos, int yPos);
void setSolutionEditCtrlsVisible();

void freeEditCtlsMatrix(int toIndex);
void deleteEditCtrls();

void showErrMsgBox(LPCWSTR errMsg);
void showWarningMsgBox(LPCWSTR errMsg);
