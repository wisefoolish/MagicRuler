#include"Graph.h"
#include<process.h>
#define HXD_LARGESTDEQUE 10

HWND HXD_global = NULL;
HWND HXD_original = NULL;
CRITICAL_SECTION HXD_gcs;
HANDLE HXD_winthread;
HDC HXD_memDc;
HBITMAP HXD_bitmap;
int HXD_width = 0;
int HXD_height = 0;
MSG HXD_message[HXD_LARGESTDEQUE];
int HXD_message_len = 0;
BOOL isBeginBatchDraw = FALSE;

void PutInMessageDeque(MSG msg)
{
    if (HXD_message_len < HXD_LARGESTDEQUE)HXD_message[HXD_message_len++] = msg;
    else
    {
        for (int i = 0; i < HXD_LARGESTDEQUE - 1; i++)HXD_message[i] = HXD_message[i + 1];
        HXD_message_len = HXD_LARGESTDEQUE - 1;
        HXD_message[HXD_message_len] = msg;
    }
}

BOOL GetHeadDeque(MSG* msg)
{
    if (HXD_message_len <= 0)return FALSE;
    if (msg != NULL)*msg = HXD_message[0];
    for (int i = 0; i < HXD_message_len - 1; i++)HXD_message[i] = HXD_message[i + 1];
    HXD_message_len--;
    return TRUE;
}

typedef struct CADParam
{
    const TCHAR* WinClassName;
    int width;
    int height;
    HWND* hwnd;
    HANDLE handle;
};

void RegisterWinClass(WNDCLASSEX* wincl, HINSTANCE hInstance, const TCHAR* ClassName, LRESULT(CALLBACK* WndProc)(HWND, UINT, WPARAM, LPARAM),
    const TCHAR* icon_Big, const TCHAR* icon_Small, const TCHAR* cursor, COLORREF background)
{
    wincl->hInstance = hInstance;           // 窗体实例，当前运行的程序实例
    wincl->lpszClassName = ClassName;       // 窗体类名称
    wincl->lpfnWndProc = WndProc;           // 为窗体类指定回调函数
    wincl->style = CS_DBLCLKS;              // 窗体风格，这个窗体支持双击事件
    wincl->cbSize = sizeof(WNDCLASSEX);     // 指定结构体大小

    HANDLE retVal = NULL;

    retVal = LoadImage(NULL, icon_Big, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    if (retVal != NULL)
        wincl->hIcon = (HICON)retVal;         // 程序图标
    else
        wincl->hIcon = LoadIcon(NULL, IDI_APPLICATION);

    retVal = LoadImage(NULL, icon_Small, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    if (retVal != NULL)
        wincl->hIconSm = (HICON)retVal;     // 窗体运行时左上角图标
    else
        wincl->hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    retVal = LoadImage(NULL, cursor, IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE);
    if (retVal != NULL)
        wincl->hCursor = (HCURSOR)retVal;     // 指定光标
    else
        wincl->hCursor = LoadCursor(NULL, IDC_ARROW);     // 指定光标

    wincl->lpszMenuName = NULL;      // 菜单
    wincl->cbClsExtra = 0;
    wincl->cbWndExtra = 0;
    wincl->hbrBackground = CreateSolidBrush(background); // 指定窗口背景色
}

LRESULT CALLBACK    CallBackFunc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        EnterCriticalSection(&HXD_gcs);
        BitBlt(hdc, 0, 0, HXD_width, HXD_height, HXD_memDc, 0, 0, SRCCOPY);
        LeaveCriticalSection(&HXD_gcs);
        EndPaint(hwnd, &ps);
        break;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
        break;
    }
    return 0;
}

unsigned __stdcall CreateAndDeal(void* lparam)
{
    CADParam param = *(CADParam*)lparam;
    *(param.hwnd) = CreateWindowEx(0,         // 传0即可
        param.WinClassName,                 // 窗体类名称
        _T("窗口应用"),                     // 窗体标题
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, 0, param.width, param.height,   // 位置和窗口大小
        HWND_DESKTOP,       // 当前窗体的父级窗体句柄，桌面就是父窗体
        NULL,               // 菜单
        NULL,               // 当前程序实例
        NULL);              // 这个传NULL就行
    ShowWindow(*(param.hwnd), SW_SHOW);
    ReleaseSemaphore(param.handle, 1, NULL);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        EnterCriticalSection(&HXD_gcs);
        PutInMessageDeque(msg);
        LeaveCriticalSection(&HXD_gcs);
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    SendMessage(HXD_original, WM_CLOSE, 0, 0);
    return msg.wParam;
}

