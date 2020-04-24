#include "Game.h"
#include "Driver.h"
#include "GameConf.h"
#include "GameData.h"
#include "GameClient.h"
#include "GameProc.h"
#include "Move.h"
#include "Item.h"
#include "Magic.h"
#include "Talk.h"
#include "Pet.h"
#include "PrintScreen.h"

#include <ShlObj_core.h>
#include <My/Common/mystring.h>
#include <My/Common/func.h>
#include <My/Common/C.h>
#include <My/Common/Explode.h>
#include <psapi.h>
#include <stdio.h>
#include <time.h>

#if 1
#if 0
#define SMSG_D(v) m_pGame->m_pClient->SendMsg2(v)
#else
#define SMSG_D(v)
#endif
#define SMSG_DP(p,...) { sprintf_s(p,__VA_ARGS__);SMSG_D(p); }
#else
#define SMSG_D(v) 
#define SMSG_DP(v) 
#endif
#define SMSG(v) SendMsg(v)
#define SMSG_P(p,...) { sprintf_s(p,__VA_ARGS__);SMSG(p); }
#define SMSG_N(n,...) {char _s[n]; SMSG_P(_s,__VA_ARGS__); }

// !!!
GameProc::GameProc(Game* p)
{
	m_pGame = p;
	m_pGameStep = new GameStep;
}

// 初始化数据
void GameProc::InitData()
{
	m_bNoVerify = false;
	m_bStop = false;
	m_bPause = false;
	m_bReStart = false;
	m_bIsCrazy = false;
	m_pStepCopy = nullptr;
	m_pStepLast = nullptr;
	m_pStepLastMove = nullptr;
	m_pStepRecord = nullptr;

	m_bIsRecordStep = false;
	m_bClearKaiRui = false;
	m_bIsResetRecordStep = false;
	m_bIsFirstMove = true;
	m_bPlayFB = true;
	m_bAtFB = false;
	m_bToBig = false;
	m_bNeedCloseBag = false;
	m_bIsPickLast = false;

	m_nBossNum = 0;
	m_nFBTimeLongOne = 0;
	m_nReOpenFB = 0;
	m_nReMoveCount = 0;
	m_nReMoveCountLast = 0;
	m_nYaoBao = 0;
	m_nYao = 0;
	m_nLiveYaoBao = 6;
	m_nLiveYao = 6;

	m_nRevivePetTime = 0;

	ZeroMemory(&m_stLast, sizeof(m_stLast));
	ZeroMemory(&m_ClickCrazy, sizeof(m_ClickCrazy));
}

// 初始化流程
bool GameProc::InitSteps()
{
	if (!m_pGameStep->ReadNPCCoor(m_pGame->m_chPath)) {
	}

	m_pGameStep->InitGoLeiMingSteps(); // 初始化神殿去雷鸣步骤
	for (int i = 0; i < m_pGame->m_pGameConf->m_Setting.FBFileCount; i++) {
		m_pGameStep->InitSteps(m_pGame->m_chPath, m_pGame->m_pGameConf->m_Setting.FBFile[i]);
	}
	return m_pGame->m_pGameConf->m_Setting.FBFileCount > 0;
}

// 切换游戏窗口
void GameProc::SwitchGameWnd(HWND hwnd)
{
	m_hWndGame = hwnd;
	m_pGame->m_hWndBig = hwnd;
}

// 切换游戏帐号
void GameProc::SwitchGameAccount(_account_ * account)
{
	m_pAccount = account;
}

// 鼠标移动到游戏窗口位置
void GameProc::SetGameCursorPos(int x, int y)
{
	HWND top = GetForegroundWindow();
	if (top != m_pAccount->Mnq->WndTop && top != m_pAccount->Mnq->Wnd) // 窗口不在最前
		return;

	RECT rect;
	::GetWindowRect(m_pAccount->Mnq->Wnd, &rect);

	x += 0;// rect.left;
	y += 0;// rect.top;

	int result = SetCursorPos(rect.left + x, rect.top + y);
	//LOGVARN2(64, "blue_r b", L"设置鼠标(%d,%d) %d %d", x, y, result, GetLastError());
}

// 窗口置前
void GameProc::SetForegroundWindow(HWND hwnd)
{
	//必须动态加载这个函数。  
	typedef void (WINAPI *PROCSWITCHTOTHISWINDOW)(HWND, BOOL);
	PROCSWITCHTOTHISWINDOW SwitchToThisWindow;
	HMODULE hUser32 = GetModuleHandle(L"user32");
	SwitchToThisWindow = (PROCSWITCHTOTHISWINDOW)
		GetProcAddress(hUser32, "SwitchToThisWindow");

	//接下来只要用任何现存窗口的句柄调用这个函数即可，
	//第二个参数表示如果窗口处于最小化状态，是否恢复。
	SwitchToThisWindow(hwnd, TRUE);
}

// 神殿去雷鸣大陆流程
void GameProc::GoLeiMing()
{
#if 0
	m_pGameStep->ResetStep(0, 0x02);
	while (ExecStep(m_pGameStep->m_GoLeiMingStep)); // 出神殿
#else
	LOG2(L"等待领取经验层打开(按F1可取消等待)...", "c6");
	for (int i = 0; true; i++) { // 出去练功场
		if (m_bPause)
			break;

		if (m_pGame->m_pTalk->LianGongChangIsOpen()) {
			Sleep(500);
			LOG2(L"领取经验.", "c6");
			Click(700, 596, 800, 620); // 领取经验
			Sleep(1000);
			if (!m_pGame->m_pTalk->LianGongChangIsOpen())
				break;
		}
		else {
			int max_wait = 50;
			if (i >= max_wait && (i&0x01) == 0x00) {
				if (m_pGame->m_pTalk->IsInGamePic() && !m_pGame->m_pGameData->IsInShenYu(m_pAccount)) {
					LOG2(L"移动到62,59.", "c6");
					if (!m_pGame->m_pMove->RunEnd(62, 59, m_pAccount, true, 3000)) {
						if (m_pGame->m_pGameData->IsInShenYu(m_pAccount)) {
							m_pGame->m_pTalk->CloseAllBox();
							break;
						}	
					}
					Sleep(1000);
					LOG2(L"对话罗德·兰.", "c6");
					Click(580, 535, 586, 558); // 罗德·兰
					Sleep(1000);
				}
			}
			else {
				if ((i % 10) == 0) {
					LOGVARN2(32, "cb", L"%d秒后对话罗德·兰.", max_wait - i)
				}
			}
		}

		Sleep(1000);
	}

	LOG2(L"等待进入遗忘神域(按F1取消等待)...", "c6");
	while (!m_pGame->m_pGameData->IsInShenYu(m_pAccount)) {
		if (m_bPause)
			break;

		Sleep(500);
	}
	while (m_pGame->m_pGameData->IsInShenYu(m_pAccount)) {
		if (m_bPause)
			break;

		while (!m_pGame->m_pTalk->IsInGamePic())
			Sleep(500);

		Sleep(1000);
		LOG2(L"对话娜塔莉.", "c6");
		Click(716, 200, 726, 206); // 点击娜塔莉
		Sleep(500);
		if (m_pGame->m_pTalk->WaitTalkOpen(0x00)) {
			LOG2(L"去雷鸣交易行.", "c6");
			Sleep(500);
			m_pGame->m_pTalk->Select(0x00, false);
			LOG2(L"等待进入雷鸣大陆...", "c6");
			Sleep(1000);
		}
	}
	while (!m_pGame->m_pTalk->IsInGamePic())
		Sleep(1000);

	Sleep(1000);
	LOG2(L"已到达雷鸣大陆.", "c6");
#endif
}

// 去领取项链
void GameProc::GoGetXiangLian()
{
}

// 询问项链数量
_account_* GameProc::AskXiangLian()
{
	Account* account = nullptr;
	int max = 0;
	m_pGame->m_pEmulator->List2();
	for (int i = 0; i < m_pGame->m_pEmulator->GetCount(); i++) {
		MNQ* m = m_pGame->m_pEmulator->GetMNQ(i);
		if (!m || !m->Account) // 没有相互绑定
			continue;

		if (!m->Account->IsReadXL) { // 没有读取项链
			SwitchGameWnd(m->Wnd);
			SetForegroundWindow(m->WndTop);
			Sleep(500);
			m->Account->XL = m_pGame->m_pItem->GetBagCount(CIN_XiangLian);
			m->Account->IsReadXL = 1;
		}
		
		if (m->Account->XL > max) {
			max = m->Account->XL;
			account = m->Account;
		}

		DbgPrint("%s有项链%d条 %08X\n", m->Account->Name, m->Account->XL, m->Wnd);
		LOGVARN2(64, "green", L"%hs有项链%d条 %08X", m->Account->Name, m->Account->XL, m->Wnd);
	}

	return account;
}

// 去副本门口
void GameProc::GoFBDoor(_account_* account)
{
	if (m_pGame->m_pGameData->IsInFBDoor(account)) {
		LOG2(L"已经在副本门口", "green b");
		return;
	}

	int i = 0;
	while (true) {
		if (m_bPause || m_bAtFB) 
			break;

		if (++i == 10)
			m_pGame->SaveScreen("去副本门口");

		m_pGame->m_pTalk->CloseAllBox();
		m_pGame->m_pItem->SwitchQuickBar(2);
		Sleep(500);
		DbgPrint("%s使用去副本门口星辰之眼\n", account->Name);
		LOGVARN2(64, "green", L"%hs使用去副本门口星辰之眼", account->Name);
		m_pGame->m_pItem->GoFBDoor();
		Sleep(2000);
		if (m_pGame->m_pGameData->IsInFBDoor(account)) {
			Sleep(2000);
			m_pGame->m_pItem->SwitchQuickBar(1);
			LOGVARN2(64, "green", L"%hs已到达门口", account->Name);
			break;
		}	
	}
}

// 进入副本
_account_* GameProc::OpenFB()
{
	if (IsInFB()) {
		LOG2(L"已经在副本", "green b");
		return m_pGame->m_pBig;
	}

	_account_* account = m_pGame->m_pBig;
	if (!account) {
		DbgPrint("没有人有项链, 无法进入副本\n");
		LOG2(L"没有人有项链, 无法进入副本", "red");
		return nullptr;
	}

	wchar_t log[64];
	DbgPrint("%s去开副本[%s], 背包项链数量:%d\n", account->Name, account->IsBig?"大号":"小号", account->XL);
	LOGVARP2(log, "green", L"%hs去开副本[%hs], 背包项链数量:%d\n", account->Name, account->IsBig ? "大号" : "小号", account->XL);

	GoFBDoor(account);
	
	int i;
_start_:
	if (m_bPause)
		return account;

	SwitchGameWnd(account->Mnq->Wnd);
	DbgPrint("%s去副本门口指定坐标\n", account->Name);
	LOGVARP2(log, "c0", L"%hs去副本门口指定坐标", account->Name);
	m_pGame->m_pMove->RunEnd(863, 500, account); //872, 495 这个坐标这里点击副本门
	Sleep(1000);

	DbgPrint("%s点击副本门\n", account->Name);
	DWORD open_x = MyRand(810, 828), open_y = MyRand(280, 295);
	LOGVARP2(log, "c0", L"%hs点击副本门(%d,%d)", account->Name, open_x, open_y);

	CloseTipBox();
	Click(open_x, open_y); //800, 323 点击副本门
	Sleep(500);
	//return account;

	if (!m_pGame->m_pTalk->WaitTalkOpen(0x00)) {
		if (m_pGame->m_pTalk->IsNeedCheckNPC()) {
			DbgPrint("选择第一个NPC\n");
			LOG2(L"选择第一个NPC", "c0");
			m_pGame->m_pTalk->SelectNPC(0x00);
		}
		if (!m_pGame->m_pTalk->WaitTalkOpen(0x00)) {
			DbgPrint("NPC对话框未打开, 重新再次对话\n");
			LOG2(L"NPC对话框未打开, 重新再次对话", "c0");
			goto _start_;
		}
	}
	Sleep(1000);
	m_pGame->m_pTalk->Select(account->IsBig ? 0x00 : 0x01); // 大号用钥匙, 小号用项链
	Sleep(1000);
	if (CloseTipBox()) {
		goto _start_;
	}
	m_pGame->m_pTalk->Select(0x00); // 确定进入里面
	Sleep(1000);
	if (CloseTipBox()) {
		goto _start_;
	}
	else if (!m_pGame->m_pTalk->WaitTalkClose(0x00)) {
		m_pGame->m_pTalk->Select(0x00);
		m_pGame->m_pTalk->WaitTalkClose(0x00);
	}

	for (i = 0; true; i++) {
		while (m_bPause) Sleep(500);
		if (IsInFB()) {
			DbgPrint("%s已经进入副本\n", account->Name);
			LOGVARP2(log, "c0", L"%hs已经进入副本\n", account->Name);
			break;
		}
		if (i == 15)
			goto _start_;

		DbgPrint("%s等待进入副本\n", account->Name);
		LOGVARP2(log, "c0", L"%hs等待进入副本", account->Name);
		Sleep(1000);
	}

	return account;
}

// 出副本
void GameProc::OutFB(_account_* account)
{
	for (int i = 1; i < 1000; i++) {
		DbgPrint("%s出副本(%d)\n", account->Name, i);
		LOGVARN2(64, "c0", L"%hs出副本(%d)\n", account->Name, i);

		m_pGame->m_pMove->RunEnd(890, 1100, account); // 移动到固定地点
		Sleep(1000);

		Click(190, 503, 195, 505); // 点击NPC
		m_pGame->m_pTalk->WaitTalkOpen(0x00);
		Sleep(1000);
		Click(67, 360, 260, 393);  // 选项0 出副本
		m_pGame->m_pTalk->WaitTalkOpen(0x00);
		Sleep(1000);
		Click(67, 360, 260, 393);  // 选项0 我要离开这里
		m_pGame->m_pTalk->WaitTalkOpen(0x00);
		Sleep(1000);

		if (m_pGame->m_pGameData->IsInFBDoor()) // 出去了
			break;
	}
	
}

// 获取开启副本帐号
_account_ * GameProc::GetOpenFBAccount()
{
	return IsBigOpenFB() ? m_pGame->GetBigAccount() : AskXiangLian();
}

// 是否由大号开启副本
bool GameProc::IsBigOpenFB()
{
	SYSTEMTIME stLocal;
	::GetLocalTime(&stLocal);

	if (stLocal.wHour < 20) // 20点到24点可以免费进
		return false;

	return stLocal.wHour == 23 ? stLocal.wMinute < 59 : true; // 至少要1分钟时间准备
}

// 执行
void GameProc::Exec(_account_* account)
{
}

