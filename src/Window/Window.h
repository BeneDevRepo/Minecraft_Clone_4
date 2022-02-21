#pragma once

#include "winapi_include.h"

#include <cstdint>

#include <cstdio> // ############    DEBUG


using SubWindowProc = LRESULT(*)(void *const win, const UINT msg, const WPARAM wParam, const LPARAM lParam);

class SubWindowListener {
public:
	SubWindowProc callback;
	void *win;
	SubWindowListener(const SubWindowProc callback, void *const win):
		callback(callback), win(win) { }
	inline LRESULT operator()(const UINT msg, const WPARAM wParam, const LPARAM lParam) const {
		return callback(win, msg, wParam, lParam);
	}
};

class Window {
	struct WindowStyle {
		DWORD baseStyle;
		DWORD extendedStyle;
	};

public:
	static constexpr WindowStyle DEFAULT_WINDOW_STYLE {
		.baseStyle = WS_OVERLAPPEDWINDOW, // Normal Window style
		.extendedStyle = WS_EX_APPWINDOW // Icon in taskbar
	};

private:
	static thread_local uint32_t numWindows;
	static thread_local void* mainFiber;
	void *messageFiber;

private:
	SubWindowListener fallbackListener;

public:
	uint32_t width, height;
	int32_t mouseX, mouseY;
	bool shouldClose, mouseCaptured; // internal state
	bool resizable, shouldCaptureMouse; // settable from outside
    HWND wnd;

private:
	static void staticFiberProc(void* instance);
	void fiberProc();

private:
	static LRESULT WINAPI StaticWndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT WndProc(const UINT msg, const WPARAM wParam, const LPARAM lParam);

private:
	void captureMouse();
	void releaseMouse();
	void pollRawInput(); // gets called by PollMsg(), polls RawInput messages

public:
    Window(const int width, const int height, const WindowStyle windowStyle = DEFAULT_WINDOW_STYLE);
    ~Window();
    void pollMsg();

	inline void setFallbackListener(const SubWindowListener& newFallbackListener) {
		this->fallbackListener.callback = newFallbackListener.callback;
		this->fallbackListener.win = newFallbackListener.win;
	}

	inline void setCaptureMouse(const bool capture) {
		shouldCaptureMouse = capture;
		if(shouldCaptureMouse)
			captureMouse();
		else
			releaseMouse();
	}
};