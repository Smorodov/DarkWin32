
#include <windows.h>
#include "IatHook.h"
#include "framework.h"
#include "delayimp.h"
#include "dark_mode.h"
#define USE_DWMAPI 1
#define USE_GETVERSION 1

#if USE_DWMAPI
#include <Dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")
#endif // USE_DWMAPI
void InitDarkMode();
void RefreshTitleBarThemeColor(HWND hWnd);

constexpr COLORREF darkBkColor = 0x383838;
constexpr COLORREF darkTextColor = 0xFFFFFF;
extern "C" BOOL g_darkModeSupported;
enum IMMERSIVE_HC_CACHE_MODE {
    IHCM_USE_CACHED_VALUE,
    IHCM_REFRESH
};

// 1903 18362
enum class PreferredAppMode {
    Default,
    AllowDark,
    ForceDark,
    ForceLight,
    Max
};

enum WINDOWCOMPOSITIONATTRIB {
    WCA_UNDEFINED = 0,
    WCA_NCRENDERING_ENABLED = 1,
    WCA_NCRENDERING_POLICY = 2,
    WCA_TRANSITIONS_FORCEDISABLED = 3,
    WCA_ALLOW_NCPAINT = 4,
    WCA_CAPTION_BUTTON_BOUNDS = 5,
    WCA_NONCLIENT_RTL_LAYOUT = 6,
    WCA_FORCE_ICONIC_REPRESENTATION = 7,
    WCA_EXTENDED_FRAME_BOUNDS = 8,
    WCA_HAS_ICONIC_BITMAP = 9,
    WCA_THEME_ATTRIBUTES = 10,
    WCA_NCRENDERING_EXILED = 11,
    WCA_NCADORNMENTINFO = 12,
    WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
    WCA_VIDEO_OVERLAY_ACTIVE = 14,
    WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
    WCA_DISALLOW_PEEK = 16,
    WCA_CLOAK = 17,
    WCA_CLOAKED = 18,
    WCA_ACCENT_POLICY = 19,
    WCA_FREEZE_REPRESENTATION = 20,
    WCA_EVER_UNCLOAKED = 21,
    WCA_VISUAL_OWNER = 22,
    WCA_HOLOGRAPHIC = 23,
    WCA_EXCLUDED_FROM_DDA = 24,
    WCA_PASSIVEUPDATEMODE = 25,
    WCA_USEDARKMODECOLORS = 26,
    WCA_LAST = 27
};

struct WINDOWCOMPOSITIONATTRIBDATA {
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
};

extern "C" {
NTSYSAPI VOID NTAPI RtlGetNtVersionNumbers(_Out_ LPDWORD major, _Out_ LPDWORD minor, _Out_ LPDWORD build);
WINUSERAPI BOOL WINAPI SetWindowCompositionAttribute(_In_ HWND hWnd, _In_ WINDOWCOMPOSITIONATTRIBDATA*);

// 1809 17763
THEMEAPI_(BOOL)
ShouldAppsUseDarkMode(); // ordinal 132
THEMEAPI_(BOOL)
AllowDarkModeForWindow(HWND hWnd, BOOL allow); // ordinal 133
THEMEAPI_(BOOL)
AllowDarkModeForApp(BOOL allow); // ordinal 135, in 1809
THEMEAPI_(VOID)
FlushMenuThemes(); // ordinal 136
THEMEAPI_(VOID)
RefreshImmersiveColorPolicyState(); // ordinal 104
THEMEAPI_(BOOL)
IsDarkModeAllowedForWindow(HWND hWnd); // ordinal 137
THEMEAPI_(BOOL)
GetIsImmersiveColorUsingHighContrast(IMMERSIVE_HC_CACHE_MODE mode); // ordinal 106
THEMEAPI_(HTHEME)
OpenNcThemeData(HWND hWnd, LPCWSTR pszClassList); // ordinal 49

// 1903 18362
THEMEAPI_(BOOL)
ShouldSystemUseDarkMode(); // ordinal 138
// THEMEAPI_(PreferredAppMode) SetPreferredAppMode(PreferredAppMode appMode); // ordinal 135, in 1903
THEMEAPI_(BOOL)
IsDarkModeAllowedForApp(); // ordinal 139
}

using fnSetPreferredAppMode = PreferredAppMode(WINAPI*)(PreferredAppMode appMode); // ordinal 135, in 1903

DWORD g_buildNumber = 0;

extern "C" BOOL IsHighContrast()
{
    HIGHCONTRASTW highContrast = { sizeof(highContrast) };
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(highContrast), &highContrast, FALSE))
        return highContrast.dwFlags & HCF_HIGHCONTRASTON;
    return false;
}

extern "C" void RefreshTitleBarThemeColor(HWND hWnd)
{
    BOOL dark = FALSE;
    if (IsDarkModeAllowedForWindow(hWnd) && ShouldAppsUseDarkMode() && !IsHighContrast()) {
        dark = TRUE;
    }
#if USE_DWMAPI
    if (SUCCEEDED(DwmSetWindowAttribute(hWnd, 20, &dark, sizeof(dark))))
        return;
    DwmSetWindowAttribute(hWnd, 19, &dark, sizeof(dark));
#else // USE_DWMAPI
    if (g_buildNumber < 18362)
        SetPropW(hWnd, L"UseImmersiveDarkModeColors", reinterpret_cast<HANDLE>(static_cast<INT_PTR>(dark)));
    else {
        WINDOWCOMPOSITIONATTRIBDATA data = { WCA_USEDARKMODECOLORS, &dark, sizeof(dark) };
        SetWindowCompositionAttribute(hWnd, &data);
    }
#endif // USE_DWMAPI
}

