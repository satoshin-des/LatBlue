#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

#include <fstream>
#include <algorithm>

#include <NTL/ZZ.h>
#include <NTL/RR.h>
#include <NTL/LLL.h>

#include "core.h"
#include "lattice.h"
#include "reduction.h"

#define TOP_PANEL_HEIGHT 180
#define TOP_HEIGHT 160
#define RIGHT_WIDTH 140
#define MARGIN 10

#define ID_FILE_OPEN 1001
#define ID_FILE_EXIT 1002
#define ID_SVP_GENERATE 1003
#define ID_EDIT_COPY 2001
#define ID_EDIT_PASTE 2002
#define ID_EDIT_REDUCE 2003
#define IDC_EDIT 3001
#define IDC_OK 3002
#define IDC_CANCEL 3003
#define ID_REDUCE_LLL 4001
#define ID_REDUCE_BKZ 4002
#define ID_REDUCE_DEEP_LLL 4003
#define ID_REDUCE_POT_LLL 4004
#define ID_REDUCE_POT_BKZ 4005
#define ID_REDUCE_DEEP_BKZ 4006
#define ID_REDUCE_SIZE 4007
#define ID_REDUCE_DUAL_SIZE 4008
#define ID_REFERENCE 6001
#define WM_APP_PROGRESS (WM_APP + 10)
#define WM_APP_FINISH (WM_APP + 11)

struct InputResult
{
    int rank = 0;
    int seed = 0;
    double slope;
    NTL::ZZ vol;
    NTL::RR rhf;
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

/**
 * @brief Widow procedure for lattice basis reduction
 * The almost of this function is written with AI
 *
 * @param hWnd
 * @param msg
 * @param wParam
 * @param lParam
 * @return LRESULT
 */
LRESULT CALLBACK ReduceWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/**
 * @brief Lattice basis reduction in other thread
 * The almost of this function is written with AI
 *
 * @param param
 * @return DWORD
 */
DWORD WINAPI ReduceWorkerThread(LPVOID param);

/**
 * @brief scroll view procedure that lattice basis matrix is printed
 * The almost of this function is written with AI
 *
 * @param hWnd
 * @param msg
 * @param wParam
 * @param lParam
 * @return LRESULT
 */
LRESULT CALLBACK ScrollViewProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/**
 * @brief
 *
 * @param hWnd
 * @param outPath
 * @return true
 * @return false
 */
bool OpenFileDialog(HWND hWnd, std::string &outPath);

/**
 * @brief Copy text to clipboard
 * The almost of this function is written with AI
 *
 * @param hWnd
 * @param text
 */
void CopyTextToClipboard(HWND hWnd, const std::string &text);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc = {};        // Window class
    WNDCLASS wcInput = {};   // Window class for input
    WNDCLASS wcReduce = {};  // Window class for lattice basis reduction
    WNDCLASS wcScrView = {}; // Window class for printing lattice basis
    HWND hWnd;               // ウィンドウ
    HMENU hMenuBar;          // メニューバー
    HMENU hFileMenu;         // File
    HMENU hEditMenu;         // Edit
    HMENU hReduceMenu;       // Reduce menu
    MSG msg;                 // メッセージ
    INITCOMMONCONTROLSEX icc{};

    InitLattice();

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

