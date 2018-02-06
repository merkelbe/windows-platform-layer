#include <Windows.h>
#include <stdint.h>

#define internal static
#define global static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

const char className[] = "myWindowClass";

internal struct readFileResult
{
	uint32 ContentSize;
	void *Contents;
};

internal struct screenBuffer 
{
	void *Memory;
	int Width;
	int Height;
};


global BITMAPINFO bitmapInfo;

global readFileResult file;
//
//global void *bitmapMemory;
//global int bitmapWidth;
//global int bitmapHeight;
global screenBuffer offScreenBuffer;
global int bytesPerPixel = 4;

global uint8 blue = 255;
global uint8 green = 0;
global uint8 red = 0;
global int derp = -1;
global int velocityX = 1;
global int velocityY = 1;
global int derpX = 0;
global int derpY = 0;

internal void freeFileMemory(void *memory)
{
	if (memory)
	{
		VirtualFree(memory, NULL, MEM_RELEASE);
	}
}

inline uint32 SafeTruncateSize32(uint64 value) 
{
	if (value > 0xFFFFFFFF) {
		// TODO: Throw exception here
	}
	uint32 result = (uint32)value;
	return result;
}


internal readFileResult readFile(char *filename)
{
	readFileResult result = {};
	HANDLE fileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER fileSize;
		if (GetFileSizeEx(fileHandle, &fileSize))
		{
			// TODO: Assert(FileSize.QuadPart <= 0xFFFFFFFF);
			// Ensure the file size is 4GB or less (otherwise file loading could break)
			// Might use HeapAlloc instead of virtual Alloc in final game (or something different).  Used just for debugging.
			uint32 fileSize32 = SafeTruncateSize32(fileSize.QuadPart);

			result.Contents = VirtualAlloc(NULL, fileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if (result.Contents)
			{
				DWORD bytesRead;
				if (ReadFile(fileHandle, result.Contents, fileSize32, &bytesRead, NULL)
					&& (fileSize32 == bytesRead)) // Making sure file size hasn't changed since read
				{
					// Read file successfully
					result.ContentSize = fileSize32;
				}
				else
				{
					freeFileMemory(result.Contents);
					result.Contents = NULL;
				}
			}
			else
			{
				// TODO: Log virtual alloc error
			}
		}
		else
		{
			// TODO: Log get file size error
		}
		CloseHandle(fileHandle);
	}
	else
	{
		// TODO: Log get file handle error
	}

	return result;
}

internal void writeFile(char *filename, uint32 memorySize, void *memory)
{
	bool result = 0;
	HANDLE fileHandle = CreateFile(filename, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		DWORD bytesWritten;
		if (WriteFile(fileHandle, memory, memorySize, &bytesWritten, NULL))
		{
			result = (bytesWritten == memorySize);
			// Wrote file successfully
		}
		else
		{
			// TODO: Log write error
		}
		CloseHandle(fileHandle);
	}
	else
	{
		// TODO: Log get file handle error
	}
	return;
}

#pragma pack(push, 1)
struct bitmapHeader {
	uint16 fileType;
	uint32 fileSize; 
	uint16 reserved1;
	uint16 reserved2;
	uint32 bitmapOffset;
	uint32 size;
	int32 width;
	int32 height;
	uint16 planes;
	uint16 bitsPerPixel;
};
#pragma pack(pop)

struct bitmap 
{
	uint32 height;
	uint32 width;
    uint32 *pixels;
};

internal bitmap loadBitmap(char *filename) 
{
	bitmap result = {};

	readFileResult bitmapFile = readFile(filename);
	if (bitmapFile.ContentSize != 0) {
		bitmapHeader *header = (bitmapHeader *)bitmapFile.Contents;
		uint32 *pixels = (uint32 *)((uint8 *)bitmapFile.Contents + header->bitmapOffset);
		
		uint32 *sourceDest = pixels;
		for (int32 y = 0; y < header->width; ++y) {
			for (int32 x = 0; x < header->height; ++x) {
				*sourceDest = (*sourceDest >> 8) | (*sourceDest << 24);
				++sourceDest;
			}
		}
		result.height = header->height;
		result.width = header->width;
		result.pixels = pixels;
	}
	return result;
}

