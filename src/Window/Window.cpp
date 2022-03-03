#include "Window.h"

#include <iostream>

#include <hidusage.h> // for RawInput

thread_local uint32_t Window::numWindows = 0;
thread_local void *Window::mainFiber = nullptr;

void Window::staticFiberProc(void* instance) {
	((Window*)instance)->fiberProc();
}

void Window::fiberProc() {
	while(true) {
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		SwitchToFiber(mainFiber);
	}
}

Window::Window(const int width, const int height, const WindowStyle windowStyle):
		fallbackListener(
			[](void *const win, const UINT msg, const WPARAM wParam, const LPARAM lParam)->LRESULT {
				return DefWindowProc(((Window*)win)->wnd, msg, wParam, lParam);
			}, this ),
		resizable(true), 
		shouldClose(false),

		shouldCaptureMouse(false), mouseCaptured(false),

		width(width), height(height),
		mouseX(0), mouseY(0),

		wnd(NULL) {

	if(numWindows++ == 0) {
		mainFiber = ConvertThreadToFiber(0);
		std::cout << "Creating First Window." << "\n";
	}

	messageFiber = CreateFiber(0, staticFiberProc, this);

	WNDCLASSW win_class{};
	win_class.hInstance = GetModuleHandle(NULL);
	win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	win_class.lpfnWndProc = StaticWndProc;
	win_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	win_class.lpszClassName = L"win32app";


	if (!RegisterClassW(&win_class)) {
		printf("Error registering Window Class: %i\n", (int)GetLastError());
		exit(-1);
	}

	CreateWindowExW(
		windowStyle.extendedStyle,
		win_class.lpszClassName, // lpClassName
		L"Title", // lpWindowName
		windowStyle.baseStyle,
		800, // x
		100, // y
		width, // width
		height, // height
		NULL, // hWndParent
		NULL, // hMenu
		NULL, // hInstance
		this  // lpParam
		);

	ShowWindow(wnd, SW_SHOW);
	UpdateWindow(wnd);

	RegisterHotKey(wnd, 11, MOD_NOREPEAT, VK_F11);

	SetTimer(wnd, 0xDEB, 1000, NULL); // debug
}

Window::~Window() {
	if(!this->wnd)
		return;

	DestroyWindow(this->wnd); // Destroy window

	if(--numWindows == 0) {
		ConvertFiberToThread();
		std::cout << "Destroyed last Window." << "\n";
	}
}

void Window::captureMouse() {
	if(mouseCaptured) // Don't try to capture Mouse if it's already captured...
		return;

	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
	Rid[0].usUsage = 0x02;              // HID_USAGE_GENERIC_MOUSE
	Rid[0].dwFlags = RIDEV_NOLEGACY | RIDEV_CAPTUREMOUSE; // | RIDEV_DEVNOTIFY;    // adds mouse
	Rid[0].hwndTarget = wnd; // wnd apparently needed for RIDEV_CAPTUREMOUSE to work

	// Rid[1].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
	// Rid[1].usUsage = 0x06;              // HID_USAGE_GENERIC_KEYBOARD
	// Rid[1].dwFlags = RIDEV_NOLEGACY;    // adds keyboard and also ignores legacy keyboard messages
	// Rid[1].hwndTarget = 0;

	if (RegisterRawInputDevices(Rid, 1, sizeof(RAWINPUTDEVICE)) == FALSE) {
		printf("Error registering Raw Input Device: %d\n", GetLastError());
		exit(1);
	}

	while(ShowCursor(false) >= 0); // make cursor invisible

	mouseCaptured = true;
}

void Window::releaseMouse() {
	if(!mouseCaptured) // Don't try to release Mouse if it hasn't even been captured...
		return;

	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
	Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;              // HID_USAGE_GENERIC_MOUSE = 0x02
	Rid[0].dwFlags = RIDEV_REMOVE; // hopefully removes mouse
	Rid[0].hwndTarget = 0; // must be 0 for RIDEV_REMOVE

	if (RegisterRawInputDevices(Rid, 1, sizeof(RAWINPUTDEVICE)) == FALSE) {
		printf("Error removing Raw Input Device: %d\n", GetLastError());
		exit(1);
	}

	while(ShowCursor(true) < 0); // make cursor visible

	// always Release Mouse cursor in the center of the Window:
	POINT center{ (int32_t)(width/2), (int32_t)(height/2) };
	ClientToScreen(wnd, &center);
	SetCursorPos(center.x, center.y);

	mouseCaptured = false;
}