HWND initgraph(int width, int height, int cmd)
{
    if (HXD_global != NULL) return HXD_global;
    if (HXD_original == NULL)
        HXD_original = GetForegroundWindow();
    if ((cmd & SHOWCONSOLE) == 0) ShowWindow(HXD_original, SW_HIDE);
    else if (HXD_global != NULL)ShowWindow(HXD_global, SW_SHOW);
    InitializeCriticalSection(&HXD_gcs);    // 初始化
    TCHAR WinClassName[] = _T("HXD_WINDOW");

    WNDCLASSEX wincl;
    RegisterWinClass(&wincl, NULL, WinClassName, CallBackFunc, NULL, NULL, NULL, RGB(230, 230, 230));
    if (!RegisterClassEx(&wincl))return HXD_global;
    HANDLE created = CreateSemaphore(NULL, 0, 1, TEXT("created"));
    CADParam cad;
    cad.handle = created;
    cad.height = height;
    cad.width = width;
    cad.hwnd = &HXD_global;
    cad.WinClassName = WinClassName;

    HXD_winthread = (HANDLE)_beginthreadex(NULL, 0, &CreateAndDeal, &cad, 0, NULL);
    WaitForSingleObject(created, INFINITE);
    CloseHandle(created);
    HDC winDC = GetDC(HXD_global);
    HXD_memDc = CreateCompatibleDC(winDC);
    HXD_bitmap = CreateCompatibleBitmap(winDC, width, height);
    SelectObject(HXD_memDc, HXD_bitmap);
    ReleaseDC(HXD_global, winDC);
    HXD_width = width;
    HXD_height = height;
    return HXD_global;
}

void putpixel(int x, int y, COLORREF col)
{
    EnterCriticalSection(&HXD_gcs);// 等待
    SetPixel(HXD_memDc, x, y, col);
    if (!isBeginBatchDraw)
    InvalidateRect(HXD_global, NULL, FALSE);
    LeaveCriticalSection(&HXD_gcs);// 释放
}

void setlinecolor(COLORREF col)
{
    EnterCriticalSection(&HXD_gcs);// 等待
    // PS_NULL 就不可见
    DeleteObject(SelectObject(HXD_memDc, CreatePen(PS_SOLID, 0, col)));
    LeaveCriticalSection(&HXD_gcs);// 释放
}

void setfillcolor(COLORREF col)
{
    EnterCriticalSection(&HXD_gcs);// 等待
    //GetStockObject(NULL_BRUSH) 就不可见
    DeleteObject(SelectObject(HXD_memDc, CreateSolidBrush(col)));
    LeaveCriticalSection(&HXD_gcs);// 释放
}

void line(int x1, int y1, int x2, int y2)
{
    EnterCriticalSection(&HXD_gcs);// 等待
    MoveToEx(HXD_memDc, x1, y1, NULL);  //画一条直线
    LineTo(HXD_memDc, x2, y2);
    if (!isBeginBatchDraw)
    InvalidateRect(HXD_global, NULL, FALSE);
    LeaveCriticalSection(&HXD_gcs);// 释放
}

void rectangle(int x1, int y1, int x2, int y2)
{
    EnterCriticalSection(&HXD_gcs);// 等待
    HGDIOBJ old = SelectObject(HXD_memDc, GetStockObject(NULL_BRUSH));
    Rectangle(HXD_memDc, x1, y1, x2, y2);
    DeleteObject(SelectObject(HXD_memDc, old));
    if (!isBeginBatchDraw)
    InvalidateRect(HXD_global, NULL, FALSE);
    LeaveCriticalSection(&HXD_gcs);// 释放
}

void fillrectangle(int x1, int y1, int x2, int y2)
{
    EnterCriticalSection(&HXD_gcs);// 等待
    Rectangle(HXD_memDc, x1, y1, x2, y2);
    if (!isBeginBatchDraw)
    InvalidateRect(HXD_global, NULL, FALSE);
    LeaveCriticalSection(&HXD_gcs);// 释放
}

void solidrectangle(int x1, int y1, int x2, int y2)
{
    EnterCriticalSection(&HXD_gcs);// 等待
    HGDIOBJ old = SelectObject(HXD_memDc, CreatePen(PS_NULL, 0, 0));
    Rectangle(HXD_memDc, x1, y1, x2, y2);
    DeleteObject(SelectObject(HXD_memDc, old));
    if (!isBeginBatchDraw)
    InvalidateRect(HXD_global, NULL, FALSE);
    LeaveCriticalSection(&HXD_gcs);// 释放
}

void floodfill(int x, int y, COLORREF col, UINT type)
{
    EnterCriticalSection(&HXD_gcs);// 等待
    // FLOODFILLBORDER  遇 col 就停止
    // FLOODFILLSURFACE 遇 col 就继续
    ExtFloodFill(HXD_memDc, x, y, col, type);
    if (!isBeginBatchDraw)
    InvalidateRect(HXD_global, NULL, FALSE);
    LeaveCriticalSection(&HXD_gcs);// 释放
}

void circle(int x, int y, int radius)
{
    EnterCriticalSection(&HXD_gcs);// 等待
    HGDIOBJ old = SelectObject(HXD_memDc, GetStockObject(NULL_BRUSH));
    Ellipse(HXD_memDc, x - radius, y - radius, x + radius, y + radius);
    DeleteObject(SelectObject(HXD_memDc, old));
    if (!isBeginBatchDraw)
    InvalidateRect(HXD_global, NULL, FALSE);
    LeaveCriticalSection(&HXD_gcs);// 释放
}

