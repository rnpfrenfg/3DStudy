#pragma once

#ifdef TETRISAPI

#else
#define TETRISAPI __declspec(dllimport)
#endif