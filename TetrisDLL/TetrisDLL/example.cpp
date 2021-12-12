#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <time.h>

#include <iostream>
#include <thread>

#include "EventListener.h"
#include "EventManager.h"
#include "Tetris.h"


/*
How to run at visual studio:
	solution explorer
	-> Right Click : TetrisDLL
	-> properties
	-> Configuration Properties -> General -> Configuration type = .exe;
*/

#define CLASSNAME L"TetrisTest"

using namespace TetrisSpace;

Tetris tetris;

void CCreateConsole()
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
}

int sudoBlockNow;
BlockMakerImpl maker;

LRESULT CALLBACK BlockRotateTestMSG(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		return 0;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
	}
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
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK PlayTestMSG(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		return 0;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
	}
	return 0;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_LEFT:
			tetris.MoveBlockLeft();
			return 0;
		case VK_RIGHT:
			tetris.MoveBlockRight();
			return 0;
		case VK_DOWN:
			tetris.Down();
			return 0;
		case VK_UP:
			tetris.RotateRight();
			return 0;
		case VK_SPACE:
			tetris.DropBlock();
			return 0;
		case VK_SHIFT:
			tetris.HoldBlock();
			return 0;
		}
		return 0;
	case WM_DESTROY:
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

		nowBoard[y + now.y][x + now.x] = 'O';
	}

	for (int i = 0; i < tetris.GetHeight(); i++)
	{
		for (int j = 0; j < tetris.GetWidth(); j++)
		{
			std::cout << nowBoard[i][j];
		}
		std::cout << '\n';
	}

	std::cout << "block : " << now.type << '\n';
	std::cout << "Rotation : " << now.rotate << '\n';
	std::cout << "Hold : " << tetris.GetHoldingType() << '\n';
	std::cout << "Gameing : " << tetris.IsGaming() << '\n';
}

void SetWindow(HINSTANCE hInst, LRESULT(CALLBACK* msgProc)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam))
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
	winc.lpszClassName = CLASSNAME;
	winc.lpfnWndProc = msgProc;

	if (!(RegisterClassEx(&winc)))
	{
		return;
	}

	auto mhMainWnd = CreateWindow(CLASSNAME, L"Commander", WS_OVERLAPPEDWINDOW, 0, 0, 300, 100, 0, 0, hInst, 0);
	if (!mhMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return;
	}

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);
	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);
}

void BlockRotateTest(HINSTANCE instance)
{
	SetWindow(instance, BlockRotateTestMSG);

	MSG msg{ 0, };
	sudoBlockNow = 0;

	tetris.NewGameReady(&maker);
	tetris.Start();

	clock_t startTime = clock();
	float dt;

	while (msg.message != WM_DESTROY)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
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

	UnregisterClass(CLASSNAME, instance);
}


class EventListenerImpl : public EventListener
{
public:
	EventListenerImpl() = default ;
	~EventListenerImpl() override = default;

	void HandleEvent(EventType type, void* pData) override
	{
		switch (type)
		{
		case EventType::GAME_END:
		{
			TetrisEventData::GameEnd* data = (TetrisEventData::GameEnd*) pData;
			wchar_t result[100];
			wchar_t msg[] = L"Your score : %d";
			wsprintf(result, msg, data->tetris->ClearedLine());
			MessageBox(NULL, result, L"Game Over!!", MB_OK);
			return;
		}
		case EventType::LINE_CLEARED:
		{
			std::cout << '\a';//beep
		}

		}
	}
};

void PlayTest(HINSTANCE instance)
{
	EventListenerImpl* eventListener = new EventListenerImpl;

	tetris.eventManager.AddListener(eventListener, EventType::LINE_CLEARED);
	tetris.eventManager.AddListener(eventListener, EventType::GAME_END);

	SetWindow(instance, PlayTestMSG);

	MSG msg{ 0, };
	sudoBlockNow = 0;

	tetris.NewGameReady(&maker);
	tetris.Start();

	clock_t lastTime = clock();
	float dt;

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Sleep(100);
			auto t = clock();
			dt = t - lastTime;
			dt /= 1000;
			lastTime = t;
			tetris.Update(dt);
			RenderBoard();
		}
	}

	UnregisterClass(CLASSNAME, instance);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int cmdShow)
{
	CCreateConsole();

	PlayTest(hInstance);
	//BlockRotateTest(hInstance);

	MessageBox(NULL, L"PROGRAM ERROR END", L"Notice", MB_OK);
	Sleep(INFINITE);
}