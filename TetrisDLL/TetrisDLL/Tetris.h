#pragma once

#include "include.h"

#include <random>

namespace TetrisSpace
{
	struct HoldingBlock
	{
		int x;
		int y;
		int rotate;
		int type;
	};

	//7= block, 4= rotate, 4= blocks, 2= x,y
	TETRISAPI int NumsToBlock[7][4][4][2];

	TETRISAPI class Tetris
	{
		enum { boardWidth = 10 };
		enum { boardHeight = 20 };
	public:
		Tetris();
		~Tetris() = default;

		int GetRandomBlock();

		void NewGameReady();
		void Start();
		void Update(float dt);
		void SudoEnd();
		bool IsGameing();

		void SudoSetBlock(int type);
		void HoldBlock();
		void DropBlock();
		void RotateLeft();
		void RotateRight();
		void MoveBlockLeft();
		void MoveBlockRight();
		void Down();

		int (*GetBoard())[boardWidth];
		int GetWidth();
		int GetHeight();

		int GetHoldingType();
		HoldingBlock GetNowBlock();

		bool IsGaming();

		//to use on nextwork
		void SetNextBlocks(int* next, int size);
		void SetRandomNextBlocks();

	private:
		int IndexToInRange(int index);

		void OnNewBlockStart();
		void MoveToDefaultHold();
		void GameEnd();
		void NextBlock();
		void SetBlock(int block, int rotation, int x, int y);
		bool CanSetBlock(int block, int rotation, int x, int y);
		bool IsInvalidX(int);
		bool IsInvalidY(int);

		void OnChangeBlock();

		std::random_device rd;
		std::mt19937 gen;
		std::uniform_int_distribution<int> distribution;

		int holding = -1;
		const int defHoldX = boardWidth/2;
		const int defHoldY = 5;
		int holdX;
		int holdY;
		int blockType;
		int rotation;

		long remainTime = 0;
		bool gameEnd = true;
		bool board[boardHeight][boardWidth] = { 0, };
		//to use on nextBlocks
		int lastBlock = 0;
		int nextBlocks[1000] = { 0, };
		int nextBlockSize = 0;
	};
}
