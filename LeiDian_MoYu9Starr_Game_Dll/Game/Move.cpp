#include "Game.h"
#include "GameData.h"
#include "Move.h"
#include "PrintScreen.h"
#include "GameProc.h"

#include <time.h>
#include <My/Common/C.h>

Move::Move(Game* p)
{
	m_pGame = p;
	InitData();
}

// 初始化数据
void Move::InitData()
{
	ClearMove();
}

// 移动到终点[达到终点返回]
bool Move::RunEnd(DWORD x, DWORD y, _account_* account, bool click, DWORD ms)
{
	if (Run(x, y, account, MyRand(300, 500), MyRand(200, 350), click) == -1)
		return true;

	for (DWORD i = 0; true; i += 800) {
		while (m_pGame->m_pGameProc->m_bPause) Sleep(500);

		if (IsMoveEnd(account))
			break;
		if (!IsMove(account)) {
			if (Run(x, y, account, MyRand(250, 500), MyRand(150, 300), click) == -1)
				return true;
		}

		if (ms && i > ms)
			return false;

		Sleep(800);
	}

	DbgPrint("已抵达目的地:%d,%d\n", x, y);
	LOGVARN2(64, "green", L"已抵达目的地:%d,%d", x, y);
	return true;
}

// 移动
int Move::Run(DWORD x, DWORD y, _account_* account, DWORD click_x, DWORD click_y, bool is_click, bool rand_click)
{
	if (x == m_dwMvX && y == m_dwMvY) {
		if ((GetTickCount() - m_dwMvTime) < 1000)
			return 0;
	}
	if (m_pGame->m_pGameData->IsInArea(x, y, 0, account)) {
		account->MvX = x;
		account->MvY = y;
		return -1;
	}
		
	if (!IsOpenMap()) {
		for (int i = 0; i < 2; i++) {
			if (OpenMap(account)) {
				break;
			}
		}
		Sleep(60);
	}

	if (is_click) {
		m_pGame->m_pGameProc->Click(click_x, click_y);
		Sleep(200);
	}
	if (0 && rand_click) {
		int count = MyRand(0, 1, x + y);
		for (int i = 0; i < count; i++) {
			m_pGame->m_pGameProc->Click(MyRand(162, 900, i), MyRand(162, 500, i));
			Sleep(MyRand(100, 150, i));
		}
	}

	DbgPrint("设置目的地(%d,%d)\n", x, y);
	LOGVARN2(64, "c6", L"设置目的地(%d,%d)", x, y);
	if (x == 0 || y == 0) {
		LOGVARN2(64, "red", L"错误目的地为(%d,%d) %hs", x, y, m_pGame->m_pGameProc->m_pStep->Cmd);
	}
	m_pGame->m_pGameData->WriteMoveCoor(x, y, account);

	GoWay();
	//DbgPrint("Move::Run:%d,%d 自己位置：%d,%d\n", x, y, m_dwX, m_dwY);
	SetMove(x, y, account);

	return 1;
}

// 打开游戏地图
bool Move::OpenMap(_account_* account)
{
	// 1120,66-1220-125
	DbgPrint("打开地图\n");
	LOG2(L"打开地图", "c0");
	//m_pGame->m_pEmulator->Tap(MyRand(1108, 1168), MyRand(55, 135));
	m_pGame->m_pGameProc->Click(1108, 55, 1168, 135, 0xff, account->Mnq->Wnd);
	Sleep(100);
	return WaitMapOpen();
}

// 关闭游戏地图
void Move::CloseMap(HWND hwnd)
{
	DbgPrint("关闭地图\n");
	LOG2(L"关闭地图", "c0");
	m_pGame->m_pGameProc->Click(1150, 50, 1175, 68, 0xff, hwnd);
}

// 地图是否打开
bool Move::IsOpenMap()
{
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 715, 620, 725, 630, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("地图已打开", nullptr, 1) > 0;
}

// 等待地图打开
bool Move::WaitMapOpen(DWORD ms)
{
	DWORD _tm = GetTickCount();
	// 截取地图自动寻路确定按钮图片
	
	while (!IsOpenMap()) {
		if ((GetTickCount() - _tm) >= ms)
			return false;

		Sleep(10);
	}

	return true;
}

