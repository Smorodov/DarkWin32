#include <windows.h>
extern "C" {
	// 1809 17763
	void __stdcall ShouldAppsUseDarkMode() {MessageBox(NULL,L"Stub",L"",0);} // ordinal 132
	void __stdcall AllowDarkModeForWindow(int, int) {MessageBox(NULL,L"Stub",L"",0);} // ordinal 133
	void __stdcall AllowDarkModeForApp(int) {MessageBox(NULL,L"Stub",L"",0);} // ordinal 135, in 1809
	void __stdcall FlushMenuThemes() {MessageBox(NULL,L"Stub",L"",0);} // ordinal 136
	void __stdcall RefreshImmersiveColorPolicyState() {MessageBox(NULL,L"Stub",L"",0);} // ordinal 104
	void __stdcall IsDarkModeAllowedForWindow(int) {MessageBox(NULL,L"Stub",L"",0);} // ordinal 137
	void __stdcall GetIsImmersiveColorUsingHighContrast(int) {MessageBox(NULL,L"Stub",L"",0);} // ordinal 106
	void __stdcall OpenNcThemeData(int, int) {MessageBox(NULL,L"Stub",L"",0);} // ordinal 49

	// 1903 18362
	void __stdcall ShouldSystemUseDarkMode() {MessageBox(NULL,L"Stub",L"",0);} // ordinal 138
	//void __stdcall SetPreferredAppMode(int) {MessageBox(NULL,L"Stub",L"",0);} // ordinal 135, in 1903
	void __stdcall IsDarkModeAllowedForApp() {MessageBox(NULL,L"Stub",L"",0);} // ordinal 139
}
