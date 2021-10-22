#include "includeAtCPP.h"
#include "Tetris.h"

namespace TetrisSpace
{
	int NumsToBlock[7][4][4][2] =
	{
		//square
		0, 0, 1, 0, 0, 1, 1, 1,
			0, 0, 1, 0, 0, 1, 1, 1,
			0, 0, 1, 0, 0, 1, 1, 1,
			0, 0, 1, 0, 0, 1, 1, 1,
			//I
			0, -1, 0, 0, 0, 1, 0, 2,
			-1, 0, 0, 0, 1, 0, 2, 0,
			0, -1, 0, 0, 0, 1, 0, 2,
			-1, 0, 0, 0, 1, 0, 2, 0,
			//L
			0, -1, 0, 0, 0, 1, 1, 1,
			-1, 0, 0, 0, 1, 0, 1, -1,
			-1, -1, 0, -1, 0, 0, 0, 1,
			-1, 1, -1, 0, 0, 0, 1, 0,
			//J
			-1, 1, 0, 1, 0, 0, 0, -1,
			-1, 0, 0, 0, 1, 0, 1, 1,
			0, -1, 1, -1, 0, 0, 0, 1,
			-1, -1, -1, 0, 0, 0, 1, 0,
			//T
			0, 0, 1, 0, -1, 0, 0, -1,
			0, 0, -1, 0, 0, 1, 0, -1,
			0, 0, -1, 0, 0, 1, 1, 0,
			0, 0, 1, 0, 0, 1, 0, -1,
			//S
			0, 0, 1, 0, 0, 1, -1, 1,
			0, 0, 0, -1, 1, 0, 1, 1,
			0, 0, 1, 0, 0, 1, -1, 1,
			0, 0, 0, -1, 1, 0, 1, 1,
			//Z
			0, 0, -1, 0, 0, 1, 1, 1,
			0, 0, 0, -1, -1, 0, -1, 1,
			0, 0, -1, 0, 0, 1, 1, 1,
			0, 0, 0, -1, -1, 0, -1, 1,
	};

	Tetris::Tetris()
	{
		holding = -1;


		gen = std::mt19937(rd());
		distribution = std::uniform_int_distribution<int>(0, 6);
	}

	void Tetris::NewGameReady()
	{
		//TODO
	}

	int Tetris::GetRandomBlock()
	{
		return distribution(gen);
	}

	void Tetris::SudoSetBlock(int type)
	{
		blockType = IndexToInRange(type);
	}

	int(*Tetris::GetBoard())[boardWidth]
	{
		return (int(*)[boardWidth])board;
	}

	void Tetris::Start()
	{
		MoveToDefaultHold();
		gameEnd = false;
		remainTime = 0;
	}
	void Tetris::MoveToDefaultHold()
	{
		holdX = defHoldX;
		holdY = defHoldY;
	}
	bool Tetris::IsGaming()
	{
		return !gameEnd;
	}
	void Tetris::Update(float dt)
	{
		while (dt - remainTime > 20)
		{
			remainTime += 20;
			Down();
		}
	}

	void Tetris::NextBlock()
	{
		if (lastBlock == nextBlockSize)
		{
			//TODO
			return;
		}
		blockType = nextBlocks[lastBlock];
		if (blockType < 0 || blockType > 6)
			return;//TODO
		lastBlock++;
		MoveToDefaultHold();
		remainTime = 0;
		if (!(CanSetBlock(blockType, rotation, holdX, holdY)))
			GameEnd();
	}

	int Tetris::GetWidth()
	{
		return boardWidth;
	}

	int Tetris::GetHeight()
	{
		return boardHeight;
	}

	int Tetris::GetHoldingType()
	{
		return holding;
	}

	HoldingBlock Tetris::GetNowBlock()
	{
		HoldingBlock block;
		block.rotate = rotation;
		block.x = holdX;
		block.y = holdY;
		block.type = blockType;
		return block;
	}

	void Tetris::OnNewBlockStart()
	{
		//TODO
	}

	void Tetris::HoldBlock()
	{
		if (holding == -1)
		{
			holding = blockType;
			NextBlock();
		}
		else
		{
			auto temp = holding;
			holding = blockType;
			blockType = temp;
		}
	}

	void Tetris::SetRandomNextBlocks()
	{
		nextBlockSize = 700;
		int i;
		for (i = 0; i < nextBlockSize; i++)
		{
			nextBlocks[i] = GetRandomBlock();
		}
	}

	int Tetris::IndexToInRange(int i)
	{
		if (i < 0)
		{
			return i + 7 * ((-i - 1) / 7 + 1);
		}
		return i % 7;
	}

	void Tetris::GameEnd()
	{
		gameEnd = true;
	}

	void Tetris::SetNextBlocks(int* next, int size)
	{
		if (size < 1)
			return;
		int lastBlock = 0;
		int nextBlockSize = size;
		memcpy(nextBlocks, next, size * sizeof(int));
	}

	void Tetris::DropBlock()
	{
		while (true)
		{
			if (!(CanSetBlock(blockType, rotation, holdX + 1, holdY)))
				break;
			holdX++;
		}
		SetBlock(blockType, rotation, holdX, holdY);
		NextBlock();
		return;
	}

	void OnChangeBlock()
	{

	}

	bool Tetris::IsInvalidX(int x)
	{
		return x < 0 || x > boardWidth;
	}
	bool Tetris::IsInvalidY(int y)
	{
		return y < 0 || y > boardHeight;
	}

	void Tetris::MoveBlockLeft()
	{
		if (CanSetBlock(blockType, rotation, holdX, holdY - 1))
		{
			holdY--;
		}
	}

	void Tetris::MoveBlockRight()
	{
		if (CanSetBlock(blockType, rotation, holdX, holdY + 1))
		{
			holdY++;
		}
	}

	void Tetris::Down()
	{
		if (CanSetBlock(blockType, rotation, holdX + 1, holdY))
		{
			holdX++;
		}
		else
		{
			SetBlock(blockType, rotation, holdX, holdY);
		}
	}

	void Tetris::RotateRight()
	{
		int temp = rotation + 1;
		if (temp > 3)
			temp = 0;
		temp &= 0b11;
		if (!(CanSetBlock(blockType, temp, holdX, holdY)))
			return;
		rotation = temp;
	}

	void Tetris::RotateLeft()
	{
		int temp = rotation - 1;
		if (temp < 0)
			temp = 3;
		temp = IndexToInRange(temp);
		temp &= 0b11;
		if (!(CanSetBlock(blockType, temp, holdX, holdY)))
			return;
		rotation = temp;
	}

	void Tetris::SetBlock(int block, int rotation, int x, int y)
	{
		int i, toX, toY;
		for (i = 0; i < 4; i++)
		{
			toX = x + NumsToBlock[block][rotation][i][0];
			toY = y + NumsToBlock[block][rotation][i][1];
			board[toX][toY] = true;
		}
	}

	bool Tetris::CanSetBlock(int block, int rotation, int x, int y)
	{
		int i, toX, toY;
		for (i = 0; i < 4; i++)
		{
			toX = x + NumsToBlock[block][rotation][i][0];
			toY = y + NumsToBlock[block][rotation][i][1];
			if (IsInvalidX(toX) || IsInvalidY(toY))
				return false;
		}
		return true;
	}
}