// 点击自动寻路
void Move::GoWay()
{
	// 725,620-790,630
	DbgPrint("自动寻路\n");
	LOGVARN2(32, "c0", L"自动寻路(%d)", m_pGame->m_pGameProc->m_nReMoveCount + 1);
	m_pGame->m_pGameProc->Click(775, 620, 825, 635);
}

// 设置移动位置
void Move::SetMove(DWORD x, DWORD y, _account_* account)
{
	m_pGame->m_pGameData->ReadCoor(&account->LastX, &account->LastY, account); // 读取当前坐标

	account->MvX = x;
	account->MvY = y;
	account->MvTime = GetTickCount();

	int now_time = time(nullptr);
	if ((now_time & 0x2f) == 0x02) {
		m_pGame->ChCRC();
	}
	if ((now_time - m_iGetPosTime) < 2) // 2秒内不取变化
		return;

	if (0 && m_dwCmpX && m_dwCmpY) {
		if (m_dwCmpX == m_dwLastX && m_dwCmpY == m_dwLastY) { // 坐标与上次一样
			if ((now_time - m_iGetPosTime) < 5) { // 获取坐标时间在最近
				if ((now_time - m_iFlagPosTime) > 30) { // 30秒坐标还一样当作掉线
					DbgPrint("可能卡住或掉线了\n");
					LOG2(L"可能卡住或掉线了", "red");
					//m_pGame->m_pGameProc->NPCLast();
				}

				int t_out = 30;
				//int t_out = m_pGame->m_iQiaZhuS >= 30 ? m_pGame->m_iQiaZhuS : 30;
				if ((now_time - m_iFlagPosTime) >= t_out) {
					//m_pGame->m_pGameProc->SendMsg("可能卡住或掉线了");
				}
			}
		}
		else {
			m_iFlagPosTime = now_time;
		}
	}
	else {
		m_iFlagPosTime = now_time;
	}

	m_dwCmpX = m_dwLastX;
	m_dwCmpY = m_dwLastY;
	m_iGetPosTime = now_time;
}

// 清除移动数据
void Move::ClearMove()
{
	m_dwX = 0;
	m_dwY = 0;
	m_dwLastX = 0;
	m_dwLastY = 0;
	m_dwLastMvX = 0;
	m_dwLastMvY = 0;
	m_dwMvX = 0;
	m_dwMvY = 0;

	m_dwIsEndTime = 0;
	m_dwIsMvTime = 0;
	m_dwMvTime = 0;

	m_dwCmpX = 0;
	m_dwCmpY = 0;
	m_iGetPosTime = 0;
	m_iFlagPosTime = 0;
}

// 是否达到终点
bool Move::IsMoveEnd(_account_* account)
{
#if 0
	DWORD ms = GetTickCount();
	if (ms < (m_dwIsEndTime + 100)) // 小于500豪秒 不判断
		return false;

	m_dwIsEndTime = ms;
#endif
	DWORD x, y;
	m_pGame->m_pGameData->ReadCoor(&x, &y, account); // 读取当前坐标
	//DbgPrint("IsMoveEnd %d,%d %d,%d\n", m_dwX, m_dwY, m_dwMvX, m_dwMvY);
	// 0x168999CB
	return x == account->MvX && y == account->MvY && (m_pGame->m_nHideFlag & 0x0000ff00) == 0x00009900;
}

// 是否移动
bool Move::IsMove(_account_* account)
{
	DWORD ms = GetTickCount();
	if (ms < (account->MvTime + 680)) // 小于500豪秒 不判断
		return true;

	account->MvTime = ms;

	if (!account->LastX || !account->LastY)
		return false;

	DWORD x = 0, y = 0;
	m_pGame->m_pGameData->ReadCoor(&x, &y, account); // 读取当前坐标
	if (x == account->LastX && y == account->LastY) // 没有移动 1秒内坐标没有任何变化
		return false;

	account->LastX = x;
	account->LastY = y;
	account->MvTime = ms;
	return true;
}