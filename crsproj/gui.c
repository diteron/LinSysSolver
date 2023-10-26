#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib,"comctl32.lib")

#include "gui.h"
#include "solve.h"
#include "resource.h"


HWND mainWindow;

HWND** coeffEditCtrls;         // Матрица коэффициентов
HWND coeffEditCtrlsText;

HWND* constEditCtrls;          // Столбец свободных членов        
HWND constEditCtrlsText;

HWND* solutionEditCtrls;       // Столбец решения системы
HWND solveButton;
HWND solutionText;

HWND variablesNumDropdown;     // Дропдаун для выбора числа переменных
HWND precUpDwnEdtCtrl;         // Edit control для выбора числа знаков после запятой

HFONT font;                    // Системный шрифт

int variablesNum = 0;


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR pCmdLine, _In_ int nCmdShow) {
    // Регистрация класса window
    WNDCLASS window;
    memset(&window, 0, sizeof(WNDCLASSA));
    window.lpszClassName = L"main window";
    window.lpfnWndProc = (WNDPROC)mainWinProc;
    window.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    window.hCursor = LoadCursor(NULL, IDC_ARROW);
    window.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    RegisterClass(&window);

    // Создание и вывод главного окна
    mainWindow = CreateWindowEx(
        0, L"main window", L"LinSysSolver",
        WS_OVERLAPPEDWINDOW,
        10, 10, MWND_WIDTH, MWND_HEIGHT,
        NULL, NULL, hInstance, NULL);
    ShowWindow(mainWindow, SW_SHOWNORMAL);

    comctl32Load();        // Загрузка библиотеки Comctl32.dll
    getSysFont();          // Получение системного шрифта

    createVarsNumDropdown(mainWindow);
    createPrecUpDownCtrl(mainWindow);

    // Обработка сообщений окна программы
    MSG message;
    BOOL getMsgReturn = 0;
    while ((getMsgReturn = GetMessage(&message, NULL, 0, 0)) != 0) {
        if (getMsgReturn == -1) {
            break;
        }

        if (!IsDialogMessage(mainWindow, &message)) {    // Проверка для перехода на другой edit control по клавише TAB
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }

    return 0;
}

LRESULT CALLBACK mainWinProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:                                                // Закрытие окна
            if (coeffEditCtrls != NULL) {
                deleteEditCtrls();
                deleteAllFilledArrays(variablesNum - 1);
            }
            DeleteObject(font);
            PostQuitMessage(0);
            break;

        case WM_COMMAND:
            if ((HWND)lParam == solveButton) {                          // Обработка нажатия на кнопку "Решить"
                if (failedSolve()) {
                    return 0;
                }
            }
            else if ((HWND)lParam == variablesNumDropdown) {            // Обработка выбора значения в дропдауне числа переменных 
                if (HIWORD(wParam) == CBN_SELCHANGE) {
                    if (failedDropDwnChange((HWND)lParam, hWnd)) {
                        return 0;
                    }
                }
            }
            break;

        case WM_GETMINMAXINFO: {                                        // Установка минимального размера окна
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
            lpMMI->ptMinTrackSize.x = MWND_WIDTH;
            lpMMI->ptMinTrackSize.y = MWND_HEIGHT;
            break;
        }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void comctl32Load() {
    INITCOMMONCONTROLSEX initControls;
    initControls.dwSize = sizeof(INITCOMMONCONTROLSEX);
    initControls.dwICC = ICC_ANIMATE_CLASS;
    InitCommonControlsEx(&initControls);
}

void getSysFont() {
    NONCLIENTMETRICS metrics;
    metrics.cbSize = sizeof(NONCLIENTMETRICS);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0);
    font = CreateFontIndirect(&metrics.lfMessageFont);
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

    // Создание элементов выпадающего списка
    SendMessage(variablesNumDropdown, CB_ADDSTRING, 0, (LPARAM)L"2");
    SendMessage(variablesNumDropdown, CB_ADDSTRING, 0, (LPARAM)L"3");
    SendMessage(variablesNumDropdown, CB_ADDSTRING, 0, (LPARAM)L"4");
    SendMessage(variablesNumDropdown, CB_ADDSTRING, 0, (LPARAM)L"5");
    SendMessage(variablesNumDropdown, CB_ADDSTRING, 0, (LPARAM)L"6");
    SendMessage(variablesNumDropdown, CB_ADDSTRING, 0, (LPARAM)L"7");
    SendMessage(variablesNumDropdown, CB_ADDSTRING, 0, (LPARAM)L"8");
}

