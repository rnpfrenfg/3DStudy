#include "includeAtCPP.h"
#include "Tetris.h"

namespace TetrisSpace
{
	TETRISAPI const extern int NumsToBlock[8][4][4][2] = 
	{
		0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
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
	}

	bool DefaultNewBlockTimeFunc(Tetris* tetris, float* timeAfterLastBlockStart)
	{
		if (*timeAfterLastBlockStart > 1)
		{
			*timeAfterLastBlockStart -= 1;
			return true;
		}
		return false;
	}

	void Tetris::NewGameReady(BlockMakerImpl* maker, newBlockTimeFunc func)
	{
		if (func == nullptr)
			newBlockFunc = DefaultNewBlockTimeFunc;
		else
			newBlockFunc = func;

		for (int i = 0; i < boardHeight; i++)
		{
			for (int j = 0; j < boardWidth; j++)
			{
				board[i][j] = 0;
			}
		}
		this->blockMaker = maker;
		blockMakerIndex = 0;
		NextBlock();
		
		clearedLine = 0;
		playTime = 0;
	}

	void Tetris::SudoSetBlock(int type)
	{
		blockType = IndexToInRange(type);
	}

	void Tetris::SudoGameOn()
	{
		gameEnd = false;
	}

	BoardPointer Tetris::GetBoard()
	{
		return (BoardPointer)board;
	}

	void Tetris::Start()
	{
		MoveToDefaultHold();
		gameEnd = false;
		remainTime = 0;
	}
	void Tetris::MoveToDefaultHold()
	{
		blockX = defHoldX;
		blockY = defHoldY;
	}
	bool Tetris::IsGaming()
	{
		return !gameEnd;
	}
	void Tetris::Update(float dt)
	{
		if (gameEnd)
			return;

		remainTime += dt;
		playTime += dt;

		while (newBlockFunc(this, &remainTime))
		{
			Down();
		}
	}

	float Tetris::PlayTime()
	{
		return playTime;
	}

	int Tetris::ClearedLine()
	{
		return clearedLine;
	}

	void Tetris::NextBlock()
	{
		blockType = blockMaker->GetBlock(blockMakerIndex);
		blockMakerIndex++;

		OnChangeBlock();
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
		block.rotate = blockRotation;
		block.x = blockX;
		block.y = blockY;
		block.type = blockType;
		return block;
	}

	void Tetris::HoldBlock()
	{
		if (gameEnd)
			return;

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
		
		TetrisEventData::GameEnd data;
		data.tetris = this;
  		eventManager.PushEvent(EventType::GAME_END, &data);
	}

	void Tetris::DropBlock()
	{
		if (gameEnd)
			return;

		while (true)
		{
			if (!(CanSetBlock(blockType, blockRotation, blockX, blockY + 1)))
				break;
			blockY++;
		}

		TetrisEventData::BlockDrop data;
		data.tetris = this;
		data.rotate = blockRotation;
		data.blockType = blockType;
		data.resultX = blockX;
		data.resultY = blockY;
		eventManager.PushEvent(EventType::BLOCK_DOWN, &data);

		SetBlock(blockType, blockRotation, blockX, blockY);
		NextBlock();
		return;
	}

	void Tetris::OnChangeBlock()
	{
		MoveToDefaultHold();
		remainTime = 0;

		TetrisEventData::NewBlockStart data;
		data.tetris = this;
		data.blockType = blockType;
		eventManager.PushEvent(EventType::NEW_BLOCK_START, &data);

		if (!(CanSetBlock(blockType, blockRotation, blockX, blockY)))
			GameEnd();
	}

	bool Tetris::IsInvalidX(int x)
	{
		return x < 0 || x >= boardWidth;
	}
	bool Tetris::IsInvalidY(int y)
	{
		return y < 0 || y >= boardHeight;
	}

