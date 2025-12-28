#include <windows.h>

#include "core.h"

#define ID_FILE_OPEN 1001
#define ID_FILE_EXIT 1002
#define ID_SVP_GENERATE 1003
#define ID_EDIT_COPY 2001
#define ID_EDIT_PASTE 2002
#define IDC_EDIT 3001
#define IDC_OK 3002
#define IDC_CANCEL 3003

struct InputResult
{
    bool ok = false;
    int rank = 0;
    int seed = 0;
};

/**
 * @brief Window procedure
 * The almost of this function is written with AI.
 *
 * @return LRESULT
 */
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

/**
 * @brief Window procedure for input
 * The almost of this function is written with AI.
 *
 * @param hWnd
 * @param msg
 * @param wParam
 * @param lParam
 * @return LRESULT
 */
LRESULT CALLBACK InputWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    WNDCLASS wc = {};      // Window class
    WNDCLASS wcInput = {}; // Window class for input
    HWND hWnd;             // ウィンドウ
    HMENU hMenuBar;        // メニューバー
    HMENU hFileMenu;       // ファイル
    HMENU hEditMenu;       // 編集
    MSG msg;               // メッセージ

    initLattice();

    // Register window classes
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("SampleMenuWindow");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    wcInput.lpfnWndProc = InputWndProc;
    wcInput.hInstance = hInstance;
    wcInput.lpszClassName = TEXT("InputPopup");
    wcInput.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcInput.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wcInput);

    // ウィンドウ作成
    hWnd = CreateWindow(wc.lpszClassName, TEXT("cryppto"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, hInstance, NULL);

    // メニューバーの作成
    hMenuBar = CreateMenu();
    hFileMenu = CreatePopupMenu();
    hEditMenu = CreatePopupMenu();

    // ファイル
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_OPEN, TEXT("Open"));
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFileMenu, MF_STRING, ID_SVP_GENERATE, TEXT("Generate"));
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_EXIT, TEXT("Quit"));

    // 編集
    AppendMenu(hEditMenu, MF_STRING, ID_EDIT_COPY, TEXT("Copy"));
    AppendMenu(hEditMenu, MF_STRING, ID_EDIT_PASTE, TEXT("Paste"));

    // メニューバーへ追加
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFileMenu, TEXT("File"));
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hEditMenu, TEXT("Edit"));

    // ウィンドウに設定
    SetMenu(hWnd, hMenuBar);

    // 表示
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // メッセージループ
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static InputResult res; // 入力結果
    static HWND hPopup;

    switch (msg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_FILE_OPEN:
            MessageBox(hWnd, TEXT("「開く」が選ばれました"), TEXT("Menu"), MB_OK);
            break;

        case ID_SVP_GENERATE:
            hPopup = CreateWindowEx(WS_EX_DLGMODALFRAME, TEXT("InputPopup"), TEXT("Generate"), (WS_POPUP | WS_CAPTION | WS_SYSMENU), CW_USEDEFAULT, CW_USEDEFAULT, 300, 140, hWnd, NULL, GetModuleHandle(NULL), &res);
            ShowWindow(hPopup, SW_SHOW);
            UpdateWindow(hPopup);
            break;

        case ID_FILE_EXIT:
            DestroyWindow(hWnd);
            break;

        case ID_EDIT_COPY:
            MessageBox(hWnd, TEXT("Copy"), TEXT("Menu"), MB_OK);
            break;

        case ID_EDIT_PASTE:
            MessageBox(hWnd, TEXT("Paste"), TEXT("Menu"), MB_OK);
            break;
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK InputWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char buf[256];               // Buffar of input string
    static InputResult *pResult; // input result
    static HWND hEditRank;       // input box for lattice rank
    static HWND hEditSeed;       // input box for seed to generate lattice

    switch (msg)
    {
    case WM_CREATE:
        pResult = (InputResult *)((CREATESTRUCT *)lParam)->lpCreateParams;

        CreateWindow("STATIC", "Please input lattice rank and seed:", (WS_CHILD | WS_VISIBLE), 10, 10, 250, 20, hWnd, NULL, NULL, NULL);
        CreateWindow("STATIC", "Rank:", (WS_CHILD | WS_VISIBLE | SS_RIGHT), 10, 38, 40, 20, hWnd, NULL, NULL, NULL);
        hEditRank = CreateWindow("EDIT", "", (WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL), 55, 35, 60, 22, hWnd, (HMENU)IDC_EDIT, NULL, NULL);
        CreateWindow("STATIC", "Seed:", (WS_CHILD | WS_VISIBLE | SS_RIGHT), 120, 38, 40, 20, hWnd, NULL, NULL, NULL);
        hEditSeed = CreateWindow("EDIT", "", (WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL), 165, 35, 60, 22, hWnd, (HMENU)IDC_EDIT, NULL, NULL);
        CreateWindow("BUTTON", "OK", (WS_CHILD | WS_VISIBLE), 60, 70, 70, 25, hWnd, (HMENU)IDC_OK, NULL, NULL);
        CreateWindow("BUTTON", "Cancel", (WS_CHILD | WS_VISIBLE), 150, 70, 70, 25, hWnd, (HMENU)IDC_CANCEL, NULL, NULL);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_OK:
            GetWindowText(hEditRank, buf, 256);
            pResult->rank = atoi(buf);
            GetWindowText(hEditSeed, buf, 256);
            pResult->seed = atoi(buf);
            pResult->ok = true;
            generator(pResult->rank, pResult->seed);

            DestroyWindow(hWnd);
            break;

        case IDC_CANCEL:
            DestroyWindow(hWnd);
            break;
        }
        break;

    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}