void Window::pollRawInput() {
	constexpr uint16_t NUM_BUFFERS = 16;
	alignas(sizeof(void*)) thread_local static RAWINPUT buffers[NUM_BUFFERS]; // Buffer to read Inputs into

	UINT size = NUM_BUFFERS * sizeof(RAWINPUT);
	const int numInputs = GetRawInputBuffer(buffers, &size, sizeof(RAWINPUTHEADER));

	for(int i = 0; i < numInputs; i++) {
		const RAWINPUT *const raw = buffers + i;

		switch(raw->header.dwType) {
			case RIM_TYPEKEYBOARD:
				{
					printf(
						"Keyboard: make=%04x Flags:%04x Reserved:%04x ExtraInformation:%08x, msg=%04x VK=%04x \n",
						raw->data.keyboard.MakeCode,
						raw->data.keyboard.Flags,
						raw->data.keyboard.Reserved,
						raw->data.keyboard.ExtraInformation,
						raw->data.keyboard.Message,
						raw->data.keyboard.VKey);
				}
				break;

			case RIM_TYPEMOUSE:
				{
					const RAWMOUSE &mouse = raw->data.mouse;
					if(mouse.usFlags & MOUSE_MOVE_ABSOLUTE) {
						// printf(
						// 	"Mouse Absolute: ulButtons=%04x usButtonFlags=%04x usButtonData=%04x ulRawButtons=%04x lLastX=%04x lLastY=%04x ulExtraInformation=%04x\n",
						// 	mouse.ulButtons,
						// 	mouse.usButtonFlags,
						// 	mouse.usButtonData,
						// 	mouse.ulRawButtons,
						// 	mouse.lLastX,
						// 	mouse.lLastY,
						// 	mouse.ulExtraInformation);
						// printf("Mouse pos absolute: %d, %d\n", mouse.lLastX, mouse.lLastY);
					} else if((mouse.lLastX | mouse.lLastY) != 0) {
						// printf(
						// 	"Mouse Relative: ulButtons=%04x usButtonFlags=%04x usButtonData=%04x ulRawButtons=%04x lLastX=%04x lLastY=%04x ulExtraInformation=%04x\n",
						// 	mouse.ulButtons,
						// 	mouse.usButtonFlags,
						// 	mouse.usButtonData,
						// 	mouse.ulRawButtons,
						// 	mouse.lLastX,
						// 	mouse.lLastY,
						// 	mouse.ulExtraInformation);
						// printf("Mouse movement relative: %d, %d\n", mouse.lLastX, mouse.lLastY);
						mouseX += mouse.lLastX;
						mouseY += mouse.lLastY;
					}

					// if(mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
					// 	printf("Left down\n");
					// if(mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
					// 	printf("Right down\n");
					// if(mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
					// 	printf("Middle down\n");

					// if(mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
					// 	printf("Left UP\n");
					// if(mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
					// 	printf("Right UP\n");
					// if(mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)
					// 	printf("Middle UP\n");
					
					// if(mouse.usButtonFlags & RI_MOUSE_WHEEL)
					// 	printf("Mouse Wheel: %hd\n", (SHORT)mouse.usButtonData);

				}
				break;

			case RIM_TYPEHID:
				printf("Received Raw HID Packet. WHY?!\n");
				break;
		}
	}
}

void Window::pollMsg() {
	if(mouseCaptured)
		pollRawInput();

	SwitchToFiber(messageFiber);
}

