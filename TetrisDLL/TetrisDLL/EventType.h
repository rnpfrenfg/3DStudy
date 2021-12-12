#pragma once

#include "include.h"

namespace TetrisSpace
{
	class TETRISAPI Tetris;

	enum class EventType
	{
		NEW_BLOCK_START,
		BLOCK_ROTATE_LEFT,
		BLOCK_ROTATE_RIGHT,
		BLOCK_MOVE,
		BLOCK_DOWN,
		BLOCK_DROP,
		SET_BLOCK,
		LINE_CLEARED,
		GAME_END,
		NUM_EVENTS
	};

	enum { CNUM_EVENTS = static_cast<int>(EventType::NUM_EVENTS) };

	namespace TetrisEventData
	{
#define struct struct TETRISAPI

		struct NewBlockStart
		{
			Tetris* tetris;
			int blockType;
		};

		struct BlockRotateLeft
		{
			Tetris* tetris;
			int resultRotate;
		};
		struct BlockRotateRight
		{
			Tetris* tetris;
			int resultRotate;
		};

		struct BlockMove
		{
			Tetris* tetris;
			int beforeX;
			int beforeY;
			int resultX;
			int resultY;
		};

		struct BlockDown
		{
			Tetris* tetris;
			int resultX;
			int resultY;
		};

		struct BlockDrop
		{
			Tetris* tetris;
			int resultX;
			int resultY;
			int rotate;
			int blockType;
		};

		struct SetBlock
		{
			Tetris* tetris;
			int resultX;
			int resultY;
			int rotate;
			int blockType;
		};

		struct LineCleared
		{
			Tetris* tetris;
			int lineIndex;
		};

		struct GameEnd
		{
			Tetris* tetris;
		};
#undef struct
	}
}