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

// 使用技能
int Magic::UseMagic(const char* name, int mv_x, int mv_y)
{
	wchar_t log[128];
	if (strcmp(name, "诸神裁决") == 0) {
		int click_x = 0, click_y = 0;
	    GetMagicClickPos(name, click_x, click_y);
		m_pGame->m_pGameProc->Click(click_x, click_y);
		return -1;
	}
	
	char key = GetMagicKey(name);
	if (key) {
		if (!mv_x && !mv_y) {
			DWORD use_ms = GetTickCount();
			DbgPrint("使用技能:%s(%d)\n", name, key);
			LOGVARP2(log, "c6", L"1.使用技能:%hs(%d)", name, key);

			DWORD use_ms_2 = use_ms;
			if (m_pGame->m_pGameProc->m_nBossNum >= 0 && strcmp(name, "最终审判") == 0) {
				UseShenPan(0, 0);
				return 1;
				for (int j = 0; j < 300; j += 100) {
					m_pGame->m_pGameProc->Keyboard(key);
					Sleep(100);
				}
			}
			else {
				m_pGame->m_pGameProc->Keyboard(key);
			}

			int wait_ms = strcmp(name, "最终审判") == 0 ? 300 : 1000;
			for (int i = 1; i <= 6; i++) {
				if (i > 2) {
					if (m_pGame->m_pMove->IsOpenMap()) { // 是否已打开了地图
						m_pGame->m_pMove->CloseMap();
					}
				}

				if (strcmp(name, "最终审判") == 0) {
					for (int n = 0; n < 2800; n += 50) {
						Sleep(50);
						if (MagicIsOut(name)) {
							LOGVARP2(log, "c6", L"技能:%hs(%d)已完成释放, 用时:<b class='c3'>(%d)</b>毫秒",
								name, m_nPixelCount, int(GetTickCount()-use_ms));
							return 1;
						}
					}
				}
				else {
					DWORD ms_one = GetTickCount(), ms_two = ms_one;
					while ((ms_two - ms_one) < wait_ms) {
						if (m_pGame->m_pGameProc->m_bPause)
							break;

						Sleep(10);
						int result = MagicIsOut(name);
						if (result == -1) {
							return -1;
						}
						if (result > 0) {
							DWORD ms = GetTickCount() - use_ms;
							if ((ms & 0x0f) == 0x08) {
								m_pGame->m_pGameProc->ChNCk();
							}
							if (ms >= 300) {
								DbgPrint("技能:%s(%d)已完成释放, 用时:%d毫秒\n", name, m_nPixelCount, ms);
								LOGVARP2(log, "c6", L"技能:%hs(%d)已释放, 用时:(%d)毫秒",
									name, m_nPixelCount, ms);
								return 1;
							}
						}

						ms_two = GetTickCount();
					}
				}
				
				//GetMagicClickPos(name, click_x, click_y);
				DWORD _ms = GetTickCount();
				DbgPrint("%d.再次使用技能:%s(%d)\n", i + 1, name, key);
				LOGVARP2(log, "c6", L"%d.再次使用技能:%hs(%d) %d/%d", i + 1, name, key,
					_ms - use_ms_2, _ms - use_ms);
				
				use_ms_2 = _ms;
				if (0 && strcmp(name, "最终审判") == 0) {
					for (int j = 0; j < 300; j += 100) {
						m_pGame->m_pGameProc->Keyboard(key);
						Sleep(100);
					}
				}
				else {
					m_pGame->m_pGameProc->Keyboard(key);
				}
			}
			DbgPrint("技能:%s未放出\n", name);
			LOGVARP2(log, "red", L"技能:%hs未放出\n", name);
		}
		else {
#if 0
			DbgPrint("使用技能:%s(%d,%d) 滑动(%d,%d)\n", name, click_x, click_y, mv_x, mv_y);
			LOGVARP2(log, "red", L"使用技能:%s(%d,%d) 滑动(%d,%d)", name, click_x, click_y, mv_x, mv_y);
			m_pGame->m_pGameProc->Click(click_x, click_y, 0x01);
			m_pGame->m_pGameProc->MouseMove(click_x, click_y, mv_x, mv_y);
			m_pGame->m_pGameProc->Click(click_x + mv_x, click_y + mv_y, 0x02);
#endif

			return -1;
		}
	}

	return 0;
}

