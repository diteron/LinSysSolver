#pragma once

#include <Windows.h>
#include <stdlib.h>
#include <commctrl.h>

//#include <crtdbg.h>
//#define _CRTDBG_MAP_ALLOC

#define MWND_WIDTH 1000		// ������ �������� ����
#define MWND_HEIGHT 640		// ������ �������� ����
#define MAX_NUM_LEN 17		// ������������ ������ ����� ��� ������� ������������� � ������� ��������� ������ (������� '-' � '.')
#define PREC_NUM_LEN 2		// ������������ ������ ����� ��� ����� ���������� ����� ����� ���������� �����
#define MAX_PREC 14			// ������������ ����� ������ ����� ������� � ����������� ������� ������� 
#define START_X_POS 10		// x ���������� ������� �������� ������� �������������		
#define START_Y_POS 70		// y ���������� ������� �������� ������� �������������		
#define UNSELECTED -1		// ��������� ������� variablesNumDropdown
#define ONECHAR_BUFF_SIZE 2	// ������ ���������� ������� ��� ���� ������
#define YSPACE_BTWN_EC 25	// ���������� ����� edit conrols �� ������
#define XSPACE_BTWN_EC 85	// ���������� ����� edit conrols �� ������
#define YSPACE_AFT_ECTXT 20	// ���������� ����� ������ static control �� ������
#define EC_HEIGHT 20		// ������ ���� edit controls
#define SYSTEM_EC_WIDTH 80	// ������ edit control ������� ����
#define SOLVE_EC_WIDTH 140	// ������ edit control ������� � ��������


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
BOOL �harBeforeMinus(HWND hWnd, WPARAM wParam);

BOOL createConstEditCtrls(HWND parentWindow, int size, int xPos, int yPos);
BOOL createSolutionEditCtrls(HWND parentWindow, int size, int xPos, int yPos);
void setSolutionEditCtrlsVisible();

void freeEditCtlsMatrix(int toIndex);
void deleteEditCtrls();

void showErrMsgBox(LPCWSTR errMsg);
void showWarningMsgBox(LPCWSTR errMsg);