// 运行
void GameProc::Run(_account_* account)
{
	SwitchGameWnd(account->Mnq->Wnd);
	SwitchGameAccount(account);
	DbgPrint("刷副本帐号:%s(%08X )\n", account->Name, m_hWndGame);
	LOGVARN2(64, "green b", L"刷副本帐号:%hs(%08X)\n", account->Name, m_hWndGame);

	//m_pGame->m_pItem->UseYao();
	//m_pGame->m_pItem->DropItem(CIN_YaoBao, 3);
	//Wait(20 * 1000);
	//m_pGame->m_pPet->Revive();
	//SellItem();
	//m_pGame->m_pItem->CheckIn(m_pGame->m_pGameConf->m_stCheckIn.CheckIns, m_pGame->m_pGameConf->m_stCheckIn.Length);
	//m_pGame->m_pItem->CheckOut(m_pGame->m_pGameConf->m_stUse.Uses, m_pGame->m_pGameConf->m_stUse.Length);
	//return;
	char log[64];

start:
	Exec(account);
	while (true) {
		if (IsInFB()) {
			ExecInFB();
		}
		Sleep(2000);
	}
}

// 去入队坐标
void GameProc::GoInTeamPos(_account_* account)
{
	if (!account->Mnq) {
		LOGVARN2(64, "red", L"%s未绑定模拟器", account->Name);
		return;
	}

	SwitchGameWnd(account->Mnq->Wnd);
	SwitchGameAccount(account);

	if (m_pGame->m_pGameData->IsInShenDian(account)) // 先离开神殿
		GoLeiMing();

	if (!m_pGame->m_pGameData->IsInFBDoor(account)) {
#if 1
		GoFBDoor(account);
#else
		Click(950, 35, 960, 40);     // 点击活动中心
		Sleep(2000);
		Click(450, 680, 500, 690);   // 点击副本按钮
		Sleep(1500);
		Click(150, 562, 200, 570);   // 点击星级副本
		Sleep(1000);
		Click(150, 510, 200, 520);   // 点击阿拉玛的哭泣
		Sleep(1000);
		Click(950, 590, 960, 600);   // 点击开始挑战
		Sleep(1000);
		while (!m_pGame->m_pGameData->IsInFBDoor(account))
			Sleep(1000);
#endif	
	}
	Sleep(1000);

	int x = 880, y = 500;
	if (account->IsBig) {
		x = 882;
		y = 507;
	}
	m_pGame->m_pMove->RunEnd(x, y, account);

	Sleep(2000);
	if (account->IsBig) { // 大号创建队伍
		CreateTeam();
	}
	else {
		//return;
		SwitchGameWnd(m_pGame->GetBigAccount()->Mnq->Wnd);
		ViteInTeam();
		Sleep(2000);
		SwitchGameWnd(account->Mnq->Wnd);
		InTeam(account);
		Sleep(1000);
		m_pGame->m_pMove->Run(872, 495, 0, 0, false, account);
		//Sleep(1000);
		//Click(425, 235);
		//872, 495 这里点击副本门
		//425, 235 点击副本门
	}
	SwitchGameAccount(m_pGame->m_pBig);
}

// 创建队伍
void GameProc::CreateTeam()
{
	DbgPrint("创建队伍...\n");
	LOG2(L"创建队伍...", "c0");
	Click(5, 360, 25, 390);     // 点击左侧组队
	Sleep(1000);
	Click(80, 345, 200, 360);   // 点击创建队伍
	Sleep(1500);
	Click(1160, 60, 1166, 66);  // 点击关闭按钮
	Sleep(1000);
	Click(5, 463, 10, 466);     // 收起左侧组队
	DbgPrint("创建队伍完成\n");
	LOG2(L"创建队伍完成", "c0");
}

// 邀请入队 大号邀请 
void GameProc::ViteInTeam()
{
	DbgPrint("准备邀请入队\n");
	LOG2(L"准备邀请入队", "c0");
	Click(766, 235);           // 点击人物模型
	Sleep(1000);
	Click(410, 35, 415, 40);   // 点击人物头像
	Sleep(1000);
	Click(886, 236, 900, 245); // 点击邀请入队
	DbgPrint("完成邀请入队\n");
	LOG2(L"完成邀请入队", "c0");
}

// 入队
void GameProc::InTeam(_account_* account)
{
	wchar_t log[64];
	DbgPrint("%s准备同意入队\n", account->Name);
	LOGVARP2(log, "blue b", L"%hs准备同意入队\n", account->Name);
	SwitchGameAccount(account);
	SwitchGameWnd(account->Mnq->Wnd);
	SetForegroundWindow(account->Mnq->WndTop);
	AgreenMsg("入队旗帜图标");
	DbgPrint("%s完成同意入队\n", account->Name);
	LOGVARP2(log, "blue b", L"%hs完成同意入队", account->Name);
}

void GameProc::InFB(_account_* account)
{
	wchar_t log[64];
	DbgPrint("%s同意进副本\n", account->Name);
	LOGVARP2(log, "blue b", L"%d同意进副本", account->Name);
	for (int i = 1; i < 2; i++) {
		DbgPrint("%d.设置窗口置前:%08X\n", i, account->Mnq->WndTop);
		LOGVARP2(log, "c0", L"%d.设置窗口置前:%08X", i, account->Mnq->WndTop);
		SetForegroundWindow(account->Mnq->WndTop);
		Sleep(1000);

		if (i & 0x01 == 0x00) {
			RECT rect;
			::GetWindowRect(account->Mnq->WndTop, &rect);
			MoveWindow(account->Mnq->WndTop, 6, 100, rect.right - rect.left, rect.bottom - rect.top, FALSE);
			Sleep(1000);
			::GetWindowRect(account->Mnq->WndTop, &rect);
			mouse_event(MOUSEEVENTF_LEFTDOWN, rect.left + 3, rect.top + 3, 0, 0); //点下左键
			mouse_event(MOUSEEVENTF_LEFTUP, rect.left + 3, rect.top + 3, 0, 0); //点下左键
			Sleep(1000);
		}
		
		HWND top = GetForegroundWindow();
		if (top == account->Mnq->WndTop || top == account->Mnq->Wnd)
			break;
	}
	int max_i = 10;
	for (int i = 1; i <= max_i; i++) {
		if (i > 1) {
			DbgPrint("%s第(%d/%d)次尝试同意进副本\n", account->Name, i, max_i);
			LOGVARP2(log, "c6", L"%hs第(%d/%d)次尝试同意进副本", account->Name, i, max_i);
		}

		SwitchGameAccount(account);
		SwitchGameWnd(account->Mnq->Wnd);
		SetForegroundWindow(account->Mnq->WndTop);
		Sleep(500);
		AgreenMsg("进副本图标");
		Sleep(5000);

		if (m_bPause || IsInFB(account))
			break;
	}
	
	DbgPrint("%s完成同意进副本\n", account->Name);
	LOGVARP2(log, "blue b", L"%hs完成同意进副本", account->Name);

}

// 所有人进副本
void GameProc::AllInFB(_account_* account_open)
{
	int count = m_pGame->m_pEmulator->List2();
	for (int i = 0; i < count; i++) {
		MNQ* m = m_pGame->m_pEmulator->GetMNQ(i);
		if (!m || !m->Account) // 没有相互绑定
			continue;
		if (account_open == m->Account)
			continue;

		InFB(m->Account);
	}
}

// 同意系统信息
void GameProc::AgreenMsg(const char* name, HWND hwnd)
{
	for (int i = 0; i < 3; i++) { // 从最下面开始算
		if (m_bPause || m_bAtFB)
			return;

		SetForegroundWindow(m_pAccount->Mnq->WndTop);
		int result = AgreenMsg(name, i, false, hwnd);
		if (result == 1)
			return;
		if (result == -1)
			break;

		Sleep(1000);
	}
	m_pGame->m_pTalk->CloseAllBox();
	AgreenMsg(name, 0, true, hwnd); // 找不到强制点一下第一个
}

// 同意系统信息
int GameProc::AgreenMsg(const char* name, int icon_index, bool click, HWND hwnd)
{
	SetGameCursorPos(325, 62);

	if (strcmp(name, "进副本图标") == 0) {
		DWORD x = 0, y = 0;
		m_pGame->m_pGameData->ReadCoor(&x, &y, m_pAccount);
		if (x < 800 && y > 415 && y < 490) {
			m_pGame->m_pMove->RunEnd(MyRand(270, 280), MyRand(386, 393), m_pAccount);
			Sleep(1000);
		}
	}

	wchar_t log[64];
	int x = 280, x2 = 306;
	int y = 0, y2 = 0;
	if (icon_index == 0) {
		y = 670;
		y2 = 680;
	}
	if (icon_index == 1) {
		y = 580;
		y2 = 585;
	}
	if (icon_index == 2) {
		y = 480;
		y2 = 485;
	}

	bool is_open = false;
	for (int i = 1; i <= 3; i++) {
		if (m_bPause || m_bAtFB)
			return 1;

		m_pGame->m_pTalk->CloseAllBox();
		DbgPrint("%d.点击社交图标(%d)展开信息\n", i, icon_index);
		LOGVARP2(log, "c6", L"%d.点击社交图标(%d)展开信息", i, icon_index);
		Click(x, y, x2, y2); // 点击社交信息图标 580
		if (m_pGame->m_pTalk->WaitForSheJiaoBox()) {
			is_open = true;
			break;
		}

		DbgPrint("信息框未打开, 再次打开\n");
		LOG2(L"信息框未打开, 再次打开", "red");
	}
	if (!is_open)
		return -1;

	CloseTipBox();
	Sleep(1000);

	// 截取信息图标
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(hwnd ? hwnd : m_hWndGame, 360, 225, 406, 276, 100, true);
	if (!m_pGame->m_pPrintScreen->ComparePixel(name, nullptr, 1)) { // 不是这个信息
		m_pGame->m_pTalk->CloseAllBox();
		DbgPrint("信息框不符合\n");
		LOG2(L"信息框不符合", "red");
		if (!click) {
			DbgPrint("关闭展开的信息框\n");
			LOG2(L"关闭展开的信息框", "c6");
			m_pGame->m_pTalk->CloseSheJiaoBox(); // 关闭
			return 0;
		}
	}

	DbgPrint("点击接受或同意按钮\n");
	LOG2(L"点击接受或同意按钮", "green");
	Click(835, 475, 850, 490); // 点击接受按钮同意
	return 1;
}

// 是否在副本
bool GameProc::IsInFB(_account_* account)
{
    //return true;
	DWORD x = 0, y = 0;
	m_pGame->m_pGameData->ReadCoor(&x, &y, account);
	if (x >= 882 && x <= 930 && y >= 1055 && y < 1115) // 刚进副本区域
		return true;

	return false;
	// 截取地图名称第一个字
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_hWndGame, 1090, 5, 1255, 23, 0, true);
	return m_pGame->m_pPrintScreen->CompareImage(CIN_InFB, nullptr, 1) > 0;
}

