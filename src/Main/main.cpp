#include "main.h"
#include "framework.h"
#include <stdint.h>

#include "DarkMode.h"
#include "UAHMenuBar.h"
#include "resource.h"
#define MAX_LOADSTRING 100
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
// Global Variables:
HINSTANCE hInst; // current instance
WCHAR szTitle[MAX_LOADSTRING]; // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING]; // the main window class name
// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);

HIMAGELIST g_hImageList = NULL;

HWND ToolBar = NULL;

HWND CreateToolbarFromResource(HWND hWndParent)
{
    int x = 0;
    int y = 0;
    int w = 500;
    int h = 22;

    // Declare and initialize local constants.
    const int ImageListID = 0;
    const int numButtons = 3;
    const int bitmapSize = 16;

    const DWORD buttonStyles = BTNS_AUTOSIZE;

    // Create the toolbar.
    HWND hWndToolbar = CreateWindowExW(WS_EX_TOOLWINDOW | TBSTYLE_EX_HIDECLIPPEDBUTTONS, TOOLBARCLASSNAME, NULL,
        CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER | WS_CHILD | WS_VISIBLE | CCS_ADJUSTABLE | TBSTYLE_ALTDRAG | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS,
        x, y, w, h,
        hWndParent,
        (HMENU)0,
        hInst,
        NULL);
    if (hWndToolbar == NULL)
        return NULL;

    // Create the image list.
    g_hImageList = ImageList_Create(bitmapSize, bitmapSize, // Dimensions of individual bitmaps.
        ILC_COLOR16 | ILC_MASK, // Ensures transparent background.
        numButtons, 0);

    // Set the image list.
    SendMessage(hWndToolbar, TB_SETIMAGELIST,
        (WPARAM)ImageListID,
        (LPARAM)g_hImageList);

    // Load the button images.
    SendMessage(hWndToolbar, TB_LOADIMAGES,
        (WPARAM)IDB_STD_SMALL_COLOR,
        (LPARAM)HINST_COMMCTRL);

    // Load the text from a resource.

    // In the string table, the text for all buttons is a single entry that
    // appears as "~New~Open~Save~~". The separator character is arbitrary,
    // but it must appear as the first character of the string. The message
    // returns the index of the first item, and the items are numbered
    // consecutively.

    int iNew = SendMessage(hWndToolbar, TB_ADDSTRING,
        (WPARAM)hInst, (LPARAM)IDS_NEW);

    // Initialize button info.
    // IDM_NEW, IDM_OPEN, and IDM_SAVE are application-defined command constants.

    TBBUTTON tbButtons[numButtons] = {
        { MAKELONG(STD_FILENEW, ImageListID), IDS_NEW, TBSTATE_ENABLED, buttonStyles, { 0 }, 0, iNew },
        { MAKELONG(STD_FILEOPEN, ImageListID), IDS_OPEN, TBSTATE_ENABLED, buttonStyles, { 0 }, 0, iNew + 1 },
        { MAKELONG(STD_FILESAVE, ImageListID), IDS_SAVE, 0, buttonStyles, { 0 }, 0, iNew + 2 }
    };

    // Add buttons.
    SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(hWndToolbar, TB_ADDBUTTONS, (WPARAM)numButtons, (LPARAM)&tbButtons);

    // Resize the toolbar, and then show it.
    SendMessage(hWndToolbar, TB_AUTOSIZE, 0, 0);
    ShowWindow(hWndToolbar, TRUE);

    return hWndToolbar;
}

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,

    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WIN32CUSTOMMENUBARAEROTHEME, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow)) {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32CUSTOMMENUBARAEROTHEME));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32CUSTOMMENUBARAEROTHEME));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WIN32CUSTOMMENUBARAEROTHEME);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable
    InitDarkMode();
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 400, 240, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) {
        return FALSE;
    }
    // https://stackoverflow.com/questions/61271578/winapi-toolbar-text-color
    ToolBar = CreateToolbarFromResource(hWnd);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lr = 0;
    if (UAHWndProc(hWnd, message, wParam, lParam, &lr)) {
        return lr;
    }

    switch (message) {
    case WM_CREATE: {
        if (g_darkModeSupported) {
            AllowDarkModeForWindow(hWnd, true);
            RefreshTitleBarThemeColor(hWnd);
            // Set window background color
            HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
            SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)brush);
        }
    } break;
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId) {
        case IDM_ABOUT:
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProcW(hWnd, message, wParam, lParam);
        }
    } break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_NOTIFY: {
        LPNMHDR lpnm = ((LPNMHDR)lParam);
        switch (lpnm->code) {
        case NM_CUSTOMDRAW: {
            LPNMTBCUSTOMDRAW data_ptr = (LPNMTBCUSTOMDRAW)lParam;
            if (data_ptr->nmcd.hdr.hwndFrom == ToolBar) {
                switch (data_ptr->nmcd.dwDrawStage) {
                case CDDS_ITEMPREPAINT: {
                    // SelectObject(data_ptr->nmcd.hdc, GetFont(L"Microsoft Sans Serif", 15));
                    FillRect(data_ptr->nmcd.hdc, &data_ptr->nmcd.rc, CreateSolidBrush(RGB(44, 44, 44)));
                    data_ptr->clrText = RGB(228, 228, 228);
                    return CDRF_NEWFONT;
                }
                case CDDS_PREPAINT: {
                    return CDRF_NOTIFYITEMDRAW;
                }
                }
            }
        }
        }
    }
    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}
