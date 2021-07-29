#pragma once

#include <functional>

using KeyCallback = std::function<void()>;

class KeyCommand
{
public:
	KeyCommand(KeyCallback onKeyDown, KeyCallback onKeyUp) :mOnKeyDown(onKeyDown), mOnKeyUp(onKeyUp) {}

	void Down() { mOnKeyDown(); }
	void Up() { mOnKeyUp(); }

private:
	KeyCallback mOnKeyDown;
	KeyCallback mOnKeyUp;
};

class KeyboardSetting
{
public://TODO
	KeyCommand VKKeySetting[0xFE + 50];

	static void DefaultKeyboardSetting()
	{

	}
};

