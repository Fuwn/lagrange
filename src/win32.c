/* Copyright 2020 Jaakko Keränen <jaakko.keranen@iki.fi>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include "win32.h"
#include "ui/window.h"
#include "app.h"
#include <SDL_syswm.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <d2d1.h>

void setDPIAware_Win32(void) {
    SetProcessDPIAware();
}

float desktopDPI_Win32(void) {
    /* Query Direct2D for the desktop DPI (not aware of which monitor, though). */
    float ratio = 1.0f;
    ID2D1Factory *d2dFactory = NULL;
    HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED, &IID_ID2D1Factory, NULL, (void **) &d2dFactory);
    if (SUCCEEDED(hr)) {
        FLOAT dpiX = 96;
        FLOAT dpiY = 96;
        ID2D1Factory_GetDesktopDpi(d2dFactory, &dpiX, &dpiY);
        ratio = (float) (dpiX / 96.0);
        ID2D1Factory_Release(d2dFactory);
    }
    return ratio;
}

void useExecutableIconResource_SDLWindow(SDL_Window *win) {
    HINSTANCE handle = GetModuleHandle(NULL);
    HICON icon = LoadIcon(handle, "IDI_ICON1");
    if (icon) {
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        if (SDL_GetWindowWMInfo(win, &wmInfo)) {
            HWND hwnd = wmInfo.info.win.window;
            SetClassLongPtr(hwnd, -14 /*GCL_HICON*/, (LONG_PTR) icon);
        }
    }
}

#if defined (LAGRANGE_CUSTOM_FRAME)
iInt2 cursor_Win32(void) {
    POINT p;
    GetPhysicalCursorPos(&p);
    return init_I2(p.x, p.y);
}

void processNativeEvent_Win32(const struct SDL_SysWMmsg *msg, iWindow *window) {
    static int winDown_[2] = { 0, 0 };
    HWND hwnd = msg->msg.win.hwnd;
    //printf("[syswm] %x\n", msg->msg.win.msg); fflush(stdout);
    const WPARAM wp = msg->msg.win.wParam;
    switch (msg->msg.win.msg) {
        case WM_ACTIVATE: {
            //LONG style = GetWindowLong(hwnd, GWL_STYLE);
            //SetWindowLog(hwnd, GWL_STYLE, style);
            iZap(winDown_); /* may have hidden the up event */
            break;
        }
        case WM_KEYDOWN: {
            if (wp == VK_LWIN) {
                //printf("lwin down\n"); fflush(stdout);
                winDown_[0] = iTrue;
            }
            else if (wp == VK_RWIN) {
                //printf("rwin down\n"); fflush(stdout);
                winDown_[1] = iTrue;
            }
            break;
        }
        case WM_KEYUP: {
            if (winDown_[0] || winDown_[1]) {
                /* Emulate the default window snapping behavior. */
                int snap = snap_Window(window);
                if (wp == VK_LEFT) {
                    snap &= ~(topBit_WindowSnap | bottomBit_WindowSnap);
                    setSnap_Window(window,
                                   snap == right_WindowSnap ? 0 : left_WindowSnap);
                }
                else if (wp == VK_RIGHT) {
                    snap &= ~(topBit_WindowSnap | bottomBit_WindowSnap);
                    setSnap_Window(window,
                                   snap == left_WindowSnap ? 0 : right_WindowSnap);
                }
                else if (wp == VK_UP) {
                    if (~snap & topBit_WindowSnap) {
                        setSnap_Window(window,
                                       snap & bottomBit_WindowSnap ? snap & ~bottomBit_WindowSnap
                                       : snap == left_WindowSnap || snap == right_WindowSnap
                                           ? snap | topBit_WindowSnap
                                           : maximized_WindowSnap);
                    }
                    else {
                        postCommand_App("window.maximize");
                    }
                }
                else if (wp == VK_DOWN) {
                    if (snap == 0 || snap & bottomBit_WindowSnap) {
                        postCommand_App("window.minimize");
                    }
                    else {
                        setSnap_Window(window,
                                       snap == maximized_WindowSnap ? 0
                                       : snap & topBit_WindowSnap   ? snap & ~topBit_WindowSnap
                                       : snap == left_WindowSnap || snap == right_WindowSnap
                                           ? snap | bottomBit_WindowSnap
                                           : 0);
                    }
                }
            }
            if (wp == VK_LWIN) {
                winDown_[0] = iFalse;
            }
            if (wp == VK_RWIN) {
                winDown_[1] = iFalse;
            }
            break;            
        }
        case WM_NCLBUTTONDBLCLK: {
            POINT point = { GET_X_LPARAM(msg->msg.win.lParam), 
                            GET_Y_LPARAM(msg->msg.win.lParam) };
            ScreenToClient(hwnd, &point);
            iInt2 pos = init_I2(point.x, point.y);
            switch (hitTest_Window(window, pos)) {
                case SDL_HITTEST_DRAGGABLE:
                    window->ignoreClick = iTrue; /* avoid hitting something inside the window */
                    postCommandf_App("window.%s",
                                     snap_Window(window) ? "restore" : "maximize toggle:1");
                    break;
                case SDL_HITTEST_RESIZE_TOP:
                case SDL_HITTEST_RESIZE_BOTTOM: {
                    window->ignoreClick = iTrue; /* avoid hitting something inside the window */
                    setSnap_Window(window, yMaximized_WindowSnap);
                    break;
                }
            }
            //fflush(stdout);
            break;
        }
#if 0
        case WM_NCLBUTTONUP: {
            POINT point = { GET_X_LPARAM(msg->msg.win.lParam), 
                            GET_Y_LPARAM(msg->msg.win.lParam) };
            printf("%d,%d\n", point.x, point.y); fflush(stdout);
            ScreenToClient(hwnd, &point);
            iInt2 pos = init_I2(point.x, point.y);
            if (hitTest_Window(window, pos) == SDL_HITTEST_DRAGGABLE) {
                printf("released draggable\n"); fflush(stdout);
            }
            break;
        }
#endif
#if 0
        /* SDL does not use WS_SYSMENU on the window, so we can't display the system menu.
           However, the only useful function in the menu would be moving-via-keyboard,
           but that doesn't work with a custom frame. We could show a custom system menu? */
        case WM_NCRBUTTONUP: {
            POINT point = { GET_X_LPARAM(msg->msg.win.lParam), 
                            GET_Y_LPARAM(msg->msg.win.lParam) };
            HMENU menu = GetSystemMenu(hwnd, FALSE);
            printf("menu at %d,%d menu:%p\n", point.x, point.y, menu); fflush(stdout);
            TrackPopupMenu(menu, TPM_RIGHTBUTTON, point.x, point.y, 0, hwnd, NULL);
            break;
        }
#endif
    }
}
#endif /* defined (LAGRANGE_CUSTOM_FRAME) */
