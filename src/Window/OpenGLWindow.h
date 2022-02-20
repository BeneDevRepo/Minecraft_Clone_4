#pragma once

#include "Window.h"

class OpenGLWindow {
public:
	Window win;

private:
	HGLRC opengl_context;
    HDC device_context;

private:
	static LRESULT StaticWndProc(void *const win, const UINT msg, const WPARAM wParam, const LPARAM lParam);
	LRESULT WndProc(const UINT msg, const WPARAM wParam, const LPARAM lParam);

public:
	OpenGLWindow(const int width, const int height);
	~OpenGLWindow();
	void updateScreen();

public:
	inline bool shouldClose() { return win.shouldClose; }
	inline void pollMsg() { win.pollMsg(); }
	inline void swapBuffers() { SwapBuffers(device_context); }
};