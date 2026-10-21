#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include "dev_window.h"

#ifdef _WIN32
static HWND hudWindow;
static char hudTitle[160] = "DEVELOPER INTELLIGENCE / PRIVATE HUD";
static char hudLines[32][160];
static int hudLineCount;
static const COLORREF bg = RGB(19, 22, 29);
static const COLORREF fg = RGB(226, 231, 239);
static const COLORREF accent = RGB(245, 190, 65);

static LRESULT CALLBACK HudWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd, &ps);
        RECT r; GetClientRect(hwnd, &r);
        HBRUSH brush = CreateSolidBrush(bg);
        FillRect(dc, &r, brush); DeleteObject(brush);
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, accent);
        HFONT titleFont = CreateFontA(20,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,
            DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,
            FIXED_PITCH | FF_MODERN,"Consolas");
        HFONT bodyFont = CreateFontA(16,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,
            DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,
            FIXED_PITCH | FF_MODERN,"Consolas");
        HFONT old = (HFONT)SelectObject(dc, titleFont);
        TextOutA(dc, 18, 16, hudTitle, (int)strlen(hudTitle));
        SelectObject(dc, bodyFont);
        SetTextColor(dc, RGB(155,165,180));
        const char *hint = "112358 toggles this window | engine_dev.log records decisions";
        TextOutA(dc, 18, 45, hint, (int)strlen(hint));
        SetTextColor(dc, fg);
        int y = 78;
        for (int i=0; i<hudLineCount && y < r.bottom-18; ++i, y+=23)
            TextOutA(dc, 18, y, hudLines[i], (int)strlen(hudLines[i]));
        SelectObject(dc, old);
        DeleteObject(titleFont); DeleteObject(bodyFont);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND: return 1;
    case WM_CLOSE: ShowWindow(hwnd, SW_HIDE); return 0;
    }
    return DefWindowProcA(hwnd,msg,wp,lp);
}
#endif

void DevWindow_Init(void) {
#ifdef _WIN32
    WNDCLASSA wc; ZeroMemory(&wc,sizeof(wc));
    wc.lpfnWndProc=HudWndProc; wc.hInstance=GetModuleHandleA(NULL);
    wc.lpszClassName="ChessEnginePrivateHud";
    wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    wc.hbrBackground=CreateSolidBrush(bg);
    RegisterClassA(&wc);
    hudWindow=CreateWindowExA(0,wc.lpszClassName,"Chess Engine - Private Intelligence",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,CW_USEDEFAULT,760,650,
        NULL,NULL,wc.hInstance,NULL);
    if (hudWindow) ShowWindow(hudWindow,SW_HIDE);
#endif
}
void DevWindow_Show(int visible) {
#ifdef _WIN32
    if (hudWindow) { ShowWindow(hudWindow,visible?SW_SHOWNORMAL:SW_HIDE);
        if (visible) { SetForegroundWindow(hudWindow); InvalidateRect(hudWindow,NULL,TRUE); } }
#else
    (void)visible;
#endif
}
void DevWindow_SetText(const char *title,const char *const *lines,int count) {
#ifdef _WIN32
    if (title) { strncpy(hudTitle,title,sizeof(hudTitle)-1); hudTitle[sizeof(hudTitle)-1]=0; }
    hudLineCount=count<32?count:32;
    for(int i=0;i<hudLineCount;i++){
        strncpy(hudLines[i],lines[i]?lines[i]:"",sizeof(hudLines[i])-1);
        hudLines[i][sizeof(hudLines[i])-1]=0;
    }
    if(hudWindow) InvalidateRect(hudWindow,NULL,TRUE);
#else
    (void)title;(void)lines;(void)count;
#endif
}
void DevWindow_Poll(void) {
#ifdef _WIN32
    if (!hudWindow) return;
    MSG msg;
    while(PeekMessageA(&msg,hudWindow,0,0,PM_REMOVE)) {
        TranslateMessage(&msg); DispatchMessageA(&msg);
    }
#endif
}
void DevWindow_Shutdown(void) {
#ifdef _WIN32
    if(hudWindow){DestroyWindow(hudWindow);hudWindow=NULL;}
#endif
}