// 执行副本流程
void GameProc::ExecInFB()
{
	if (!m_pAccount) {
		DbgPrint("请选择执行副本帐号\n");
		LOG2(L"请选择执行副本帐号", "red");
		return;
	}
	m_bAtFB = true;

	// 调整游戏窗口
	m_pGame->m_pEmulator->ReGameWndSize(m_pAccount->Mnq->Index);
	// 保护当前进程
	m_pGame->m_pDriver->SetProtectPid(0);
	if (m_pAccount->Mnq->VBoxPid) {
		// 保护模拟器进程
		m_pGame->m_pDriver->SetProtectVBoxPid(m_pAccount->Mnq->VBoxPid);
		//LOG2(L"保护模拟器进程", "green b");
	}

#if 0
	for (int i = 0; i < 600; i += 100) {
		m_pGame->m_pGameProc->Click(1052, 503);
		Sleep(100);
	}
	
	while (1) Sleep(680);
#endif

	wchar_t log[128];
	if (1) {
		DbgPrint("执行副本流程\n");
		LOG2(L"执行副本流程", "green b");

		InitData();
		m_pGame->SetStatus(m_pGame->m_pBig, ACCSTA_ATFB, true);

		char* step_file = m_pGameStep->SelectRandStep();

		wchar_t file[128];
		wsprintfW(file, L"文件:%hs", step_file);
		m_pGame->UpdateText("step_file", file);
		
		//m_pGameStep->ResetStep();
		//SendMsg("开始刷副本");
		int start_time = time(nullptr);
		m_nStartFBTime = start_time;
		m_nUpdateFBTimeLongTime = start_time;

		//m_pGame->m_pItem->DropItem(CIN_YaoBao);
		//while (168) Sleep(600);

#if 0
		/* 只有点击了副本地图, 修改坐标才可以自动寻路 */
		m_pGame->m_pMove->OpenMap(m_pGame->m_pBig);
		Sleep(100);
		Click(250, 600, 255, 605, 0xff, m_pGame->m_pBig->Mnq->Wnd); // 点击地图上
		Sleep(100);
		m_pGame->m_pMove->CloseMap(m_pGame->m_pBig->Mnq->Wnd);
		Wait(1 * 1000);
		if (m_pGame->m_pMove->IsOpenMap()) {
			m_pGame->m_pMove->CloseMap(m_pGame->m_pBig->Mnq->Wnd);
			Sleep(1000);
		}
#endif
		m_pGame->m_pTalk->WaitForInGamePic();
		Sleep(500);
		if (IsBigOpenFB()) {
			// 邀请进入副本
			SetGameCursorPos(750, 536);
			Sleep(260);
			int vx = MyRand(700, 800), vy = MyRand(535, 550);
			//m_pGame->m_pEmulator->Tap(vx, vy);
			Click(vx, vy);
			LOGVARP2(log, "blue_r b", L"邀请进入副本:(%d,%d)", vx, vy);
			m_pGame->m_pServer->InFB(m_pGame->m_pBig, nullptr, 0);
			//while (true) Sleep(1000);
		}

		// 画面缩小下
		MouseWheel(-240);
		
		m_pGame->m_pItem->GetQuickYaoOrBaoNum(m_nYaoBao, m_nYao);
		if (m_nYaoBao == 0 && m_nYao == 0) {
			m_pGame->m_pItem->SwitchQuickBar(1);
			Sleep(500);
		}
		// 切换到技能快捷栏
		m_pGame->m_pItem->SwitchMagicQuickBar();
		Sleep(500);
		// 关闭系统社交信息
		m_pGame->m_pTalk->CloseSheJiaoBox(true);
		// 收起左侧组队
		Click(5, 463, 10, 466);

		while (1 && ExecStep(m_pGameStep->m_Step, true)); // 大号刷副本
		m_bLockGoFB = false;

		int end_time = time(nullptr);
		int second = end_time - start_time;
		DbgPrint("已通关，总用时:%02d分%02d秒\n", second / 60, second % 60);
		LOGVARP2(log, "green b", L"已通关，总用时:%02d分%02d秒\n", second / 60, second % 60);

		m_pGame->SetStatus(m_pGame->m_pBig, ACCSTA_ONLINE, true);

		m_pGameStep->ResetStep();
		int game_flag = 0;
		if (0 || m_nReOpenFB == 2) { // 重新开启副本
			m_pGame->UpdateReOpenFBCount(++m_nReOpenFBCount);
			game_flag = 1; // 重新点击登录

			m_pGame->m_pServer->SmallOutFB(m_pGame->m_pBig, nullptr, 0);

			m_pGame->m_pTalk->CloseAllBox();
			m_pGame->LogOut(m_pAccount);
			Sleep(1000);
			if (!m_pGame->m_pTalk->WaitForInLoginPic(NULL, 15 * 1000)) {
				DbgPrint("\n未等待到登录画面\n\n");
				LOG2(L"\n未等待到登录画面", "red");
				game_flag = 2; // 重开游戏
			}
		}
		else {
			if (!m_pAccount->LastX || !m_pAccount->LastY) {
				game_flag = 1; // 重新点击登录
				m_pGame->m_pServer->SmallOutFB(m_pGame->m_pBig, nullptr, 0);
				if (!m_pGame->m_pTalk->WaitForInLoginPic(NULL, 15 * 1000)) {
					DbgPrint("\n未等待到登录画面\n\n");
					LOG2(L"\n未等待到登录画面", "red");
					game_flag = 2; // 重开游戏
				}
			}
		}

		if (game_flag == 0) { // 游戏正常运行
			if (m_nReOpenFB == 0) {
				m_pGame->UpdateFBCountText(++m_nPlayFBCount, true);
			}
			if (m_nReOpenFB == 1) { // 出去重新开始
				OutFB(m_pAccount);
			}
		}
		else { // 游戏已退出到登录界面
			if (game_flag == 2) {
				m_pGame->m_pEmulator->CloseGame(m_pAccount->Index);
				Sleep(1000);
				m_pGame->m_pEmulator->StartGame(m_pAccount->Index);
				Sleep(1000);
				DbgPrint("等待进入登录界面...\n");
				LOG2(L"等待进入登录界面...", "c0");

				int wait_second = 0, wait_second_allow = 180;
				while (!m_pGame->m_pTalk->IsInLoginPic(NULL)) {
					m_pGame->m_pEmulator->List2();
					wait_second += 3;
					Sleep(3000);

					if (wait_second >= wait_second_allow) { // 2分钟等不到, 可能已卡住重启模拟器
						LOG2(L"关闭模拟器.", "red b");
						m_pGame->m_pEmulator->Close(m_pAccount->Index); 
						Sleep(5000);
						LOG2(L"启动模拟器.", "green b");
						m_pGame->m_pEmulator->Open(m_pAccount);
						Sleep(5000);
						m_pGame->m_pEmulator->List2();
						m_pGame->m_pPrintScreen->InjectVBox(m_pGame->m_chPath, m_pAccount->Mnq->UiPid);
						SwitchGameWnd(m_pAccount->Mnq->Wnd);
						LOG2(L"等待进入登录界面...", "c0");

						wait_second = 0;
						wait_second_allow = 300;
					}
				}

				m_pAccount->IsLogin = 1;
				m_pAccount->Addr.MoveX = 0;
				m_pAccount->IsGetAddr = 0;
			}

			Sleep(1000);
			DbgPrint("点击进入游戏\n");
			LOG2(L"点击进入游戏", "blue");
			Click(600, 505, 700, 530);
			Sleep(2000);
			DbgPrint("点击弹框登录\n");
			LOG2(L"点击弹框登录", "blue");
			Click(526, 436, 760, 466);
			Sleep(2000);
			DbgPrint("点击登录\n");
			LOG2(L"点击登录", "blue");
			Click(575, 405);
			DbgPrint("等待进入游戏...\n");
			LOG2(L"等待进入游戏...", "c0");

			int iii = 0, log_click_num = 0;
			do {
				Sleep(1000);
				if (++iii == 3) {
					LOG2(L"点击弹框登录", "blue_r b");
					m_pGame->m_pGameProc->Click(526, 436, 760, 466);
					Sleep(1000);
					LOGVARP2(log, "red", L"点击登录(%d)", ++log_click_num);
					Click(575, 405);
					iii = 0;
				}
				if (game_flag == 2) {
					m_pGame->m_pGameData->WatchGame();
				}

				m_pGame->m_pGameData->ReadCoor(&m_pAccount->LastX, &m_pAccount->LastY, m_pAccount);
				Sleep(100);
			} while (!m_pAccount->IsGetAddr || !m_pAccount->Addr.MoveX || m_pAccount->LastX == 0);

			DbgPrint("等待进入游戏画面...\n");
			LOG2(L"等待进入游戏画面...", "c0");
			do {
				Sleep(1000);
			} while (!m_pGame->m_pTalk->IsInGamePic());

			Sleep(1000);
			if (IsInFB())
				OutFB(m_pAccount);
		}
		
		m_pGame->UpdateFBTimeLongText(end_time - m_nStartFBTime + m_nFBTimeLong, end_time - m_nUpdateFBTimeLongTime); // 更新时间
		m_pGame->InsertFBRecord(m_nStartFBTime, end_time, m_nReOpenFB);
		m_nUpdateFBTimeLongTime = end_time;
		m_nFBTimeLong = end_time - m_nStartFBTime + m_nFBTimeLong;

		m_nReOpenFB = 0; // 设置不重开副本
		//GoFBDoor();
		Sleep(1000);
		while (!m_pGame->m_pGameData->IsInFBDoor(m_pGame->GetBigAccount())) { // 等待出来副本
			m_pGame->m_pTalk->CloseAllBox();
			Sleep(1000);
		}
		m_pGame->m_pTalk->WaitForInGamePic();
		MouseWheel(-240);

		m_bAtFB = false;

		int wait_s = 60;
		int out_time = time(nullptr);

		// 卖东西
		SellItem();

		//if (!GetOpenFBAccount()) // 没有帐号了
		//	return;
		while (true) {
			int c = (time(nullptr) - out_time);
			if (c > wait_s)
				break;

			DbgPrint("等待%d秒再次开启副本, 还剩%d秒\n", wait_s, wait_s - c);
			LOGVARP2(log, "c6", L"等待%d秒再次开启副本, 还剩%d秒", wait_s, wait_s - c);
			Sleep(1000);
		}

		m_pGame->m_pServer->AskXLCount("重新开启副本");
		//Wait((wait_s - (time(nullptr) - out_time)) * 1000);
	}
}

