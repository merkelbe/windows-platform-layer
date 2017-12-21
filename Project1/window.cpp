#include <Windows.h>

const char className[] = "myWindowClass";

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

	// Step 3: Message Loop
	while (GetMessage(&message, NULL, 0, 0) > 0) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
	return message.wParam;
}