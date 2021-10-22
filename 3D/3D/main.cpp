#include "include.h"

#include "Dx.h"

void CCreateConsole()
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
}

Dx* dx;

inline void Test()
{
	//__try
	{
		if (dx->Init())
		{
			dx->Run();
		}
	}
	//__except (EXCEPTION_EXECUTE_HANDLER)
	{
		MessageBox(NULL, L"Error", L"Error", MB_OK);
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int cmdShow)
{
	if (!DX::XMVerifyCPUSupport())
	{
		MessageBox(NULL, L"XMVerifyCPUSupport", TEXT("Notice"), MB_OK);
		return 0;
	}

	dx = new Dx(hInstance);
	Test();

	return 0;
}