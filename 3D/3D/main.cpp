#include "include.h"

#include "Dx.h"

void CCreateConsole()
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int cmdShow)
{
	CCreateConsole();

	if (!DX::XMVerifyCPUSupport())
	{
		MessageBox(NULL, L"XMVerifyCPUSupport", TEXT("Notice"), MB_OK);
		return 0;
	}

	try
	{
		Dx* dx =  new Dx(hInstance);
		if (!dx->Init())
		{
			return 0;
		}
		dx->Run();
	}
	catch(int i)
	{
		MessageBox(NULL, L"Error", L"Error", MB_OK);
	}

	return 0;
}