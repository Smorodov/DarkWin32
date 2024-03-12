#ifndef DARK_MODE_H_
#define DARK_MODE_H_
#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif
void InitDarkMode();
void OnWmCreate(HWND hWnd);
BOOL IsHighContrast();
void RefreshTitleBarThemeColor(HWND hWnd);
BOOL IsColorSchemeChangeMessage(LPARAM lParam);
//BOOL IsColorSchemeChangeMessage(UINT message, LPARAM lParam);
void SetAppDarkMode(BOOL allowDark);
void FixDarkScrollBar();
BOOL SetDarkThemeColors(HBRUSH* hbrBkgnd, HDC hdc);
#ifdef __cplusplus    
}
#endif
#endif /* DARK_MODE_H_ */