    wcReduce.lpfnWndProc = ReduceWndProc;
    wcReduce.hInstance = hInstance;
    wcReduce.lpszClassName = TEXT("ReducePopup");
    wcReduce.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcReduce.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wcReduce);

    wcScrView.lpfnWndProc = ScrollViewProc;
    wcScrView.hInstance = hInstance;
    wcScrView.lpszClassName = "ScrollView";
    wcScrView.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wcScrView);

    // Progress bar
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icc);

    // ウィンドウ作成
    hWnd = CreateWindow(wc.lpszClassName, TEXT("LatBlue"), (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX), CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, hInstance, NULL);

    // メニューバーの作成
    hMenuBar = CreateMenu();
    hFileMenu = CreatePopupMenu();
    hEditMenu = CreatePopupMenu();
    hReduceMenu = CreatePopupMenu();

    // ファイル
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_OPEN, TEXT("Open"));
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFileMenu, MF_STRING, ID_SVP_GENERATE, TEXT("Generate"));
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_EXIT, TEXT("Quit"));

    // Reduce
    AppendMenu(hReduceMenu, MF_STRING, ID_REDUCE_SIZE, TEXT("size"));
    AppendMenu(hReduceMenu, MF_STRING, ID_REDUCE_LLL, TEXT("LLL"));
    AppendMenu(hReduceMenu, MF_STRING, ID_REDUCE_DEEP_LLL, TEXT("DeepLLL"));
    AppendMenu(hReduceMenu, MF_STRING, ID_REDUCE_POT_LLL, TEXT("PotLLL"));
    AppendMenu(hReduceMenu, MF_STRING, ID_REDUCE_BKZ, TEXT("BKZ"));
    AppendMenu(hReduceMenu, MF_STRING, ID_REDUCE_DEEP_BKZ, TEXT("DeepBKZ"));
    AppendMenu(hReduceMenu, MF_STRING, ID_REDUCE_POT_BKZ, TEXT("PotBKZ"));
    AppendMenu(hReduceMenu, MF_STRING, ID_REDUCE_DUAL_SIZE, TEXT("Dual-size"));

    // Edit
    AppendMenu(hEditMenu, MF_POPUP, (UINT_PTR)hReduceMenu, TEXT("Reduce"));
    AppendMenu(hEditMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hEditMenu, MF_STRING, ID_EDIT_COPY, TEXT("Copy"));

    // Append to menu bar
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFileMenu, TEXT("File"));
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hEditMenu, TEXT("Edit"));
    AppendMenu(hMenuBar, MF_STRING, ID_REFERENCE, TEXT("Reference"));

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
    int w, h, x;
    char buf[1024];         // Buffar to print result of lattice
    char buf_delta[64];     // Buffer for reduction parameter
    char buf_gamma[64];     // Buffer for reduction parameter for deep-insertion
    char buf_beta[64];      // Buffar for block size
    static InputResult res; // result for input
    static HWND hResultText;
    static HWND hLabelDelta;
    static HWND hLabelBeta;
    static HWND hLabelGamma;
    static HWND hEditDelta;
    static HWND hEditGamma;
    static HWND hEditBeta;
    static HWND hPopup;
    static HWND hScrollView;
    static HWND hCheckPruning;
    RECT rc;
    const int height = 200;

    switch (msg)
    {
    case WM_CREATE:
        // The window that displays now lattice basis information
        hResultText = CreateWindow("STATIC", "Result will be shown here.", (WS_CHILD | WS_VISIBLE | SS_LEFT), 10, 10, 400, 150, hWnd, NULL, GetModuleHandle(NULL), NULL);

        // The window that we can input reduction parameters
        hLabelDelta = CreateWindowW(L"STATIC", L"δ:", (WS_CHILD | WS_VISIBLE), 410, 10, 30, 20, hWnd, NULL, GetModuleHandle(NULL), NULL);
        hEditDelta = CreateWindowW(L"EDIT", L"0.99", (WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL), 440, 10, 90, 22, hWnd, (HMENU)5001, GetModuleHandle(NULL), NULL);
        hLabelGamma = CreateWindowW(L"STATIC", L"γ:", (WS_CHILD | WS_VISIBLE), 410, 40, 30, 20, hWnd, NULL, GetModuleHandle(NULL), NULL);
        hEditGamma = CreateWindowW(L"EDIT", L"", (WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL), 440, 40, 90, 22, hWnd, (HMENU)5001, GetModuleHandle(NULL), NULL);
        hLabelBeta = CreateWindowW(L"STATIC", L"β:", (WS_CHILD | WS_VISIBLE), 410, 70, 30, 20, hWnd, NULL, GetModuleHandle(NULL), NULL);
        hEditBeta = CreateWindowW(L"EDIT", L"40", (WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL), 440, 70, 90, 22, hWnd, (HMENU)5001, GetModuleHandle(NULL), NULL);
        hCheckPruning = CreateWindow("BUTTON", "pruning", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 410, 100, 120, 20, hWnd, (HMENU)7002, GetModuleHandle(NULL), NULL);

        // The window that lattice basis matrix is printed
        GetClientRect(hWnd, &rc);
        hScrollView = CreateWindowEx(0, "ScrollView", NULL, (WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL), 0, rc.bottom - height, rc.right, height, hWnd, NULL, GetModuleHandle(NULL), NULL);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        // Load file
        case ID_FILE_OPEN:
        {
            std::string path;
            if (OpenFileDialog(hWnd, path))
            {
                std::ifstream ifs(path);
                if (!ifs)
                {
                    MessageBox(hWnd, "Failed to open file.", "Error", MB_OK | MB_ICONERROR);
                    break;
                }

                ifs >> res.rank;
                lattice.rank = res.rank;
                lattice.basis.SetDims(lattice.rank, lattice.rank);
                for (int i = 0; i < res.rank; ++i)
                {
                    for (int j = 0; j < res.rank; ++j)
                    {
                        ifs >> lattice.basis[i][j];
                    }
                }
                ComputeGSO();
                res.slope = NTL::to_double(ComputeSlope());
                res.vol = Volume();
                res.rhf = ComputeRHF();

                PostMessage(hWnd, WM_APP + 1, 0, 0);
            }
            break;
        }

        // Generates SVP-challenge lattice basis
        case ID_SVP_GENERATE:
            hPopup = CreateWindowEx(WS_EX_DLGMODALFRAME, TEXT("InputPopup"), TEXT("Generate"), (WS_POPUP | WS_CAPTION | WS_SYSMENU), CW_USEDEFAULT, CW_USEDEFAULT, 300, 140, hWnd, NULL, GetModuleHandle(NULL), &res);
            ShowWindow(hPopup, SW_SHOW);
            UpdateWindow(hPopup);
            ShowWindow(hScrollView, SW_SHOW);
            UpdateWindow(hScrollView);
            break;

        case ID_FILE_EXIT:
            DestroyWindow(hWnd);
            break;

        // LLL reduce
        case ID_REDUCE_LLL:
            GetWindowText(hEditDelta, buf_delta, 64);
            delta = std::clamp(atof(buf_delta), 0.25, 1.0);
            reduce = REDUCE::LLL;
            hPopup = CreateWindowEx(WS_EX_DLGMODALFRAME, TEXT("ReducePopup"), TEXT("Reduce"), (WS_POPUP | WS_CAPTION | WS_SYSMENU), CW_USEDEFAULT, CW_USEDEFAULT, 300, 140, hWnd, NULL, GetModuleHandle(NULL), &res);
            ShowWindow(hPopup, SW_SHOW);
            UpdateWindow(hPopup);
            ShowWindow(hScrollView, SW_SHOW);
            UpdateWindow(hScrollView);
            break;

        // DeepLLL reduce
        case ID_REDUCE_DEEP_LLL:
            GetWindowText(hEditDelta, buf_delta, 64);
            delta = std::clamp(atof(buf_delta), 0.2501, 0.999);
            GetWindowText(hEditGamma, buf_gamma, 64);
            if (atoi(buf_gamma) <= 0)
            {
                gamma = lattice.rank;
            }
            gamma = std::clamp(atoi(buf_gamma), 1, (int)lattice.rank);
            reduce = REDUCE::DEEP_LLL;
            hPopup = CreateWindowEx(WS_EX_DLGMODALFRAME, TEXT("ReducePopup"), TEXT("Reduce"), (WS_POPUP | WS_CAPTION | WS_SYSMENU), CW_USEDEFAULT, CW_USEDEFAULT, 300, 140, hWnd, NULL, GetModuleHandle(NULL), &res);
            ShowWindow(hPopup, SW_SHOW);
            UpdateWindow(hPopup);
            ShowWindow(hScrollView, SW_SHOW);
            UpdateWindow(hScrollView);
            break;

        // PotLLL reduce
        case ID_REDUCE_POT_LLL:
            GetWindowText(hEditDelta, buf_delta, 64);
            delta = std::clamp(atof(buf_delta), 0.25, 1.0);
            reduce = REDUCE::POT_LLL;
            hPopup = CreateWindowEx(WS_EX_DLGMODALFRAME, TEXT("ReducePopup"), TEXT("Reduce"), (WS_POPUP | WS_CAPTION | WS_SYSMENU), CW_USEDEFAULT, CW_USEDEFAULT, 300, 140, hWnd, NULL, GetModuleHandle(NULL), &res);
            ShowWindow(hPopup, SW_SHOW);
            UpdateWindow(hPopup);
            ShowWindow(hScrollView, SW_SHOW);
            UpdateWindow(hScrollView);
            break;

        // BKZ reduce
        case ID_REDUCE_BKZ:
            GetWindowText(hEditDelta, buf_delta, 64);
            delta = std::clamp(atof(buf_delta), 0.25, 1.0);
            GetWindowText(hEditBeta, buf_beta, 64);
            beta = std::clamp(atoi(buf_beta), 2, (int)lattice.rank);
            pruning = (SendMessage(hCheckPruning, BM_GETCHECK, 0, 0) == BST_CHECKED);
            reduce = REDUCE::BLOCK_KZ;
            hPopup = CreateWindowEx(WS_EX_DLGMODALFRAME, TEXT("ReducePopup"), TEXT("Reduce"), (WS_POPUP | WS_CAPTION | WS_SYSMENU), CW_USEDEFAULT, CW_USEDEFAULT, 300, 140, hWnd, NULL, GetModuleHandle(NULL), &res);
            ShowWindow(hPopup, SW_SHOW);
            UpdateWindow(hPopup);
            ShowWindow(hScrollView, SW_SHOW);
            UpdateWindow(hScrollView);
            break;

        // DeepBKZ reduce
        case ID_REDUCE_DEEP_BKZ:
            GetWindowText(hEditDelta, buf_delta, 64);
            delta = std::clamp(atof(buf_delta), 0.25, 1.0);
            GetWindowText(hEditBeta, buf_beta, 64);
            beta = std::clamp(atoi(buf_beta), 2, (int)lattice.rank);
            pruning = (SendMessage(hCheckPruning, BM_GETCHECK, 0, 0) == BST_CHECKED);
            GetWindowText(hEditGamma, buf_gamma, 64);
            if (atoi(buf_gamma) <= 0)
            {
                gamma = lattice.rank;
            }
            gamma = std::clamp(atoi(buf_gamma), 1, (int)lattice.rank);
            reduce = REDUCE::DEEP_BKZ;
            hPopup = CreateWindowEx(WS_EX_DLGMODALFRAME, TEXT("ReducePopup"), TEXT("Reduce"), (WS_POPUP | WS_CAPTION | WS_SYSMENU), CW_USEDEFAULT, CW_USEDEFAULT, 300, 140, hWnd, NULL, GetModuleHandle(NULL), &res);
            ShowWindow(hPopup, SW_SHOW);
            UpdateWindow(hPopup);
            ShowWindow(hScrollView, SW_SHOW);
            UpdateWindow(hScrollView);
            break;

        // PotBKZ reduce
        case ID_REDUCE_POT_BKZ:
            GetWindowText(hEditDelta, buf_delta, 64);
            delta = std::clamp(atof(buf_delta), 0.25, 1.0);
            GetWindowText(hEditBeta, buf_beta, 64);
            beta = std::clamp(atoi(buf_beta), 2, (int)lattice.rank);
            reduce = REDUCE::POT_BKZ;
            hPopup = CreateWindowEx(WS_EX_DLGMODALFRAME, TEXT("ReducePopup"), TEXT("Reduce"), (WS_POPUP | WS_CAPTION | WS_SYSMENU), CW_USEDEFAULT, CW_USEDEFAULT, 300, 140, hWnd, NULL, GetModuleHandle(NULL), &res);
            ShowWindow(hPopup, SW_SHOW);
            UpdateWindow(hPopup);
            ShowWindow(hScrollView, SW_SHOW);
            UpdateWindow(hScrollView);
            break;

        // size reduce
        case ID_REDUCE_SIZE:
            reduce = REDUCE::SIZE_REDUCE;
            hPopup = CreateWindowEx(WS_EX_DLGMODALFRAME, TEXT("ReducePopup"), TEXT("Reduce"), (WS_POPUP | WS_CAPTION | WS_SYSMENU), CW_USEDEFAULT, CW_USEDEFAULT, 300, 140, hWnd, NULL, GetModuleHandle(NULL), &res);
            ShowWindow(hPopup, SW_SHOW);
            UpdateWindow(hPopup);
            ShowWindow(hScrollView, SW_SHOW);
            UpdateWindow(hScrollView);
            break;

        case ID_REDUCE_DUAL_SIZE:
            reduce = REDUCE::DUAL_SIZE;
            hPopup = CreateWindowEx(WS_EX_DLGMODALFRAME, TEXT("ReducePopup"), TEXT("Reduce"), (WS_POPUP | WS_CAPTION | WS_SYSMENU), CW_USEDEFAULT, CW_USEDEFAULT, 300, 140, hWnd, NULL, GetModuleHandle(NULL), &res);
            ShowWindow(hPopup, SW_SHOW);
            UpdateWindow(hPopup);
            ShowWindow(hScrollView, SW_SHOW);
            UpdateWindow(hScrollView);
            break;

        case ID_EDIT_COPY:
        {
            if (lattice.rank <= 0)
            {
                MessageBox(hWnd, "No matrix to copy.", "Info", MB_OK);
                break;
            }

            std::string mat = mat_ZZToString(lattice.basis);
            CopyTextToClipboard(hWnd, mat);

            MessageBox(hWnd, "Lattice basis copied to clipboard!", "Copy", MB_OK);
            break;
        }

        case ID_REFERENCE:
            PrintRef(hWnd);
            break;
        }
        return 0;

    case WM_APP + 1:
        InvalidateRect(hScrollView, NULL, TRUE);
        SendMessage(hScrollView, WM_PAINT, 0, 0);
        sprintf(buf, "Lattice basis generated!\r\nVolume = %s\r\nslope = %lf\r\nRHF = %s", ZZToString(res.vol).c_str(), res.slope, RRToString(res.rhf).c_str());
        SetWindowTextA(hResultText, buf);
        return 0;

    case WM_APP + 2:
        InvalidateRect(hScrollView, NULL, TRUE);
        SendMessage(hScrollView, WM_PAINT, 0, 0);
        sprintf(buf, "Lattice basis reduced!\r\nVolume = %s\r\nslope = %lf\r\nRHF = %s", ZZToString(res.vol).c_str(), res.slope, RRToString(res.rhf).c_str());
        SetWindowTextA(hResultText, buf);
        return 0;

    case WM_SIZE:
        w = LOWORD(lParam);
        h = HIWORD(lParam);

        MoveWindow(hResultText, MARGIN, MARGIN, w - RIGHT_WIDTH - 3 * MARGIN, TOP_HEIGHT - 2 * MARGIN, TRUE);

        x = w - RIGHT_WIDTH + 10;

        MoveWindow(hLabelDelta, x, 10, 30, 20, TRUE);
        MoveWindow(hLabelGamma, x, 40, 30, 20, TRUE);
        MoveWindow(hLabelBeta, x, 70, 30, 20, TRUE);

        MoveWindow(hEditDelta, x + 30, 10, 90, 22, TRUE);
        MoveWindow(hEditGamma, x + 30, 40, 90, 22, TRUE);
        MoveWindow(hEditBeta, x + 30, 70, 90, 22, TRUE);

        MoveWindow(hCheckPruning, x, 100, 120, 20, TRUE);

        MoveWindow(hScrollView, 0, TOP_HEIGHT, w, h - TOP_HEIGHT, TRUE);

        break;

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
            if (pResult->rank <= 0)
            {
                MessageBox(hWnd, "Rank must be a positive integer.", "Invalid input", MB_OK | MB_ICONWARNING);

                SetFocus(hEditRank);
                SendMessage(hEditRank, EM_SETSEL, 0, -1);
                break;
            }

            Generator(pResult->rank, pResult->seed);
            ComputeGSO();
            pResult->slope = NTL::to_double(ComputeSlope());
            pResult->vol = Volume();
            pResult->rhf = ComputeRHF();
            PostMessage(GetParent(hWnd), WM_APP + 1, 0, 0);
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

