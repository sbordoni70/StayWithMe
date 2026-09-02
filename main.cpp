// StayWithMe - main module
#include <windows.h>
#include <shellapi.h>
#include "resource.h"


// [ private data ]
// misc data
#define TS_IDLE_TIME                        ((ULONGLONG) (115 * 1000))
// 
// timer id
#define TIMERID_PING_LIFE                   1
#define WUM_TRAYBARNOTIFY                   (WM_USER + 0)
// right-click menu ID
#define TBRCMENU_ENABLE                     100
#define TBRCMENU_EXIT                       IDCANCEL


// window data
static const wchar_t wzWndClassName[] =
    L"StayWithMe_Wnd";
static const wchar_t wzWndTitle[] =
    L"StayWithMe";


static struct {
    bool bEnabled;
    HINSTANCE hInstance;
    struct {
        UINT id;
        HICON hIcon_OFF;
        HICON hIcon_ON;
        HMENU hMenu;
        } trayBar;
    } dt;



// [ main window logic ]
// traybar - startup
bool TrayBar_Startup (HINSTANCE hInstance)
{
    // load 'local' icons
    dt.trayBar.hIcon_OFF =
            LoadIconW (hInstance, MAKEINTRESOURCEW(ICON_DISABLED));
    dt.trayBar.hIcon_ON =
            LoadIconW (hInstance, MAKEINTRESOURCEW(ICON_PROGRAM));
    if ((dt.trayBar.hIcon_OFF != NULL) && (dt.trayBar.hIcon_ON != NULL))
    {
        // create popup menu
        dt.trayBar.hMenu = CreatePopupMenu ();
        if (dt.trayBar.hMenu != NULL)
        {
            MENUITEMINFO mi;
static const wchar_t wzItem0[] =
    L"Enable";
static const wchar_t wzItem2[] =
    L"Exit";

            // init menu item - generic
            memset (&mi, 0, sizeof(mi));
            mi.cbSize = sizeof(mi);
            mi.fMask = MIIM_ID | MIIM_STATE | MIIM_TYPE;
            mi.fState = MFS_ENABLED;
            // set item[0]
            mi.wID = TBRCMENU_ENABLE;
            mi.fType = MFT_STRING;
            mi.dwTypeData = (LPWSTR) wzItem0;
            mi.cch = (sizeof(wzItem0) / sizeof(wzItem0[0])) - 1;
            InsertMenuItem (dt.trayBar.hMenu, 0, TRUE, &mi);
            // set item[1]
            mi.wID = 0;
            mi.fType = MFT_SEPARATOR;
            mi.dwTypeData = NULL;
            mi.cch = 0;
            InsertMenuItem (dt.trayBar.hMenu, 1, TRUE, &mi);
            // set item[1]
            mi.wID = TBRCMENU_EXIT;
            mi.fType = MFT_STRING;
            mi.dwTypeData = (LPWSTR) wzItem2;
            mi.cch = (sizeof(wzItem2) / sizeof(wzItem2[0])) - 1;
            InsertMenuItem (dt.trayBar.hMenu, 2, TRUE, &mi);
            // done
            return true;
        }
    }   
    return false;
}


// traybar - shutdown
bool TrayBar_Shutdown (HWND hWnd)
{
    if (dt.trayBar.id != 0)
    {
        NOTIFYICONDATA nidt;

        memset (&nidt, 0, sizeof(nidt));
        nidt.cbSize = sizeof(nidt);
        nidt.hWnd = hWnd;
        nidt.uID = dt.trayBar.id;
        Shell_NotifyIcon (NIM_DELETE, &nidt);
    }
    // free resources
    DestroyMenu (dt.trayBar.hMenu);
    dt.trayBar.hMenu = NULL;
    // reset data
    dt.trayBar.id = 0;
    return true;    
}


// taskbar - open (disabled)
bool TrayBar_Open (HWND hWnd)
{
    NOTIFYICONDATA nidt;

    UINT id = (UINT) (UINT_PTR) hWnd;
    // add/init icon
    memset (&nidt, 0, sizeof(nidt));
    nidt.cbSize = sizeof(nidt);
    nidt.hWnd = hWnd;
    nidt.uID = id;
    nidt.hIcon = dt.trayBar.hIcon_OFF;
    nidt.uCallbackMessage = WUM_TRAYBARNOTIFY;
    nidt.uFlags = NIF_ICON | NIF_MESSAGE;
    if (Shell_NotifyIcon (NIM_ADD, &nidt))
    {
        // set trayicon behavior
        nidt.uVersion = NOTIFYICON_VERSION;
        nidt.uFlags = 0;
        if (Shell_NotifyIcon (NIM_SETVERSION, &nidt))
        {
            // all done.
            dt.trayBar.id = id;
            return true;
        }
    }
    // reset data
    dt.trayBar.id = 0;
    return false;
}


// taskbar - update
bool TrayBar_Update (HWND hWnd, bool bEnabled)
{
    NOTIFYICONDATA nidt;

    // add/init icon
    memset (&nidt, 0, sizeof(nidt));
    nidt.cbSize = sizeof(nidt);
    nidt.hWnd = hWnd;
    nidt.uID = dt.trayBar.id;
    nidt.hIcon = bEnabled ? dt.trayBar.hIcon_ON : dt.trayBar.hIcon_OFF;
    nidt.uFlags = NIF_ICON;
    return Shell_NotifyIcon (NIM_MODIFY, &nidt);
}


