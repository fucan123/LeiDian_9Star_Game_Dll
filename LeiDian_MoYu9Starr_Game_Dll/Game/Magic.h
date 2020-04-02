#pragma once
#include <Windows.h>

class Game;
class Magic
{
public:
	Magic(Game* p);

	// 使用技能
	int UseMagic(const char* name, int mv_x=0, int mv_y=0);
	// 使用最终审判
	int UseShenPan(int click_x, int click_y);
	// 获取技能点击坐标
	void GetMagicClickPos(const char* name, int& x, int& y);
	// 技能是否放出
	int MagicIsOut(const char* name);
public:
	// Game类
	Game* m_pGame;
	// 放出技能颜色数量
	int m_nPixelCount = 0;
};