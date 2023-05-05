#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib,"comctl32.lib")

#include "gui.h"
#include "solve.h"

#define EC_TEXTLEN	17		// Максимальный размер числа в Edit Control (включая '-' и '.')
#define START_X_POS	10
#define START_Y_POS	70

HWND** coeffEditCtrls;		// Матрица коэффициентов
HWND coeffEditCtrlsText;

HWND* constEditCtrls;			// Столбец свободных членов
HWND constEditCtrlsText;	

HWND solveButton;
HWND solutionText;
HWND* solutionEditCtrls = NULL;	// Столбец решения системы

HWND variablesNumDropdown;	
int currentDropdownElem = -1;

HFONT font;

int variablesNum = 0;
static WNDPROC OriginalEditProc = NULL;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR pCmdLine, _In_ int nCmdShow) {
	// Регистрация класса window
	WNDCLASS window;
	memset(&window, 0, sizeof(WNDCLASSA));
	window.lpszClassName = L"main window";
	window.lpfnWndProc = (WNDPROC)mainWinProc;
	window.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	window.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClass(&window);

	// Создание и вывод главного окна
	HWND mainWindow = CreateWindowEx(
		0, L"main window", L"LinSysSolver", 
		WS_OVERLAPPEDWINDOW,
		10, 10, 940, 680,
		NULL, NULL, hInstance, NULL);
	ShowWindow(mainWindow, SW_SHOWNORMAL);

	// Проверка загрузки библиотеки Comctl32.dll
	comctl32Check();
	// Получение системного шрифта
	getSysFont();

	createVarsNumDropdown(mainWindow);

	// Обработка сообщений окна программы
	MSG message;
	BOOL getMsgReturn = 0;
	while (getMsgReturn = GetMessage(&message, NULL, 0, 0)) {	
		if (getMsgReturn == -1) break;

		if (!IsDialogMessage(mainWindow, &message)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}

	return 0;
}

LRESULT CALLBACK mainWinProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	static BOOL solutionButtonPressed = FALSE;
	
	switch (uMsg) {
		case WM_DESTROY:
			if (coeffEditCtrls != NULL) {
				deleteEditCtrls();
			}
			DeleteObject(font);
			PostQuitMessage(0);
			break;

		case WM_COMMAND:
			if ((HWND)lParam == solveButton) {
				if (!solutionButtonPressed) {
					setSolutionEditCtrlsVisible();
					fillCoeffMatrix(coeffEditCtrls, variablesNum);
					solutionButtonPressed = TRUE;
				}
			}
			else if ((HWND)lParam == variablesNumDropdown) {
				if (HIWORD(wParam) == CBN_SELCHANGE) {
					int dropdownElemId = (int)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
					if (currentDropdownElem != dropdownElemId) {
						currentDropdownElem = dropdownElemId;
						if (coeffEditCtrls != NULL) {
							deleteEditCtrls(TRUE);
							solutionButtonPressed = FALSE;
						}
						variablesNum = dropdownElemId + 2;
						if (!createCoeffEditCtrls(hWnd, variablesNum, START_X_POS, START_Y_POS)) return 0;
					}
				}
			}
			break;
	
		case WM_GETMINMAXINFO: {
			LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
			lpMMI->ptMinTrackSize.x = 940;
			lpMMI->ptMinTrackSize.y = 680;
			break;
		}		
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void createVarsNumDropdown(HWND parentWindow) {
	HWND dropdownText = CreateWindow(
		L"static", L"Число переменных в системе:",
		WS_VISIBLE | WS_CHILD,
		START_X_POS, 20, 170, 25,
		parentWindow, NULL, NULL, NULL);
	SendMessage(dropdownText, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
	
	variablesNumDropdown = CreateWindow(
		L"combobox", L"",
		WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
		START_X_POS + 180, 20, 70, 30,
		parentWindow, NULL, NULL, NULL);
	SendMessage(variablesNumDropdown, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));

	SendMessage(variablesNumDropdown, CB_ADDSTRING, 0, (LPARAM)L"2");	// Создание элементов выпадающего списка
	SendMessage(variablesNumDropdown, CB_ADDSTRING, 0, (LPARAM)L"3");
	SendMessage(variablesNumDropdown, CB_ADDSTRING, 0, (LPARAM)L"4");
	SendMessage(variablesNumDropdown, CB_ADDSTRING, 0, (LPARAM)L"5");
	SendMessage(variablesNumDropdown, CB_ADDSTRING, 0, (LPARAM)L"6");
	SendMessage(variablesNumDropdown, CB_ADDSTRING, 0, (LPARAM)L"7");
}

BOOL createCoeffEditCtrls(HWND parentWindow, int size, int xPos, int yPos) {
	coeffEditCtrls = calloc(size, sizeof(HWND*));
	if (coeffEditCtrls == NULL) return FALSE;

	coeffEditCtrlsText = CreateWindow(
		L"static", L"Матрица коэффициентов:",
		WS_VISIBLE | WS_CHILD,
		xPos, yPos, 180, 20,
		parentWindow, NULL, NULL, NULL);
	SendMessage(coeffEditCtrlsText, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
	yPos += 20;

	WNDPROC oldProc;
	int editCtrlsEnd_xPos = xPos;
	for (int i = 0; i < size; ++i) {
		coeffEditCtrls[i] = calloc(size, sizeof(HWND));
		if (coeffEditCtrls[i] == NULL) {
			for (i = i - 1; i >= 0; --i) {
				free(coeffEditCtrls[i]);
			}
			return FALSE;
		}

		for (int j = 0; j < size; ++j) {
			coeffEditCtrls[i][j] = CreateWindowEx(
				WS_EX_CLIENTEDGE,
				L"edit", L"0",
				WS_VISIBLE | WS_CHILD | ES_RIGHT | ES_AUTOHSCROLL | WS_TABSTOP,
				xPos, yPos, 80, 20,
				parentWindow, NULL, NULL, NULL);

			oldProc = (WNDPROC)(SetWindowLongPtr(coeffEditCtrls[i][j], GWLP_WNDPROC, (LONG_PTR)(editCtrlProc)));
			if (OriginalEditProc == NULL) {		// Запомнить старую процедуру обработки edit control
				OriginalEditProc = oldProc;		// Она будет вызываться в конце функции editCtrlProc
			}

			SendMessage(coeffEditCtrls[i][j], EM_SETLIMITTEXT, EC_TEXTLEN, 0);
			SendMessage(coeffEditCtrls[i][j], WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
			xPos += 85;
		}
		editCtrlsEnd_xPos = xPos;
		xPos = START_X_POS;
		yPos += 25;
	}

	solveButton = CreateWindow(
		L"button", L"Решить",
		WS_VISIBLE | WS_CHILD,
		START_X_POS, yPos + 10, 80, 30,
		parentWindow, NULL, NULL, NULL);
	SendMessage(solveButton, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));

	if (!createConstEditCtrls(parentWindow, size, editCtrlsEnd_xPos + 100, START_Y_POS)) return FALSE;

	if (!createSolutionEditCtrls(parentWindow, variablesNum, START_X_POS + 100, yPos + 10)) return FALSE;

	return TRUE;
}

LRESULT CALLBACK editCtrlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_CHAR) {
		if (wParam == '.') {
			WCHAR buff[17] = L"";
			GetWindowText(hWnd, buff, 17);
			if (buff[0] == '\0') {	// Если поле пустое точку вводить нельзя
				return 0;
			}
			for (int i = 0; i < 17; ++i) {	// Точка может быть только одна
				if (buff[i] == '.') {
					return 0;
				}
			}
		}
		else if (wParam == '-') {
			DWORD from = 0, to = 0;
			SendMessage(hWnd, EM_GETSEL, (WPARAM)&from, (WPARAM)&to);
			if (from == 0) {  // Если текстовый курсор находится в начале (или текст выделяется с самого начала) минус вводить можно
				if (to == 0) {
					WCHAR buffer[2] = L"";
					SendMessage(hWnd, WM_GETTEXT, (WPARAM)2, (WPARAM)buffer);
					if (buffer[0] == '-') { // Минус может быть только один
						return 0;
					}
				}
			}
			else {
				return 0;
			}
		}

		if ((wParam >= '0' && wParam <= '9') || wParam == '.') {	// Запрет на ввод цифр и точки перед знаком '-'
			DWORD from = 0, to = 0;
			SendMessage(hWnd, EM_GETSEL, (WPARAM)&from, (WPARAM)&to);
			if (from == 0) {
				if (to == 0) {
					WCHAR buffer[2] = L"";
					SendMessage(hWnd, WM_GETTEXT, (WPARAM)2, (WPARAM)buffer);
					if (buffer[0] == '-') {
						return 0;
					}
				}
			}
		}

		if (!((wParam >= '0' && wParam <= '9')		// Запред ввода любых символов (или нажатия клавиш), кроме указанных
			|| wParam == '.'
			|| wParam == '-'
			|| wParam == VK_RETURN
			|| wParam == VK_DELETE
			|| wParam == VK_BACK)) {

			return 0;
		}
	}
	else if (uMsg == WM_CONTEXTMENU) {	// Запрет открытия контестного меню по клику ПКМ
		return 0;
	}

	return CallWindowProc(OriginalEditProc, hWnd, uMsg, wParam, lParam);
}

BOOL createConstEditCtrls(HWND parentWindow, int size, int xPos, int yPos) {
	constEditCtrlsText = CreateWindow(
		L"static", L"Столбец свободных членов:",
		WS_VISIBLE | WS_CHILD,
		xPos, yPos, 200, 20,
		parentWindow, NULL, NULL, NULL);
	SendMessage(constEditCtrlsText, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
	yPos += 20;

	constEditCtrls = calloc(size, sizeof(HWND));
	if (constEditCtrls == NULL) return FALSE;

	for (int i = 0; i < size; ++i) {
		constEditCtrls[i] = CreateWindowEx(
			WS_EX_CLIENTEDGE,
			L"edit", L"0",
			WS_VISIBLE | WS_CHILD | ES_RIGHT | ES_AUTOHSCROLL | WS_TABSTOP,
			xPos, yPos, 80, 20,
			parentWindow, NULL, NULL, NULL);

		SetWindowLongPtr(constEditCtrls[i], GWLP_WNDPROC, (LONG_PTR)(editCtrlProc));

		SendMessage(constEditCtrls[i], EM_SETLIMITTEXT, EC_TEXTLEN, 0);
		SendMessage(constEditCtrls[i], WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
		yPos += 25;
	}

	return TRUE;
}

BOOL createSolutionEditCtrls(HWND parentWindow, int size, int xPos, int yPos) {
	solutionText = CreateWindow(
		L"static", L"Решение системы:",
		WS_CHILD,
		xPos, yPos, 170, 25,
		parentWindow, NULL, NULL, NULL);
	SendMessage(solutionText, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));

	solutionEditCtrls = calloc(size, sizeof(HWND));
	if (solutionEditCtrls == NULL) return FALSE;

	yPos += 20;
	for (int i = 0; i < size; ++i) {
		solutionEditCtrls[i] = CreateWindowEx(
			WS_EX_CLIENTEDGE,
			L"edit", L"",
			WS_CHILD | ES_RIGHT | ES_AUTOHSCROLL | ES_READONLY,
			xPos, yPos, 110, 20,
			parentWindow, NULL, NULL, NULL);
		SendMessage(solutionEditCtrls[i], EM_SETLIMITTEXT, EC_TEXTLEN, 0);
		SendMessage(solutionEditCtrls[i], WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
		yPos += 25;
	}

	return TRUE;
}

void setSolutionEditCtrlsVisible() {
	ShowWindow(solutionText, SW_SHOWNORMAL);
	for (int i = 0; i < variablesNum; ++i) {
		ShowWindow(solutionEditCtrls[i], SW_SHOWNORMAL);
	}
}

void deleteEditCtrls() {
	for (int i = 0; i < variablesNum; ++i) {
		for (int j = 0; j < variablesNum; ++j) {
			DestroyWindow(coeffEditCtrls[i][j]);
		}
		free(coeffEditCtrls[i]);
		DestroyWindow(constEditCtrls[i]);
		DestroyWindow(solutionEditCtrls[i]);
	}
	free(coeffEditCtrls);
	free(constEditCtrls);
	free(solutionEditCtrls);

	DestroyWindow(solutionText);
	DestroyWindow(solveButton);
	DestroyWindow(coeffEditCtrlsText);
	DestroyWindow(constEditCtrlsText);
}

void getSysFont() {
	NONCLIENTMETRICS metrics;
	metrics.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0);
	font = CreateFontIndirect(&metrics.lfMessageFont);
}

void comctl32Check() {
	INITCOMMONCONTROLSEX initControls;
	initControls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	initControls.dwICC = ICC_ANIMATE_CLASS;
	InitCommonControlsEx(&initControls);
}