#include "TextureLoader.h"

#include <string>

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))


#define StrToInt(a,b,c) MAKEFOURCC('0', a,b,c)

HRESULT TextureLoader::Load(const wchar_t* p, ComPtr<ID3D12GraphicsCommandList> cmdList, ComPtr<ID3D12Device> device, _Out_ CTexture& texture)
{
	std::wstring path(p);
	int strLen = path.length();
	
	if (strLen < 4)
		return E_FAIL;

	std::wstring ext = path.substr(strLen - 3, 3);
	std::string extA;
	extA.assign(ext.begin(), ext.end());
	
	for (int i = 0; i < 3; i++)
		extA[i] = toupper(ext[i]);

	switch (StrToInt(extA[0], extA[1], extA[2]))
	{
	case StrToInt('D','D','S'):
		return _ReadFromDDSFile(p, cmdList, device, texture);
	}
	
	return E_FAIL;
}