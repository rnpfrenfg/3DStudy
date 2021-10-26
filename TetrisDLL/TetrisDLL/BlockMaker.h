#pragma once

#include "include.h"

#include <random>

namespace TetrisSpace
{
	class TETRISAPI BlockMaker
	{
		virtual int GetBlock(int idx) = 0;
	};

	class TETRISAPI BlockMakerImpl : BlockMaker
	{
	public:
		BlockMakerImpl(int newBlockSize = 700);
		~BlockMakerImpl();

		int GetBlock(int idx) override;

	private:
		void InitBlock(int* arr, int size);

		int newSize;
		int** blocks = nullptr;

		int blockPSize = 0;
		int nowBlockPSize = 0;

		std::random_device rd;
		std::mt19937 gen;
		std::uniform_int_distribution<int> distribution;
	};
}