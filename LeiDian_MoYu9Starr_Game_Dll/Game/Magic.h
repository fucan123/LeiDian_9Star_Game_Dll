#pragma once
#include <Windows.h>

class Game;
class Magic
{
public:
	Magic(Game* p);

	// ʹ�ü���
	int  UseMagic(const char* name, int mv_x=0, int mv_y=0);
	// ʹ������þ�[mv_x=����ƶ�x, mv_y����ƶ�y]
	int  UseCaiJue(int mv_x, int mv_y);
	// ʹ����������
	int  UseShenPan(int click_x, int click_y);
	// ʹ�ù�����
	void UseGongJiFu();
	// ��ȡ���ܵİ���
	char GetMagicKey(const char* name);
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