// 使用诸神裁决[mv_x=鼠标移动x, mv_y鼠标移动y]
int Magic::UseCaiJue(int mv_x, int mv_y)
{
	int x, y;
	GetMagicClickPos("诸神裁决", x, y);

	int mx = 0, my = 0;
	DWORD ms = 20000;
	m_pGame->m_pGameProc->Click_Send(x, y, 0x01);
	Sleep(500);
	for (DWORD i = 0; i < ms; i += 1) {
		if (++mx > mv_x)
			mx = mv_x;
		if (++my > mv_y)
			my = mv_y;

		m_pGame->m_pGameProc->MouseMove(x + mx, y + my);
		Sleep(1);
	}
	m_pGame->m_pGameProc->MouseMove(x, y, mv_x, mv_y);
	m_pGame->m_pGameProc->Click_Send(x + mv_x, y + mv_y, 0x02);

	return 0;
}

// 使用最终审判
int Magic::UseShenPan(int click_x, int click_y)
{
	int n = 0, flag = 0;
	DWORD start_ms = GetTickCount(), use_ms, now_ms;
	while (n++ < 10) {
		use_ms = GetTickCount();
		//m_pGame->m_pGameProc->Click_Send(click_x, click_y);
		m_pGame->m_pGameProc->Keyboard('3');
		now_ms = GetTickCount();
		if ((now_ms - use_ms) >= 100) { // 接受事件达到200毫秒
			flag = 1;
			break;
		}
		if ((now_ms - start_ms) >= 500) {
			flag = 2;
			break;
		}
		Sleep(100);
	}

	DWORD ms = GetTickCount() - start_ms;
	LOGVARN2(64, "c6", L"技能:最终审判已完成(%d), 次数:(%d), 用时:(%d)毫秒",
		flag, n, ms);
	return 0;
}

// 使用攻击符
void Magic::UseGongJiFu()
{
	LOG2(L"使用攻击符.", "green");
	m_pGame->m_pGameProc->Click(1153, 355, 1170, 372);
}

// 获取技能的按键
char Magic::GetMagicKey(const char* name)
{
	if (strcmp(name, "星陨") == 0)         // 按键1 最下面
		return '1';
	else if (strcmp(name, "影魂契约") == 0) // 按键2
		return '2';
	else if (strcmp(name, "最终审判") == 0) // 按键3
		return '3';
	else if (strcmp(name, "虚无空间") == 0) // 按键4
		return '4';

	return 0;
}

// 获取技能点击坐标
void Magic::GetMagicClickPos(const char* name, int& x, int& y)
{
	if (strcmp(name, "星陨") == 0) {         // 按键1 最下面
		x = MyRand(1006, 1050);
		y = MyRand(650, 690);
	}
	else if (strcmp(name, "影魂契约") == 0) { // 按键2
		x = MyRand(990, 1030);
		y = MyRand(540, 586);
	}
	else if (strcmp(name, "最终审判") == 0) { // 按键3
		x = MyRand(1052, 1095);
		y = MyRand(452, 503);
	}
	else if (strcmp(name, "虚无空间") == 0) { // 按键4
		x = MyRand(1153, 1196);
		y = MyRand(475, 485);
	}
	else if (strcmp(name, "诸神裁决") == 0) { // ...
		x = MyRand(1045, 1086);
		y = MyRand(360, 395);
	}
	else if (strcmp(name, "炎舞燎原") == 0) { // ...
		x = MyRand(1125, 1160);
		y = MyRand(585, 615);
	}
}

// 技能是否放出
int Magic::MagicIsOut(const char* name)
{
	DWORD color = 0xFFFFFFFF, diff = 0x003A3A3A;
	int need_count = 60;
	int max_count = 300;

	int x = 0, y = 0, x2 = 0, y2 = 0;
	if (strcmp(name, "星陨") == 0) {         // 按键1 最下面
		x = 1015, y = 666;
		x2 = 1050, y2 = 680;
	}
	else if (strcmp(name, "影魂契约") == 0) { // 按键2
		x = 1005, y = 562;
		x2 = 1035, y2 = 575;
	}
	else if (strcmp(name, "虚无空间") == 0) { // 按键4
		x = 1166, y = 453;
		x2 = 1196, y2 = 466;
	}
	else if (strcmp(name, "最终审判") == 0) { // 按键3
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
	if (0 && result == 0 && strcmp(name, "最终审判") == 0) { // 最终审判再判断有否冷却时间数字
		result = m_pGame->m_pPrintScreen->GetPixelCount(0xFFFFFFFF, 0x003A3A3A) >= need_count ? 1 : 0;
	}
	return result;
}