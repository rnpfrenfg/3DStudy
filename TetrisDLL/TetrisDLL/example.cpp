#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <time.h>

#include <iostream>

#include "Tetris.h"


/*
How to run at visual studio:
	solution explorer
	-> Right Click : TetrisDLL
	-> properties
	-> Common Properties -> General -> Configuration type = .exe;
*/

using namespace TetrisSpace;

Tetris tetris;

void CCreateConsole()
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
}

int sudoBlockNow;

LRESULT CALLBACK BlockRotateTestMSG(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		return 0;
	case WM_PAINT:
		return 0;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_LEFT:
			tetris.RotateLeft();
			return 0;
		case VK_RIGHT:
			tetris.RotateRight();
			return 0;
		case VK_UP:
			sudoBlockNow++;
			tetris.SudoSetBlock(sudoBlockNow);
			return 0;
		case VK_DOWN:
			sudoBlockNow--;
			tetris.SudoSetBlock(sudoBlockNow);
			return 0;
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

char nowBoard[100][100];

void RenderBoard()
{
	system("cls");

	auto board = tetris.GetBoard();

	for (int i = 0; i < tetris.GetHeight(); i++)
	{
		for (int j = 0; j < tetris.GetWidth(); j++)
		{
			nowBoard[i][j] = board[i][j] ? 'O' : '.';
		}
	}

	auto now = tetris.GetNowBlock();
	
	for (int i = 0; i < 4; i++)
	{
		int x = TetrisSpace::NumsToBlock[now.type][now.rotate][i][0];
		int y = TetrisSpace::NumsToBlock[now.type][now.rotate][i][1];

		nowBoard[x + now.x][y + now.y] = 'O';
	}

	for (int i = 0; i < tetris.GetHeight(); i++)
	{
		for (int j = 0; j < tetris.GetWidth(); j++)
		{
			std::cout << nowBoard[i][j];
		}
		std::cout << '\n';
	}

	std::cout << "Holding : " << now.type<<'\n';
	std::cout << "Rotation : " << now.rotate << '\n';

}

void SetWindow(HINSTANCE hInst, LRESULT (CALLBACK *msgProc)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam))
{
	WNDCLASSEX winc;
	winc.cbSize = sizeof(winc);
	winc.style = CS_HREDRAW | CS_VREDRAW;
	winc.hInstance = hInst;
	winc.hCursor = LoadCursor(NULL, IDC_ARROW);
	winc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	winc.cbClsExtra = 0;
	winc.cbWndExtra = 0;
	winc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	winc.lpszMenuName = NULL;
	winc.lpszClassName = L"TetrisTest";
	winc.lpfnWndProc = msgProc;

	if (!(RegisterClassEx(&winc)))
	{
		return;
	}

	auto mhMainWnd = CreateWindow(L"TetrisTest", L"Commander", WS_OVERLAPPEDWINDOW, 0, 0, 300, 100, 0, 0, hInst, 0);
	if (!mhMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return;
	}

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);
}

int BlockRotateTest(HINSTANCE instance)
{
	SetWindow(instance, BlockRotateTestMSG);

	MSG msg{ 0, };
	sudoBlockNow = 0;

	tetris.NewGameReady();
	tetris.Start();

	clock_t startTime = clock();
	float dt;
	
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE| PM_QS_INPUT))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Sleep(100);
			dt = clock() - startTime;
			dt /= 1000;
			tetris.Update(dt);
			RenderBoard();
		}
	}
	return (int)msg.wParam;
}

void PlayTest(HINSTANCE)
{
	tetris.NewGameReady();
	tetris.Start();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int cmdShow)
{
	CCreateConsole();

	while (true)
	{
		BlockRotateTest(hInstance);
		PlayTest(hInstance);
	}

	MessageBox(NULL, L"PROGRAM ERROR END", L"Notice", MB_OK);
	Sleep(INFINITE);
}