LRESULT CALLBACK ReduceWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static InputResult *pResult;
    static HWND hProgress;

    switch (msg)
    {
    case WM_CREATE:
        pResult = (InputResult *)((CREATESTRUCT *)lParam)->lpCreateParams;

        CreateWindowW(L"STATIC", L"Now Reducing...\n少女簡約中", (WS_CHILD | WS_VISIBLE), 10, 10, 260, 40, hWnd, NULL, NULL, NULL);

        hProgress = CreateWindowEx(0, PROGRESS_CLASS, NULL, (WS_CHILD | WS_VISIBLE | PBS_SMOOTH), 20, 50, 240, 20, hWnd, NULL, GetModuleHandle(NULL), NULL);

        SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
        SendMessage(hProgress, PBM_SETPOS, 0, 0);

        CreateThread(NULL, 0, ReduceWorkerThread, hWnd, 0, NULL);
        break;

    case WM_APP_PROGRESS:
        SendMessage(hProgress, PBM_SETPOS, wParam, 0);
        break;

    case WM_APP_FINISH:
        pResult->slope = NTL::to_double(ComputeSlope());
        pResult->rhf = ComputeRHF();
        PostMessage(GetParent(hWnd), WM_APP + 2, 0, 0);
        DestroyWindow(hWnd);
        break;

    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

DWORD WINAPI ReduceWorkerThread(LPVOID param)
{
    HWND hWnd = (HWND)param;

    switch (reduce)
    {
    case REDUCE::SIZE_REDUCE:
        SizeReduce(hWnd, WM_APP_PROGRESS);
        break;

    case REDUCE::LLL:
        L2Reduce(hWnd, WM_APP_PROGRESS);
        break;

    case REDUCE::DEEP_LLL:
        DeepLLLReduce(hWnd, WM_APP_PROGRESS, lattice.rank, 0);
        break;

    case REDUCE::POT_LLL:
        PotLLLReduce(hWnd, WM_APP_PROGRESS, lattice.rank, 0);
        break;

    case REDUCE::BLOCK_KZ:
        BKZReduce(hWnd, WM_APP_PROGRESS);
        break;

    case REDUCE::DEEP_BKZ:
        DeepBKZReduce(hWnd, WM_APP_PROGRESS);
        break;

    case REDUCE::POT_BKZ:
        PotBKZReduce(hWnd, WM_APP_PROGRESS);
        break;

    case REDUCE::DUAL_SIZE:
        DualSize(hWnd, WM_APP_PROGRESS);
        break;

    case REDUCE::NONE:
        break;
    }

    ComputeGSO();

    PostMessage(hWnd, WM_APP_FINISH, 0, 0);
    return 0;
}

bool OpenFileDialog(HWND hWnd, std::string &outPath)
{
    char fileName[MAX_PATH] = "";

    OPENFILENAME ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter =
        "Text Files (*.txt)\0*.txt\0"
        "All Files (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = (OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST);
    ofn.lpstrTitle = "Open File";

    if (GetOpenFileName(&ofn))
    {
        outPath = fileName;
        return true;
    }
    return false;
}

void CopyTextToClipboard(HWND hWnd, const std::string &text)
{
    if (!OpenClipboard(hWnd))
    {
        return;
    }

    EmptyClipboard();

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
    if (!hMem)
    {
        CloseClipboard();
        return;
    }

    char *p = (char *)GlobalLock(hMem);
    memcpy(p, text.c_str(), text.size() + 1);
    GlobalUnlock(hMem);

    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
}

LRESULT CALLBACK ScrollViewProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    std::string mat_str;
    static int xPos = 0;
    static int yPos = 0;

    switch (msg)
    {
    case WM_SIZE:
    {
        RECT rc;
        GetClientRect(hWnd, &rc);

        int cx = rc.right - rc.left;
        int cy = rc.bottom - rc.top;

        int lineHeight = 12;
        int contentHeight = lineHeight * 200; // lattice.rank * lineHeight + 20;
        int contentWidth = 4000;              // 行の最大幅（仮）

        SCROLLINFO si{sizeof(si), (SIF_RANGE | SIF_PAGE | SIF_POS)};

        si.nMin = 0;
        si.nMax = contentWidth - 1;
        si.nPage = cx;
        si.nPos = xPos;
        SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);

        si.nMax = contentHeight - 1;
        si.nPage = cy;
        si.nPos = yPos;
        SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
        return 0;
    }

    case WM_HSCROLL:
    {
        SCROLLINFO si{sizeof(si), SIF_ALL};
        GetScrollInfo(hWnd, SB_HORZ, &si);

        int old = si.nPos;
        switch (LOWORD(wParam))
        {
        case SB_LINELEFT:
            si.nPos -= 20;
            break;
        case SB_LINERIGHT:
            si.nPos += 20;
            break;
        case SB_PAGELEFT:
            si.nPos -= si.nPage;
            break;
        case SB_PAGERIGHT:
            si.nPos += si.nPage;
            break;
        case SB_THUMBTRACK:
            si.nPos = si.nTrackPos;
            break;
        }
        SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
        xPos = si.nPos;

        if (old != xPos)
            InvalidateRect(hWnd, NULL, TRUE);
        break;
    }

    case WM_VSCROLL:
    {
        SCROLLINFO si{sizeof(si), SIF_ALL};
        GetScrollInfo(hWnd, SB_VERT, &si);

        int old = si.nPos;
        switch (LOWORD(wParam))
        {
        case SB_LINEUP:
            si.nPos -= 20;
            break;
        case SB_LINEDOWN:
            si.nPos += 20;
            break;
        case SB_PAGEUP:
            si.nPos -= si.nPage;
            break;
        case SB_PAGEDOWN:
            si.nPos += si.nPage;
            break;
        case SB_THUMBTRACK:
            si.nPos = si.nTrackPos;
            break;
        }
        SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
        yPos = si.nPos;

        if (old != yPos)
        {
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        SetViewportOrgEx(hdc, -xPos, -yPos, NULL);

        if (lattice.rank >= 1)
        {
            for (int i = 0; i < lattice.rank; ++i)
            {
                mat_str = vec_ZZToString(lattice.basis[i]);
                TextOut(hdc, 10, 10 + 17 * i, mat_str.c_str(), mat_str.length());
            }
        }

        EndPaint(hWnd, &ps);
        break;
    }
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}