LRESULT WINAPI Window::StaticWndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	Window *targetWindow;
	if(msg == WM_NCCREATE) {
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
		targetWindow = (Window*)(lpcs->lpCreateParams);
		targetWindow->wnd = wnd;
		SetWindowLongPtr(wnd, GWLP_USERDATA, (LONG_PTR)targetWindow);
	} else
 		targetWindow = (Window*)GetWindowLongPtr(wnd, GWLP_USERDATA);
	if (targetWindow)
		return targetWindow->WndProc(msg, wParam, lParam);
	return DefWindowProc(wnd, msg, wParam, lParam);
}

LRESULT Window::WndProc(const UINT msg, const WPARAM wParam, const LPARAM lParam) {
    switch (msg) {
		// case WM_KEYDOWN:
		// std::cout << (char)wParam << "\n";
		// 	break;

		// case WM_CHAR:
		// 	std::cout << (char)wParam << "\n";
		// 	break;

		case WM_NCLBUTTONDOWN: //WM_NCLBUTTONUP
			switch(wParam) {
				case HTCLOSE:
					shouldClose = true;
        			return 0;

				case HTMINBUTTON:
					ShowWindow(this->wnd, SW_MINIMIZE);
					return 0;

				// case HTMAXBUTTON:
				// 	ShowWindow(this->wnd, SW_MAXIMIZE);
				// 	return 0;
			}
			break;

		case WM_QUERYOPEN:
			return TRUE;

		case WM_HOTKEY:
			std::cout << "Hotkey Pressed: " << wParam << "\n";
			break;


		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
			if(!shouldCaptureMouse && !mouseCaptured) // when using legacy mouse input, capture Mouse while Mousebutton is pressed.
				SetCapture(wnd);
			break;

		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
			if(!shouldCaptureMouse && !mouseCaptured) // when using legacy mouse input, capture Mouse while Mousebutton is pressed.
				ReleaseCapture();
			break;


		case WM_SETFOCUS:
			if(shouldCaptureMouse)
				captureMouse();
			// std::cout << "WM_SETFOCUS\n";
			break;

		case WM_KILLFOCUS:
			releaseMouse();
			// std::cout << "WM_KILLFOCUS\n";
			break;

		// case WM_MOUSEMOVE:
		// 	if(captureMouse && mouseCaptured) {
		// 		// int32_t dx = GET_X_LPARAM(lParam) - width/2; // - clientRect.left;
		// 		// int32_t dy = GET_Y_LPARAM(lParam) - height/2; // - clientRect.top;
		// 		// this->mouseX += dx;
		// 		// this->mouseY += dy;
		// 		// // if(dx != 0 || dy != 0) {
		// 		// 	POINT center{ (int32_t)(width/2), (int32_t)(height/2) };
		// 		// 	ClientToScreen(wnd, &center);
		// 		// 	SetCursorPos(center.x, center.y);
		// 		// // }
		// 	} else if(!captureMouse) {
		// 		this->mouseX = GET_X_LPARAM(lParam);
		// 		this->mouseY = GET_Y_LPARAM(lParam);
		// 	}
		// 	break;

		case WM_ENTERSIZEMOVE:
			std::cout << "WM EnterSizeMove\n";
			SetTimer(wnd, 0x69420, 0, NULL);
			break;

		case WM_EXITSIZEMOVE:
			std::cout << "WM ExitSizeMove\n";
			KillTimer(wnd, 0x69420);
			break;

		case WM_TIMER:
			if(wParam == 0x69420)
				SwitchToFiber(mainFiber);
			// if(wParam == 0xDEB)
			// 	printf("Active: %d, Focused: %d\n", GetActiveWindow()==wnd, GetFocus()==wnd);
			return 0;

		case WM_SIZE:
		case WM_SIZING:
			{
				RECT winRect;
				GetClientRect(wnd, &winRect);
				this->width = winRect.right - winRect.left;
				this->height = winRect.bottom - winRect.top;
			}
			break;


        // case WM_CLOSE:
		// 	// std::cout << "WM_CLOSE";
        // case WM_DESTROY:
		// 	// shouldClose = true;
        //     // PostQuitMessage(0);
        //     return 0;
    }
    return fallbackListener(msg, wParam, lParam);
}