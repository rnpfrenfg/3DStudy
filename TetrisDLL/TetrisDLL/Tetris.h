#pragma once

#include "include.h"

#include "EventManager.h"
#include "BlockMaker.h"

namespace TetrisSpace
{
	class TETRISAPI Tetris;

	enum { boardWidth = 10 };
	enum { boardHeight = 20 };

	typedef int(*BoardPointer)[boardWidth];
	typedef bool(*newBlockTimeFunc)(Tetris* tetris, float* timeAfterLastBlockStart);

	struct TETRISAPI HoldingBlock
	{
		int x;
		int y;
		int rotate;
		int type;
	};

	//7= block, 4= rotate, 4= blocks, 2= x,y
	TETRISAPI extern const int NumsToBlock[8][4][4][2];

	class TETRISAPI Tetris
	{
	public:
		Tetris();
		~Tetris() = default;

		EventManager eventManager;

		void NewGameReady(BlockMakerImpl* maker, newBlockTimeFunc = nullptr);
		void Start();
		void Update(float dt);
		void SudoEnd();
		bool IsGaming();
		void SudoGameOn();

		void SudoSetBlock(int type);
		void HoldBlock();
		void DropBlock();
		void RotateLeft();
		void RotateRight();
		void MoveBlockLeft();
		void MoveBlockRight();
		void Down();

		float PlayTime();
		int ClearedLine();

		BoardPointer GetBoard();
		int GetWidth();
		int GetHeight();

		int GetHoldingType();
		HoldingBlock GetNowBlock();

		//return : cleared
		int LineClear();

	private:
		int IndexToInRange(int index);

		void MoveToDefaultHold();
		void GameEnd();
		void NextBlock();
		void SetBlock(int block, int rotation, int x, int y);
		bool CanSetBlock(int block, int rotation, int x, int y);
		bool IsInvalidX(int);
		bool IsInvalidY(int);

		void OnChangeBlock();

		int holding = -1;
		const int defHoldX = boardWidth / 2;
		const int defHoldY = 2;
		int blockX;
		int blockY;
		int blockType;
		int blockRotation;

		int clearedLine;
		float playTime;

		float remainTime = 0;
		bool gameEnd = true;
		int board[boardHeight][boardWidth] = { 0, };

		BlockMakerImpl* blockMaker;
		int blockMakerIndex = 0;

		newBlockTimeFunc newBlockFunc;
	};
}