void fillcircle(int x, int y, int radius)
{
    EnterCriticalSection(&HXD_gcs);// 等待
    Ellipse(HXD_memDc, x - radius, y - radius, x + radius, y + radius);
    if (!isBeginBatchDraw)
    InvalidateRect(HXD_global, NULL, FALSE);
    LeaveCriticalSection(&HXD_gcs);// 释放
}

void solidcircle(int x, int y, int radius)
{
    EnterCriticalSection(&HXD_gcs);// 等待
    HGDIOBJ old = SelectObject(HXD_memDc, CreatePen(PS_NULL, 0, 0));
    Ellipse(HXD_memDc, x - radius, y - radius, x + radius, y + radius);
    DeleteObject(SelectObject(HXD_memDc, old));
    if (!isBeginBatchDraw)
        InvalidateRect(HXD_global, NULL, FALSE);
    LeaveCriticalSection(&HXD_gcs);// 释放
}

ExMessage getmessage()
{
    MSG msg;
    ExMessage result = { 0 };
    while (1)
    {
        BOOL isGetMsg = FALSE;
        EnterCriticalSection(&HXD_gcs);
        isGetMsg = GetHeadDeque(&msg);
        LeaveCriticalSection(&HXD_gcs);
        if (isGetMsg == TRUE)break;
        Sleep(1);
    }
    result.message = msg.message;
    result.vkcode = msg.wParam;
    result.x = LOWORD(msg.lParam);
    result.y = HIWORD(msg.lParam);
    result.lbutton = (msg.message == WM_LBUTTONDOWN);
    result.rbutton = (msg.message == WM_RBUTTONDOWN);
    result.mbutton = (msg.message == WM_MBUTTONDOWN);
    result.wheel = GET_WHEEL_DELTA_WPARAM(msg.wParam);
    return result;
}

BOOL peekmessage(ExMessage* message)
{
    MSG msg;
    BOOL result = FALSE;
    EnterCriticalSection(&HXD_gcs);
    result = GetHeadDeque(&msg);
    LeaveCriticalSection(&HXD_gcs);
    if (message != NULL && result != FALSE)
    {
        message->message = msg.message;
        message->vkcode = msg.wParam;
        message->x = LOWORD(msg.lParam);
        message->y = HIWORD(msg.lParam);
        if (msg.message == WM_LBUTTONDOWN)
            message->lbutton = TRUE;
        else if (msg.message == WM_LBUTTONUP)
            message->lbutton = FALSE;
        if (msg.message == WM_RBUTTONDOWN)
            message->rbutton = TRUE;
        else if (msg.message == WM_RBUTTONUP)
            message->rbutton = FALSE;
        if (msg.message == WM_MBUTTONDOWN)
            message->mbutton = TRUE;
        else if (msg.message == WM_MBUTTONUP)
            message->mbutton = FALSE;
        message->wheel = GET_WHEEL_DELTA_WPARAM(msg.wParam);
    }
    return result;
}

void closegraph()
{
    if (HXD_global != NULL)
    {
        SendMessage(HXD_global, WM_CLOSE, 0, 0);
        WaitForSingleObject(HXD_winthread, INFINITE);
        DeleteCriticalSection(&HXD_gcs);
        ReleaseDC(HXD_global, HXD_memDc);
        DeleteObject(HXD_bitmap);
        CloseHandle(HXD_winthread);
    }
}

void cleardevice()
{
    COLORREF col = GetBkColor(HXD_memDc);
    HBITMAP newBmp = CreateCompatibleBitmap(HXD_memDc, HXD_width, HXD_height);
    HDC newDC = CreateCompatibleDC(HXD_memDc);
    SelectObject(newDC, newBmp);
    HBRUSH brush = CreateSolidBrush(col);
    RECT rect = { 0, 0, HXD_width, HXD_height };
    FillRect(newDC, &rect, brush);
    DeleteObject(brush);
    BitBlt(HXD_memDc, 0, 0, HXD_width, HXD_height, newDC, 0, 0, SRCCOPY);
    SelectObject(HXD_memDc, HXD_bitmap);
    DeleteDC(newDC);
    DeleteObject(newBmp);
    if(!isBeginBatchDraw)
    InvalidateRect(HXD_global, NULL, FALSE);
}

void setbkcolor(COLORREF col)
{
    SetBkColor(HXD_memDc, col);
}

void BeginBatchDraw()
{
    isBeginBatchDraw = TRUE;
}

void FlushBatchDraw()
{
    if (isBeginBatchDraw)
    {
        InvalidateRect(HXD_global, NULL, FALSE);
        Sleep(1);
    }
}

void EndBatchDraw()
{
    isBeginBatchDraw = FALSE;
}