//internal void drawBitmap(char *bitmapFilename, uint32 x, uint32 y) 
//{
//	readFileResult bitmapFile = readFile(bitmapFilename);
//	bitmapHeader *header = NULL;
//	uint32 *bitmapPixels = NULL;
//	if (bitmapFile.ContentSize != 0) {
//		header = (bitmapHeader *)bitmapFile.Contents;
//		bitmapPixels = (uint32 *)((uint8 *)bitmapFile.Contents + header->bitmapOffset);
//	}
//
//	int pitch = offScreenBuffer.Width * bytesPerPixel;
//	uint8 *endOfBuffer = (uint8 *)offScreenBuffer.Memory + pitch*offScreenBuffer.Height;
//	
//	int top = y;
//	int bottom = y + header->height;
//	int left = x; 
//	int right = x + header->width;
//
//	for (int pixelX = left; pixelX < right; ++pixelX) 
//	{
//		uint8 *pixel = ((uint8 *)offScreenBuffer.Memory + pixelX * bytesPerPixel + top * pitch);
//		int bitmapPixelOffset =  pixelX * bytesPerPixel;
//
//		for (int pixelY = top; pixelY < bottom; ++pixelY) 
//		{
//			if ((pixel >= offScreenBuffer.Memory) && ((pixel + 4) <= endOfBuffer)) 
//			{
//				*(uint32 *)pixel = *bitmapPixels + bitmapPixelOffset;
//			}
//			pixel += pitch;
//			bitmapPixelOffset += bytesPerPixel * header->width;
//		}
//	}
//}

internal void drawRectangle(uint32 x, uint32 y, uint32 width, uint32 height)
{
	int pitch = offScreenBuffer.Width * bytesPerPixel;
	uint8 *endOfBuffer = (uint8 *)offScreenBuffer.Memory + pitch*offScreenBuffer.Height;

	uint32 color = 0xFFFFFFFF; //White
	int top = y;
	int bottom = y + height;
	int left = x;
	int right = x + width;
	for (int pixelX = left; pixelX < right; ++pixelX)
	{
		uint8 *pixel = ((uint8 *)offScreenBuffer.Memory + pixelX * bytesPerPixel + top * pitch);

		for (int pixelY = top; pixelY < bottom; ++pixelY)
		{
			if ((pixel >= offScreenBuffer.Memory) && ((pixel + 4) <= endOfBuffer)) 
			{
				*(uint32 *)pixel = color;
			}
			pixel += pitch;
		}
	}
}

internal void drawToBuffer() 
{
	int pitch = offScreenBuffer.Width * bytesPerPixel;
	uint8 *row = (uint8 *)offScreenBuffer.Memory;

	for (int y = 0; y < offScreenBuffer.Height; ++y)
	{
		uint32 *pixel = (uint32 *)row;
		for (int x = 0; x < offScreenBuffer.Width; ++x)
		{
			*pixel++ = (red << 16 | green << 8 | blue);
		}
		row += pitch;
	}
}

