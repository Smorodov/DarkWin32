#include "main.h"
#include "framework.h"
#include <CommCtrl.h>
#include <stdint.h>
// #include "DarkMode.h"
#include "UAHMenuBar.h"
#include "dark_mode.h"
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
RECT rc;
HIMAGELIST g_hImageList = NULL;

HWND ToolBar = NULL;
HWND ReBar = NULL;
BOOL g_darkModeSupported = TRUE;

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HBRUSH hbrBkgnd = NULL;
    switch (message) {
    case WM_INITDIALOG: {
        if (g_darkModeSupported) {
            HWND hButton = GetDlgItem(hDlg, IDOK);
            AllowDarkModeForWindow(hDlg, TRUE);

            HWND hwndCheckbox = GetDlgItem(hDlg, IDC_CHECK1);
            SetWindowTheme(hwndCheckbox, L"wstr", L"wstr");

            HWND hwndCombobox = GetDlgItem(hDlg, IDC_COMBO1);
            SetWindowTheme(hwndCombobox, L"TextBox", NULL);

            AllowDarkModeForWindow(hButton, TRUE);
            SetWindowTheme(hButton, L"Explorer", NULL);
            SendMessageW(hDlg, WM_THEMECHANGED, 0, 0);
        }
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:

        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
        /*
         case WM_CTLCOLORBTN:
                 SetTextColor((HDC)(wParam), RGB(0,0,0));    // <-- whatever color you want
                 SetDCBrushColor((HDC)(wParam), RGB(0,0,0)); // <-- whatever color you want
                 return (INT_PTR)(hbrBkgnd);
         break;
         */
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC: {
        if (g_darkModeSupported && ShouldAppsUseDarkMode() && !IsHighContrast() && SetDarkThemeColors(&hbrBkgnd, (HDC)(wParam))) {
            return (INT_PTR)(hbrBkgnd);
        }
    } break;
    case WM_DESTROY:
        if (hbrBkgnd) {
            DeleteObject(hbrBkgnd);
            hbrBkgnd = NULL;
        }
        break;
    case WM_SETTINGCHANGE: {
        if (g_darkModeSupported && IsColorSchemeChangeMessage(lParam))
            SendMessageW(hDlg, WM_THEMECHANGED, 0, 0);
    } break;
    case WM_THEMECHANGED: {
        if (g_darkModeSupported) {
            RefreshTitleBarThemeColor(hDlg);

            SendMessageW(GetDlgItem(hDlg, IDOK), WM_THEMECHANGED, 0, 0);

            InvalidateRect(hDlg, NULL, TRUE);
        }
    } break;
    }
    return (INT_PTR)FALSE;
}

HWND CreateToolbarFromResource(HWND hWndParent)
{
    int x = 0;
    int y = 0;
    int w = 500;
    int h = 22;

// Declare and initialize local constants.
#define ImageListID 0
#define numButtons 4
#define bitmapSize 16

    const DWORD buttonStyles = BTNS_AUTOSIZE;

    // Create the toolbar.
    HWND hWndToolbar = CreateWindowExW(
        WS_EX_TOOLWINDOW | TBSTYLE_EX_HIDECLIPPEDBUTTONS,
        TOOLBARCLASSNAME,
        NULL,
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

  //  int iNew = SendMessage(hWndToolbar, TB_ADDSTRING,
  //      (WPARAM)hInst, (LPARAM)IDS_NEW);

    // Initialize button info.
    // IDM_NEW, IDM_OPEN, and IDM_SAVE are application-defined command constants.

    TBBUTTON tbButtons[numButtons] = {
        { MAKELONG(STD_FILENEW, ImageListID), IDM_NEW, TBSTATE_ENABLED, buttonStyles, { 0 }, 0, 0 },
        { MAKELONG(STD_FILEOPEN, ImageListID),IDM_OPEN, TBSTATE_ENABLED, buttonStyles, { 0 }, 0,0 + 1 },
        { MAKELONG(STD_FILESAVE, ImageListID),IDM_SAVE, TBSTATE_ENABLED, buttonStyles, { 0 }, 0, 0 + 2 },
        { MAKELONG(STD_DELETE, ImageListID),IDM_EXIT, TBSTATE_ENABLED, buttonStyles, { 0 }, 0, 0 + 2 }
    };

    // Add buttons.
    SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(hWndToolbar, TB_ADDBUTTONS, (WPARAM)numButtons, (LPARAM)&tbButtons);

    return hWndToolbar;
}
// ------------------------------------------
//
// ------------------------------------------
HWND createReBar(HWND hwnd, HINSTANCE hInstance, HWND hndToolbar)
{
    LRESULT lResult;
    HWND hwndReBar = CreateWindowExW(
        WS_EX_TOOLWINDOW |
        //TBSTYLE_EX_HIDECLIPPEDBUTTONS |
        0,
        REBARCLASSNAME,
        NULL,0|
        //CCS_NORESIZE |
        //CCS_NOPARENTALIGN |
        //CCS_NODIVIDER |
        WS_CHILD |
        WS_VISIBLE |
        //CCS_ADJUSTABLE |
        //TBSTYLE_ALTDRAG |
        //TBSTYLE_FLAT |
        TBSTYLE_TOOLTIPS
        ,
        0, 0, 400, 22,
        hwnd,
        (HMENU)0,
        hInstance,
        NULL);

    REBARBANDINFO rbBand;
    rbBand.cbSize = sizeof(REBARBANDINFO);
    rbBand.fMask = 
        RBBIM_STYLE |
        RBBIM_CHILD |
        RBBIM_CHILDSIZE |
        0;
    rbBand.fStyle =0 |
        RBBS_FIXEDSIZE |
        0;
    rbBand.lpText = (LPWSTR)L"Toolbar";
    rbBand.hwndChild = hndToolbar;
    rbBand.cxMinChild = 0;
    rbBand.cyMinChild = 22;
    rbBand.cx = 0;
    SendMessage(hwndReBar, RB_INSERTBAND, (WPARAM)(-1), (LPARAM)&rbBand);
    return hwndReBar;
}
// ------------------------------------------
//
// ------------------------------------------
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,

    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
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
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int)msg.wParam;
}