// check and trigger life
bool WndEvent_InjectLife (HWND hWnd)
{
#ifdef _DEBUG
    hWnd;
#endif

    if (!dt.bEnabled)
        return false;
    // generate life pulse
    SetThreadExecutionState (ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
    return true;
}


// [ main window events processing ]
// process WM_CREATE
LRESULT WndEvent_CREATE (HWND hWnd)
{
    dt.bEnabled = false;
    // taskbar startup + init/open
    if (!TrayBar_Startup (dt.hInstance) || !TrayBar_Open (hWnd))
        return -1;
    // set watchdog timer
    SetTimer (hWnd, TIMERID_PING_LIFE, TS_IDLE_TIME, nullptr);
    return 0;
}


// process WM_DESTROY
LRESULT WndEvent_DESTROY (HWND hWnd)
{
    // release resources
    KillTimer (hWnd, TIMERID_PING_LIFE);
    TrayBar_Shutdown (hWnd);
    // exit    
    PostQuitMessage (0);
    return 0;
}


// process WM_CREATE
LRESULT WndEvent_TIMER (HWND hWnd, WPARAM wParam)
{
    switch (wParam)
    {
        case TIMERID_PING_LIFE:
            WndEvent_InjectLife (hWnd);
            break;
    }
    return 0;
}


// proceess enable/disable command
void WndEvent_Command_Enable (HWND hWnd)
{
    dt.bEnabled ^= true;
    // update popup menu
    CheckMenuItem (dt.trayBar.hMenu, TBRCMENU_ENABLE,
            dt.bEnabled ? (MF_BYCOMMAND | MF_CHECKED) : MF_BYCOMMAND);
    // update traybar icon
    TrayBar_Update (hWnd, dt.bEnabled);   
}


// process <taskbar_notifications>
LRESULT WndEvent_TaskBarNotifications (HWND hWnd, LPARAM lParam)
{
    switch (lParam)
    {
        case WM_RBUTTONUP:
            if (dt.trayBar.hMenu != NULL)
            {
                POINT pt;

                // show menu
                GetCursorPos (&pt);
                SetForegroundWindow (hWnd);
                TrackPopupMenu (dt.trayBar.hMenu, 0, pt.x, pt.y, 0, hWnd, NULL);
                PostMessage (hWnd, WM_NULL, 0, 0);
            }
            break;
        case WM_LBUTTONUP:
            PostMessage (hWnd, WM_COMMAND, TBRCMENU_ENABLE, 0);
            break;
    }
    return 0;
}


// window events callback
LRESULT CALLBACK WndEventProc (HWND hWnd, UINT message,
        WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_TIMER:
            return WndEvent_TIMER (hWnd, wParam);

        case WUM_TRAYBARNOTIFY:
            return WndEvent_TaskBarNotifications (hWnd, lParam);

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case TBRCMENU_EXIT:              // menu -> exit
                    PostMessage (hWnd, WM_CLOSE, 0, 0);
                    break;
                case TBRCMENU_ENABLE:
                    WndEvent_Command_Enable (hWnd);
                    break;
            }
            return 0;

        case WM_CREATE:
            return WndEvent_CREATE (hWnd);

        case WM_DESTROY:
            return WndEvent_DESTROY (hWnd);
    }
    // use the default event processing
    return DefWindowProcW (hWnd, message, wParam, lParam);
}


// winmain - entry point
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
        LPSTR lpCmdLine, int nCmdShow)
{
    HANDLE hMutex;
#ifdef _DEBUG
    hPrevInstance;
    lpCmdLine;
    nCmdShow;
#endif

    // check the presence of other instances
    {
static const wchar_t wzMutex[] =
    L"StayWithMe_mutex";

        hMutex = OpenMutex (SYNCHRONIZE, FALSE, wzMutex);
        if (hMutex != NULL)
        {
            // quit it
            CloseHandle (hMutex);
            return -1;
        }
        // create a new one
        hMutex = CreateMutex (NULL, FALSE, wzMutex);
    }
    // init data
    memset (&dt, 0, sizeof(dt));
    dt.hInstance = hInstance;
    // register window class
    {
        WNDCLASSEXW wcex;

        memset (&wcex, 0, sizeof(wcex));
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = &WndEventProc;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIconW (hInstance, MAKEINTRESOURCEW(ICON_PROGRAM));
        wcex.hCursor = LoadCursorW (nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszClassName = wzWndClassName;

        if (!RegisterClassExW (&wcex)) {
            MessageBoxW (nullptr, L"Failed to register window class.",
                    wzWndTitle, MB_ICONERROR);
            return -1;
        }
    }

    // Create the window
    HWND hWnd = CreateWindowExW (0,
            wzWndClassName, wzWndTitle,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
            nullptr, nullptr, hInstance, nullptr);

    if (hWnd == NULL) {
        MessageBoxW (NULL, L"Failed to create window.", L"Error", MB_ICONERROR);
        return -1;
    }

    // hide it
    ShowWindow (hWnd, SW_HIDE);
    UpdateWindow (hWnd);

    // Message loop
    {
        MSG msg;

        while (GetMessageW (&msg, NULL, 0, 0))
        {
            TranslateMessage (&msg);
            DispatchMessageW (&msg);
        }
    }

    // free mutex
    CloseHandle (hMutex);
    return 0;
}
