#pragma once
#include <Windows.h>

class Game;
class Magic
{
public:
	Magic(Game* p);

	// ʹ�ü���
	int UseMagic(const char* name, int mv_x=0, int mv_y=0);
	// ʹ����������
	int UseShenPan(int click_x, int click_y);
	// ��ȡ���ܵ������
	void GetMagicClickPos(const char* name, int& x, int& y);
	// �����Ƿ�ų�
	int MagicIsOut(const char* name);
public:
	// Game��
	Game* m_pGame;
	// �ų�������ɫ����
	int m_nPixelCount = 0;
};