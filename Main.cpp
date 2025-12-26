#include <windows.h>

#define ID_FILE_OPEN 1001
#define ID_FILE_EXIT 1002
#define ID_EDIT_COPY 2001
#define ID_EDIT_PASTE 2002

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    WNDCLASS wc = {}; // ウィンドウクラス
    HWND hWnd;        // ウィンドウ
    HMENU hMenuBar;   // メニューバー
    HMENU hFileMenu;  // ファイル
    HMENU hEditMenu;  // 編集
    MSG msg;          // メッセージ

    // ----------------------------
    // ウィンドウクラス登録
    // ----------------------------
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("SampleMenuWindow");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    // ウィンドウ作成
    hWnd = CreateWindow(
        wc.lpszClassName,
        TEXT("cryppto"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        640, 480,
        NULL, NULL,
        hInstance,
        NULL);

    // メニューバーの作成
    hMenuBar = CreateMenu();
    hFileMenu = CreatePopupMenu();
    hEditMenu = CreatePopupMenu();

    // ファイル
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_OPEN, TEXT("Open"));
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

    // ----------------------------
    // 表示
    // ----------------------------
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // ----------------------------
    // メッセージループ
    // ----------------------------
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

// ===================================
// ウィンドウプロシージャ
// ===================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_FILE_OPEN:
            MessageBox(hWnd, TEXT("「開く」が選ばれました"),
                       TEXT("メニュー"), MB_OK);
            break;

        case ID_FILE_EXIT:
            DestroyWindow(hWnd);
            break;

        case ID_EDIT_COPY:
            MessageBox(hWnd, TEXT("コピー"),
                       TEXT("メニュー"), MB_OK);
            break;

        case ID_EDIT_PASTE:
            MessageBox(hWnd, TEXT("貼り付け"),
                       TEXT("メニュー"), MB_OK);
            break;
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}
