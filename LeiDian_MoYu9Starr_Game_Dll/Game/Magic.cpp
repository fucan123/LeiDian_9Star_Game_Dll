#include "Game.h"
#include "Magic.h"
#include "Move.h"
#include "GameProc.h"
#include "PrintScreen.h"
#include <My/Common/C.h>

Magic::Magic(Game* p)
{
	m_pGame = p;
}

// ʹ�ü���
int Magic::UseMagic(const char* name, int mv_x, int mv_y)
{
	wchar_t log[128];
	int click_x = 0, click_y = 0;
	GetMagicClickPos(name, click_x, click_y);
	if (click_x && click_y) {
		if (!mv_x && !mv_y) {
			DWORD use_ms = GetTickCount();
			DbgPrint("ʹ�ü���:%s(%d,%d)\n", name, click_x, click_y);
			LOGVARP2(log, "c6", L"ʹ�ü���:%hs(%d,%d)", name, click_x, click_y);
			m_pGame->m_pGameProc->Click(click_x, click_y);

			int wait_ms = strcmp(name, "��������") == 0 ? 2500 : 1000;
			for (int i = 1; i <= 10; i++) {
				if (i > 2) {
					if (m_pGame->m_pMove->IsOpenMap()) { // �Ƿ��Ѵ��˵�ͼ
						m_pGame->m_pMove->CloseMap();
					}
				}

				Sleep(100);
				for (int j = 0; j < wait_ms; j += 100) {
					if (m_pGame->m_pGameProc->m_bPause)
						break;

					int result = MagicIsOut(name);
					if (result == -1) {
						return -1;
					}
					if (result > 0) {
						DWORD ms = GetTickCount() - use_ms;
						if (ms >= 300) {
							DbgPrint("����:%s(%d)������ͷ�, ��ʱ:%d����\n", name, m_nPixelCount, ms);
							LOGVARP2(log, "c6", L"����:%hs<b class='c3'>(%d)</b>���ͷ�, ��ʱ:<b class='c3'>(%d)</b>����",
								name, m_nPixelCount, ms);
							return 1;
						}
					}

					Sleep(100);
				}
				DbgPrint("%d.�ٴ�ʹ�ü���:%s(%d,%d)\n", i + 1, name, click_x, click_y);
				LOGVARP2(log, "c6", L"%d.�ٴ�ʹ�ü���:%hs(%d,%d)", i + 1, name, click_x, click_y);

				m_pGame->m_pGameProc->Click(click_x, click_y);
			}
			DbgPrint("����:%sδ�ų�\n", name);
			LOGVARP2(log, "red", L"����:%hsδ�ų�\n", name);
		}
		else {
			DbgPrint("ʹ�ü���:%s(%d,%d) ����(%d,%d)\n", name, click_x, click_y, mv_x, mv_y);
			LOGVARP2(log, "red", L"ʹ�ü���:%s(%d,%d) ����(%d,%d)", name, click_x, click_y, mv_x, mv_y);
			m_pGame->m_pGameProc->Click(click_x, click_y, 0x01);
			m_pGame->m_pGameProc->MouseMove(click_x, click_y, mv_x, mv_y);
			m_pGame->m_pGameProc->Click(click_x + mv_x, click_y + mv_y, 0x02);

			return -1;
		}
	}

	return 0;
}

// ��ȡ���ܵ������
void Magic::GetMagicClickPos(const char* name, int& x, int& y)
{
	if (strcmp(name, "����") == 0) {         // ����1 ������
		x = MyRand(1006, 1050);
		y = MyRand(650, 690);
	}
	else if (strcmp(name, "Ӱ����Լ") == 0) { // ����2
		x = MyRand(990, 1030);
		y = MyRand(540, 586);
	}
	else if (strcmp(name, "��������") == 0) { // ����3
		x = MyRand(1052, 1095);
		y = MyRand(452, 503);
	}
	else if (strcmp(name, "���޿ռ�") == 0) { // ����4
		x = MyRand(1153, 1196);
		y = MyRand(440, 482);
	}
	else if (strcmp(name, "����þ�") == 0) { // ...
		x = MyRand(1045, 1086);
		y = MyRand(360, 395);
	}
	else if (strcmp(name, "������ԭ") == 0) { // ...
		x = MyRand(1125, 1160);
		y = MyRand(585, 615);
	}
}

// �����Ƿ�ų�
int Magic::MagicIsOut(const char* name)
{
	DWORD color = 0xFFFFFFFF, diff = 0x003A3A3A;
	int need_count = 20;
	int max_count = 300;

	int x = 0, y = 0, x2 = 0, y2 = 0;
	if (strcmp(name, "����") == 0) {         // ����1 ������
		x = 1015, y = 666;
		x2 = 1050, y2 = 680;
	}
	else if (strcmp(name, "Ӱ����Լ") == 0) { // ����2
		x = 1005, y = 562;
		x2 = 1035, y2 = 575;
	}
	else if (strcmp(name, "���޿ռ�") == 0) { // ����4
		x = 1166, y = 453;
		x2 = 1196, y2 = 466;
	}
	else if (strcmp(name, "��������") == 0) { // ����3
		x = 1060, y = 460;
		x2 = 1090, y2 = 472;

		color = 0xFFFF0000;
		diff = 0x00205050;
	}
	else {
		return -1;
	}

	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, x, y, x2, y2, 0, true);
	m_nPixelCount = m_pGame->m_pPrintScreen->GetPixelCount(color, diff);
	int result = m_nPixelCount >= need_count && m_nPixelCount <= max_count ? 1 : 0;
	if (0 && result == 0 && strcmp(name, "��������") == 0) { // �����������ж��з���ȴʱ������
		result = m_pGame->m_pPrintScreen->GetPixelCount(0xFFFFFFFF, 0x003A3A3A) >= need_count ? 1 : 0;
	}
	return result;
}