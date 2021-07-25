#include "include.h"

#include "Dx.h"

const auto& windowName = TEXT("3D Program");
const auto& className = TEXT("ClassName3D");

HDC hdc;
HWND handle;
int maxWidth;
int maxHeight;
int width;
int height;

std::ostream& operator<<(std::ostream& out, DX::FXMVECTOR v)
{
	DX::XMFLOAT4 p;
	DX::XMStoreFloat4(&p, v);
	out << '(' << p.x << ' ' << p.y << ' ' << p.z << ' ' << p.w << ')';
	return out;
}

std::ostream& operator<<(std::ostream& out, DX::FXMMATRIX m)
{
	for (int i = 0; i < 4; i++)
	{
		out << m.r[i] << '\n';
	}
	return out;
}

#include "MMFile.h"

void CCreateConsole()
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);

	DX::XMFLOAT3 v1(3, 3, 3);

	DX::XMVECTOR xv1 = DX::XMLoadFloat3(&v1);
	DX::XMVECTOR xv2 = DX::XMVectorSet(4, 5, 6, 0);
	//__fastcall -> __vecorcall
	
	xv2 = DX::XMVector3Normalize(xv2);

	DX::XMFLOAT3 result;
	DX::XMStoreFloat3(&result, xv1);
	
	std::cout << xv1 << '\n';
	std::cout << DX::XMVector3Length(xv1) << '\n';

	//_In_reads_(16) const float* pArray;
	DX::XMFLOAT4X4 m1;

	DX::XMMATRIX xm(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 4);
	std::cout << xm;
	DX::XMVECTOR det = DX::XMMatrixDeterminant(xm);
	DX::XMMATRIX res = DX::XMMatrixInverse(&det, xm);
	std::cout << res << '\n';
	std::cout << xm * res;
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
		Dx* dx;
		dx = new Dx(hInstance);
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

	UnregisterClass(className, hInstance);
	return 0;
}