	int Tetris::LineClear()
	{
		TetrisEventData::LineCleared data;
		data.tetris = this;

		int cleared = 0;
		for (int y = boardHeight - 1; y >= 0; y--)
		{
			for (int x = 0; x < boardWidth; x++)
			{
				if (board[y][x] == 0)
					goto NEXTLINE;
			}
			data.lineIndex = y - cleared;
			eventManager.PushEvent(EventType::LINE_CLEARED, &data);

			cleared++;

			for (int i = y; i > 0; i--)
			{
				for (int j = 0; j < boardWidth; j++)
				{
					board[i][j] = board[i - 1][j];
				}
			}
			for (int j = 0; j < boardWidth; j++)
				board[0][j] = 0;

			NEXTLINE:
			continue;
		}

		clearedLine += cleared;

		return cleared;
	}

	void Tetris::MoveBlockLeft()
	{
		if (gameEnd)
			return;

		if (CanSetBlock(blockType, blockRotation, blockX - 1, blockY))
		{
			blockX--;

			TetrisEventData::BlockMove data;
			data.tetris = this;
			data.beforeX = blockX + 1;
			data.beforeY = blockY;
			data.resultX = blockX;
			data.resultY = blockY;
			eventManager.PushEvent(EventType::BLOCK_MOVE, &data);
		}
	}

	void SudoEnd()
	{

	}

	void Tetris::MoveBlockRight()
	{
		if (gameEnd)
			return;

		if (CanSetBlock(blockType, blockRotation, blockX + 1, blockY))
		{
			blockX++;

			TetrisEventData::BlockMove data;
			data.tetris = this;
			data.beforeX = blockX + 1;
			data.beforeY = blockY;
			data.resultX = blockX;
			data.resultY = blockY;
			eventManager.PushEvent(EventType::BLOCK_MOVE, &data);
		}
	}

	void Tetris::Down()
	{
		if (gameEnd)
			return;

		if (CanSetBlock(blockType, blockRotation, blockX, blockY + 1))
		{
			blockY++;

			TetrisEventData::BlockDown data;
			data.tetris = this;
			data.resultX = blockX;
			data.resultY = blockY;
			eventManager.PushEvent(EventType::BLOCK_DOWN, &data);
		}
		else
		{
			SetBlock(blockType, blockRotation, blockX, blockY);
			NextBlock();
		}
	}

	void Tetris::RotateRight()
	{
		if (gameEnd)
			return;

		int temp = blockRotation + 1;
		if (temp > 3)
			temp = 0;
		temp &= 0b11;
		if (!(CanSetBlock(blockType, temp, blockX, blockY)))
			return;
		blockRotation = temp;

		TetrisEventData::BlockRotateLeft data;
		data.tetris = this;
		data.resultRotate = blockRotation;
		eventManager.PushEvent(EventType::BLOCK_ROTATE_RIGHT, &data);
	}

	void Tetris::RotateLeft()
	{
		if (gameEnd)
			return;

		int temp = blockRotation - 1;
		if (temp < 0)
			temp = 3;
		temp = IndexToInRange(temp);
		temp &= 0b11;
		if (!(CanSetBlock(blockType, temp, blockX, blockY)))
			return;
		blockRotation = temp;

		TetrisEventData::BlockRotateLeft data;
		data.tetris = this;
		data.resultRotate = blockRotation;
		eventManager.PushEvent(EventType::BLOCK_ROTATE_LEFT, &data);
	}

	void Tetris::SudoEnd()
	{
		GameEnd();
	}

	void Tetris::SetBlock(int block, int blockRotation, int x, int y)
	{
		int i, toX, toY;
		for (i = 0; i < 4; i++)
		{
			toX = x + NumsToBlock[block][blockRotation][i][0];
			toY = y + NumsToBlock[block][blockRotation][i][1];
			board[toY][toX] = block;
		}

		TetrisSpace::TetrisEventData::SetBlock data;
		data.tetris = this;
		data.blockType = block;
		data.rotate = blockRotation;
		data.resultX = x;
		data.resultY = y;
		eventManager.PushEvent(EventType::SET_BLOCK, &data);

		LineClear();
	}

	bool Tetris::CanSetBlock(int block, int blockRotation, int x, int y)
	{
		int i, toX, toY;
		for (i = 0; i < 4; i++)
		{
			toX = x + NumsToBlock[block][blockRotation][i][0];
			toY = y + NumsToBlock[block][blockRotation][i][1];
			if (IsInvalidX(toX) || IsInvalidY(toY))
				return false;
			if (board[toY][toX] != 0)
				return false;
		}
		return true;
	}
}