// 执行流程
bool GameProc::ExecStep(Link<_step_*>& link, bool isfb)
{
	m_pStep = m_pGameStep->Current(link);
	if (!m_pStep) {
		return false;
	}

	if (m_pStep->OpCode == OP_MOVE) {
		_step_* next = m_pGameStep->GetNext(link);
		if (next && next->OpCode == OP_PICKUP && strcmp(next->Name, "速效治疗包") == 0) {
			int yaobao = 0, yao = 0;
			m_pGame->m_pItem->GetQuickYaoOrBaoNum(yaobao, yao);
			if (yaobao >= 6) { // 无须再俭药包
				LOGVARN2(32, "blue_r b", L"无须再俭药包, 数量:%d\n", yaobao);
				return m_pGameStep->CompleteExec(link) != nullptr;
			}
		}
	}

	wchar_t log[128];
	_step_* m_pTmpStep = m_pStep; // 临时的

	int now_time = time(nullptr);
	if (isfb) {
		m_pGame->UpdateFBTimeLongText(now_time - m_nStartFBTime + m_nFBTimeLong, now_time - m_nUpdateFBTimeLongTime); // 更新时间
		m_nUpdateFBTimeLongTime = now_time;

		m_nFBTimeLongOne += now_time - m_nStartFBTime;

		if (m_pStepCopy) { // 已经执行到的步骤
			if (m_pStep == m_pStepCopy) {
				m_pStepCopy = nullptr;
			}
			else {
				if (m_pStep->OpCode != OP_MOVE && m_pStep->OpCode != OP_MAGIC) { // 不是移动或放技能
					return m_pGameStep->CompleteExec(link) != nullptr;
				}
			}
		}

		bool check_box = true;
		if (m_pStep->OpCode == OP_MOVE || m_pStep->OpCode == OP_MOVENPC || m_pStep->OpCode == OP_NPC || m_pStep->OpCode == OP_WAIT
			|| m_pStep->OpCode == OP_PICKUP || m_pStep->OpCode == OP_KAIRUI) {
			CloseTipBox(); // 关闭弹出框
			CloseSystemViteBox(); // 关闭系统邀请框

			if (((now_time - m_nRevivePetTime) > 5)) {
				if (m_pGame->m_pPet->Revive()) {
					m_nRevivePetTime = now_time;
				}
			}
			if (m_pGame->m_pTalk->SpeakIsOpen()) {
				m_pGame->m_pTalk->CloseSpeakBox();
				Sleep(500);
			}
			if (m_pStep->OpCode != OP_WAIT && m_pStep->OpCode != OP_PICKUP && m_pGame->m_pItem->BagIsOpen()) {
				m_pGame->m_pItem->CloseBag();
				check_box = false;
				Sleep(500);
			}
		}
		if (m_bNeedCloseBag) {
			Sleep(300);
			if (m_pGame->m_pItem->BagIsOpen()) {
				m_pGame->m_pItem->CloseBag();
				Sleep(500);
			}
			m_bNeedCloseBag = false;
		}

		if (check_box) {
			if (m_pGame->m_pTalk->CommonBoxIsOpen()) {
				m_pGame->m_pTalk->CloseCommonBox();
				Sleep(500);
				
				check_box = false;
			}
		}
	}

	if ((now_time & 0xff) == 0x00) {
		ChNCk();
	}

	bool bk = false;
	char msg[128];
	switch (m_pStep->OpCode)
	{
	case OP_MOVE:
	case OP_MOVEPICKUP:
		DbgPrint("流程->移动:%d.%d至%d.%d\n", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		LOGVARP2(log, "c0 b", L"流程->移动:%d.%d至%d.%d", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		if (m_pStep->OpCode == OP_MOVEPICKUP) {
			m_pGame->m_pItem->GetQuickYaoOrBaoNum(m_nYaoBao, m_nYao);
			if (m_nYaoBao >= 6) {
				LOG2(L"无须再俭药包.", "blue_r b");
				bk = true;
				break;
			}
		}

		if (m_pStep->X2 && m_pStep->Y2) {
			m_pStep->Extra[0] = MyRand(m_pStep->X, m_pStep->X2, m_nFBTimeLongOne);
			m_pStep->Extra[1] = MyRand(m_pStep->Y, m_pStep->Y2, m_nFBTimeLongOne);

			DbgPrint("实际移动位置:%d,%d\n", m_pStep->Extra[0], m_pStep->Extra[1]);
			LOGVARP2(log, "c0", L"实际移动位置:%d,%d", m_pStep->Extra[0], m_pStep->Extra[1]);
		}
		else {
			m_pStep->Extra[0] = m_pStep->X;
			m_pStep->Extra[1] = m_pStep->Y;
		}
		Move(true);
		break;
	case OP_MOVEFAR:
		DbgPrint("流程->传送:%d.%d\n", m_pStep->X, m_pStep->Y);
		LOGVARP2(log, "c0 b", L"流程->传送:%d.%d", m_pStep->X, m_pStep->Y);
		Move();
		break;
	case OP_MOVERAND:
		DbgPrint("流程->随机移动:%d.%d至%d.%d\n", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		LOGVARP2(log, "c0 b", L"流程->随机移动:%d.%d至%d.%d", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		if (MyRand(1, 2, MyRand(m_pStep->X, m_pStep->Y)) == 1) {
			if (m_pStep->X2 && m_pStep->Y2) {
				m_pStep->Extra[0] = MyRand(m_pStep->X, m_pStep->X2, m_nFBTimeLongOne);
				m_pStep->Extra[1] = MyRand(m_pStep->Y, m_pStep->Y2, m_nFBTimeLongOne);

				DbgPrint("实际移动位置:%d,%d\n", m_pStep->Extra[0], m_pStep->Extra[1]);
				LOGVARP2(log, "c0", L"实际移动位置:%d,%d", m_pStep->Extra[0], m_pStep->Extra[1]);
				Move(true);
			}
		}
		else {
			DbgPrint("随机移动跳过\n\n");
			LOG2(L"随机移动跳过\n", "c0 b");
			bk = true;
		}
		break;
	case OP_MOVENPC:
		if (m_pStep->p_npc) {
			LOGVARP2(log, "c0 b", L"流程->移至NPC:%hs", m_pStep->p_npc->Name);
			if (MoveNPC()) {
				m_pStepLastMove = m_pStep;
				LOG2(L"无须再移动", "c0 b");
				bk = true;
			}
		}
		break;
	case OP_NPC:
		DbgPrint("流程->NPC:%s(点击:%d,%d 至 %d,%d)\n", m_pStep->NPCName, m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		LOGVARP2(log, "c0 b", L"流程->NPC:%hs(点击:%d,%d 至 %d,%d)", m_pStep->NPCName, m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		m_stLast.NPCOpCode = OP_NPC;
		NPC();
		break;
	case OP_SELECT:
		DbgPrint("流程->选项:%d\n", m_pStep->SelectNo);
		LOGVARP2(log, "c0 b", L"流程->选项:%d", m_pStep->SelectNo);
		Select();
		break;
	case OP_MAGIC:
		DbgPrint("流程->技能.%s 使用次数:%d\n", m_pStep->Magic, m_pStep->OpCount);
		LOGVARP2(log, "c0 b", L"流程->技能.%hs 使用次数:%d", m_pStep->Magic, m_pStep->OpCount);
		Magic();
		break;
	case OP_MAGIC_PET:
		DbgPrint("流程->宠物技能:%s\n", m_pStep->Magic);
		LOGVARP2(log, "c0 b", L"流程->宠物技能:%hs", m_pStep->Magic);
		//MagicPet();
		break;
	case OP_KAWEI:
		DbgPrint("卡位->使用技能:%s 等待:%d秒\n", m_pStep->Magic, m_pStep->WaitMs / 1000);
		LOGVARP2(log, "c0 b", L"卡位->使用技能:%hs 等待:%d秒", m_pStep->Magic, m_pStep->WaitMs / 1000);
		KaWei();
		break;
	case OP_CRAZY:
		SMSG_D("流程->疯狂");
		//Crazy();
		break;
	case OP_CLEAR:
		SMSG_D("流程->清理");
		//Clear();
		break;
	case OP_KAIRUI:
		DbgPrint("流程->凯瑞 移动:(%d,%d) 截屏:(%d,%d)-(%d,%d)\n", m_pStep->X, m_pStep->Y, 
			m_pStep->Extra[0], m_pStep->Extra[1], m_pStep->Extra[2], m_pStep->Extra[3]);
		LOGVARP2(log, "c0 b", L"流程->凯瑞 移动:(%d,%d) 截屏:(%d,%d)-(%d,%d)", m_pStep->X, m_pStep->Y,
			m_pStep->Extra[0], m_pStep->Extra[1], m_pStep->Extra[2], m_pStep->Extra[3]);
		KaiRui();
		break;
	case OP_PICKUP:
		DbgPrint("流程->捡拾物品:%s, 截屏:(%d,%d)-(%d,%d)\n", m_pStep->Name, m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		LOGVARP2(log, "c0 b", L"流程->捡拾物品:%hs (%d,%d)-(%d,%d)", m_pStep->Name, m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		PickUp();
		break;
	case OP_CHECKIN:
		DbgPrint("流程->存入物品\n");
		LOG2(L"流程->存入物品", "c0 b");
		CheckIn();
		break;
	case OP_USEITEM:
		DbgPrint("流程->使用物品\n");
		LOG2(L"流程->使用物品", "c0 b");
		//UseItem();
		break;
	case OP_DROPITEM:
		DbgPrint("流程->丢弃物品\n");
		LOG2(L"流程->丢弃物品", "c0 b");
		DropItem();
		break;
	case OP_SELL:
		DbgPrint("流程->售卖物品\n");
		LOG2(L"流程->售卖物品", "c0 b");
		//SellItem();
		break;
	case OP_BUTTON:
		DbgPrint("流程->确定\n");
		LOG2(L"流程->确定", "c0 b");
		Button();
		break;
	case OP_CLICK:
		m_stLast.NPCOpCode = OP_CLICK;
		m_stLast.ClickX = m_pStep->X;
		m_stLast.ClickY = m_pStep->Y;
		m_stLast.ClickX2 = 0;
		m_stLast.ClickY2 = 0;
		DbgPrint("流程->点击:%d,%d %d,%d\n", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		LOGVARP2(log, "c0 b", L"流程->点击:%d,%d %d,%d", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		if (m_pStep->X2 && m_pStep->Y2) {
			m_stLast.ClickX2 = m_pStep->X2;
			m_stLast.ClickY2 = m_pStep->Y2;
			Click(m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		}
		else {
			Click(m_pStep->X, m_pStep->Y);
		}
		break;
	case OP_CLICKRAND:
		DbgPrint("流程->随机点击:%d,%d %d,%d 次数:%d\n", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2, m_pStep->OpCount);
		LOGVARP2(log, "c0 b", L"流程->随机点击:%d,%d %d,%d 次数:%d", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2, m_pStep->OpCount);
		ClickRand();
		break;
	case OP_CLICKCRAZ:
		DbgPrint("流程->开启狂点:%d,%d %d,%d 次数:%d\n", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2, m_pStep->OpCount);
		LOGVARP2(log, "c0 b", L"流程->开启狂点:%d,%d %d,%d 次数:%d", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2, m_pStep->OpCount);
		m_ClickCrazy.X = m_pStep->X;
		m_ClickCrazy.Y = m_pStep->Y;
		m_ClickCrazy.X2 = m_pStep->X2;
		m_ClickCrazy.Y2 = m_pStep->Y2;
		m_ClickCrazy.Count = m_pStep->OpCount;
		bk = true;
		break;
	case OP_WAIT:
		DbgPrint("流程->等待:%d %d\n", m_pStep->WaitMs / 1000, m_pStep->Extra[0]);
		LOGVARP2(log, "c0 b", L"流程->等待:%d %d", m_pStep->WaitMs / 1000, m_pStep->Extra[0]);
		Wait();
		break;
	case OP_SMALL:
		if (m_pStep->SmallV == -1) {
			DbgPrint("流程->小号:未知操作\n");
			LOG2(L"流程->小号:未知操作", "red b");
		}
		else {
			DbgPrint("流程->小号:%d %s\n", m_pStep->SmallV, m_pStep->SmallV == 0 ? "出副本" : "进副本");
			LOGVARP2(log, "c0 b", L"流程->小号:%d %hs", m_pStep->SmallV, m_pStep->SmallV == 0 ? "出副本" : "进副本");
			Small();
		}
		break;
	case OP_RECORD:
		DbgPrint("\n--------准备记录下一步骤--------\n\n");
		LOG2(L"\n--------准备记录下一步骤--------\n", "blue_r b");
		m_bIsRecordStep = true;
		bk = true;
		break;
	default:
		SMSG_D("流程->未知");
		LOG2(L"流程->未知", "red b");
		break;
	}

	SMSG_D("流程->执行完成");
	if (bk) {
		return m_pGameStep->CompleteExec(link) != nullptr;
	}
	if (m_bIsResetRecordStep) {
		DbgPrint("\n--------重置到已记录步骤------\n\n");
		LOG2(L"\n--------重置到已记录步骤--------\n", "blue_r b");
		m_pGameStep->ResetStep(m_pStepRecord->Index, 0x01);
		m_bIsResetRecordStep = false;
		return true;
	}

	if ((now_time & 0xff) == 0x02) {
		m_pGame->ChCRC();
	}

	m_nYaoBao = 0;
	m_nYao = 0;

	bool no_mov_screen = false; // 卡住是否已截图了
	int mov_i = 0;
	int drop_i = 0;
	int move_far_i = 0;
	do {
		while (m_bNoVerify); // 没有通过验证

		do {
			if (m_bStop || m_bReStart)
				return false;
			if (m_bPause)
				Sleep(500);
		} while (m_bPause);

		if (isfb && m_pGame->m_Setting.FBTimeOutErvry > 0) { // 副本最大允许时间
			int use_second = time(nullptr) - m_nStartFBTime;
			if (use_second >= m_pGame->m_Setting.FBTimeOutErvry) {
				DbgPrint("\n副本时间达到%d秒, 超过%d秒, 重开副本\n\n", use_second, m_pGame->m_Setting.FBTimeOutErvry);
				LOGVARP2(log, "red b", L"\n副本时间已达到最大允许时间(%d)秒, 超过(%d)秒, 重新开启副本\n", use_second, m_pGame->m_Setting.FBTimeOutErvry);
				m_nReOpenFB = 2;
				return false;
			}
		}

		Sleep(10);

		bool use_yao_bao = false;
		if (m_pStep->OpCode == OP_MOVE || m_pStep->OpCode == OP_MOVERAND || m_pStep->OpCode == OP_MOVENPC) {
			mov_i++;
			if (mov_i == 50 && mov_i <= 60) {
				CloseTipBox(); // 关闭弹出框
			}
			if (mov_i > 0 && (mov_i % 45) == 0) {
				m_pGame->m_pItem->GetQuickYaoOrBaoNum(m_nYaoBao, m_nYao);
				if (m_bIsFirstMove) {
					m_pGame->m_pItem->SwitchMagicQuickBar(); // 切换到技能快捷栏
					m_pGame->m_pTalk->CloseSheJiaoBox(true);
				}
			}
			if ((mov_i % 50) == 0) {
				if (m_bIsFirstMove) {
					m_pGame->m_pTalk->CloseSheJiaoBox(true);
				}
			}
			if (0 && mov_i > 0 && (mov_i % 35) == 0 && m_nReMoveCount == 0) {
				if ((GetTickCount() & 0xff) == 0) {
					Click(298, 40, 327, 68);
				}
			}
			if (mov_i > 0 && mov_i == 300) {
				if (m_bToBig) {
					MouseWheel(-240);
				}
			}

			if (mov_i > 50 && (mov_i % 45) == 0) {
				// 边走路边用药
				if (m_nYaoBao > m_nLiveYaoBao) { // 药包大于6
					if (m_nYao < 2) { // 药小于2
						m_pGame->m_pItem->UseYaoBao(); // 用药包
						m_nYaoBao--;
						m_nYao += 6;
					}
					else {
						m_pGame->m_pItem->UseYao(1, false, 100); // 用药
						m_nYao--;
					}

					use_yao_bao = true;
				}
				else {
					if (m_nYao > m_nLiveYao) { // 药多于6
						m_pGame->m_pItem->UseYao(1, false, 100);
						m_nYao--;

						use_yao_bao = true;
					}
				}
			}

			// 疯狂点击坐标
			if (m_ClickCrazy.Count) {
				ClickCrazy();
			}
		}

		if (m_pStep->OpCode != OP_DROPITEM) {
			SMSG_D("判断加血");
			int life_flag = IsNeedAddLife(12600);
			if (life_flag == 1) { // 需要加血
				SMSG_D("加血");
				m_pGame->m_pItem->UseYao();
				SMSG_D("加血->完成");
			}
			else if (life_flag == -1) { // 需要复活
				m_pGame->SaveScreen("无血");

				m_pGame->m_pGameData->ReadCoor(&m_pAccount->LastX, &m_pAccount->LastY, m_pAccount);
				if (!m_pAccount->LastX || !m_pAccount->LastY) {
					DbgPrint("\n~~~~~~~~~~~~~~~~~~游戏已掉线或已异常结束~~~~~~~~~~~~~~~~~~\n\n");
					LOG2(L"\n~~~~~~~~~~~~~~~~~~游戏已掉线或已异常结束~~~~~~~~~~~~~~~~~~\n", "red");
					return false;
				}

				if (m_nBossNum > 1) {
					m_pGame->m_pServer->SmallOutFB(m_pGame->m_pBig, nullptr, 0);
				}

				ReBorn(); // 复活
				m_pStepCopy = m_pStep; // 保存当前执行副本
				m_pGameStep->ResetStep(0); // 重置到第一步
				DbgPrint("重置到第一步\n");
				LOG2(L"重置到第一步", "red");
				if (m_nBossNum > 0) {
					LOG2(L"重新开启副本", "red");
					m_nReOpenFB = 1;
				}
				return m_nBossNum <= 0;
			}
			SMSG_D("判断加血->完成");
		}

		//SMSG_D("判断流程");
		bool complete = StepIsComplete();
		//SMSG_D("判断流程->完成");
		if (m_pStep->OpCode == OP_MOVEFAR) {
			if (++move_far_i == 300) {
				DbgPrint("传送超时\n");
				LOG2(L"传送超时", "red");
				complete = true;
			}
		}

		if ((m_pGame->m_nHideFlag & 0x000000ff) != 0x000000CB) { // 不是正常启动的0x168999CB
			while (true);
		}

		if (m_nReMoveCount > 0 && (m_nReMoveCount % 5) == 0) {
			if (m_pGame->m_pItem->BagIsOpen()) {
				m_pGame->m_pItem->CloseBag();
				Sleep(500);
			}
		}

		if (m_nReMoveCount > 5) {
			Sleep(100);
			if  (m_nReMoveCount > 8 && (m_stLast.OpCode == OP_NPC || m_stLast.OpCode == OP_SELECT)) {
				if (m_pGame->m_pTalk->TalkBtnIsOpen()) {
					NPCLast(1);
					m_pGame->m_pTalk->WaitTalkOpen(0x00);
					Sleep(1000);
					m_pGame->m_pTalk->Select(0x00);
					Sleep(500);
				}
			}
		}

		if (m_nReMoveCount != m_nReMoveCountLast && m_nReMoveCount >= 15 && m_nReMoveCount < 18) {
			m_pGame->m_pMagic->UseMagic("诸神裁决");
		}

		if (m_nReMoveCount != m_nReMoveCountLast && m_nReMoveCount > 19) {
			DbgPrint("--------可能已卡住--------\n");
			LOG2(L"--------可能已卡住--------", "red");
			if (!no_mov_screen) {
				Sleep(800);
				if (m_pGame->m_pMove->IsOpenMap()) {
					m_pGame->m_pMove->CloseMap(m_hWndGame);
					Sleep(1000);
				}
				
				m_pGame->SaveScreen("卡住");
				no_mov_screen = true;
			}

			if (m_pGame->m_Setting.FBTimeOut > 0) {
				int use_second = time(nullptr) - m_nStartFBTime;
				if (use_second >= m_pGame->m_Setting.FBTimeOut) {
					DbgPrint("\n副本时间达到%d秒, 超过%d秒, 重开副本\n\n", use_second, m_pGame->m_Setting.FBTimeOut);
					LOGVARP2(log, "red b", L"\n副本时间达到%d秒, 超过%d秒, 重开副本\n", use_second, m_pGame->m_Setting.FBTimeOut);
					m_nReOpenFB = 2;
					return false;
				}
			}

			if (m_nReMoveCount < 19 && (m_stLast.OpCode == OP_NPC || m_stLast.OpCode == OP_SELECT)) {
				DbgPrint("--------尝试再次对话NPC--------\n");
				LOG2(L"--------尝试再次对话NPC--------", "blue");
				Sleep(800);
				if (m_pGame->m_pMove->IsOpenMap()) {
					m_pGame->m_pMove->CloseMap(m_hWndGame);
					Sleep(800);
				}
				NPCLast(1);
				m_pGame->m_pTalk->WaitTalkOpen(0x00);
				Sleep(1000);
				m_pGame->m_pTalk->Select(0x00);
				Sleep(500);
				DbgPrint("--------尝试对话NPC完成，需要移动到:(%d,%d)-------\n", m_pStep->X, m_pStep->Y);
				LOGVARP2(log, "blue", L"--------尝试对话NPC完成，需要移动到:(%d,%d)-------", m_pStep->X, m_pStep->Y);
				m_nReMoveCount++;
			}
			DbgPrint("--------%d次后重置到记录步骤--------\n", 19 - m_nReMoveCount);
			LOGVARP2(log, "red", L"--------%d次后重置到记录步骤--------", 19 - m_nReMoveCount);
			if (m_nReMoveCount >= 23 && m_pStepRecord) {
				DbgPrint("--------重置到已记录步骤------\n");
				LOG2(L"\n--------重置到已记录步骤--------\n", "blue_r b");
				m_pGameStep->ResetStep(m_pStepRecord->Index, 0x01);
				m_nReMoveCount = 0;
				m_nReMoveCountLast = 0;
				return true;
			}

			m_nReMoveCountLast = m_nReMoveCount;
		}

		if (complete) { // 已完成此步骤
			DbgPrint("流程->已完成此步骤\n\n");
			LOG2(L"流程->已完成此步骤\n", "c0 b");
			if (m_bIsRecordStep) { // 记录此步骤
				m_pStepRecord = m_pStep;
				m_bIsRecordStep = false;
				DbgPrint("--------已记录此步骤--------\n\n");
				LOG2(L"--------已记录此步骤--------\n", "blue_r b");
			}

			m_stLast.OpCode = m_pStep->OpCode;
			m_pStepLast = m_pStep;
			
			if (m_pStep->OpCode != OP_NPC && m_pStep->OpCode != OP_SELECT && m_pStep->OpCode != OP_WAIT) {
				//if (m_pGame->m_pTalk->NPCTalkStatus()) // 对话框还是打开的
				//	m_pGame->m_pTalk->NPCTalk(0xff);   // 关掉它
			}

			_step_* next = m_pGameStep->CompleteExec(link);
			if (m_pStep->OpCode != OP_WAIT && next) {
				Sleep(use_yao_bao ? 100 : 5);
			}
			return next != nullptr;
		}
	} while (true);

	return false;
}

// 步骤是否已执行完毕
bool GameProc::StepIsComplete()
{
	bool result = false;
	switch (m_pStep->OpCode)
	{
	case OP_MOVE:
	case OP_MOVERAND:
	case OP_MOVENPC:
		//result = true;
		SMSG_D("判断是否移动到终点&是否移动", true);
		if (m_pGame->m_pMove->IsMoveEnd(m_pAccount)) { // 已到指定位置
			m_pStepLastMove = m_pStep;
			m_stLast.MvX = m_pStep->Extra[0];
			m_stLast.MvY = m_pStep->Extra[1];
			m_nReMoveCount = 0;
			m_nReMoveCountLast = 0;
			result = true;

			if (m_bIsFirstMove) {
				// 切换到技能快捷栏
				m_pGame->m_pItem->SwitchMagicQuickBar();
				m_bIsFirstMove = false;
			}
			goto end;
		}
		SMSG_D("判断是否移动到终点->完成", true);
		if (!m_pGame->m_pMove->IsMove(m_pAccount)) {   // 已经停止移动
			Move(false);
			m_nReMoveCountLast = m_nReMoveCount;
			m_nReMoveCount++;
		}
		SMSG_D("判断是否移动->完成", true);
		break;
	case OP_MOVEFAR:
		if (m_pGame->m_pGameData->IsNotInArea(m_pStep->X, m_pStep->Y, 50, m_pAccount)) { // 已不在此区域
			result = true;
			m_nReMoveCount = 0;
			m_nReMoveCountLast = 0;
			goto end;
		}
		if (m_pGame->m_pMove->IsMoveEnd(m_pAccount)) { // 已到指定位置
			m_pGame->m_pMove->Run(m_pStep->X - 5, m_pStep->Y, m_pAccount, m_pStep->X2, m_pStep->Y2);
			m_nReMoveCount = 0;
			m_nReMoveCountLast = 0;
		}
		if (!m_pGame->m_pMove->IsMove(m_pAccount)) {   // 已经停止移动
			m_pGame->m_pMove->Run(m_pStep->X, m_pStep->Y, m_pAccount, m_pStep->X2, m_pStep->Y2);
			m_nReMoveCountLast = m_nReMoveCount;
			m_nReMoveCount++;
		}
		break;
	default:
		result = true;
		break;
	}
end:
	return result;
}

// 移动
void GameProc::Move(bool rand_click)
{
	if (m_bIsFirstMove) {
		DWORD x = MyRand(162, 900);
		DWORD y = MyRand(162, 500);
		m_pGame->m_pMove->Run(m_pStep->Extra[0], m_pStep->Extra[1], m_pAccount, x, y, true, rand_click);
	}
	else {
		m_pGame->m_pMove->Run(m_pStep->Extra[0], m_pStep->Extra[1], m_pAccount, m_pStep->X2, m_pStep->Y2, false, rand_click);
	}
	//m_pGame->m_pItem->UseYao();
	//OpenMap();
	//Sleep(500);
	//Click(215, 473, NULL);
	//Sleep(100);
	//GoWay();
}

// 移至NPC
bool GameProc::MoveNPC()
{
	_npc_coor_* p = m_pStep->p_npc;
	if (!p || p->MvLength == 0)
		return true;

	DWORD click_x, click_y;
	if (CheckInNPCPos(p, click_x, click_y, false) == 1) {
		LOG2(L"已达到目的地", "c0");
		return true;
	}
		
	GetMoveNPCPos(p, m_pStep->Extra[0], m_pStep->Extra[1]);
	Move(false);
	LOGVARN2(32, "c0", L"移至:%d,%d", m_pStep->Extra[0], m_pStep->Extra[1]);

	return false;
}

// 获取要移至位置
int GameProc::GetMoveNPCPos(_npc_coor_* p_npc, DWORD & x, DWORD & y)
{
	int index = 0;
	if (p_npc->MvLength == 2)
		index = 1;
	else if (p_npc->MvLength > 2)
		index = MyRand(1, p_npc->MvLength - 1, m_nFBTimeLongOne);

	if (p_npc->Point[index].MvX2 && p_npc->Point[index].MvY2) {
		x = MyRand(p_npc->Point[index].MvX, p_npc->Point[index].MvX2, m_nFBTimeLongOne);
		y = MyRand(p_npc->Point[index].MvY, p_npc->Point[index].MvY2, m_nFBTimeLongOne);

		LOGVARN2(64, "c9", L"取值:%d,%d (%d,%d)-(%d,%d) %d", x, y,
			p_npc->Point[index].MvX, p_npc->Point[index].MvY, p_npc->Point[index].MvX2, p_npc->Point[index].MvY2,
			p_npc->MvLength);
	}
	else {
		x = p_npc->Point[index].MvX;
		y = p_npc->Point[index].MvY;
	}

	return index;
}

// 对话
void GameProc::NPC()
{
	SetGameCursorPos(325, 62);

	if (m_pGame->m_pMove->IsOpenMap()) {
		m_pGame->m_pMove->CloseMap(m_hWndGame);
		Sleep(500);
	}

	bool kill_boss = false;
	if (strcmp("嗜血骑士", m_pStep->NPCName) == 0) {
		m_pGame->m_pItem->SwitchMagicQuickBar();
	}
	else if (strcmp("四骑士祭坛", m_pStep->NPCName) == 0) {
		kill_boss = true;
		m_nBossNum = 1;
		m_bPlayFB = true;
	}
	else if (strcmp("女伯爵祭坛", m_pStep->NPCName) == 0) {
		kill_boss = true;
		m_nBossNum = 2;
		m_bPlayFB = false;
	}
	else if (strcmp("炎魔督军祭坛", m_pStep->NPCName) == 0) {
		kill_boss = true;
		m_nBossNum = 3;
		m_nLiveYaoBao = 5;
		m_nLiveYao = 1;
		m_bPlayFB = false;
	}
	else if (strcmp("阿拉玛的怨念", m_pStep->NPCName) == 0) {
		kill_boss = true;
		m_nBossNum = 4;
		m_nLiveYaoBao = 6;
		m_nLiveYao = 2;
		m_bPlayFB = false;
	}
	if (strcmp("传送门", m_pStep->NPCName) == 0) {
		MouseWheel(-240);
	}

	if (kill_boss) { // 使用攻击符
		m_pGame->m_pMagic->UseGongJiFu();
		Sleep(500);
	}

	if (m_pStepLastMove && m_pStepLastMove->OpCode == OP_MOVENPC && m_pStepLastMove->p_npc) { // 移至NPC步骤
		DWORD click_x = 0, click_y = 0;
		CheckInNPCPos(m_pStepLastMove->p_npc, click_x, click_y);
		NPC(m_pStep->NPCName, click_x, click_y, 0, 0);
		m_stLast.ClickX = click_x;
		m_stLast.ClickY = click_y;
	}
	else {
		NPC(m_pStep->NPCName, m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		m_stLast.ClickX = m_pStep->X;
		m_stLast.ClickY = m_pStep->Y;
		m_stLast.ClickX2 = m_pStep->X2;
		m_stLast.ClickY2 = m_pStep->Y2;
	}

	strcpy(m_stLast.NPCName, m_pStep->NPCName);
}

// NPC点击
void GameProc::NPC(const char* name, int x, int y, int x2, int y2)
{
	if (m_bToBig) {
		m_pGame->m_pTalk->CloseSheJiaoBox(true);

		MouseWheel(-240);
		m_bToBig = false;
	}

	CloseItemUseTipBox();

	bool click_btn = true;
	if (name && strstr(name, "之书"))
		click_btn = false;

	int i = 0;
_click:
	if (click_btn && m_pGame->m_pTalk->TalkBtnIsOpen()) {
		DbgPrint("点击对话按钮\n");
		LOGVARN2(32, "c0", L"点击对话按钮(%d,%d)", x, y);
		m_pGame->m_pTalk->NPC();
	}
	else {
		if (!x || !y) { // 没有写坐标
			int count = 0;
			while (!m_pGame->m_pTalk->TalkBtnIsOpen()) {
				if (++count == 1)
					break;
				Sleep(500);
			}
			m_pGame->m_pTalk->NPC();
			LOG2(L"点击对话框", "c0");
		}
		else {
			if (x2) {
				x = MyRand(x, x2);
				//DbgPrint("x2:%d\n", x2);
			}

			if (y2) {
				y = MyRand(y, y2);
				//DbgPrint("y2:%d\n", y2);
			}

			Click(x, y);
			DbgPrint("点击NPC:%d,%d\n", x, y);
			LOGVARN2(32, "c0", L"点击:%d,%d", x, y);
		}
	}
	if (strstr(name, "封印机关")) { // 检查机关点完了否
		if (++i == 3)
			return;

		for (int ms = 0; ms < 800; ms += 100) {
			if (m_pGame->m_pTalk->SureBtnIsOpen())
				return;
			Sleep(100);
		}

		LOGVARN2(32, "red b", L"封印机关未解开, 再次点击解开");
		goto _click;
	}
}

// 最后一个对话的NPC
bool GameProc::NPCLast(bool check_pos, DWORD mov_sleep_ms)
{
	CloseTipBox();

	wchar_t log[64];
	bool is_move = false;
	if (check_pos) {
		bool need_move = true;
		if (m_pStep->OpCount == 1) {
			need_move = !m_pGame->m_pTalk->TalkBtnIsOpen();
		}
		if (need_move) {
			if (m_pStepLastMove && m_pStepLastMove->OpCode == OP_MOVENPC && m_pStepLastMove->p_npc) { // 移至NPC步骤
				DWORD click_x = 0, click_y = 0;
				int result = CheckInNPCPos(m_pStepLastMove->p_npc, click_x, click_y);
				if (result) {
					if (result == -1 && mov_sleep_ms) {
						Sleep(mov_sleep_ms);
					}
					NPC(m_pStep->NPCName, click_x, click_y, 0, 0);
					return false;
				}
			}

			DWORD pos_x = 0, pos_y = 0;
			m_pGame->m_pGameData->ReadCoor(&pos_x, &pos_y, m_pAccount);
			if (pos_x != m_stLast.MvX || pos_y != m_stLast.MvY) {
				while (m_bPause) Sleep(500);

				DbgPrint("重新移动到:(%d,%d) 现在位置:(%d,%d)\n", m_stLast.MvX, m_stLast.MvY, pos_x, pos_y);
				LOGVARP2(log, "blue", L"重新移动到:(%d,%d) 现在位置:(%d,%d)", m_stLast.MvX, m_stLast.MvY, pos_x, pos_y);
				if (mov_sleep_ms) {
					Sleep(mov_sleep_ms);
				}

				DWORD time_out = strcmp("传送门", m_stLast.NPCName) == 0 ? (6*1000) : (15*1000);
				if (!m_pGame->m_pMove->RunEnd(m_stLast.MvX, m_stLast.MvY, m_pAccount, false, time_out))
					return false;

				Wait(1 * 1000);
				is_move = true;
			}
		}
		
	}

	if (m_stLast.NPCOpCode == OP_NPC) {
		DbgPrint("再次点击对话打开NPC(%s)\n", m_stLast.NPCName);
		LOGVARP2(log, "blue", L"再次点击对话打开NPC(%hs)", m_stLast.NPCName);
		NPC(m_stLast.NPCName, m_stLast.ClickX, m_stLast.ClickY, m_stLast.ClickX2, m_stLast.ClickY2);
	}
	else if (m_stLast.NPCOpCode == OP_CLICK) {
		if (0 && m_stLast.ClickX2 && m_stLast.ClickY2) {
			Click(m_stLast.ClickX, m_stLast.ClickY);
			DbgPrint("再次点击(%d,%d)-(%d,%d)打开NPC(%s)\n", m_stLast.ClickX, m_stLast.ClickY, m_stLast.ClickX2, m_stLast.ClickY2, m_stLast.NPCName);
			LOGVARP2(log, "blue", L"再次点击(%d,%d)-(%d,%d)打开NPC(%hs)", m_stLast.ClickX, m_stLast.ClickY, m_stLast.ClickX2, m_stLast.ClickY2, m_stLast.NPCName);
		}
		else {
			Click(m_stLast.ClickX, m_stLast.ClickY);
			DbgPrint("再次点击(%d,%d)打开NPC(%s)\n", m_stLast.ClickX, m_stLast.ClickY, m_stLast.NPCName);
			LOGVARP2(log, "blue", L"再次点击(%d,%d)打开NPC(%hs)", m_stLast.ClickX, m_stLast.ClickY, m_stLast.NPCName);
		}
	}

	return is_move;
}

// 检查是否在可点击NPC范围内 is_move=是否移动到范围
int GameProc::CheckInNPCPos(_npc_coor_* p_npc, DWORD& click_x, DWORD& click_y, bool is_move)
{
	if (!p_npc)
		return 0;

	DWORD pos_x, pos_y;
	if (!m_pGame->m_pGameData->ReadCoor(&pos_x, &pos_y, m_pAccount))
		return 0;

	_npc_coor_* p = p_npc;
	int i = 0;
	// 循环判断是否在位置范围内
	for (i = 0; i < p_npc->Length; i++) {
		bool result = false;
		if (!p->Point[i].MvX2 || !p->Point[i].MvY2) { // 只配置了一个
			result = (pos_x == p->Point[i].MvX) && (pos_y == p->Point[i].MvY);
		}
		else { // 配置了两个
			DWORD min_x = min(p->Point[i].MvX, p->Point[i].MvX2),
				max_x = max(p->Point[i].MvX, p->Point[i].MvX2);
			DWORD min_y = min(p->Point[i].MvY, p->Point[i].MvY2),
				max_y = max(p->Point[i].MvY, p->Point[i].MvY2);

			result = (pos_x >= min_x && pos_x <= max_x) && (pos_y >= min_y && pos_y <= max_y);
		}

		if (result) { // 在位置范围里面
			if (p->Point[i].ClkX && p->Point[i].ClkY) {
				click_x = p->Point[i].ClkX2 ? MyRand(p->Point[i].ClkX, p->Point[i].ClkX2) : p->Point[i].ClkX;
				click_y = p->Point[i].ClkY2 ? MyRand(p->Point[i].ClkY, p->Point[i].ClkY2) : p->Point[i].ClkY;
			}
			else {
				GetNPCClickPos(p, pos_x, pos_y, click_x, click_y);
			}
#if 0
			LOGVARN2(64, "green", L"已在目的地(%d,%d) (%d,%d)-(%d,%d)", pos_x, pos_y,
				p->Point[i].MvX, p->Point[i].MvY, p->Point[i].MvX2, p->Point[i].MvY2);
#endif
			return 1;
		}
	}


	if (!is_move)
		return 0;

	DWORD mv_x, mv_y;
	i = GetMoveNPCPos(p_npc, mv_x, mv_y);
	LOGVARN2(64, "blue_r", L"重新移动到:(%d,%d) 现在位置:(%d,%d)", mv_x, mv_y, pos_x, pos_y);

	DWORD time_out = strcmp("传送门", m_stLast.NPCName) == 0 ? (6 * 1000) : (15*1000);
	m_pGame->m_pMove->RunEnd(mv_x, mv_y, m_pAccount, false, time_out);
	if (p->Point[i].ClkX && p->Point[i].ClkY) {
		click_x = p->Point[i].ClkX2 ? MyRand(p->Point[i].ClkX, p->Point[i].ClkX2) : p->Point[i].ClkX;
		click_y = p->Point[i].ClkY2 ? MyRand(p->Point[i].ClkY, p->Point[i].ClkY2) : p->Point[i].ClkY;
	}
	else {
		GetNPCClickPos(p, mv_x, mv_y, click_x, click_y);
	}
	return -1;
}

// 获取点击NPC坐标
bool GameProc::GetNPCClickPos(_npc_coor_ * p_npc, DWORD pos_x, DWORD pos_y, DWORD & click_x, DWORD & click_y)
{
	if (p_npc->Length == 0)
		return false;

	// 已知: 坐标x+1那么点击x-30, y-15
	// 已知: 坐标y+1那么点击x+30, y-15
	int x = (int)pos_x - (int)p_npc->Point[0].MvX;
	int y = (int)pos_y - (int)p_npc->Point[0].MvY;

	int clx_x = 0, clx_y = 0, clx_x2 = 0, clx_y2 = 0;
	// 计算x坐标变化
	clx_x = (int)p_npc->Point[0].ClkX - (x * 30);
	clx_y = (int)p_npc->Point[0].ClkY - (x * 15);
	// 计算y坐标变化
	clx_x += (y * 30);
	clx_y -= (y * 15);

	if (p_npc->Point[0].ClkX2 && p_npc->Point[0].ClkY2) {
		// 计算x坐标变化
		clx_x2 = (int)p_npc->Point[0].ClkX2 - (x * 30);
		clx_y2 = (int)p_npc->Point[0].ClkY2 - (x * 15);
		// 计算y坐标变化
		clx_x2 += (y * 30);
		clx_y2 -= (y * 15);
	}

	click_x = clx_x, click_y = clx_y;
	if (clx_x2 && clx_y2) { // 两个取随机
		click_x = MyRand(clx_x, clx_x2, m_nFBTimeLongOne);
		click_y = MyRand(clx_y, clx_y2, m_nFBTimeLongOne);

#if 0
		LOGVARN2(64, "c9", L"点击取值:%d,%d (%d,%d)-(%d,%d) 位置:%d,%d", 
			click_x, click_y, clx_x, clx_y, clx_x2, clx_y2, pos_x, pos_y);
#endif
	}

	return true;
}

// 选择
void GameProc::Select()
{
	wchar_t log[64];
	if (strcmp("传送门", m_stLast.NPCName) == 0) {
		int n = 0;
		while (true) {
			if (m_pGame->m_pTalk->WaitTalkOpen(m_pStep->SelectNo)) {
				Sleep(500);
				m_pGame->m_pTalk->Select(m_pStep->SelectNo);
				Sleep(1000);
				m_pGame->m_pTalk->Select(m_pStep->SelectNo);
				Sleep(1000);
			}
			
			if (m_pGame->m_pGameData->IsInFBDoor(m_pAccount))
				break;

			if (n++ >= 10) {
				m_bIsResetRecordStep = true;
				break;
			}

			NPCLast(true, 1000);
			if (m_pGame->m_pGameData->IsInFBDoor(m_pAccount))
				break;
		}
		return;
	}

	bool is_add_life = false;
	DWORD life = 0, life_add_i = 0;
	m_pGame->m_pGameData->ReadLife(&life, nullptr, m_pAccount);
	int add_count = m_pStep->OpCount == 1 ? 0 : 0;
	for (DWORD i = 1; i <= m_pStep->OpCount; i++) {
		int max_j = i < 5 ? 3 : 2;
		bool open = false;
		for (int j = 0; j < max_j; j++) {
			while (m_bPause) Sleep(500);

			if (m_pGame->m_pTalk->WaitTalkOpen(m_pStep->SelectNo)) {
				open = true;
				break;
			}
			else {
				if (i > 1) {
					CloseItemUseTipBox();
				}

				if (j >= 1) {
					LOG2(L"检测地图是否打开", "blue");
					if (m_pGame->m_pMove->IsOpenMap()) {
						m_pGame->m_pMove->CloseMap(m_hWndGame);
						Sleep(500);
					}
				}
				    
				bool check_pos = i < 6 && strcmp("传送门", m_stLast.NPCName) != 0;
				if (NPCLast(check_pos, 600 + j * 300)) {
					max_j += add_count;
					add_count = 0;
				}
			}
		}
		if (!open) {
			int _clk = 0;
			if (0 && !IsForegroundWindow()) {
				Sleep(MyRand(680, 800, i));
				m_pGame->m_pTalk->Select(m_pStep->SelectNo);
				_clk = 1;
			}

			if (_clk) {
				m_pGame->m_pTalk->Select(m_pStep->SelectNo);
			}

			DbgPrint("等待对话框打开超过%d次,跳过. 点击:%d\n", max_j, _clk);
			LOGVARP2(log, "c6", L"等待对话框打开超过%d次,跳过. 点击:%d", max_j, _clk);
			goto _check_;
		}

		Sleep(MyRand(100, 150, i));

		DbgPrint("第(%d)次选择 选项:%d\n", i, m_pStep->SelectNo);
		LOGVARP2(log, "c6", L"第(%d)次选择 选项:%d", i, m_pStep->SelectNo);

		DWORD no = m_pStep->SelectNo;
		if (m_pStep->SelectNo == 0x01 && m_pStep->OpCount >= 10 && i > 5) {
			no = m_pGame->m_pTalk->NPCTalkStatus(0x02) ? 0x01 : 0x00;
		}
		m_pGame->m_pTalk->Select(no, false);
		if (!m_pGame->m_pTalk->WaitTalkClose(m_pStep->SelectNo)) {
			if (m_pStep->OpCount == 1 && m_pStep->SelectNo == 0x00) {
				m_pGame->m_pTalk->Select(m_pStep->SelectNo);
				m_pGame->m_pTalk->WaitTalkClose(m_pStep->SelectNo);
			}
		}

		if (i > 1) {
			Sleep(150);
			if (IsNeedAddLife(6000) == 1) {
				is_add_life = true;
				m_pGame->m_pItem->UseYao(5);
				NPCLast();
				i--;
				DbgPrint("i--:%d\n", i);
			}
			else if (i >= 9 && m_stLast.OpCode == OP_SELECT) {
				//m_pGame->m_pItem->UseYao(3);
				//NPCLast();
			}
		}
		if (!is_add_life && life_add_i != i && (i == 5 || i == 8 || i == 10)) { // 献血5次, 血量还跟最开始一样
			DWORD now_life = 0;
			m_pGame->m_pGameData->ReadLife(&now_life, nullptr, m_pAccount);
			if (life == now_life) { // 有可能血量显示不正确
				LOGVARN2(64, "c0", L"%d.血量可能显示错误, 加血(%d!=%d).", i, life, now_life);
				m_pGame->m_pItem->UseYao(5);
				NPCLast();
				life_add_i = i--;
			}
		}
	}

_check_:
	if (m_pStep->OpCount >= 10 && m_pStep->Extra[0]) {
		DbgPrint("加满血\n");
		LOG2(L"加满血", "green");
		m_pGame->m_pItem->AddFullLife();
	}
	if (m_pStep->OpCount == 1 || m_pStep->OpCount >= 10) {
		if (strcmp(m_stLast.NPCName, "四骑士祭坛") == 0 || strcmp(m_stLast.NPCName, "女伯爵祭坛") == 0) {
			Sleep(100);
			if (CloseTipBox()) {
				DbgPrint("--------有封印未解开，需要重置到记录步骤--------\n");
				LOG2(L"--------有封印未解开，需要重置到记录步骤--------", "red");
				m_bIsResetRecordStep = true;
				return;
			}
			else {
				DbgPrint("--------封印已全部解开--------\n");
				LOG2(L"--------封印已全部解开--------", "c0");
			}
		}

		if (IsCheckNPC(m_stLast.NPCName)) {
			DbgPrint("↑↑↑检查NPC:%s↑↑↑\n", m_stLast.NPCName);
			LOGVARP2(log, "c3 b", L"↑↑↑检查NPC:%hs↑↑↑", m_stLast.NPCName);
			int no = m_pStep->SelectNo;
			int i_max = m_pStep->OpCount >= 10 ? 6 : 3;
			for (int i = 1; i <= i_max; i++) {
				if (m_pStep->SelectNo == 0x02)
					break;

				bool is_open = true;
				for (int n = 0; n < 10; n++) {
					Sleep(50);
					if (!m_pGame->m_pTalk->TalkBtnIsOpen()) { // talk box close
						is_open = false;
						break;
					}	
				}
				if (!is_open)
					break;

				if ((i & 0x01) == 0x00) {
					if (m_pStep->OpCount >= 10) {
						m_pGame->m_pItem->UseYao(5);
					}
					if (i == 1)
						continue;
				}

				DbgPrint("%d.NPC未对话完成, 重新点击对话按钮\n", i);
				LOGVARP2(log, "red", L"%d.NPC未对话完成, 重新点击对话按钮", i);
				NPCLast(true);
				if (m_pGame->m_pTalk->WaitTalkOpen(m_pStep->SelectNo)) {
					if (m_pStep->OpCount >= 10) {
						if (IsNeedAddLife(6000) == 1) {
							m_pGame->m_pItem->UseYao(3);
							continue;
						}
					}

					Sleep(MyRand(200, 260, i));
					if (i >= 3 && m_pStep->OpCount >= 10) {
						no = 0x00;
						i--;
					}

					if (m_pStep->SelectNo == 0x01 && m_pStep->OpCount >= 10) {
						no = m_pGame->m_pTalk->NPCTalkStatus(0x02) ? 0x01 : 0x00;
					}

					DbgPrint("%d.选项:%d\n", i, no);
					LOGVARP2(log, "c6", L"%d.选项:%d", i, no);
					m_pGame->m_pTalk->Select(no, false);
					m_pGame->m_pTalk->WaitTalkClose(m_pStep->SelectNo);

					if (strcmp(m_stLast.NPCName, "四骑士祭坛") == 0 || strcmp(m_stLast.NPCName, "女伯爵祭坛") == 0) {
						Sleep(500);
						if (CloseTipBox()) {
							DbgPrint("--------有封印未解开，需要重置到记录步骤--------\n");
							LOG2(L"--------有封印未解开，需要重置到记录步骤--------", "red");
							m_bIsResetRecordStep = true;
							return;
						}
						else {
							DbgPrint("--------封印已全部解开--------\n");
							LOG2(L"--------封印已全部解开--------", "c0");
						}
					}
				}
			}
		}
		else if (IsCheckNPCTipBox(m_stLast.NPCName)) {
			DbgPrint("↑↑↑检查NPC弹框:%s↑↑↑\n", m_stLast.NPCName);
			LOGVARP2(log, "c3 b", L"↑↑↑检查NPC弹框:%hs↑↑↑", m_stLast.NPCName);

			bool result = false;
			for (int i = 0; i < 3; i++) {
				for (int ms = 0; ms < 1500; ms += 50) {
					if (m_pGame->m_pTalk->SureBtnIsOpen()) {
						result = true;
						break;
					}
					Sleep(50);
				}
				if (result)
					break;

				DbgPrint("%d.未发现弹框, 重新点击对话按钮\n", i);
				LOGVARP2(log, "red", L"%d.未发现弹框, 重新点击对话按钮", i);
				NPCLast(true);
				if (m_pGame->m_pTalk->WaitTalkOpen(m_pStep->SelectNo)) {
					Sleep(MyRand(100, 150, i));
					m_pGame->m_pTalk->Select(m_pStep->SelectNo);
					m_pGame->m_pTalk->WaitTalkClose(m_pStep->SelectNo);
				}
			}
			if (!result && strcmp("阿拉玛的怨念", m_stLast.NPCName) == 0) { // 最后BOSS未解封印
				DbgPrint("--------有封印未解开，需要重置到记录步骤--------\n");
				LOG2(L"--------有封印未解开，需要重置到记录步骤--------", "red");
				m_bIsResetRecordStep = true;
				return;
			}
		}
	}
}

// 技能
void GameProc::Magic()
{
	if (strcmp("诸神裁决", m_pStep->Magic) == 0) {
		m_pGame->m_pItem->SwitchMagicQuickBar(); // 切换到技能快捷栏
	}

	for (int i = 1; i <= m_pStep->OpCount; i++) {
		int result = m_pGame->m_pMagic->UseMagic(m_pStep->Magic, m_pStep->X, m_pStep->Y);
		if (m_pStep->OpCount == 1) {
			Sleep(result == -1 ? 500 : 10);
		}
		else {
			Sleep(666);
		}
	}
}

// 卡位
void GameProc::KaWei()
{
	wchar_t log[64];
	int click_x = 0, click_y = 0;
	m_pGame->m_pMagic->GetMagicClickPos(m_pStep->Magic, click_x, click_y);
	if (click_x && click_y) {
		int mv_x = m_pStep->X, mv_y = m_pStep->Y;
		DbgPrint("使用技能:%s(%d,%d) 滑动(%d,%d)\n", m_pStep->Name, click_x, click_y, mv_x, mv_y);
		LOGVARP2(log, "c6", L"使用技能:%hs(%d,%d) 滑动(%d,%d)", m_pStep->Name, click_x, click_y, mv_x, mv_y);
		m_pGame->m_pGameProc->Click(click_x, click_y, 0x01);
		Sleep(500);
		for (int i = 0; i < 1500; i += 50) {
			m_pGame->m_pGameProc->MouseMove(click_x, click_y);
			Sleep(50);
		}
		m_pGame->m_pGameProc->MouseMove(click_x, click_y, mv_x, mv_y);

		int n = 0;
		for (int i = 0; i < m_pStep->WaitMs; i += 500) {
			if (m_bPause)
				break;

			int ls = (m_pStep->WaitMs - i) / 1000;

			if (n != ls) {
				DbgPrint("卡位等待%02d秒，还剩%02d秒.\n", m_pStep->WaitMs / 1000, ls);
				LOGVARP2(log, "c6", L"卡位等待%02d秒，还剩%02d秒.", m_pStep->WaitMs / 1000, ls);
			}
			n = ls;

			m_pGame->m_pGameProc->MouseMove(click_x + mv_x, click_y + mv_y);
			Sleep(500);
		}
		m_pGame->m_pGameProc->Click(click_x + mv_x, click_y + mv_y, 0x02);
	}
	else {
		DbgPrint("KaWei：%d,%d\n", click_x, click_y);
	}
}

// 凯瑞
void GameProc::KaiRui()
{
	if ((m_pGame->m_nHideFlag & 0x00ff0000) != 0x00890000) { // 不是正常启动的0x168999CB
		while (true);
	}
	if (m_bClearKaiRui && m_pStep->Extra[4] != 3) {
		DbgPrint("========凯瑞已被清除，跳过========\n");
		LOG2(L"========凯瑞已被清除，跳过========", "c9");
		return;
	}
	// 移动到指定位置
	m_pGame->m_pMove->RunEnd(m_pStep->X, m_pStep->Y, m_pAccount, false, 60*1000);
	CloseTipBox();
	Sleep(100);
	// 截取弹框确定按钮图片
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_hWndGame, 
		m_pStep->Extra[0], m_pStep->Extra[1], m_pStep->Extra[2], m_pStep->Extra[3], 0, true);

	int red_count = m_pGame->m_pPrintScreen->GetPixelCount(0xffff0000, 0x00101010);
	int yellow_count = m_pGame->m_pPrintScreen->GetPixelCount(0xffffff00, 0x00020202);
	DbgPrint("颜色: 红色(%d), 黄色(%d)\n", red_count, yellow_count);
	LOGVARN2(64, "c0", L"颜色: 红色(%d), 黄色(%d)\n", red_count, yellow_count);

	bool pick_btn = true;
	int pickup_count = 0;
	if ((red_count > 10 && yellow_count > 50) || yellow_count > 200) {
		DbgPrint("========发现凯瑞, 准备干掉它========\n");
		LOG2(L"========发现凯瑞, 准备干掉它========", "c0");
		int max = 3;
		for (int i = 1; i <= max; i++) {
			DbgPrint("========使用技能->诸神裁决========\n");
			LOG2(L"========使用技能->诸神裁决========", "c0");
			m_pGame->m_pMagic->UseMagic("诸神裁决");
			if (IsNeedAddLife(5000) == -1)
				m_pGame->m_pItem->UseYao(2);

			Sleep(500);

			if (0x01) {
				pickup_count = m_pGame->m_pItem->PickUpItem("速效圣兽灵药", m_pStep->Extra[0], m_pStep->Extra[1], m_pStep->Extra[2], m_pStep->Extra[3], 2);
				if (pickup_count > 0) {
					LOGVARN2(32, "c0 b", L"捡物数量:%d", pickup_count);
					if (pickup_count == 1) { // 可能有两个, 再扫描一次
						if (m_pGame->m_pItem->PickUpItem("速效圣兽灵药", 450, 300, 780, 475, 2)) {
							pick_btn = false;
						}
					}
					break;
				}
			}
		}
		m_pGame->m_pMagic->UseMagic("诸神裁决");
		m_bClearKaiRui = true;
	}
	else { 
		if (m_pStep->Extra[4] == 2) {
			m_pGame->m_pMagic->UseMagic("诸神裁决");
			Sleep(800);
			pick_btn = false;
		}

		if (m_pStep->Extra[4] && m_pStep->Extra[4] != 3) { // 扫描圣兽物品
			pickup_count = m_pGame->m_pItem->PickUpItem("速效圣兽灵药", m_pStep->Extra[0], m_pStep->Extra[1], m_pStep->Extra[2], m_pStep->Extra[3], 2);
			if (pickup_count > 0) {
				DbgPrint("========发现凯瑞, 但已被清除========\n");
				LOG2(L"========凯瑞已被清除，跳过========", "c9");
				if (pickup_count > 1) {
					pick_btn = false;
				}
				m_bClearKaiRui = true;
			}
		}
		else {
			pick_btn = false;
			m_pGame->m_pMagic->UseMagic("诸神裁决");
			Sleep(500);
		}
		
	}
	if (pick_btn) {
		for (int n = 0; n < (2 - pickup_count); n++) {
			m_pGame->m_pItem->PickUpItemByBtn();
		}
	}	
}

// 捡物
void GameProc::PickUp()
{
	if (strstr(m_pStep->Name, "按钮")) {
		int max_num = 1;
		if (strstr(m_pStep->Name, "*")) {
			Explode arr("*", m_pStep->Name);
			max_num = arr.GetValue2Int(1);
		}

		//LOGVARN2(32, "c0", L"按钮拾取数量:%d", max_num);
		for (int i = 1; i <= 35; i++) {
			//LOGVARN2(32, "c0", L"%d.按钮捡物.", i);
			if (!m_pGame->m_pItem->PickUpItemByBtn() && i >= max_num) {
				LOGVARN2(32, "c0", L"终止捡物(%d/%d).", i, max_num);
				break;
			}
		}
		return;
	}

	if (strcmp("速效圣兽灵药", m_pStep->Name) == 0) {
		Wait(600);
	}
	if (strcmp("速效治疗包", m_pStep->Name) == 0) {
		Wait(600);
	}
	
	if (CloseTipBox())
		Sleep(500);

	if (m_pGame->m_pTalk->CloseAllBox())
		Sleep(500);

	if (strcmp(m_pStep->Name, "全部") == 0) {
		m_pGame->m_pItem->PickUpItemByBtn();
		CloseItemUseTipBox();
		return;
	}

	bool to_big = false; // 是否放大屏幕
	if (strstr(m_pStep->Name, "."))
		to_big = true;

	if (to_big) {
		MouseWheel(240);
		CloseTipBox();
		m_pGame->m_pTalk->CloseAllBox();
		Sleep(1000);
		m_bToBig = true;
	}

	int pickup_max_num = to_big ? 8 : 6;
	if (strcmp(m_pStep->Name, "速效治疗包") == 0) {
		int yaobao = 0, yao = 0;
		m_pGame->m_pItem->GetQuickYaoOrBaoNum(yaobao, yao);
		if (yaobao == 0) {
			Sleep(500);
			m_pGame->m_pItem->GetQuickYaoOrBaoNum(yaobao, yao);
		}
		if (yaobao >= 6) {
			LOG2(L"无须再捡了.", "c0");
			return;
		}

		pickup_max_num = 6 - yaobao;
		if (pickup_max_num > 5)
			pickup_max_num = 5;

		LOGVARN2(32, "c0", L"需要捡药包数量:%d, 已有:%d.", pickup_max_num, yaobao);
	}
		
	if (strcmp(m_pStep->Name, ".") == 0)
		pickup_max_num = 10;
	if (strcmp(m_pStep->Name, ".nowait") == 0)
		pickup_max_num = 15;

	int pickup_count = m_pGame->m_pItem->PickUpItem(m_pStep->Name, m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2, pickup_max_num);
#if 1
	if (strcmp(m_pStep->Name, ".") == 0 && !m_bIsPickLast) {
		m_bIsPickLast = true;

		if (pickup_count > 5)
			pickup_count = 5;

		for (int n = 0; n < (6 - pickup_count); n++) {
			m_pGame->m_pItem->PickUpItemByBtn();
		}

		int wait_s = 15 - (3 * pickup_count);
		Wait(wait_s * 900);
	}
#endif

	if (to_big) {
		CloseTipBox();
		m_pGame->m_pTalk->CloseAllBox();
		if (pickup_count == 0)
			Sleep(1000);

		MouseWheel(-240);
	}

	if (pickup_count < 2 && m_nBossNum == 3 && strcmp("速效圣兽灵药", m_pStep->Name) == 0) {
		m_pGame->m_pItem->PickUpItemByBtn();
	}

	CloseItemUseTipBox();
}

// 存物
DWORD GameProc::CheckIn(bool in)
{
	CloseTipBox(); // 关闭弹出框
	m_pGame->m_pItem->CheckIn(m_pGame->m_pGameConf->m_stCheckIn.CheckIns, m_pGame->m_pGameConf->m_stCheckIn.Length);
	return 0;
}

// 使用物
void GameProc::UseItem()
{
}

// 丢物
void GameProc::DropItem()
{
	SetGameCursorPos(700, 88);
	CloseTipBox(); // 关闭弹出框
	m_pGame->m_pItem->DropItem(CIN_YaoBao);
	m_bNeedCloseBag = true;
}

// 售卖物品
void GameProc::SellItem()
{
	SetGameCursorPos(325, 62);
	//MouseWheel(-240);
	//Sleep(800);
	wchar_t log[64];
	DbgPrint("\n-----------------------------\n");
	LOG2(L"\n-----------------------------", "c0 b");
	DbgPrint("准备去卖东西\n");
	LOG2(L"准备去卖东西", "green b");
	int pos_x = 271, pos_y = 377;
	DWORD _tm = GetTickCount();
	if (!m_pGame->m_pGameData->IsInArea(pos_x, pos_y, 15)) { // 不在商店那里
		int i = 0;
	use_pos_item:
		if ((i % 5) == 0) {
			CloseTipBox();
			m_pGame->m_pTalk->CloseAllBox();
		}
		if (i > 0 && (i % 10) == 0 && i <= 100) {
			m_pGame->SaveScreen("去商店卡住");
		}
		m_pGame->m_pTalk->CloseAllBox(); // 关闭所有遮挡
		if (!m_pGame->m_pItem->QuickBarIsXingChen()) { // 切换
			DbgPrint("(%d)切换快捷栏\n", i + 1);
			LOGVARP2(log, "blue", L"(%d)切换快捷栏", i + 1);
			m_pGame->m_pItem->SwitchQuickBar(2); // 切换快捷栏
			Sleep(500);
		}
		
		DbgPrint("(%d)使用星辰之眼\n", i + 1);
		LOGVARP2(log, "blue", L"(%d)使用星辰之眼", i + 1);
		m_pGame->m_pItem->GoShop();         // 去商店旁边
		Sleep(500);
		for (; i < 100;) {
			if (i > 0 && (i % 10) == 0) {
				i++;
				goto use_pos_item;
			}
			if (m_pGame->m_pGameData->IsInArea(pos_x, pos_y, 15)) { // 已在商店旁边
				Sleep(100);
				break;
			}

			i++;
			Sleep(1000);
		}
		m_pGame->m_pItem->SwitchQuickBar(1); // 切回快捷栏
		Sleep(300);
	}

	m_pGame->m_pMove->RunEnd(pos_x, pos_y, m_pGame->m_pBig); // 移动到固定点好点击
	CloseTipBox();
	Sleep(1000);
	//MouseWheel(-240);
	//Sleep(1000);
	//m_pGame->m_pMove->RunEnd(pos_x, pos_y, m_pGame->m_pBig); // 移动到固定点好点击
	//MouseWheel(-240);
	//Sleep(800);

	int rand_v = GetTickCount() % 2;
	int clk_x, clk_y, clk_x2, clk_y2;
	if (rand_v == 0) { // 装备商
		clk_x = 276, clk_y = 263;
		clk_x2 = 295, clk_y2 = 305;
	}
	else { // 武器商
		clk_x = 562, clk_y = 367;
		clk_x2 = 586, clk_y2 = 399;
	}

	//m_pGame->m_pEmulator->Tap(MyRand(clk_x, clk_x2), MyRand(clk_y, clk_y2));
	if (m_pGame->m_pTalk->TalkBtnIsOpen()) {
		LOG2(L"点击对话按钮", "c0");
		m_pGame->m_pTalk->NPC();
	}
	else {
		Click(clk_x, clk_y, clk_x2, clk_y2);      // 对话商店人物
	}
	m_pGame->m_pTalk->WaitTalkOpen(0x00);
	Sleep(500);
_select_no_:
	m_pGame->m_pTalk->Select(0x00); // 购买物品
	Sleep(1500);
	if (m_pGame->m_pTalk->NPCTalkStatus(0x00)) {
		goto _select_no_;
	}

	m_pGame->m_pItem->SellItem(m_pGame->m_pGameConf->m_stSell.Sells, m_pGame->m_pGameConf->m_stSell.Length);
	Sleep(500);
	m_pGame->m_pItem->SetBag();
	Sleep(500);
	m_pGame->m_pItem->CloseShop();
	Sleep(500);
	// 在那个位置才卖
	// m_pGame->m_pGameData->IsInArea(pos_x, pos_y, 0, m_pGame->m_pBig)
	if (m_pGame->m_pItem->CheckOut(m_pGame->m_pGameConf->m_stSell.Sells, m_pGame->m_pGameConf->m_stSell.Length)) {
		Sleep(500);
		m_pGame->m_pMove->RunEnd(pos_x, pos_y, m_pGame->m_pBig); // 移动到固定点好点击

		m_pGame->m_pTalk->CloseAllBox();
		Sleep(500);
		//m_pGame->m_pEmulator->Tap(MyRand(clk_x, clk_x2), MyRand(clk_y, clk_y2));
		if (m_pGame->m_pTalk->TalkBtnIsOpen()) {
			LOG2(L"点击对话按钮2", "c0");
			m_pGame->m_pTalk->NPC();
		}
		else {
			Click(clk_x, clk_y, clk_x2, clk_y2);      // 对话商店人物
		}
		
		m_pGame->m_pTalk->WaitTalkOpen(0x00);
		Sleep(500);
	_select_no2_:
		m_pGame->m_pTalk->Select(0x00); // 购买物品
		Sleep(1500);
		if (m_pGame->m_pTalk->NPCTalkStatus(0x00)) {
			goto _select_no2_;
		}
		m_pGame->m_pItem->SellItem(m_pGame->m_pGameConf->m_stSell.Sells, m_pGame->m_pGameConf->m_stSell.Length);
		Sleep(500);
#if 1
		DbgPrint("点击左侧修理装备\n");
		LOG2(L"点击左侧修理装备", "c0");
		Click(8, 260, 35, 358); // 左侧修理装备
		Sleep(1000);
		DbgPrint("点击全部修理\n");
		LOG2(L"点击全部修理", "c0");
		Click(390, 645, 500, 666); // 按钮全部修理
		if (m_pGame->m_pTalk->WaitForConfirmBtnOpen(1500)) {
			Sleep(150);
			DbgPrint("确定修理装备\n");
			LOG2(L"确定修理装备", "c0");
			m_pGame->m_pTalk->ClickConfirmBtn(1); // 确定
			Sleep(500);
		}
		else {
			DbgPrint("无须修理装备\n");
			LOG2(L"无须修理装备", "c0");
		}
#endif
		Click(16, 150, 20, 186); // 展开宠物列表
		Sleep(350);
		m_pGame->m_pItem->CloseShop();
		Sleep(500);
	}

	CloseTipBox();
	m_pGame->m_pTalk->CloseAllBox();

	_tm = GetTickCount() - _tm;
	DbgPrint("卖东西用时%.2f秒, %d毫秒\n", (float)_tm / 1000.0f, _tm);
	LOGVARP2(log, "green b", L"卖东西用时%d秒, %d毫秒", _tm / 1000, _tm);
	DbgPrint("\n-----------------------------\n");
	LOG2(L"\n-----------------------------", "c0 b");
}

// 按钮
void GameProc::Button()
{
	// 确定按钮 597,445 606,450
	CloseTipBox();
}

// 关闭弹框
bool GameProc::CloseTipBox()
{
	// 截取弹框确定按钮图片
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_hWndGame, 585, 450, 600, 465, 0, true);
	if (m_pGame->m_pPrintScreen->ComparePixel("提示框", nullptr, 1)) {
		DbgPrint("关闭提示框\n");
		LOG2(L"关闭提示框", "orange b");
		Click(576, 440, 695, 468);
		Sleep(100);

		return true;
	}

	return false;
}

// 关闭物品使用提示框
bool GameProc::CloseItemUseTipBox()
{
	// 截取物品使用提示框按钮前面图片
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_hWndGame, 855, 376, 865, 386, 0, true);
	if (m_pGame->m_pPrintScreen->ComparePixel("物品使用提示框", nullptr, 1)) {
		DbgPrint("关闭物品使用提示框\n");
		LOG2(L"关闭物品使用提示框", "orange b");
		Click(1032, 180);
		Sleep(300);

		return true;
	}

	return false;
}

// 关闭系统邀请提示框
bool GameProc::CloseSystemViteBox()
{
	// 截取邀请框标题栏背景色
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_hWndGame, 765, 187, 775, 197, 0, true);
	if (m_pGame->m_pPrintScreen->ComparePixel("邀请确认", nullptr, 1)) {
		DbgPrint("关闭邀请确认框\n");
		LOG2(L"关闭邀请确认框", "orange b");
		Click(450, 435, 550, 450);
		Sleep(300);

		return true;
	}

	return false;
}

// 随机点击
void GameProc::ClickRand()
{
	int n = MyRand(2, 3, GetTickCount());
	for (int i = 0; i < m_pStep->OpCount; i++) {
		if (MyRand(1, n, i) == 1)
			continue;

		int x = m_pStep->X, y = m_pStep->Y;
		if (m_pStep->X2)
			x = MyRand(m_pStep->X, m_pStep->X2, i * 20);
		if (m_pStep->Y2)
			y = MyRand(m_pStep->Y, m_pStep->Y2, i * 20);

		Click(x, y);
		DbgPrint("随机点击:%d,%d\n", x, y);
		Sleep(MyRand(300, 1000, i * 20));
	}
}

// 狂暴点击
void GameProc::ClickCrazy()
{
	if (m_ClickCrazy.Count <= 0)
		return;

	if (MyRand(1, 2, m_ClickCrazy.Count) == 1) {
		int x = m_ClickCrazy.X, y = m_ClickCrazy.Y;
		if (m_pStep->X2)
			x = MyRand(m_ClickCrazy.X, m_ClickCrazy.X2, m_ClickCrazy.Count * 20);
		if (m_pStep->Y2)
			y = MyRand(m_ClickCrazy.Y, m_ClickCrazy.Y2, m_ClickCrazy.Count * 20);

		Click(x, y);
		DbgPrint("狂暴点击:%d,%d 还剩%d次\n", x, y, m_ClickCrazy.Count - 1);
		LOGVARN2(64, "c6", L"狂暴点击:%d,%d 还剩%d次", x, y, m_ClickCrazy.Count - 1);
		Sleep(MyRand(300, 500, m_ClickCrazy.Count));
	}
	m_ClickCrazy.Count--;
}

// 等待
void GameProc::Wait()
{
#if 0
	if (m_stLast.OpCode != OP_NPC && m_stLast.OpCode != OP_SELECT) {
		Click(1150, 352, 1180, 372); // 最上面物品栏
	}
#endif
	
	Wait(m_pStep->WaitMs, m_pStep->Extra[0]);
}

// 等待
void GameProc::Wait(DWORD ms, int no_open)
{
	wchar_t log[64];
	if (ms < 1000) {
		DbgPrint("等待%d毫秒\n", ms);
		LOGVARP2(log, "c6", L"等待%d毫秒", ms);
		Sleep(ms);
		return;
	}

	if (0 && ms >= 2000 && !no_open) {
		DWORD start_ms = GetTickCount();
		if ((start_ms & 0x0f) < 3) {
			Click(16, 26, 66, 80); // 点击头像
			Sleep(1500);
		
			int click_count = MyRand(0, 2 + (ms / 1000 / 3), ms);
			for (int i = 0; i < click_count; i++) { // 胡乱操作一下
				Click(315, 186, 1160, 360);
				Sleep(MyRand(500, 1000, i));
			}

			Click(1155, 55, 1160, 60); // 关闭
			Sleep(500);

			DWORD use_ms = GetTickCount() - start_ms;
			if (use_ms > ms)
				ms = 0;
			else
				ms -= use_ms;
		}
	}

	if (ms >= 12000 && IsNeedAddLife(100) != -1) {
		DbgPrint("等待时间达到%d秒(%d毫秒), 先整理背包\n", ms / 1000, ms);
		LOGVARP2(log, "c0", L"等待时间达到%d秒(%d毫秒), 先整理背包", ms / 1000, ms, ms);
		DWORD _tm = 0;
		m_pGame->m_pItem->DropItem(CIN_YaoBao, 6, &_tm);
		if (_tm > ms)
			ms = 0;
		else
			ms -= _tm;
	}

	int n = 0;
	for (int i = 0; i < ms; i += 100) {
		int ls = (ms - i) / 1000;

		if (n != ls) {
			DbgPrint("等待%02d秒，还剩%02d秒.\n", ms / 1000, ls);
			LOGVARP2(log, "c6", L"等待%02d秒，还剩%02d秒.", ms / 1000, ls);
		}
		n = ls;
		Sleep(100);
	}
}

// 小号
void GameProc::Small()
{
#if 1
	if (m_pStep->SmallV == 0) { // 出副本
		m_pGame->m_pServer->SmallOutFB(m_pGame->m_pBig, nullptr, 0);
	}
	if (m_pStep->SmallV == 1) { // 进副本
		m_pGame->m_pServer->SmallInFB(m_pGame->m_pBig, nullptr, 0);
	}
#else
	if (m_pStep->SmallV == 0) { // 出副本
		Click(597, 445, 606, 450); // 关闭提示框
		Sleep(500);
		Click(597, 445, 606, 450); // 点击复活人物
		Sleep(2000);

		int count = m_pGame->m_pEmulator->List2();
		for (int i = 0; i < count; i++) {
			MNQ* m = m_pGame->m_pEmulator->GetMNQ(i);
			if (m && m->Account && !m->Account->IsBig) {
				SwitchGameWnd(m->Wnd);
				SetForegroundWindow(m->WndTop);
				Sleep(100);
				m_pGame->m_pMove->Run(890, 1100, 0, 0, false, m->Account); // 移动到固定地点
			}
		}
		Sleep(1000);

		ClickOther(190, 503, 195, 505, m_pGame->m_pBig); // 点击NPC
		Sleep(2000);
		ClickOther(67, 360, 260, 393, m_pGame->m_pBig);  // 选项0 出副本
		Sleep(1000);
		ClickOther(67, 360, 260, 393, m_pGame->m_pBig);  // 选项0 我要离开这里

		SwitchGameWnd(m_pGame->m_pBig->Mnq->Wnd);
		SetForegroundWindow(m_pGame->m_pBig->Mnq->WndTop);
	}
#endif
}

// 复活
void GameProc::ReBorn()
{
	LOG2(L"\n等待人物复活...", "red b");
	Wait(26 * 1000);
	CloseTipBox();
	m_pGame->m_pTalk->CloseAllBox();

	m_pGame->SaveScreen("复活前");
	for (int i = 1; true; i++) {
		if (CloseTipBox())
			Sleep(500);
		if (m_pGame->m_pTalk->CloseAllBox())
			Sleep(500);

		LOGVARN2(32, "c0 b", L"点击复活(%d)", i);
		Click(537, 415, 550, 425); // 点复活
		Sleep(2000);

		if (IsNeedAddLife(100) != -1)
			break;

	}

	m_pGame->SaveScreen("点击复活");

	LOG2(L"等待进入游戏画面...", "red b");
	Wait(2000);
	m_pGame->SaveScreen("复活等待进入游戏");
	do {
		m_pGame->m_pTalk->CloseAllBox();
		Sleep(1000);
	} while (!m_pGame->m_pTalk->IsInGamePic());

	LOG2(L"宠物全部合体", "red b");
	m_pGame->m_pPet->PetOut(-1);
	Sleep(1000);
}

// 是否检查此NPC对话完成
bool GameProc::IsCheckNPC(const char* name)
{
	return strcmp(name, "魔封障壁") == 0 || strcmp(name, "地狱火障壁") == 0 || strcmp(name, "爆雷障壁") == 0
		|| strcmp(name, "仇之缚灵柱") == 0 || strcmp(name, "怨之缚灵柱") == 0 || strcmp(name, "嗜血骑士") == 0
		|| strcmp(name, "四骑士祭坛") == 0 || strcmp(name, "女伯爵祭坛") == 0
		|| strcmp(name, "往昔之书") == 0
		|| strcmp(name, "阿拉玛的怨念") == 0
		|| strstr(name, "元素障壁") != nullptr;
}

// 是否检查此NPC对话完成
bool GameProc::IsCheckNPCTipBox(const char* name)
{
	return strstr(name, "封印机关") != nullptr
		|| strstr(name, "图腾") != nullptr
		|| strstr(name, "印记") != nullptr
		|| strcmp(name, "炎魔督军祭坛") == 0
		|| strcmp(name, "爱娜的灵魂") == 0
		|| strcmp(name, "献身之书") == 0
		|| strcmp(name, "复仇之书") == 0;
}


// 鼠标移动[相对于x或y移动rx或ry距离]
void GameProc::MouseMove(int x, int y, int rx, int ry, HWND hwnd)
{
	LOGVARN2(64, "c0 b", L"鼠标滑动(%d,%d)-(%d,%d)", x, y, x + rx, y + ry);

	int dx = abs(rx), dy = abs(ry);
	for (int i = 1; true;) {
		if (i > dx && i > dy)
			break;

		int nx = MyRand(2, 3), ny = MyRand(2, 3);

		Sleep(MyRand(1, 2));
		for (int j = 0; j < 1; j++) {
			int mv_x = 0, mv_y = 0;
			if (i < dx) {
				mv_x = rx > 0 ? i : -i;
			}
			else {
				mv_x = rx;
			}
			if (i < dy) {
				mv_y = ry > 0 ? i : -i;
			}
			else {
				mv_y = ry;
			}
			MouseMove(x + mv_x, y + mv_y, hwnd);
		}
		

		//DbgPrint("MouseMove:%d,%d\n", x + mv_x, y + mv_y);
		//LOGVARN2(64, "c0 b", L"MouseMove:%d,%d\n", x + mv_x, y + mv_y);
		i += MyRand(1, 2);
	}

	LOG2(L"鼠标滑动完成", "c0 b");
}

// 鼠标移动
void GameProc::MouseMove(int x, int y, HWND hwnd)
{
	if (!hwnd)
		hwnd = m_hWndGame;

	::SendMessage(hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x, y));
	//DbgPrint("鼠标移动:%d,%d\n", x, y);
}

// 鼠标滚轮
void GameProc::MouseWheel(int x, int y, int z, HWND hwnd)
{
	if (!hwnd)
		hwnd = m_hWndGame;

	::SendMessage(hwnd, WM_MOUSEWHEEL, MAKEWPARAM(MK_CONTROL, z), MAKELPARAM(x, y));
}

// 鼠标滚轮
void GameProc::MouseWheel(int z, HWND hwnd)
{
	DWORD dwPicScale = z > 0 ? 256 : 128; // 正常128 最大256
	if (m_pGame->m_pGameData->WritePicScale(dwPicScale, m_pAccount)) { // 成功写入了
		Sleep(500);
		return;
	}

	if (!hwnd)
		hwnd = m_hWndGame;

	RECT rect;
	::GetWindowRect(hwnd, &rect);
	MouseWheel(MyRand(rect.left+500, rect.left+800), MyRand(rect.top+236, rect.top+350), z, hwnd);
	Sleep(1500);
}

// 鼠标左键点击
void GameProc::Click(int x, int y, int ex, int ey, int flag, HWND hwnd)
{
	Click(MyRand(x, ex), MyRand(y, ey), flag, hwnd);
}

// 鼠标左键点击
void GameProc::Click(int x, int y, int flag, HWND hwnd)
{
	if (!hwnd)
		hwnd = m_hWndGame;

	if (flag & 0x01) {
		::PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
	}
	if (flag & 0x02) {
		::PostMessage(hwnd, WM_LBUTTONUP, 0, MAKELPARAM(x, y));
	}
	Sleep(15);
	//DbgPrint("===点击:%d,%d===\n", x, y);
}

// 鼠标左键点击
void GameProc::Click_Send(int x, int y, int ex, int ey, int flag, HWND hwnd)
{
	Click_Send(MyRand(x, ex), MyRand(y, ey), flag, hwnd);
}

// 鼠标左键点击
void GameProc::Click_Send(int x, int y, int flag, HWND hwnd)
{
	if (!hwnd)
		hwnd = m_hWndGame;

	if (flag & 0x01) {
		::SendMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
	}
	if (flag & 0x02) {
		::SendMessage(hwnd, WM_LBUTTONUP, 0, MAKELPARAM(x, y));
	}
	Sleep(10);
	//DbgPrint("===点击:%d,%d===\n", x, y);
}

// 鼠标左键点击[不包括此帐号]
void GameProc::ClickOther(int x, int y, int ex, int ey, _account_* account_no)
{
	int count = m_pGame->m_pEmulator->List2();
	for (int i = 0; i < count; i++) {
		MNQ* m = m_pGame->m_pEmulator->GetMNQ(i);
		if (!m || !m->Account) // 没有相互绑定
			continue;
		if (m->Account == account_no) // 不包活此帐号
			continue;

		DbgPrint("GameProc::ClickOther %s\n", m->Account->Name);
		Click(x, y, ex, ey, 0xff, m->Wnd);
	}
}

// 鼠标左键双击
void GameProc::DBClick(int x, int y, HWND hwnd)
{
	if (!hwnd)
		hwnd = m_hWndGame;

	//::PostMessage(hwnd, WM_LBUTTONDBLCLK, MK_LBUTTON, MAKELPARAM(x, y));
	Click(x, y, 0xff, hwnd);
	Click(x, y, 0xff, hwnd);
}

// 按键
void GameProc::Keyboard(char key, int flag, HWND hwnd)
{
	if (!hwnd)
		hwnd = m_hWndGame;

	if (flag & 0x01)
		::PostMessage(hwnd, WM_KEYDOWN, key, 0);
	if (flag & 0x02)
		::PostMessage(hwnd, WM_KEYUP, key, 0);
}

int GameProc::IsNeedAddLife(int low_life)
{
	m_pGame->m_pGameData->ReadLife();
	if (m_pGame->m_pGameData->m_dwLife == 0)
		return -1;
	if (m_pGame->m_pGameData->m_dwLife < low_life)
		return 1;
	return 0;
}

// 是否最前窗口
bool GameProc::IsForegroundWindow()
{
	if (!m_pGame->m_pBig->Mnq)
		return false;

	HWND hWndTop = ::GetForegroundWindow();
	return hWndTop == m_pGame->m_pBig->Mnq->WndTop || hWndTop == m_pGame->m_pBig->Mnq->Wnd;
}

// 检查是否修改了数据
bool GameProc::ChNCk()
{
	//printf("bool GameProc::ChNCk().\n");
	FuncAddr func;
	func.f = &Home::IsValid;
	char* p = (char*)func.v;
	char v[] = { 0x40, 0x53, 0x48, 0x83, 0xEC, 0x20, 0x80, 0xB9, 0xC4, 0x01, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xD9, 0x74, 0x08, 0xB0, 0x01, 0x48, 0x83, 0xC4, 0x20, 0x5B, 0xC3, 0x83, 0xB9, 0x8C, 0x01, 0x00, 0x00, 0x00, 0x74, 0x31, 0x83, 0xB9, 0x90, 0x01, 0x00, 0x00, 0x00, 0x74, 0x28, 0x83, 0xB9, 0x94, 0x01, 0x00, 0x00, 0x00, 0x74, 0x1F, 0x33, 0xC9, 0xFF, 0x15, 0x63, 0xE5, 0x0A, 0x00, 0x3B, 0x83, 0x8C, 0x01, 0x00, 0x00, 0x7C, 0x0F, 0x3B, 0x83, 0x94, 0x01, 0x00, 0x00, 0x0F, 0x9E, 0xC0, 0x48, 0x83, 0xC4, 0x20, 0x5B, 0xC3, 0x32, 0xC0, 0x48, 0x83, 0xC4, 0x20, 0x5B, 0xC3, 0xCC, 0xCC, 0xCC, 0xCC };
	for (int i = 0; i < 0x60; i++) {
		if ((p[i]&0xff) == 0xff) {
			//printf("%d:0xFF.\n", i);
			i += 5;
			continue;
		}
		//printf("%02X ", *p & 0xff);
		if (p[i] != v[i]) {
			//printf("%d(%02X).已修改2(%02X!=%02X).\n", i, i&0xff, p[i] & 0xff, v[i] & 0xff);
			//::MessageBox(NULL, L"ok", L"BBB", MB_OK);
			m_pGame->m_pDriver->BB();
			while (true);
		}
	}
	return false;
}
