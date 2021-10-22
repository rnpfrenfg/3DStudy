#pragma once
class BlockMaker
{
public:
	BlockMaker();
	BlockMaker(int defMakeBlocks, int);

	void Make();

	int Size();
	int GetBlock(int idx);

private:
	int blocks;

};