// ------------------------------------------
//
// ------------------------------------------
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
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WIN32CUSTOMMENUBARAEROTHEME);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    return RegisterClassExW(&wcex);
}
// ------------------------------------------
//
// ------------------------------------------
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable
    // Initialize dark mode
    InitDarkMode();
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 400, 240, NULL, NULL, hInstance, NULL);
    if (!hWnd) {
        return FALSE;
    }
    // https://stackoverflow.com/questions/61271578/winapi-toolbar-text-color
    ToolBar = CreateToolbarFromResource(hWnd);
    ReBar = createReBar(hWnd, hInstance, ToolBar);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}
// ------------------------------------------
//
// ------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    LRESULT lr = 0;
    if (UAHWndProc(hWnd, message, wParam, lParam, &lr)) {
        return lr;
    }
    switch (message) {
    // For repainting toolbar and rebar
    case WM_NOTIFY: {
        NMHDR* lpnm = lParam;
        switch (lpnm->code) {
        case NM_CUSTOMDRAW: {
            LPNMCUSTOMDRAW data_ptr = (LPNMCUSTOMDRAW)lParam;
            if (
                data_ptr->hdr.hwndFrom == ReBar |
                data_ptr->hdr.hwndFrom ==ToolBar
                ) 
            {
                switch (data_ptr->dwDrawStage) {                
                case CDDS_ITEMPREPAINT: {                    
                    FillRect(data_ptr->hdc, &data_ptr->rc, CreateSolidBrush(RGB(94, 94, 94)));                    
                    return CDRF_NEWFONT;
                }
                case CDDS_PREPAINT: {
                    FillRect(data_ptr->hdc, &data_ptr->rc, CreateSolidBrush(RGB(94, 94, 94)));                    
                    return CDRF_NOTIFYITEMDRAW;
                }
                }
            }
        }
        }
    }

    case WM_CREATE: {
        if (g_darkModeSupported) {
            OnWmCreate(hWnd);
            // Set window background color
            HBRUSH brush = CreateSolidBrush(RGB(64, 64, 64));
            SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)brush);
        }
    } break;

    case WM_PAINT:

    {
        hdc = BeginPaint(hWnd, &ps);
        //    Initializing original object
        HGDIOBJ original = NULL;
        //    Saving the original object
        original = SelectObject(hdc, GetStockObject(DC_PEN));
        //    Rectangle function is defined as...
        //    BOOL Rectangle(hdc, xLeft, yTop, yRight, yBottom);
        //    Drawing a rectangle with just a black pen
        //    The black pen object is selected and sent to the current device context
        //  The default brush is WHITE_BRUSH
        SelectObject(hdc, GetStockObject(BLACK_PEN));
        Rectangle(hdc, 0, 0, 200, 200);

        //    Select DC_PEN so you can change the color of the pen with
        //    COLORREF SetDCPenColor(HDC hdc, COLORREF color)
        SelectObject(hdc, GetStockObject(DC_PEN));

        //    Select DC_BRUSH so you can change the brush color from the
        //    default WHITE_BRUSH to any other color
        SelectObject(hdc, GetStockObject(BLACK_BRUSH));

        //    Set the DC Brush to Red
        //    The RGB macro is declared in "Windowsx.h"
        SetDCBrushColor(hdc, RGB(255, 0, 0));
        //    Set the Pen to Blue
        SetDCPenColor(hdc, RGB(0, 0, 255));

        //    Drawing a rectangle with the current Device Context
        Rectangle(hdc, 0, 0, 1000, 1000);
        //    Restoring the original object
        SelectObject(hdc, original);
    }

    break;

    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId) {
        case IDM_ABOUT:
            DialogBoxW(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
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

    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}
