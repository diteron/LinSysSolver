#pragma once
#include <Windows.h>
#include <stdlib.h>
#include <commctrl.h>


LRESULT CALLBACK mainWinProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam);
void createVarsNumDropdown(HWND parentWindow);
BOOL createCoeffEditCtrls(HWND parentWindow, int size, int xPos, int yPos);
LRESULT editCtrlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL createConstEditCtrls(HWND parentWindow, int size, int xPos, int yPos);
BOOL createSolutionEditCtrls(HWND parentWindow, int size, int xPos, int yPos);
void setSolutionEditCtrlsVisible();
void deleteEditCtrls();
void getSysFont();
void comctl32Check();
