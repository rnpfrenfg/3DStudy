#include "includeAtCPP.h"
#include "BlockMaker.h"

#define NEWMEMORYBLOCK 3

namespace TetrisSpace
{
	BlockMakerImpl::BlockMakerImpl(int newBlockSize)
	{
		gen = std::mt19937(rd());
		distribution = std::uniform_int_distribution<int>(1, 7);

		newSize = newBlockSize;

		blockPSize = NEWMEMORYBLOCK;
		nowBlockPSize = 1;

		blocks = new int* [blockPSize];
		blocks[0] = new int[newSize];
		InitBlock(blocks[0], newSize);
		return;
	}

	BlockMakerImpl::~BlockMakerImpl()
	{
		for (int i = 0; i < nowBlockPSize; i++)
		{
			delete[] blocks[i];
		}

		delete[] blocks;
	}

	int BlockMakerImpl::GetBlock(int idx)
	{
		if (idx >= nowBlockPSize * newSize)
		{
			while (idx >= nowBlockPSize * newSize)
			{
				if (blockPSize == nowBlockPSize)
				{
					blockPSize += NEWMEMORYBLOCK;
					int** tempP = new int* [blockPSize];
					for (int i = 0; i < nowBlockPSize; i++)
					{
						tempP[i] = blocks[i];
					}
					delete blocks;
					blocks = tempP;
				}

				blocks[nowBlockPSize] = new int[newSize];
				InitBlock(blocks[nowBlockPSize], newSize);
				nowBlockPSize++;
			}
		}
		return blocks[idx / newSize][idx % newSize];
	}

	void BlockMakerImpl::InitBlock(int* arr, int size)
	{
		for (int i = 0; i < size; i++)
			arr[i] = distribution(gen);
	}
}