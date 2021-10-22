#pragma once

#ifdef TETRISAPI

#else
#define TETRISAPI extern "C" __declspec(dllimport)
#endif