#include <Windows.h>
#include <stdint.h>

#define internal static
#define global static

#define uint8 unsigned char
#define uint32 UINT32

const char className[] = "myWindowClass";

global BITMAPINFO bitmapInfo;
global void *bitmapMemory;
global int bitmapWidth;
global int bitmapHeight;
global int bytesPerPixel = 4;

global uint8 blue = 255;
global uint8 green = 0;
global uint8 red = 0;
global int derp = -1;

internal void drawToBuffer() 
{
	int pitch = bitmapWidth * bytesPerPixel;
	uint8 *row = (uint8 *)bitmapMemory;

	for (int y = 0; y < bitmapHeight; ++y)
	{
		uint32 *pixel = (uint32 *)row;
		for (int x = 0; x < bitmapWidth; ++x)
		{
			*pixel++ = (red << 16 | green << 8 | blue);
		}
		row += pitch;
	}
}

internal void Win32ResizeDIBSection(int width, int height) 
{
	if (bitmapMemory)
	{
		VirtualFree(bitmapMemory, NULL, MEM_RELEASE);
	}

	bitmapWidth = width;
	bitmapHeight = height;

	bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
	bitmapInfo.bmiHeader.biWidth = width;
	bitmapInfo.bmiHeader.biHeight = height;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;
	
	int bitmapMemorySize = bytesPerPixel*bitmapWidth*bitmapHeight;
	bitmapMemory = VirtualAlloc(NULL, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

	drawToBuffer();
}

internal void updateGameWindow(HDC deviceContext, RECT *windowRectangle, LONG x, LONG y, int width, int height) 
{
	int windowWidth = windowRectangle->right - windowRectangle->left;
	int windowHeight = windowRectangle->bottom - windowRectangle->top;

	StretchDIBits(deviceContext, 0, 0, bitmapWidth, bitmapHeight, 0, 0, windowWidth, windowHeight, bitmapMemory, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}


// Step 4: the Window Procedure
LRESULT CALLBACK WndProc(HWND windowHandle, UINT message, WPARAM UIntMessageParameter, LPARAM LongMessageParameter) 
{
	switch (message)
	{
		case WM_LBUTTONDOWN: 
		{
			char fileNameSize[MAX_PATH];
			HINSTANCE handleInstance = GetModuleHandle(NULL);

			GetModuleFileName(handleInstance, fileNameSize, MAX_PATH);
			MessageBox(windowHandle, fileNameSize, "This program is: ", MB_OK | MB_ICONINFORMATION);
			break;
		}
		case WM_SIZE: 
		{
			RECT windowRectangle;
			GetClientRect(windowHandle, &windowRectangle);
			int width = windowRectangle.right - windowRectangle.left;
			int height = windowRectangle.bottom - windowRectangle.top;
			Win32ResizeDIBSection(width,height);
			break;
		}
		case WM_CLOSE:
		{
			DestroyWindow(windowHandle);
			break;
		}
		case WM_DESTROY: 
		{
			PostQuitMessage(0);
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC deviceContext = BeginPaint(windowHandle, &Paint);
			INT x = Paint.rcPaint.left;
			INT y = Paint.rcPaint.top;
			INT width = Paint.rcPaint.right - x;
			INT height = Paint.rcPaint.bottom - y;
			RECT windowRectangle;
			GetClientRect(windowHandle, &windowRectangle);
			updateGameWindow(deviceContext, &windowRectangle, x, y, width, height);
			EndPaint(windowHandle, &Paint);
			break;
		}
		default: 
		{
			return DefWindowProc(windowHandle, message, UIntMessageParameter, LongMessageParameter);
		}
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE handleInstance, HINSTANCE previousHandleInstance, LPSTR lpCmdLine, int nCmdShow) 
{
	WNDCLASSEX windowClass;
	HWND windowHandler;
	MSG message;

	// Step 1: Registering the Window Class
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = 0;
	windowClass.lpfnWndProc = WndProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = handleInstance;
	windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = className;
	windowClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&windowClass)) {
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	// Step 2: Creating the window
	windowHandler = CreateWindowEx(WS_EX_CLIENTEDGE, className, "Window naaaame", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 240, 120, NULL, NULL, handleInstance, NULL);

	if (windowHandler == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	ShowWindow(windowHandler, nCmdShow);
	UpdateWindow(windowHandler);

	bool running = true;
	// Step 3: Message Loop
	while (running) 
	{
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) 
		{
			if (message.message == WM_QUIT) {
				running = false;
			}

			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		++derp;
		switch (derp / 255)
		{
			case 0:
				++green;
				break;
			case 1:
				--blue;
				break;
			case 2:
				++red;
				break;
			case 3:
				--green;
				break;
			case 4:
				++blue;
				break;
			case 5:
				--red;
				break;
			case 6:
				derp = -1;
				break;
		}

		drawToBuffer();
		HDC deviceContext = GetDC(windowHandler);
		RECT windowRectangle;
		GetClientRect(windowHandler, &windowRectangle);
		int width = windowRectangle.right - windowRectangle.left;
		int height = windowRectangle.bottom - windowRectangle.top;
		updateGameWindow(deviceContext, &windowRectangle, 0, 0, width, height);
		ReleaseDC(windowHandler, deviceContext);
	}
	return message.wParam;
}