BOOL failedDropDwnChange(HWND drpDwnHwnd, HWND parentHwnd) {
    static int currDropDwnElem = UNSELECTED;
    
    int newDropDwnElemId = (int)SendMessage(drpDwnHwnd, CB_GETCURSEL, 0, 0);
    if (currDropDwnElem != newDropDwnElemId) {          // Если выбран другой элемент в дропдауне 
        currDropDwnElem = newDropDwnElemId;
        if (coeffEditCtrls != NULL) {                   // И были созданы edit controls для ввода СЛАУ,
            deleteEditCtrls();                          // то удаляем их
            deleteAllFilledArrays(variablesNum - 1);    // Удаление заполненных массивов при решении системы
        }
        variablesNum = newDropDwnElemId + 2;
        if (!createSystemEditCtrls(parentHwnd, variablesNum, START_X_POS, START_Y_POS)) {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL failedSolve() {
    int precision = getPrecision();
    if (precision >= 0 && precision <= MAX_PREC) {
        if (solveSystem(coeffEditCtrls, constEditCtrls, solutionEditCtrls, variablesNum, precision)) {
            setSolutionEditCtrlsVisible();
            return FALSE;
        }
    }
    else {
        showWarningMsgBox((LPCWSTR)L"Число цифр после десятичной точки\nдолжно быть в диапазоне от 0 до 15.");
    }

    return TRUE;
}

int getPrecision() {
    WCHAR precBuff[PREC_NUM_LEN + 1];
    GetWindowText(precUpDwnEdtCtrl, precBuff, PREC_NUM_LEN + 1);
    int precision = _wtoi(precBuff);

    return precision;
}

void createPrecUpDownCtrl(HWND parentWindow) {
    HWND updwnEdtCtrlText = CreateWindow(
        L"static", L"Число знаков после десятичной точки (0-15):",
        WS_VISIBLE | WS_CHILD,
        START_X_POS + 300, 20, 260, 20,
        parentWindow, NULL, NULL, NULL);
    SendMessage(updwnEdtCtrlText, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));

    precUpDwnEdtCtrl = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        L"edit", L"3",
        WS_VISIBLE | WS_CHILD | ES_RIGHT | ES_NUMBER,
        START_X_POS + 560, 20, SYSTEM_EC_WIDTH, EC_HEIGHT,
        parentWindow, NULL, NULL, NULL);
    SendMessage(precUpDwnEdtCtrl, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
    SendMessage(precUpDwnEdtCtrl, EM_SETLIMITTEXT, PREC_NUM_LEN, 0);

    HWND updwnCtrl = CreateWindowEx(
        WS_EX_LEFT | WS_EX_LTRREADING,
        UPDOWN_CLASS,
        NULL,
        WS_CHILDWINDOW | WS_VISIBLE
        | UDS_AUTOBUDDY | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK,
        0, 0, 0, 0,
        parentWindow, NULL, NULL, NULL);
    SendMessage(updwnCtrl, UDM_SETRANGE, 0, MAKELPARAM(MAX_PREC, 0));
    SendMessage(updwnCtrl, UDM_SETPOS, 0, 3);
}

BOOL createSystemEditCtrls(HWND parentWindow, int size, int xPos, int yPos) {
    coeffEditCtrls = calloc(size, sizeof(HWND*));
    if (coeffEditCtrls == NULL) {
        showErrMsgBox((LPCWSTR)L"Ошибка выделения памяти.");
        return FALSE;
    }

    coeffEditCtrlsText = CreateWindow(
        L"static", L"Матрица коэффициентов:",
        WS_VISIBLE | WS_CHILD,
        xPos, yPos, 180, 20,
        parentWindow, NULL, NULL, NULL);
    SendMessage(coeffEditCtrlsText, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));

    yPos += YSPACE_AFT_ECTXT;            // Переход к позиции для создания первого edit control матрицы коэффициентов

    for (int i = 0; i < size; ++i) {
        coeffEditCtrls[i] = calloc(size, sizeof(HWND));
        if (coeffEditCtrls[i] == NULL) {
            freeEditCtlsMatrix(i - 1);
            showErrMsgBox((LPCWSTR)L"Ошибка выделения памяти.");
            return FALSE;
        }

        xPos = START_X_POS;
        for (int j = 0; j < size; ++j) {
            coeffEditCtrls[i][j] = CreateWindowEx(
                WS_EX_CLIENTEDGE,
                L"edit", L"0",
                WS_VISIBLE | WS_CHILD | ES_RIGHT | ES_AUTOHSCROLL | WS_TABSTOP,
                xPos, yPos, SYSTEM_EC_WIDTH, EC_HEIGHT,
                parentWindow, NULL, NULL, NULL);
            SetWindowSubclass(coeffEditCtrls[i][j], systemEditCtrlsProc,
                0, 0);
            SendMessage(coeffEditCtrls[i][j], EM_SETLIMITTEXT, MAX_NUM_LEN, 0);
            SendMessage(coeffEditCtrls[i][j], WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
            xPos += XSPACE_BTWN_EC;
        }

        yPos += YSPACE_BTWN_EC;           // Переход вниз для создания следующей строки матрицы 
    }

    if (!createConstEditCtrls(parentWindow, size, xPos + 100, START_Y_POS)) {
        freeEditCtlsMatrix(variablesNum - 1);
        return FALSE;
    }

    if (!createSolutionEditCtrls(parentWindow, variablesNum, START_X_POS + 100, yPos + 10)) {
        freeEditCtlsMatrix(variablesNum - 1);
        free(constEditCtrls);
        return FALSE;
    }

    return TRUE;
}

LRESULT CALLBACK systemEditCtrlsProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {

    switch (uMsg) {
        case WM_CHAR:
            if (сharBeforeMinus(hWnd, wParam)) {
                return 0;
            }

            if (wParam == '.' && incorrectDotPos(hWnd)) {
                return 0;
            }

            if (wParam == '-' && incorrectMinusPos(hWnd)) {
                return 0;
            }

            if (!((wParam >= '0' && wParam <= '9')        // Запрет ввода любых других символов (и нажатия клавиш), кроме указанных
                    || wParam == '.'
                    || wParam == '-'
                    || wParam == VK_RETURN
                    || wParam == VK_BACK)) {
                return 0;
            }
            break;

        case WM_CONTEXTMENU:                              // Запрет открытия контекстного меню по клику ПКМ
            return 0;

        default:
            break;
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

BOOL сharBeforeMinus(HWND hWnd, WPARAM wParam) {
    DWORD from = 0, to = 0;
    SendMessage(hWnd, EM_GETSEL, (WPARAM)&from, (WPARAM)&to);
    if (from == 0) {                       // Если курсор находится в начале
        if (to == 0) {
            WCHAR buffer[ONECHAR_BUFF_SIZE] = L"";
            SendMessage(hWnd, WM_GETTEXT, (WPARAM)2, (WPARAM)buffer);
            if (buffer[0] == '-') {        // И в edit control есть знак минус
                return TRUE;               // возвращем TRUE, т.к. такой ввод некорректый
            }
        }
    }

    return FALSE;
}

BOOL incorrectDotPos(HWND hWnd) {
    WCHAR buff[MAX_NUM_LEN] = L"";
    GetWindowText(hWnd, buff, MAX_NUM_LEN);

    if (buff[0] == '\0') {                        // Если поле пустое точку вводить нельзя
        return TRUE;
    }

    for (int i = 0; i < MAX_NUM_LEN; ++i) {       // Точка может быть только одна
        if (buff[i] == '.') {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL incorrectMinusPos(HWND hWnd) {
    DWORD from = 0, to = 0;
    SendMessage(hWnd, EM_GETSEL, (WPARAM)&from, (WPARAM)&to);
    if (from == 0 || to == 0) {                 // Если текстовый курсор находится в начале, минус вводить можно
        return FALSE;
    }
    else {
        return TRUE;
    }
}

BOOL createConstEditCtrls(HWND parentWindow, int size, int xPos, int yPos) {
    constEditCtrlsText = CreateWindow(
        L"static", L"Столбец свободных членов:",
        WS_VISIBLE | WS_CHILD,
        xPos, yPos, 200, 20,
        parentWindow, NULL, NULL, NULL);
    SendMessage(constEditCtrlsText, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));

    yPos += YSPACE_AFT_ECTXT;           // Переход к позиции для создания столбца свободных членов

    constEditCtrls = calloc(size, sizeof(HWND));
    if (constEditCtrls == NULL) {
        showErrMsgBox((LPCWSTR)L"Ошибка выделения памяти.");
        return FALSE;
    }

    for (int i = 0; i < size; ++i) {
        constEditCtrls[i] = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            L"edit", L"0",
            WS_VISIBLE | WS_CHILD | ES_RIGHT | ES_AUTOHSCROLL | WS_TABSTOP,
            xPos, yPos, SYSTEM_EC_WIDTH, EC_HEIGHT,
            parentWindow, NULL, NULL, NULL);
        SetWindowSubclass(constEditCtrls[i], systemEditCtrlsProc, 0, 0);
        SendMessage(constEditCtrls[i], EM_SETLIMITTEXT, MAX_NUM_LEN, 0);
        SendMessage(constEditCtrls[i], WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));

        yPos += YSPACE_BTWN_EC;        // Переход для создания следующего edit control
    }

    return TRUE;
}

BOOL createSolutionEditCtrls(HWND parentWindow, int size, int xPos, int yPos) {
    solveButton = CreateWindow(
        L"button", L"Решить",
        WS_VISIBLE | WS_CHILD,
        START_X_POS, yPos + 10, 80, 30,
        parentWindow, NULL, NULL, NULL);
    SendMessage(solveButton, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));

    solutionText = CreateWindow(
        L"static", L"Решение системы:",
        WS_CHILD,
        xPos, yPos, 170, 25,
        parentWindow, NULL, NULL, NULL);
    SendMessage(solutionText, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));

    solutionEditCtrls = calloc(size, sizeof(HWND));
    if (solutionEditCtrls == NULL) {
        showErrMsgBox((LPCWSTR)L"Ошибка выделения памяти.");
        return FALSE;
    }

    yPos += YSPACE_AFT_ECTXT;            // Переход к позиции для создания столбца решения системы

    for (int i = 0; i < size; ++i) {
        solutionEditCtrls[i] = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            L"edit", L"",
            WS_CHILD | ES_RIGHT | ES_AUTOHSCROLL | ES_READONLY,
            xPos, yPos, SOLVE_EC_WIDTH, EC_HEIGHT,
            parentWindow, NULL, NULL, NULL);
        SendMessage(solutionEditCtrls[i], WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));

        yPos += YSPACE_BTWN_EC;          // Переход для создания следующего edit control
    }

    return TRUE;
}

void setSolutionEditCtrlsVisible() {
    ShowWindow(solutionText, SW_SHOWNORMAL);
    for (int i = 0; i < variablesNum; ++i) {
        ShowWindow(solutionEditCtrls[i], SW_SHOWNORMAL);
    }
}

void freeEditCtlsMatrix(int toIndex) {
    for (int i = toIndex; i >= 0; --i) {
        free(coeffEditCtrls[i]);
    }
    free(coeffEditCtrls);
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

void showErrMsgBox(LPCWSTR errMsg) {
    int msgboxID = MessageBox(
        mainWindow,
        errMsg,
        (LPCWSTR)L"LinSysSolver",
        MB_ICONERROR | MB_OK | MB_APPLMODAL
    );
}

void showWarningMsgBox(LPCWSTR errMsg) {
    int msgboxID = MessageBox(
        mainWindow,
        errMsg,
        (LPCWSTR)L"LinSysSolver",
        MB_ICONWARNING | MB_OK | MB_APPLMODAL
    );
}