extern "C" BOOL IsColorSchemeChangeMessage(LPARAM lParam)
{
    BOOL is = false;
    if (lParam && CompareStringOrdinal(reinterpret_cast<LPCWCH>(lParam), -1, L"ImmersiveColorSet", -1, TRUE) == CSTR_EQUAL) {
        RefreshImmersiveColorPolicyState();
        is = true;
    }
    GetIsImmersiveColorUsingHighContrast(IHCM_REFRESH);
    return is;
}

BOOL IsColorSchemeChangeMessage(UINT message, LPARAM lParam)
{
    if (message == WM_SETTINGCHANGE)
        return IsColorSchemeChangeMessage(lParam);
    return false;
}

extern "C" void SetAppDarkMode(BOOL allowDark)
{
    if (g_buildNumber < 18362)
        AllowDarkModeForApp(allowDark);
    else
        reinterpret_cast<fnSetPreferredAppMode>(AllowDarkModeForApp)(allowDark ? PreferredAppMode::AllowDark : PreferredAppMode::Default);
}

extern "C" void FixDarkScrollBar()
{
    HMODULE hComctl = LoadLibraryExW(L"comctl32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (hComctl) {
        auto addr = FindDelayLoadThunkInModule((void*)hComctl, "uxtheme.dll", 49); // OpenNcThemeData
        if (addr) {
            DWORD oldProtect;
            if (VirtualProtect(addr, sizeof(IMAGE_THUNK_DATA), PAGE_READWRITE, &oldProtect)) {
                auto MyOpenThemeData = [](HWND hWnd, LPCWSTR classList) -> HTHEME {
                    if (wcscmp(classList, L"ScrollBar") == 0) {
                        hWnd = nullptr;
                        classList = L"Explorer::ScrollBar";
                    }
                    return OpenNcThemeData(hWnd, classList);
                };

                addr->u1.Function = reinterpret_cast<ULONG_PTR>(static_cast<decltype(OpenNcThemeData)*>(MyOpenThemeData));
                VirtualProtect(addr, sizeof(IMAGE_THUNK_DATA), oldProtect, &oldProtect);
            }
        }
    }
}

constexpr BOOL CheckBuildNumber(DWORD buildNumber)
{
    return (buildNumber == 17763 || // 1809
        buildNumber == 18362 || // 1903 & 1909
        buildNumber == 19041); // 2004 & 20H2 & 21H1
}

extern "C" void InitDarkMode()
{
    g_darkModeSupported = FALSE;
    DWORD major, minor;
#if USE_GETVERSION
#pragma warning(push)
#pragma warning(disable : 4996)
    DWORD version = GetVersion();
#pragma warning(pop)
    major = static_cast<DWORD>(LOBYTE(LOWORD(version)));
    minor = static_cast<DWORD>(HIBYTE(LOWORD(version)));
#else // USE_GETVERSION
    DWORD build;
    RtlGetNtVersionNumbers(&major, &minor, &build);
#endif // USE_GETVERSION
    if (major == 10 && minor == 0) {
        __try {
            __HrLoadAllImportsForDll("UxTheme.dll"); // Case sensitive
        } __except (GetExceptionCode() == VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
            return;
        }

        wchar_t path[MAX_PATH];
        constexpr wchar_t uxtheme[] = L"uxtheme.dll";
        UINT i = GetSystemDirectoryW(path, ARRAYSIZE(path));
        if (i + 1 + ARRAYSIZE(uxtheme) <= ARRAYSIZE(path)) {
            path[i] = L'\\';
            memcpy(path + i + 1, uxtheme, ARRAYSIZE(uxtheme) * sizeof(wchar_t));

            DWORD handle;
            DWORD size = GetFileVersionInfoSizeExW(0, path, &handle);
            if (size != 0) {
                LPVOID data = malloc(size);
                if (data) {
                    if (GetFileVersionInfoExW(0, path, handle, size, data)) {
                        VS_FIXEDFILEINFO* fileInfo;
                        UINT len;
                        if (VerQueryValueW(data, L"\\", reinterpret_cast<LPVOID*>(&fileInfo), &len)) {
                            g_buildNumber = static_cast<DWORD>(HIWORD(fileInfo->dwFileVersionLS));
                            if (CheckBuildNumber(g_buildNumber))
                                g_darkModeSupported = true;
                        }
                    }
                    free(data);
                }
            }
        }

        if (!g_darkModeSupported)
            return;

        SetAppDarkMode(true);
        RefreshImmersiveColorPolicyState();

        FixDarkScrollBar();
    }
}

extern "C" BOOL SetDarkThemeColors(HBRUSH* hbrBkgnd, HDC hdc)
{
    if (!*hbrBkgnd)
        *hbrBkgnd = CreateSolidBrush(darkBkColor);
    SetTextColor(hdc, darkTextColor);
    SetBkColor(hdc, darkBkColor);
    return !!*hbrBkgnd;
}

extern "C" void OnWmCreate(HWND hWnd)
{
    AllowDarkModeForWindow(hWnd, true);
    RefreshTitleBarThemeColor(hWnd);
    // Set window background color
    HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
    SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)brush);
}