internal void Win32ResizeDIBSection(int width, int height) 
{
	if (offScreenBuffer.Memory)
	{
		VirtualFree(offScreenBuffer.Memory, NULL, MEM_RELEASE);
	}

	offScreenBuffer.Width = width;
	offScreenBuffer.Height = height;

	bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
	bitmapInfo.bmiHeader.biWidth = width;
	bitmapInfo.bmiHeader.biHeight = height;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;
	
	int bitmapMemorySize = bytesPerPixel*offScreenBuffer.Width*offScreenBuffer.Height;
	offScreenBuffer.Memory = VirtualAlloc(NULL, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

	drawToBuffer();
}

internal void updateGameWindow(HDC deviceContext, RECT *windowRectangle, LONG x, LONG y, int width, int height) 
{
	int windowWidth = windowRectangle->right - windowRectangle->left;
	int windowHeight = windowRectangle->bottom - windowRectangle->top;

	StretchDIBits(deviceContext, 0, 0, offScreenBuffer.Width, offScreenBuffer.Height, 0, 0, windowWidth, windowHeight, offScreenBuffer.Memory, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
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
	windowHandler = CreateWindowEx(WS_EX_CLIENTEDGE, className, "Window naaaame", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 960, 540, NULL, NULL, handleInstance, NULL);

	if (windowHandler == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	ShowWindow(windowHandler, nCmdShow);
	UpdateWindow(windowHandler);

	// Commented code is how to read and write shiiiit
	//char *fileName = __FILE__;
	//file = readFile(fileName);

	//if (file.Contents) 
	//{
	//	writeFile("C:/Users/Bluth/derp.txt",file.ContentSize,file.Contents);
	//	freeFileMemory(file.Contents);
	//}
/*
	uint32 *bitmapPixels = loadBitmap("C:/Users/Bluth/Desktop/Temporary/windows-platform-layer/derpTest.bmp");
	bitmapPixels = loadBitmap("C:/Users/Bluth/Desktop/Temporary/windows-platform-layer/helloThereSmall.bmp");*/

	bitmap bitmap = loadBitmap("../helloThere.bmp");


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

		derpX += velocityX;
		derpY += velocityY;

		if (derpX < 0) 
		{
			// Clamping
			derpX = 0;
			// Bouncing
			velocityX *= -1;
		}
		else if (derpX + bitmap.width > offScreenBuffer.Width) 
		{
			// Clamping
			derpX = offScreenBuffer.Width - bitmap.width;
			// Bouncing
			velocityX *= -1;
		}
		if (derpY < 0) 
		{
			// Clamping
			derpY = 0;
			// Bouncing
			velocityY *= -1;
		} 
		else if (derpY + bitmap.height > offScreenBuffer.Height) 
		{
			// Clamping
			derpY = offScreenBuffer.Height - bitmap.height;
			// Bouncing
			velocityY *= -1;
		}

		drawToBuffer();
		//drawRectangle(derpX, derpY, 50, 50);
		
		int32 bitmapWidth = bitmap.width;
		int32 bitmapHeight = bitmap.height;

		uint32 *source = bitmap.pixels;
		uint8 *destRow = (uint8 *)offScreenBuffer.Memory;
		
		int pitch = offScreenBuffer.Width * bytesPerPixel;
		uint8 *endOfBuffer = (uint8 *)offScreenBuffer.Memory + pitch*offScreenBuffer.Height;

		int top = derpY;
		int bottom = derpY + bitmapHeight;
		int left = derpX;
		int right = derpX + bitmapWidth;
		
		/*for (int pixelX = left; pixelX < right; ++pixelX)
		{
			uint8 *pixel = ((uint8 *)offScreenBuffer.Memory + pixelX * bytesPerPixel + top * pitch);

			for (int pixelY = top; pixelY < bottom; ++pixelY)
			{
				if ((pixel >= offScreenBuffer.Memory) && ((pixel + 4) <= endOfBuffer))
				{
					*(uint32 *)pixel = *source;
				}
				*source++;
				pixel += pitch;
			}
		}*/

		for (int pixelY = 0; pixelY <bitmapHeight; ++pixelY) {
			uint8 *pixel((uint8 *)offScreenBuffer.Memory + (top + pixelY) * pitch + left * bytesPerPixel);
			for (int pixelX = 0; pixelX < bitmapWidth; ++pixelX) 
			{
				if ((pixel >= offScreenBuffer.Memory) && ((pixel + 4) <= endOfBuffer))
				{
					*(uint32 *)pixel = *source;
				}
				pixel += bytesPerPixel;
				*source++;
			}
		}


/*
		for (int32 y = derpY; y < derpY + bitmapHeight; ++y) 
		{
			uint32 *dest = (uint32 *)destRow;
			for (int32 x = derpX; x < derpX + bitmapWidth; ++x) 
			{
				*dest++ = *source++;
			}
			destRow += offScreenBuffer.Width * bytesPerPixel;
		}*/


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