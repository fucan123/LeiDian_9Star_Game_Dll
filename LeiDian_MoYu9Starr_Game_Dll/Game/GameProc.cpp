#include "Game.h"
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

// ��ʼ������
void GameProc::InitData()
{
	m_bStop = false;
	m_bPause = false;
	m_bReStart = false;
	m_bIsCrazy = false;
	m_pStepCopy = nullptr;
	m_pStepLast = nullptr;
	m_pStepRecord = nullptr;

	m_bIsRecordStep = false;
	m_bClearKaiRui = false;
	m_bIsResetRecordStep = false;
	m_bIsFirstMove = true;
	m_bPlayFB = true;
	m_bAtFB = false;

	m_nReOpenFB = 0;
	m_nReMoveCount = 0;
	m_nYaoBao = 0;
	m_nYao = 0;
	m_nLiveYaoBao = 6;
	m_nLiveYao = 6;

	ZeroMemory(&m_stLast, sizeof(m_stLast));
	ZeroMemory(&m_ClickCrazy, sizeof(m_ClickCrazy));
}

// ��ʼ������
bool GameProc::InitSteps()
{
	m_pGameStep->InitGoLeiMingSteps(); // ��ʼ�����ȥ��������
	for (int i = 0; i < m_pGame->m_pGameConf->m_Setting.FBFileCount; i++) {
		m_pGameStep->InitSteps(m_pGame->m_chPath, m_pGame->m_pGameConf->m_Setting.FBFile[i]);
	}
	return m_pGame->m_pGameConf->m_Setting.FBFileCount > 0;
}

// �л���Ϸ����
void GameProc::SwitchGameWnd(HWND hwnd)
{
	m_hWndGame = hwnd;
}

// �л���Ϸ�ʺ�
void GameProc::SwitchGameAccount(_account_ * account)
{
	m_pAccount = account;
}

// ����ƶ�����Ϸ����λ��
void GameProc::SetGameCursorPos(int x, int y)
{
	RECT rect;
	::GetWindowRect(m_pAccount->Mnq->Wnd, &rect);

	x += 0;// rect.left;
	y += 0;// rect.top;

	int result = SetCursorPos(rect.left + x, rect.top + y);
	//LOGVARN2(64, "blue_r b", L"�������(%d,%d) %d %d", x, y, result, GetLastError());
}

// ������ǰ
void GameProc::SetForegroundWindow(HWND hwnd)
{
	//���붯̬�������������  
	typedef void (WINAPI *PROCSWITCHTOTHISWINDOW)(HWND, BOOL);
	PROCSWITCHTOTHISWINDOW SwitchToThisWindow;
	HMODULE hUser32 = GetModuleHandle(L"user32");
	SwitchToThisWindow = (PROCSWITCHTOTHISWINDOW)
		GetProcAddress(hUser32, "SwitchToThisWindow");

	//������ֻҪ���κ��ִ洰�ڵľ����������������ɣ�
	//�ڶ���������ʾ������ڴ�����С��״̬���Ƿ�ָ���
	SwitchToThisWindow(hwnd, TRUE);
}

// ���ȥ������½����
void GameProc::GoLeiMing()
{
	m_pGameStep->ResetStep(0, 0x02);
	while (ExecStep(m_pGameStep->m_GoLeiMingStep)); // �����
}

// ȥ��ȡ����
void GameProc::GoGetXiangLian()
{
}

// ѯ����������
_account_* GameProc::AskXiangLian()
{
	Account* account = nullptr;
	int max = 0;
	m_pGame->m_pEmulator->List2();
	for (int i = 0; i < m_pGame->m_pEmulator->GetCount(); i++) {
		MNQ* m = m_pGame->m_pEmulator->GetMNQ(i);
		if (!m || !m->Account) // û���໥��
			continue;

		if (!m->Account->IsReadXL) { // û�ж�ȡ����
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

		DbgPrint("%s������%d�� %08X\n", m->Account->Name, m->Account->XL, m->Wnd);
		LOGVARN2(64, "green", L"%hs������%d�� %08X", m->Account->Name, m->Account->XL, m->Wnd);
	}

	return account;
}

// ȥ�����ſ�
void GameProc::GoFBDoor(_account_* account)
{
	if (m_pGame->m_pGameData->IsInFBDoor(account))
		return;
		
	while (true) {
		if (m_bPause) 
			break;

		m_pGame->m_pItem->SwitchQuickBar(2);
		Sleep(500);
		DbgPrint("%sʹ��ȥ�����ſ��ǳ�֮��\n", account->Name);
		LOGVARN2(64, "green", L"%hsʹ��ȥ�����ſ��ǳ�֮��", account->Name);
		m_pGame->m_pItem->GoFBDoor();
		Sleep(1000);
		if (m_pGame->m_pGameData->IsInFBDoor(account)) {
			Sleep(2000);
			m_pGame->m_pItem->SwitchQuickBar(1);
			LOGVARN2(64, "green", L"%hs�ѵ����ſ�", account->Name);
			break;
		}	
	}
}

// ���븱��
_account_* GameProc::OpenFB()
{
	_account_* account = m_pGame->m_pBig;
	if (!account) {
		DbgPrint("û����������, �޷����븱��\n");
		LOG2(L"û����������, �޷����븱��", "red");
		return nullptr;
	}

	wchar_t log[64];
	DbgPrint("%sȥ������[%s], ������������:%d\n", account->Name, account->IsBig?"���":"С��", account->XL);
	LOGVARP2(log, "green", L"%hsȥ������[%hs], ������������:%d\n", account->Name, account->IsBig ? "���" : "С��", account->XL);

	GoFBDoor(account);
	int i;
_start_:
	if (m_bPause)
		return account;

	SwitchGameWnd(account->Mnq->Wnd);
	DbgPrint("%sȥ�����ſ�\n", account->Name);
	LOGVARP2(log, "c0", L"%hsȥ�����ſ�", account->Name);
	m_pGame->m_pMove->RunEnd(863, 500, account); //872, 495 �������������������
	Sleep(1000);

	DbgPrint("%s���������\n", account->Name);
	LOGVARP2(log, "c0", L"%hs���������", account->Name);

	CloseTipBox();
	Click(800, 323); //800, 323 ���������
	Sleep(500);
	//return account;

	if (!m_pGame->m_pTalk->WaitTalkOpen(0x00)) {
		if (m_pGame->m_pTalk->IsNeedCheckNPC()) {
			DbgPrint("ѡ���һ��NPC\n");
			LOG2(L"ѡ���һ��NPC", "c0");
			m_pGame->m_pTalk->SelectNPC(0x00);
		}
		if (!m_pGame->m_pTalk->WaitTalkOpen(0x00)) {
			DbgPrint("NPC�Ի���δ��, �����ٴζԻ�\n");
			LOG2(L"NPC�Ի���δ��, �����ٴζԻ�", "c0");
			goto _start_;
		}
	}
	Sleep(1000);
	m_pGame->m_pTalk->Select(account->IsBig ? 0x00 : 0x01); // �����Կ��, С��������
	Sleep(1000);
	m_pGame->m_pTalk->Select(0x00); // ȷ����������
	Sleep(2000);

	for (i = 0; true; i++) {
		while (m_bPause) Sleep(500);
		if (IsInFB()) {
			DbgPrint("%s�Ѿ����븱��\n", account->Name);
			LOGVARP2(log, "c0", L"%hs�Ѿ����븱��\n", account->Name);
			break;
		}
		if (i == 10)
			goto _start_;

		DbgPrint("%s�ȴ����븱��\n", account->Name);
		LOGVARP2(log, "c0", L"%hs�ȴ����븱��", account->Name);
		Sleep(1000);
	}

	return account;
}

// ������
void GameProc::OutFB(_account_* account)
{
	for (int i = 1; i < 1000; i++) {
		DbgPrint("%s������(%d)\n", account->Name, i);
		LOGVARN2(64, "c0", L"%hs������(%d)\n", account->Name, i);

		m_pGame->m_pMove->RunEnd(890, 1100, account); // �ƶ����̶��ص�
		Sleep(1000);

		Click(190, 503, 195, 505); // ���NPC
		m_pGame->m_pTalk->WaitTalkOpen(0x00);
		Sleep(1000);
		Click(67, 360, 260, 393);  // ѡ��0 ������
		m_pGame->m_pTalk->WaitTalkOpen(0x00);
		Sleep(1000);
		Click(67, 360, 260, 393);  // ѡ��0 ��Ҫ�뿪����
		m_pGame->m_pTalk->WaitTalkOpen(0x00);
		Sleep(1000);

		if (m_pGame->m_pGameData->IsInFBDoor()) // ��ȥ��
			break;
	}
	
}

// ��ȡ���������ʺ�
_account_ * GameProc::GetOpenFBAccount()
{
	return IsBigOpenFB() ? m_pGame->GetBigAccount() : AskXiangLian();
}

// �Ƿ��ɴ�ſ�������
bool GameProc::IsBigOpenFB()
{
	SYSTEMTIME stLocal;
	::GetLocalTime(&stLocal);

	if (stLocal.wHour < 20) // 20�㵽24�������ѽ�
		return false;

	return stLocal.wHour == 23 ? stLocal.wMinute < 59 : true; // ����Ҫ1����ʱ��׼��
}

// ִ��
void GameProc::Exec(_account_* account)
{
}

// ����
void GameProc::Run(_account_* account)
{
	SwitchGameWnd(account->Mnq->Wnd);
	SwitchGameAccount(account);
	DbgPrint("ˢ�����ʺ�:%s(%08X )\n", account->Name, m_hWndGame);
	LOGVARN2(64, "green b", L"ˢ�����ʺ�:%hs(%08X)\n", account->Name, m_hWndGame);
	m_pGame->m_pItem->UseYao();
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

// ȥ�������
void GameProc::GoInTeamPos(_account_* account)
{
	if (!account->Mnq) {
		LOGVARN2(64, "red", L"%sδ��ģ����", account->Name);
		return;
	}

	SwitchGameWnd(account->Mnq->Wnd);
	SwitchGameAccount(account);

	if (m_pGame->m_pGameData->IsInShenDian(account)) // ���뿪���
		GoLeiMing();

	if (!m_pGame->m_pGameData->IsInFBDoor(account)) {
#if 1
		GoFBDoor(account);
#else
		Click(950, 35, 960, 40);     // ��������
		Sleep(2000);
		Click(450, 680, 500, 690);   // ���������ť
		Sleep(1500);
		Click(150, 562, 200, 570);   // ����Ǽ�����
		Sleep(1000);
		Click(150, 510, 200, 520);   // ���������Ŀ���
		Sleep(1000);
		Click(950, 590, 960, 600);   // �����ʼ��ս
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
	if (account->IsBig) { // ��Ŵ�������
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
		//872, 495 ������������
		//425, 235 ���������
	}
	SwitchGameAccount(m_pGame->m_pBig);
}

// ��������
void GameProc::CreateTeam()
{
	DbgPrint("��������...\n");
	LOG2(L"��������...", "c0");
	Click(5, 360, 25, 390);     // ���������
	Sleep(1000);
	Click(80, 345, 200, 360);   // �����������
	Sleep(1500);
	Click(1160, 60, 1166, 66);  // ����رհ�ť
	Sleep(1000);
	Click(5, 463, 10, 466);     // ����������
	DbgPrint("�����������\n");
	LOG2(L"�����������", "c0");
}

// ������� ������� 
void GameProc::ViteInTeam()
{
	DbgPrint("׼���������\n");
	LOG2(L"׼���������", "c0");
	Click(766, 235);           // �������ģ��
	Sleep(1000);
	Click(410, 35, 415, 40);   // �������ͷ��
	Sleep(1000);
	Click(886, 236, 900, 245); // ����������
	DbgPrint("����������\n");
	LOG2(L"����������", "c0");
}

// ���
void GameProc::InTeam(_account_* account)
{
	wchar_t log[64];
	DbgPrint("%s׼��ͬ�����\n", account->Name);
	LOGVARP2(log, "blue b", L"%hs׼��ͬ�����\n", account->Name);
	SwitchGameAccount(account);
	SwitchGameWnd(account->Mnq->Wnd);
	SetForegroundWindow(account->Mnq->WndTop);
	AgreenMsg("�������ͼ��");
	DbgPrint("%s���ͬ�����\n", account->Name);
	LOGVARP2(log, "blue b", L"%hs���ͬ�����", account->Name);
}

void GameProc::InFB(_account_* account)
{
	wchar_t log[64];
	DbgPrint("%sͬ�������\n", account->Name);
	LOGVARP2(log, "blue b", L"%dͬ�������", account->Name);
	for (int i = 1; true; i++) {
		DbgPrint("%d.���ô�����ǰ:%08X\n", i, account->Mnq->WndTop);
		LOGVARP2(log, "c0", L"%d.���ô�����ǰ:%08X", i, account->Mnq->WndTop);
		SetForegroundWindow(account->Mnq->WndTop);
		Sleep(1000);

		if (i & 0x01 == 0x00) {
			RECT rect;
			::GetWindowRect(account->Mnq->WndTop, &rect);
			MoveWindow(account->Mnq->WndTop, 6, 100, rect.right - rect.left, rect.bottom - rect.top, FALSE);
			Sleep(1000);
			::GetWindowRect(account->Mnq->WndTop, &rect);
			mouse_event(MOUSEEVENTF_LEFTDOWN, rect.left + 3, rect.top + 3, 0, 0); //�������
			mouse_event(MOUSEEVENTF_LEFTUP, rect.left + 3, rect.top + 3, 0, 0); //�������
			Sleep(1000);
		}

		HWND top = GetForegroundWindow();
		if (top == account->Mnq->WndTop || top == account->Mnq->Wnd)
			break;
	}
	int max_i = 10;
	for (int i = 1; i <= max_i; i++) {
		if (i > 1) {
			DbgPrint("%s��(%d/%d)�γ���ͬ�������\n", account->Name, i, max_i);
			LOGVARP2(log, "c6", L"%hs��(%d/%d)�γ���ͬ�������", account->Name, i, max_i);
		}

		SwitchGameAccount(account);
		SwitchGameWnd(account->Mnq->Wnd);
		SetForegroundWindow(account->Mnq->WndTop);
		Sleep(500);
		AgreenMsg("������ͼ��");
		Sleep(5000);

		if (m_bPause || IsInFB(account))
			break;
	}
	
	DbgPrint("%s���ͬ�������\n", account->Name);
	LOGVARP2(log, "blue b", L"%hs���ͬ�������", account->Name);

}

// �����˽�����
void GameProc::AllInFB(_account_* account_open)
{
	int count = m_pGame->m_pEmulator->List2();
	for (int i = 0; i < count; i++) {
		MNQ* m = m_pGame->m_pEmulator->GetMNQ(i);
		if (!m || !m->Account) // û���໥��
			continue;
		if (account_open == m->Account)
			continue;

		InFB(m->Account);
	}
}

// ͬ��ϵͳ��Ϣ
void GameProc::AgreenMsg(const char* name, HWND hwnd)
{
	for (int i = 0; i < 3; i++) { // �������濪ʼ��
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
	AgreenMsg(name, 0, true, hwnd); // �Ҳ���ǿ�Ƶ�һ�µ�һ��
}

// ͬ��ϵͳ��Ϣ
int GameProc::AgreenMsg(const char* name, int icon_index, bool click, HWND hwnd)
{
	wchar_t log[64];
	int x = 320, x2 = 350;
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

		DbgPrint("%d.����罻ͼ��(%d)չ����Ϣ\n", i, icon_index);
		LOGVARP2(log, "c6", L"%d.����罻ͼ��(%d)չ����Ϣ", i, icon_index);
		Click(x, y, x2, y2); // ����罻��Ϣͼ�� 580
		if (m_pGame->m_pTalk->WaitForSheJiaoBox()) {
			is_open = true;
			break;
		}

		DbgPrint("��Ϣ��δ��, �ٴδ�\n");
		LOG2(L"��Ϣ��δ��, �ٴδ�", "red");
	}
	if (!is_open)
		return -1;

	Sleep(1000);

	// ��ȡ��Ϣͼ��
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(hwnd ? hwnd : m_hWndGame, 360, 225, 406, 276, 100, true);
	if (!m_pGame->m_pPrintScreen->ComparePixel(name, nullptr, 1)) { // ���������Ϣ
		DbgPrint("��Ϣ�򲻷���\n");
		LOG2(L"��Ϣ�򲻷���", "red");
		if (!click) {
			DbgPrint("�ر�չ������Ϣ��\n");
			LOG2(L"�ر�չ������Ϣ��", "c6");
			m_pGame->m_pTalk->CloseSheJiaoBox(); // �ر�
			return 0;
		}
	}

	DbgPrint("������ܻ�ͬ�ⰴť\n");
	LOG2(L"������ܻ�ͬ�ⰴť", "green");
	Click(835, 475, 850, 490); // ������ܰ�ťͬ��
	return 1;
}

// �Ƿ��ڸ���
bool GameProc::IsInFB(_account_* account)
{
    //return true;
	DWORD x = 0, y = 0;
	m_pGame->m_pGameData->ReadCoor(&x, &y, account);
	if (x >= 882 && x <= 930 && y >= 1055 && y < 1115) // �ս���������
		return true;

	return false;
	// ��ȡ��ͼ���Ƶ�һ����
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_hWndGame, 1090, 5, 1255, 23, 0, true);
	return m_pGame->m_pPrintScreen->CompareImage(CIN_InFB, nullptr, 1) > 0;
}

// ִ�и�������
void GameProc::ExecInFB()
{
	if (!m_pAccount) {
		DbgPrint("��ѡ��ִ�и����ʺ�\n");
		LOG2(L"��ѡ��ִ�и����ʺ�", "red");
		return;
	}
	m_bAtFB = true;

	wchar_t log[128];
	if (1) {
		DbgPrint("ִ�и�������\n");
		LOG2(L"ִ�и�������", "green b");

		InitData();
		m_pGame->SetStatus(m_pGame->m_pBig, ACCSTA_ATFB, true);

		char* step_file = m_pGameStep->SelectRandStep();

		wchar_t file[128];
		wsprintfW(file, L"�ļ�:%hs", step_file);
		m_pGame->UpdateText("step_file", file);
		
		//m_pGameStep->ResetStep();
		//SendMsg("��ʼˢ����");
		int start_time = time(nullptr);
		m_nStartFBTime = start_time;
		m_nUpdateFBTimeLongTime = start_time;

#if 0
		/* ֻ�е���˸�����ͼ, �޸�����ſ����Զ�Ѱ· */
		m_pGame->m_pMove->OpenMap(m_pGame->m_pBig);
		Sleep(100);
		Click(250, 600, 255, 605, 0xff, m_pGame->m_pBig->Mnq->Wnd); // �����ͼ��
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
			// ������븱��
			SetGameCursorPos(750, 536);
			int vx = MyRand(700, 800), vy = MyRand(535, 550);
			m_pGame->m_pEmulator->Tap(vx, vy);
			LOGVARP2(log, "blue_r b", L"������븱��:(%d,%d)", vx, vy);
			m_pGame->m_pServer->InFB(m_pGame->m_pBig, nullptr, 0);
			//while (true) Sleep(1000);
		}
		
		// �л������ܿ����
		m_pGame->m_pItem->SwitchMagicQuickBar();
		Sleep(500);
		// �ر�ϵͳ�罻��Ϣ
		m_pGame->m_pTalk->CloseSheJiaoBox(true);
		// ����������
		Click(5, 463, 10, 466);

		while (1 && ExecStep(m_pGameStep->m_Step, true)); // ���ˢ����
		m_bLockGoFB = false;

		int end_time = time(nullptr);
		int second = end_time - start_time;
		DbgPrint("��ͨ�أ�����ʱ:%02d��%02d��\n", second / 60, second % 60);
		LOGVARP2(log, "green b", L"��ͨ�أ�����ʱ:%02d��%02d��\n", second / 60, second % 60);

		m_pGameStep->ResetStep();
		int game_flag = 0;
		if (0 || m_nReOpenFB == 2) { // ���¿�������
			m_pGame->UpdateReOpenFBCount(++m_nReOpenFBCount);
			game_flag = 1; // ���µ����¼

			m_pGame->m_pServer->SmallOutFB(m_pGame->m_pBig, nullptr, 0);

			m_pGame->m_pTalk->CloseAllBox();
			m_pGame->LogOut(m_pAccount);
			Sleep(1000);
			if (!m_pGame->m_pTalk->WaitForInLoginPic(NULL, 15 * 1000)) {
				DbgPrint("\nδ�ȴ�����¼����\n\n");
				LOG2(L"\nδ�ȴ�����¼����", "red");
				game_flag = 2; // �ؿ���Ϸ
			}
		}
		else {
			if (!m_pAccount->LastX || !m_pAccount->LastY) {
				game_flag = 1; // ���µ����¼
				m_pGame->m_pServer->SmallOutFB(m_pGame->m_pBig, nullptr, 0);
				if (!m_pGame->m_pTalk->WaitForInLoginPic(NULL, 15 * 1000)) {
					DbgPrint("\nδ�ȴ�����¼����\n\n");
					LOG2(L"\nδ�ȴ�����¼����", "red");
					game_flag = 2; // �ؿ���Ϸ
				}
			}
		}

		if (game_flag == 0) { // ��Ϸ��������
			if (m_nReOpenFB == 0) {
				m_pGame->UpdateFBCountText(++m_nPlayFBCount, true);
			}
			if (m_nReOpenFB == 1) { // ��ȥ���¿�ʼ
				OutFB(m_pAccount);
			}
		}
		else { // ��Ϸ���˳�����¼����
			if (game_flag == 2) {
				m_pGame->m_pEmulator->CloseGame(m_pAccount->Index);
				Sleep(1000);
				m_pGame->m_pEmulator->StartGame(m_pAccount->Index);
				Sleep(1000);
				DbgPrint("�ȴ������¼����...\n");
				LOG2(L"�ȴ������¼����...", "c0");
				while (!m_pGame->m_pTalk->IsInLoginPic(NULL)) {
					Sleep(3000);
				}

				m_pAccount->IsLogin = 1;
				m_pAccount->Addr.MoveX = 0;
				m_pAccount->IsGetAddr = 0;
			}

			Sleep(1000);
			DbgPrint("���������Ϸ\n");
			LOG2(L"���������Ϸ", "blue");
			Click(600, 505, 700, 530);
			Sleep(2000);
			DbgPrint("��������¼\n");
			LOG2(L"��������¼", "blue");
			Click(526, 436, 760, 466);
			Sleep(2000);
			DbgPrint("�����¼\n");
			LOG2(L"�����¼", "blue");
			Click(575, 405);
			DbgPrint("�ȴ�������Ϸ...\n");
			LOG2(L"�ȴ�������Ϸ...", "c0");

			int iii = 0, log_click_num = 0;
			do {
				Sleep(1000);
				if (++iii == 3) {
					LOGVARP2(log, "red", L"�����¼(%d)", ++log_click_num);
					Click(575, 405);
					iii = 0;
				}
				if (game_flag == 2) {
					m_pGame->m_pGameData->WatchGame();
				}

				m_pGame->m_pGameData->ReadCoor(&m_pAccount->LastX, &m_pAccount->LastY, m_pAccount);
				Sleep(100);
			} while (!m_pAccount->IsGetAddr || !m_pAccount->Addr.MoveX || m_pAccount->LastX == 0);

			DbgPrint("�ȴ�������Ϸ����...\n");
			LOG2(L"�ȴ�������Ϸ����...", "c0");
			do {
				Sleep(1000);
			} while (!m_pGame->m_pTalk->IsInGamePic());

			Sleep(1000);
			if (IsInFB())
				OutFB(m_pAccount);
		}
		
		m_pGame->UpdateFBTimeLongText(end_time - m_nStartFBTime + m_nFBTimeLong, end_time - m_nUpdateFBTimeLongTime); // ����ʱ��
		m_pGame->InsertFBRecord(m_nStartFBTime, end_time, m_nReOpenFB);
		m_nUpdateFBTimeLongTime = end_time;
		m_nFBTimeLong = end_time - m_nStartFBTime + m_nFBTimeLong;

		m_nReOpenFB = 0; // ���ò��ؿ�����
		//GoFBDoor();
		Sleep(1000);
		while (!m_pGame->m_pGameData->IsInFBDoor(m_pGame->GetBigAccount())) { // �ȴ���������
			Sleep(1000);
		}
		m_pGame->m_pTalk->WaitForInGamePic();
		Sleep(500);

		m_bAtFB = false;

		int wait_s = 60;
		int out_time = time(nullptr);

		// ������
		SellItem();

		//if (!GetOpenFBAccount()) // û���ʺ���
		//	return;
		while (true) {
			int c = (time(nullptr) - out_time);
			if (c > wait_s)
				break;

			DbgPrint("�ȴ�%d���ٴο�������, ��ʣ%d��\n", wait_s, wait_s - c);
			LOGVARP2(log, "c6", L"�ȴ�%d���ٴο�������, ��ʣ%d��", wait_s, wait_s - c);
			Sleep(1000);
		}

		m_pGame->m_pServer->AskXLCount();
		//Wait((wait_s - (time(nullptr) - out_time)) * 1000);
	}
}

// ִ������
bool GameProc::ExecStep(Link<_step_*>& link, bool isfb)
{
	m_pStep = m_pGameStep->Current(link);
	if (!m_pStep) {
		return false;
	}

	if (m_pStep->OpCode == OP_MOVE) {
		_step_* next = m_pGameStep->GetNext(link);
		if (next && next->OpCode == OP_PICKUP && strcmp(next->Name, "��Ч���ư�") == 0) {
			int yaobao = 0, yao = 0;
			m_pGame->m_pItem->GetQuickYaoOrBaoNum(yaobao, yao);
			if (yaobao >= 6) { // �����ټ�ҩ��
				LOGVARN2(32, "blue_r b", L"�����ټ�ҩ��, ����:%d\n", yaobao);
				return m_pGameStep->CompleteExec(link) != nullptr;
			}
		}
	}

	wchar_t log[128];
	_step_* m_pTmpStep = m_pStep; // ��ʱ��

	if (isfb) {
		int now_time = time(nullptr);
		m_pGame->UpdateFBTimeLongText(now_time - m_nStartFBTime + m_nFBTimeLong, now_time - m_nUpdateFBTimeLongTime); // ����ʱ��
		m_nUpdateFBTimeLongTime = now_time;

		if (m_pStepCopy) { // �Ѿ�ִ�е��Ĳ���
			if (m_pStep == m_pStepCopy) {
				m_pStepCopy = nullptr;
			}
			else {
				if (m_pStep->OpCode != OP_MOVE && m_pStep->OpCode != OP_MAGIC) { // �����ƶ���ż���
					return m_pGameStep->CompleteExec(link) != nullptr;
				}
			}
		}

		bool check_box = true;
		if (m_pStep->OpCode == OP_MOVE || m_pStep->OpCode == OP_NPC || m_pStep->OpCode == OP_WAIT
			|| m_pStep->OpCode == OP_PICKUP || m_pStep->OpCode == OP_KAIRUI) {
			CloseTipBox(); // �رյ�����
			CloseSystemViteBox(); // �ر�ϵͳ�����

			if (m_pStep->OpCode != OP_NPC) {
				m_pGame->m_pPet->Revive();
			}
			if (m_pGame->m_pTalk->SpeakIsOpen()) {
				m_pGame->m_pTalk->CloseSpeakBox();
				Sleep(500);
			}
			if (m_pGame->m_pItem->BagIsOpen()) {
				m_pGame->m_pItem->CloseBag();
				check_box = false;
				Sleep(500);
			}
		}

		if (check_box) {
			if (m_pGame->m_pTalk->CommonBoxIsOpen()) {
				m_pGame->m_pTalk->CloseCommonBox();
				Sleep(500);
				
				check_box = false;
			}
		}
	}

	bool bk = false;
	char msg[128];
	switch (m_pStep->OpCode)
	{
	case OP_MOVE:
		DbgPrint("����->�ƶ�:%d.%d��%d.%d\n", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		LOGVARP2(log, "c0 b", L"����->�ƶ�:%d.%d��%d.%d", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		if (m_pStep->X2 && m_pStep->Y2) {
			m_pStep->Extra[0] = MyRand(m_pStep->X, m_pStep->X2);
			m_pStep->Extra[1] = MyRand(m_pStep->Y, m_pStep->Y2);

			DbgPrint("ʵ���ƶ�λ��:%d,%d\n", m_pStep->Extra[0], m_pStep->Extra[1]);
			LOGVARP2(log, "c0", L"ʵ���ƶ�λ��:%d,%d", m_pStep->Extra[0], m_pStep->Extra[1]);
		}
		else {
			m_pStep->Extra[0] = m_pStep->X;
			m_pStep->Extra[1] = m_pStep->Y;
		}
		Move(true);
		break;
	case OP_MOVEFAR:
		DbgPrint("����->����:%d.%d\n", m_pStep->X, m_pStep->Y);
		LOGVARP2(log, "c0 b", L"����->����:%d.%d", m_pStep->X, m_pStep->Y);
		Move();
		break;
	case OP_MOVERAND:
		DbgPrint("����->����ƶ�:%d.%d��%d.%d\n", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		LOGVARP2(log, "c0 b", L"����->����ƶ�:%d.%d��%d.%d", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		if (MyRand(1, 2, MyRand(m_pStep->X, m_pStep->Y)) == 1) {
			if (m_pStep->X2 && m_pStep->Y2) {
				m_pStep->Extra[0] = MyRand(m_pStep->X, m_pStep->X2);
				m_pStep->Extra[1] = MyRand(m_pStep->Y, m_pStep->Y2);

				DbgPrint("ʵ���ƶ�λ��:%d,%d\n", m_pStep->Extra[0], m_pStep->Extra[1]);
				LOGVARP2(log, "c0", L"ʵ���ƶ�λ��:%d,%d", m_pStep->Extra[0], m_pStep->Extra[1]);
				Move(true);
			}
		}
		else {
			DbgPrint("����ƶ�����\n\n");
			LOG2(L"����ƶ�����\n", "c0 b");
			bk = true;
		}
		break;
	case OP_NPC:
		DbgPrint("����->NPC:%s(���:%d,%d �� %d,%d)\n", m_pStep->NPCName, m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		LOGVARP2(log, "c0 b", L"����->NPC:%hs(���:%d,%d �� %d,%d)", m_pStep->NPCName, m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		m_stLast.NPCOpCode = OP_NPC;
		NPC();
		break;
	case OP_SELECT:
		DbgPrint("����->ѡ��:%d\n", m_pStep->SelectNo);
		LOGVARP2(log, "c0 b", L"����->ѡ��:%d", m_pStep->SelectNo);
		Select();
		break;
	case OP_MAGIC:
		DbgPrint("����->����.%s ʹ�ô���:%d\n", m_pStep->Magic, m_pStep->OpCount);
		LOGVARP2(log, "c0 b", L"����->����.%hs ʹ�ô���:%d", m_pStep->Magic, m_pStep->OpCount);
		Magic();
		break;
	case OP_MAGIC_PET:
		DbgPrint("����->���＼��:%s\n", m_pStep->Magic);
		LOGVARP2(log, "c0 b", L"����->���＼��:%hs", m_pStep->Magic);
		//MagicPet();
		break;
	case OP_KAWEI:
		DbgPrint("��λ->ʹ�ü���:%s �ȴ�:%d��\n", m_pStep->Magic, m_pStep->WaitMs / 1000);
		LOGVARP2(log, "c0 b", L"��λ->ʹ�ü���:%hs �ȴ�:%d��", m_pStep->Magic, m_pStep->WaitMs / 1000);
		KaWei();
		break;
	case OP_CRAZY:
		SMSG_D("����->���");
		//Crazy();
		break;
	case OP_CLEAR:
		SMSG_D("����->����");
		//Clear();
		break;
	case OP_KAIRUI:
		DbgPrint("����->���� �ƶ�:(%d,%d) ����:(%d,%d)-(%d,%d)\n", m_pStep->X, m_pStep->Y, 
			m_pStep->Extra[0], m_pStep->Extra[1], m_pStep->Extra[2], m_pStep->Extra[3]);
		LOGVARP2(log, "c0 b", L"����->���� �ƶ�:(%d,%d) ����:(%d,%d)-(%d,%d)", m_pStep->X, m_pStep->Y,
			m_pStep->Extra[0], m_pStep->Extra[1], m_pStep->Extra[2], m_pStep->Extra[3]);
		KaiRui();
		break;
	case OP_PICKUP:
		DbgPrint("����->��ʰ��Ʒ:%s, ����:(%d,%d)-(%d,%d)\n", m_pStep->Name, m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		LOGVARP2(log, "c0 b", L"����->��ʰ��Ʒ:%hs (%d,%d)-(%d,%d)", m_pStep->Name, m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		PickUp();
		break;
	case OP_CHECKIN:
		DbgPrint("����->������Ʒ\n");
		LOG2(L"����->������Ʒ", "c0 b");
		CheckIn();
		break;
	case OP_USEITEM:
		DbgPrint("����->ʹ����Ʒ\n");
		LOG2(L"����->ʹ����Ʒ", "c0 b");
		//UseItem();
		break;
	case OP_DROPITEM:
		DbgPrint("����->������Ʒ\n");
		LOG2(L"����->������Ʒ", "c0 b");
		DropItem();
		break;
	case OP_SELL:
		DbgPrint("����->������Ʒ\n");
		LOG2(L"����->������Ʒ", "c0 b");
		//SellItem();
		break;
	case OP_BUTTON:
		DbgPrint("����->ȷ��\n");
		LOG2(L"����->ȷ��", "c0 b");
		Button();
		break;
	case OP_CLICK:
		m_stLast.NPCOpCode = OP_CLICK;
		m_stLast.ClickX = m_pStep->X;
		m_stLast.ClickY = m_pStep->Y;
		m_stLast.ClickX2 = 0;
		m_stLast.ClickY2 = 0;
		DbgPrint("����->���:%d,%d %d,%d\n", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
		LOGVARP2(log, "c0 b", L"����->���:%d,%d %d,%d", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);
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
		DbgPrint("����->������:%d,%d %d,%d ����:%d\n", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2, m_pStep->OpCount);
		LOGVARP2(log, "c0 b", L"����->������:%d,%d %d,%d ����:%d", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2, m_pStep->OpCount);
		ClickRand();
		break;
	case OP_CLICKCRAZ:
		DbgPrint("����->�������:%d,%d %d,%d ����:%d\n", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2, m_pStep->OpCount);
		LOGVARP2(log, "c0 b", L"����->�������:%d,%d %d,%d ����:%d", m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2, m_pStep->OpCount);
		m_ClickCrazy.X = m_pStep->X;
		m_ClickCrazy.Y = m_pStep->Y;
		m_ClickCrazy.X2 = m_pStep->X2;
		m_ClickCrazy.Y2 = m_pStep->Y2;
		m_ClickCrazy.Count = m_pStep->OpCount;
		bk = true;
		break;
	case OP_WAIT:
		DbgPrint("����->�ȴ�:%d %d\n", m_pStep->WaitMs / 1000, m_pStep->Extra[0]);
		LOGVARP2(log, "c0 b", L"����->�ȴ�:%d %d", m_pStep->WaitMs / 1000, m_pStep->Extra[0]);
		Wait();
		break;
	case OP_SMALL:
		if (m_pStep->SmallV == -1) {
			DbgPrint("����->С��:δ֪����\n");
			LOG2(L"����->С��:δ֪����", "red b");
		}
		else {
			DbgPrint("����->С��:%d %s\n", m_pStep->SmallV, m_pStep->SmallV == 0 ? "������" : "������");
			LOGVARP2(log, "c0 b", L"����->С��:%d %hs", m_pStep->SmallV, m_pStep->SmallV == 0 ? "������" : "������");
			Small();
		}
		break;
	case OP_RECORD:
		DbgPrint("\n--------׼����¼��һ����--------\n\n");
		LOG2(L"\n--------׼����¼��һ����--------\n", "blue_r b");
		m_bIsRecordStep = true;
		bk = true;
		break;
	default:
		SMSG_D("����->δ֪");
		LOG2(L"����->δ֪", "red b");
		break;
	}

	SMSG_D("����->ִ�����");
	if (bk) {
		return m_pGameStep->CompleteExec(link) != nullptr;
	}
	if (m_bIsResetRecordStep) {
		DbgPrint("\n--------���õ��Ѽ�¼����------\n\n");
		LOG2(L"\n--------���õ��Ѽ�¼����--------\n", "blue_r b");
		m_pGameStep->ResetStep(m_pStepRecord->Index, 0x01);
		m_bIsResetRecordStep = false;
		return true;
	}

	bool no_mov_screen = false; // ��ס�Ƿ��ѽ�ͼ��
	int mov_i = 0;
	int drop_i = 0;
	int move_far_i = 0;
	do {
		do {
			if (m_bStop || m_bReStart)
				return false;
			if (m_bPause)
				Sleep(500);
		} while (m_bPause);

		Sleep(100);

		bool use_yao_bao = false;
		if (m_pStep->OpCode == OP_MOVE || m_pStep->OpCode == OP_MOVERAND) {
			mov_i++;
			if (mov_i == 3 && mov_i <= 6) {
				CloseTipBox(); // �رյ�����
			}
			if (mov_i > 0 && (mov_i % 20) == 0) {
				m_pGame->m_pItem->GetQuickYaoOrBaoNum(m_nYaoBao, m_nYao);
				if (m_bIsFirstMove) {
					m_pGame->m_pItem->SwitchMagicQuickBar(); // �л������ܿ����
					m_pGame->m_pTalk->CloseSheJiaoBox(true);
				}
			}
			if (mov_i > 10 && m_nReMoveCount == 0) {
				if ((GetTickCount() & 0xbf) == 0) {
					Click(298, 40, 327, 68);
				}
			}

			if (1 && (mov_i % 5) == 0) {
				// ����·����ҩ
				if (m_nYaoBao > m_nLiveYaoBao) { // ҩ������6
					if (m_nYao < 2) { // ҩС��2
						m_pGame->m_pItem->UseYaoBao(); // ��ҩ��
						m_nYaoBao--;
						m_nYao += 6;
					}
					else {
						m_pGame->m_pItem->UseYao(1, false, 100); // ��ҩ
						m_nYao--;
					}

					use_yao_bao = true;
				}
				else {
					if (m_nYao > m_nLiveYao) { // ҩ����6
						m_pGame->m_pItem->UseYao(1, false, 100);
						m_nYao--;

						use_yao_bao = true;
					}
				}
			}

			// ���������
			if (m_ClickCrazy.Count) {
				ClickCrazy();
			}
		}

		if (m_pStep->OpCode != OP_DROPITEM) {
			SMSG_D("�жϼ�Ѫ");
			int life_flag = IsNeedAddLife();
			if (life_flag == 1) { // ��Ҫ��Ѫ
				SMSG_D("��Ѫ");
				m_pGame->m_pItem->UseYao();
				SMSG_D("��Ѫ->���");
			}
			else if (life_flag == -1) { // ��Ҫ����
				m_pGame->SaveScreen("��Ѫ");

				m_pGame->m_pGameData->ReadCoor(&m_pAccount->LastX, &m_pAccount->LastY, m_pAccount);
				if (!m_pAccount->LastX || !m_pAccount->LastY) {
					DbgPrint("\n~~~~~~~~~~~~~~~~~~��Ϸ�ѵ��߻����쳣����~~~~~~~~~~~~~~~~~~\n\n");
					LOG2(L"\n~~~~~~~~~~~~~~~~~~��Ϸ�ѵ��߻����쳣����~~~~~~~~~~~~~~~~~~\n", "red");
					return false;
				}

				if (!m_bPlayFB) {
					m_pGame->m_pServer->SmallOutFB(m_pGame->m_pBig, nullptr, 0);
				}

				ReBorn(); // ����
				m_pStepCopy = m_pStep; // ���浱ǰִ�и���
				m_pGameStep->ResetStep(0); // ���õ���һ��
				DbgPrint("���õ���һ��\n");
				LOG2(L"���õ���һ��", "red");
				if (!m_bPlayFB) {
					LOG2(L"���¿�������", "red");
					m_nReOpenFB = 1;
				}
				return m_bPlayFB;
			}
			SMSG_D("�жϼ�Ѫ->���");
		}

		//SMSG_D("�ж�����");
		bool complete = StepIsComplete();
		//SMSG_D("�ж�����->���");
		if (m_pStep->OpCode == OP_MOVEFAR) {
			if (++move_far_i == 300) {
				DbgPrint("���ͳ�ʱ\n");
				LOG2(L"���ͳ�ʱ", "red");
				complete = true;
			}
		}

		if (m_nReMoveCount > 20) {
			DbgPrint("--------�����ѿ�ס--------\n");
			LOG2(L"--------�����ѿ�ס--------", "red");
			if (!no_mov_screen) {
				if (m_pGame->m_pMove->IsOpenMap()) {
					m_pGame->m_pMove->CloseMap();
					Sleep(800);
				}
				
				m_pGame->SaveScreen("��ס");
				no_mov_screen = true;
			}

			if (m_pGame->m_Setting.FBTimeOut > 0) {
				int use_second = time(nullptr) - m_nStartFBTime;
				if (use_second >= m_pGame->m_Setting.FBTimeOut) {
					DbgPrint("\n����ʱ��ﵽ%d��, ����%d��, �ؿ�����\n\n", use_second, m_pGame->m_Setting.FBTimeOut);
					LOGVARP2(log, "red b", L"\n����ʱ��ﵽ%d��, ����%d��, �ؿ�����\n", use_second, m_pGame->m_Setting.FBTimeOut);
					m_nReOpenFB = 2;
					return false;
				}
			}

			if (m_nReMoveCount < 25 && (m_stLast.OpCode == OP_NPC || m_stLast.OpCode == OP_SELECT)) {
				DbgPrint("--------�����ٴζԻ�NPC--------\n");
				LOG2(L"--------�����ٴζԻ�NPC--------", "blue");
				NPCLast(1);
				m_pGame->m_pTalk->WaitTalkOpen(0x00);
				Sleep(1000);
				m_pGame->m_pTalk->Select(0x00);
				Sleep(500);
				DbgPrint("--------���ԶԻ�NPC��ɣ���Ҫ�ƶ���:(%d,%d)-------\n", m_pStep->X, m_pStep->Y);
				LOGVARP2(log, "blue", L"--------���ԶԻ�NPC��ɣ���Ҫ�ƶ���:(%d,%d)-------", m_pStep->X, m_pStep->Y);
				m_nReMoveCount++;
			}
			DbgPrint("--------%d�κ����õ���¼����--------\n", 25 - m_nReMoveCount);
			LOGVARP2(log, "red", L"--------%d�κ����õ���¼����--------", 25 - m_nReMoveCount);
			if (m_nReMoveCount >= 25 && m_pStepRecord) {
				DbgPrint("--------���õ��Ѽ�¼����------\n");
				LOG2(L"\n--------���õ��Ѽ�¼����--------\n", "blue_r b");
				m_pGameStep->ResetStep(m_pStepRecord->Index, 0x01);
				m_nReMoveCount = 0;
				return true;
			}
		}

		if (complete) { // ����ɴ˲���
			DbgPrint("����->����ɴ˲���\n\n");
			LOG2(L"����->����ɴ˲���\n", "c0 b");
			if (m_bIsRecordStep) { // ��¼�˲���
				m_pStepRecord = m_pStep;
				m_bIsRecordStep = false;
				DbgPrint("--------�Ѽ�¼�˲���--------\n\n");
				LOG2(L"--------�Ѽ�¼�˲���--------\n", "blue_r b");
			}

			m_stLast.OpCode = m_pStep->OpCode;
			m_pStepLast = m_pStep;
			
			if (m_pStep->OpCode != OP_NPC && m_pStep->OpCode != OP_SELECT && m_pStep->OpCode != OP_WAIT) {
				//if (m_pGame->m_pTalk->NPCTalkStatus()) // �Ի����Ǵ򿪵�
				//	m_pGame->m_pTalk->NPCTalk(0xff);   // �ص���
			}

			_step_* next = m_pGameStep->CompleteExec(link);
			if (m_pStep->OpCode != OP_WAIT && next) {
				Sleep(use_yao_bao ? 100 : 50);
			}
			return next != nullptr;
		}
	} while (true);

	return false;
}

// �����Ƿ���ִ�����
bool GameProc::StepIsComplete()
{
	bool result = false;
	switch (m_pStep->OpCode)
	{
	case OP_MOVE:
	case OP_MOVERAND:
		//result = true;
		SMSG_D("�ж��Ƿ��ƶ����յ�&�Ƿ��ƶ�", true);
		if (m_pGame->m_pMove->IsMoveEnd(m_pAccount)) { // �ѵ�ָ��λ��
			m_stLast.MvX = m_pStep->Extra[0];
			m_stLast.MvY = m_pStep->Extra[1];
			m_nReMoveCount = 0;
			result = true;

			if (m_bIsFirstMove) {
				// �л������ܿ����
				m_pGame->m_pItem->SwitchMagicQuickBar();
				m_bIsFirstMove = false;
			}
			goto end;
		}
		SMSG_D("�ж��Ƿ��ƶ����յ�->���", true);
		if (!m_pGame->m_pMove->IsMove(m_pAccount)) {   // �Ѿ�ֹͣ�ƶ�
			Move(false);
			m_nReMoveCount++;
		}
		SMSG_D("�ж��Ƿ��ƶ�->���", true);
		break;
	case OP_MOVEFAR:
		if (m_pGame->m_pGameData->IsNotInArea(m_pStep->X, m_pStep->Y, 50, m_pAccount)) { // �Ѳ��ڴ�����
			result = true;
			m_nReMoveCount = 0;
			goto end;
		}
		if (m_pGame->m_pMove->IsMoveEnd(m_pAccount)) { // �ѵ�ָ��λ��
			m_pGame->m_pMove->Run(m_pStep->X - 5, m_pStep->Y, m_pAccount, m_pStep->X2, m_pStep->Y2);
			m_nReMoveCount = 0;
		}
		if (!m_pGame->m_pMove->IsMove(m_pAccount)) {   // �Ѿ�ֹͣ�ƶ�
			m_pGame->m_pMove->Run(m_pStep->X, m_pStep->Y, m_pAccount, m_pStep->X2, m_pStep->Y2);
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

// �ƶ�
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

// �Ի�
void GameProc::NPC()
{
	SetGameCursorPos(325, 62);

	if (strcmp("��Ѫ��ʿ", m_pStep->NPCName) == 0) {
		m_pGame->m_pItem->SwitchMagicQuickBar();
	}
	else if (strcmp("Ů������̳", m_pStep->NPCName) == 0) {
		m_bPlayFB = false;
	}
	else if (strcmp("��ħ������̳", m_pStep->NPCName) == 0) {
		m_nLiveYaoBao = 2;
		m_nLiveYao = 1;
	}
	else if (strcmp("������", m_pStep->NPCName) == 0) {
		MouseWheel(-240);
		Sleep(1000);
	}

	NPC(m_pStep->NPCName, m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2);

	strcpy(m_stLast.NPCName, m_pStep->NPCName);
	m_stLast.ClickX = m_pStep->X;
	m_stLast.ClickY = m_pStep->Y;
	m_stLast.ClickX2 = m_pStep->X2;
	m_stLast.ClickY2 = m_pStep->Y2;
}

// NPC���
void GameProc::NPC(const char* name, int x, int y, int x2, int y2)
{
	CloseItemUseTipBox();

	bool click_btn = true;
	if (name && strstr(name, "֮��"))
		click_btn = false;

	int i = 0;
_click:
	if (click_btn && m_pGame->m_pTalk->TalkBtnIsOpen()) {
		DbgPrint("����Ի���ť\n");
		LOG2(L"����Ի���ť", "c0");
		m_pGame->m_pTalk->NPC();
		Sleep(100);
	}
	else {
		if (!x || !y) { // û��д����
			int count = 0;
			while (!m_pGame->m_pTalk->TalkBtnIsOpen()) {
				if (++count == 3)
					break;
				Sleep(500);
			}
			m_pGame->m_pTalk->NPC();
		}

		if (x2) {
			x = MyRand(x, x2);
			//DbgPrint("x2:%d\n", x2);
		}
			
		if (y2) {
			y = MyRand(y, y2);
			//DbgPrint("y2:%d\n", y2);
		}
			
		Click(x, y);
		DbgPrint("���NPC:%d,%d\n", x, y);
		LOGVARN2(32, "c0", L"���:%d,%d", x, y);
	}
	if (strstr(name, "��ӡ����")) { // �����ص����˷�
		if (++i == 3)
			return;

		for (int ms = 0; ms < 800; ms += 100) {
			if (m_pGame->m_pTalk->SureBtnIsOpen())
				return;
			Sleep(100);
		}

		LOGVARN2(32, "red b", L"��ӡ����δ�⿪, �ٴε���⿪");
		goto _click;
	}
}

// ���һ���Ի���NPC
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
			DWORD pos_x = 0, pos_y = 0;
			m_pGame->m_pGameData->ReadCoor(&pos_x, &pos_y, m_pAccount);
			if (pos_x != m_stLast.MvX || pos_y != m_stLast.MvY) {
				while (m_bPause) Sleep(500);

				DbgPrint("�����ƶ���:(%d,%d) ����λ��:(%d,%d)\n", m_stLast.MvX, m_stLast.MvY, pos_x, pos_y);
				LOGVARP2(log, "blue", L"�����ƶ���:(%d,%d) ����λ��:(%d,%d)", m_stLast.MvX, m_stLast.MvY, pos_x, pos_y);
				if (mov_sleep_ms) {
					Sleep(mov_sleep_ms);
				}
				m_pGame->m_pMove->RunEnd(m_stLast.MvX, m_stLast.MvY, m_pAccount);
				Wait(1 * 1000);
				is_move = true;
			}
		}
		
	}

	if (m_stLast.NPCOpCode == OP_NPC) {
		DbgPrint("�ٴε���Ի���NPC(%s)\n", m_stLast.NPCName);
		LOGVARP2(log, "blue", L"�ٴε���Ի���NPC(%hs)", m_stLast.NPCName);
		NPC(m_stLast.NPCName, m_stLast.ClickX, m_stLast.ClickY, m_stLast.ClickX2, m_stLast.ClickY2);
	}
	else if (m_stLast.NPCOpCode == OP_CLICK) {
		if (0 && m_stLast.ClickX2 && m_stLast.ClickY2) {
			Click(m_stLast.ClickX, m_stLast.ClickY);
			DbgPrint("�ٴε��(%d,%d)-(%d,%d)��NPC(%s)\n", m_stLast.ClickX, m_stLast.ClickY, m_stLast.ClickX2, m_stLast.ClickY2, m_stLast.NPCName);
			LOGVARP2(log, "blue", L"�ٴε��(%d,%d)-(%d,%d)��NPC(%hs)", m_stLast.ClickX, m_stLast.ClickY, m_stLast.ClickX2, m_stLast.ClickY2, m_stLast.NPCName);
		}
		else {
			Click(m_stLast.ClickX, m_stLast.ClickY);
			DbgPrint("�ٴε��(%d,%d)��NPC(%s)\n", m_stLast.ClickX, m_stLast.ClickY, m_stLast.NPCName);
			LOGVARP2(log, "blue", L"�ٴε��(%d,%d)��NPC(%hs)", m_stLast.ClickX, m_stLast.ClickY, m_stLast.NPCName);
		}
	}

	return is_move;
}

// ѡ��
void GameProc::Select()
{
	wchar_t log[64];
	if (strcmp("������", m_stLast.NPCName) == 0) {
		int n = 0;
		while (true) {
			if (m_pGame->m_pTalk->WaitTalkOpen(m_pStep->SelectNo)) {
				Sleep(500);
				m_pGame->m_pTalk->Select(m_pStep->SelectNo);
				Sleep(1500);
				m_pGame->m_pTalk->Select(m_pStep->SelectNo);
				Sleep(1500);
			}
			
			if (m_pGame->m_pGameData->IsInFBDoor(m_pAccount))
				break;

			if (n++ >= 10) {
				m_bIsResetRecordStep = true;
				break;
			}

			NPCLast(true, 1000);
		}
		return;
	}

	int add_count = m_pStep->OpCount == 1 ? 0 : 0;
	for (DWORD i = 1; i <= m_pStep->OpCount; i++) {
		int max_j = 2;
		bool open = false;
		for (int j = 0; j < max_j; j++) {
			while (m_bPause) Sleep(500);

			if (m_pGame->m_pTalk->WaitTalkOpen(m_pStep->SelectNo)) {
				open = true;
				break;
			}
			else {
				if (i > 1)
				    CloseItemUseTipBox();

				bool check_pos = i < 6 && strcmp("������", m_stLast.NPCName) != 0;
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

			DbgPrint("�ȴ��Ի���򿪳���%d��,����. ���:%d\n", max_j, _clk);
			LOGVARP2(log, "c6", L"�ȴ��Ի���򿪳���%d��,����. ���:%d", max_j, _clk);
			goto _check_;
		}

		Sleep(MyRand(300, 350, i));

		DbgPrint("��(%d)��ѡ�� ѡ��:%d\n", i, m_pStep->SelectNo);
		LOGVARP2(log, "c6", L"��(%d)��ѡ�� ѡ��:%d", i, m_pStep->SelectNo);
		m_pGame->m_pTalk->Select(m_pStep->SelectNo);
		if (!m_pGame->m_pTalk->WaitTalkClose(m_pStep->SelectNo)) {
			//m_pGame->m_pTalk->Select(m_pStep->SelectNo);
			//m_pGame->m_pTalk->WaitTalkClose(m_pStep->SelectNo);
		}

		if (i > 1) {
			Sleep(150);
			if (IsNeedAddLife() == 1) {
				Sleep(300);
				m_pGame->m_pItem->UseYao(3);
				NPCLast();
				i--;
				DbgPrint("i--:%d\n", i);
			}
		}
	}

_check_:
	if (m_pStep->OpCount >= 10 && m_pStep->Extra[0]) {
		DbgPrint("����Ѫ\n");
		LOG2(L"����Ѫ", "green");
		m_pGame->m_pItem->AddFullLife();
	}
	if (m_pStep->OpCount == 1 || m_pStep->OpCount >= 10) {
		if (strcmp(m_stLast.NPCName, "����ʿ��̳") == 0 || strcmp(m_stLast.NPCName, "Ů������̳") == 0) {
			Sleep(500);
			if (CloseTipBox()) {
				DbgPrint("--------�з�ӡδ�⿪����Ҫ���õ���¼����--------\n");
				LOG2(L"--------�з�ӡδ�⿪����Ҫ���õ���¼����--------", "red");
				m_bIsResetRecordStep = true;
				return;
			}
			else {
				DbgPrint("--------��ӡ��ȫ���⿪--------\n");
				LOG2(L"--------��ӡ��ȫ���⿪--------", "c0");
			}
		}

		if (IsCheckNPC(m_stLast.NPCName)) {
			DbgPrint("���������NPC:%s������\n", m_stLast.NPCName);
			LOGVARP2(log, "c3 b", L"���������NPC:%hs������", m_stLast.NPCName);
			int no = m_pStep->SelectNo;
			int i_max = no >= 10 ? 5 : 3;
			for (int i = 1; i <= i_max; i++) {
				if (m_pStep->SelectNo == 0x02)
					break;

				Sleep(i & 0x01 ? 300 : 300);
				if (!m_pGame->m_pTalk->TalkBtnIsOpen())
					break;

				if (i == 1)
					continue;

				DbgPrint("%d.NPCδ�Ի����, ���µ���Ի���ť\n", i);
				LOGVARP2(log, "red", L"%d.NPCδ�Ի����, ���µ���Ի���ť", i);
				NPCLast(true);
				if (m_pGame->m_pTalk->WaitTalkOpen(m_pStep->SelectNo)) {
					Sleep(MyRand(300, 350, i));
					if (i >= 3 && m_pStep->OpCount >= 10) {
						no = 0x00;
						i--;
					}

					DbgPrint("%d.ѡ��:%d\n", i, no);
					LOGVARP2(log, "c6", L"%d.ѡ��:%d", i, no);
					m_pGame->m_pTalk->Select(no);
					m_pGame->m_pTalk->WaitTalkClose(m_pStep->SelectNo);

					if (strcmp(m_stLast.NPCName, "����ʿ��̳") == 0 || strcmp(m_stLast.NPCName, "Ů������̳") == 0) {
						Sleep(500);
						if (CloseTipBox()) {
							DbgPrint("--------�з�ӡδ�⿪����Ҫ���õ���¼����--------\n");
							LOG2(L"--------�з�ӡδ�⿪����Ҫ���õ���¼����--------", "red");
							m_bIsResetRecordStep = true;
							return;
						}
						else {
							DbgPrint("--------��ӡ��ȫ���⿪--------\n");
							LOG2(L"--------��ӡ��ȫ���⿪--------", "c0");
						}
					}
				}
			}
		}
		else if (IsCheckNPCTipBox(m_stLast.NPCName)) {
			DbgPrint("���������NPC����:%s������\n", m_stLast.NPCName);
			LOGVARP2(log, "c3 b", L"���������NPC����:%hs������", m_stLast.NPCName);

			bool result = false;
			for (int i = 0; i < 3; i++) {
				for (int ms = 0; ms < 1500; ms += 100) {
					if (m_pGame->m_pTalk->SureBtnIsOpen()) {
						result = true;
						break;
					}
					Sleep(100);
				}
				if (result)
					break;

				DbgPrint("%d.δ���ֵ���, ���µ���Ի���ť\n", i);
				LOGVARP2(log, "red", L"%d.δ���ֵ���, ���µ���Ի���ť", i);
				NPCLast(true);
				if (m_pGame->m_pTalk->WaitTalkOpen(m_pStep->SelectNo)) {
					Sleep(MyRand(300, 350, i));
					m_pGame->m_pTalk->Select(m_pStep->SelectNo);
					m_pGame->m_pTalk->WaitTalkClose(m_pStep->SelectNo);
				}
			}
			if (!result && strcmp("�������Թ��", m_stLast.NPCName) == 0) { // ���BOSSδ���ӡ
				DbgPrint("--------�з�ӡδ�⿪����Ҫ���õ���¼����--------\n");
				LOG2(L"--------�з�ӡδ�⿪����Ҫ���õ���¼����--------", "red");
				m_bIsResetRecordStep = true;
				return;
			}
		}
	}
}

// ����
void GameProc::Magic()
{
	if (strcmp("����þ�", m_pStep->Magic) == 0) {
		m_pGame->m_pItem->SwitchMagicQuickBar(); // �л������ܿ����
	}

	for (int i = 1; i <= m_pStep->OpCount; i++) {
		int result = m_pGame->m_pMagic->UseMagic(m_pStep->Magic, m_pStep->X, m_pStep->Y);
		if (m_pStep->OpCount == 1) {
			Sleep(result == -1 ? 500 : 10);
		}
		else {
			Sleep(500);
		}
	}
}

// ��λ
void GameProc::KaWei()
{
	wchar_t log[64];
	int click_x = 0, click_y = 0;
	m_pGame->m_pMagic->GetMagicClickPos(m_pStep->Magic, click_x, click_y);
	if (click_x && click_y) {
		int mv_x = m_pStep->X, mv_y = m_pStep->Y;
		DbgPrint("ʹ�ü���:%s(%d,%d) ����(%d,%d)\n", m_pStep->Name, click_x, click_y, mv_x, mv_y);
		LOGVARP2(log, "c6", L"ʹ�ü���:%hs(%d,%d) ����(%d,%d)", m_pStep->Name, click_x, click_y, mv_x, mv_y);
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
				DbgPrint("��λ�ȴ�%02d�룬��ʣ%02d��.\n", m_pStep->WaitMs / 1000, ls);
				LOGVARP2(log, "c6", L"��λ�ȴ�%02d�룬��ʣ%02d��.", m_pStep->WaitMs / 1000, ls);
			}
			n = ls;

			m_pGame->m_pGameProc->MouseMove(click_x + mv_x, click_y + mv_y);
			Sleep(500);
		}
		m_pGame->m_pGameProc->Click(click_x + mv_x, click_y + mv_y, 0x02);
	}
	else {
		DbgPrint("KaWei��%d,%d\n", click_x, click_y);
	}
}

// ����
void GameProc::KaiRui()
{
	if (m_bClearKaiRui) {
		DbgPrint("========�����ѱ����������========\n");
		LOG2(L"========�����ѱ����������========", "c9");
		return;
	}
	// �ƶ���ָ��λ��
	m_pGame->m_pMove->RunEnd(m_pStep->X, m_pStep->Y, m_pAccount);
	CloseTipBox();
	Sleep(100);
	// ��ȡ����ȷ����ťͼƬ
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_hWndGame, 
		m_pStep->Extra[0], m_pStep->Extra[1], m_pStep->Extra[2], m_pStep->Extra[3], 0, true);

	int red_count = m_pGame->m_pPrintScreen->GetPixelCount(0xffff0000, 0x00101010);
	int yellow_count = m_pGame->m_pPrintScreen->GetPixelCount(0xffffff00, 0x00020202);
	DbgPrint("��ɫ: ��ɫ(%d), ��ɫ(%d)\n", red_count, yellow_count);
	LOGVARN2(64, "c0", L"��ɫ: ��ɫ(%d), ��ɫ(%d)\n", red_count, yellow_count);

	bool pick_btn = true;
	if ((red_count > 10 && yellow_count > 50) || yellow_count > 200) {
		DbgPrint("========���ֿ���, ׼���ɵ���========\n");
		LOG2(L"========���ֿ���, ׼���ɵ���========", "c0");
		int max = m_pStep->Extra[4] ? 1 : 5;
		for (int i = 1; i <= max; i++) {
			DbgPrint("========ʹ�ü���->����þ�========\n");
			LOG2(L"========ʹ�ü���->����þ�========", "c0");
			m_pGame->m_pMagic->UseMagic("����þ�");
			if (IsNeedAddLife() == -1)
				m_pGame->m_pItem->UseYao(2);

			Sleep(500);

			if (i & 0x01) {
				int pickup_count = m_pGame->m_pItem->PickUpItem("��Чʥ����ҩ", m_pStep->Extra[0], m_pStep->Extra[1], m_pStep->Extra[2], m_pStep->Extra[3], 2);
				if (pickup_count > 0) {
					if (pickup_count == 1) { // ����������, ��ɨ��һ��
						m_pGame->m_pItem->PickUpItem("��Чʥ����ҩ", 450, 300, 780, 475, 2);
					}
					pick_btn = false;
					break;
				}
			}
		}
		m_pGame->m_pMagic->UseMagic("����þ�");
		m_bClearKaiRui = true;
	}
	else if (m_pStep->Extra[4] != 0) { // ɨ��ʥ����Ʒ
		if (m_pStep->Extra[4] == 2) {
			m_pGame->m_pMagic->UseMagic("����þ�");
			Sleep(800);
			pick_btn = false;
		}

		int pickup_count = m_pGame->m_pItem->PickUpItem("��Чʥ����ҩ", m_pStep->Extra[0], m_pStep->Extra[1], m_pStep->Extra[2], m_pStep->Extra[3], 2);
		if (pickup_count > 0) {
			DbgPrint("========���ֿ���, ���ѱ����========\n");
			LOG2(L"========�����ѱ����������========", "c9");
			pick_btn = false;
			m_bClearKaiRui = true;
		}
	}
	if (pick_btn)
		m_pGame->m_pItem->PickUpItemByBtn();
}

// ����
void GameProc::PickUp()
{
	if (CloseTipBox())
		Sleep(500);

	if (strcmp(m_pStep->Name, "ȫ��") == 0) {
		m_pGame->m_pItem->PickUpItemByBtn();
		CloseItemUseTipBox();
		return;
	}

	bool to_big = false; // �Ƿ�Ŵ���Ļ
	if (strstr(m_pStep->Name, "."))
		to_big = true;

	if (to_big) {
		MouseWheel(240);
		Sleep(1000);
		CloseTipBox();
		m_pGame->m_pTalk->CloseAllBox();
	}

	int pickup_max_num = to_big ? 6 : 6;
	if (strcmp(m_pStep->Name, "��Ч���ư�") == 0)
		pickup_max_num = 5;
	if (strcmp(m_pStep->Name, ".") == 0)
		pickup_max_num = 10;

	int pickup_count = m_pGame->m_pItem->PickUpItem(m_pStep->Name, m_pStep->X, m_pStep->Y, m_pStep->X2, m_pStep->Y2, pickup_max_num);

#if 0
	if (strcmp(m_pStep->Name, "30��������Ƭ+3") == 0) {
		if (pickup_count > 5)
			pickup_count = 5;

		int wait_s = 15 - (3 * pickup_count);
		Wait(wait_s * 1000);
	}
#endif

	if (to_big) {
		CloseTipBox();
		m_pGame->m_pTalk->CloseAllBox();
		if (pickup_count == 0)
			Sleep(1000);

		MouseWheel(-240);
		Sleep(1000);
		MouseWheel(-240);
		Sleep(500);
	}

	CloseItemUseTipBox();
}

// ����
DWORD GameProc::CheckIn(bool in)
{
	CloseTipBox(); // �رյ�����
	m_pGame->m_pItem->CheckIn(m_pGame->m_pGameConf->m_stCheckIn.CheckIns, m_pGame->m_pGameConf->m_stCheckIn.Length);
	return 0;
}

// ʹ����
void GameProc::UseItem()
{
}

// ����
void GameProc::DropItem()
{
	SetGameCursorPos(700, 88);
	CloseTipBox(); // �رյ�����
	m_pGame->m_pItem->DropItem(CIN_YaoBao);
}

// ������Ʒ
void GameProc::SellItem()
{
	SetGameCursorPos(700, 89);
	wchar_t log[64];
	DbgPrint("\n-----------------------------\n");
	LOG2(L"\n-----------------------------", "c0 b");
	DbgPrint("׼��ȥ������\n");
	LOG2(L"׼��ȥ������", "green b");
	int pos_x = 265, pos_y = 370;
	DWORD _tm = GetTickCount();
	if (!m_pGame->m_pGameData->IsInArea(pos_x, pos_y, 15)) { // �����̵�����
		int i = 0;
	use_pos_item:
		if (i > 0 && (i % 10) == 0 && i <= 100) {
			m_pGame->SaveScreen("ȥ�̵꿨ס");
		}
		m_pGame->m_pTalk->CloseAllBox(); // �ر������ڵ�
		if (!m_pGame->m_pItem->QuickBarIsXingChen()) { // �л�
			DbgPrint("(%d)�л������\n", i + 1);
			LOGVARP2(log, "blue", L"(%d)�л������", i + 1);
			m_pGame->m_pItem->SwitchQuickBar(2); // �л������
			Sleep(500);
		}
		
		DbgPrint("(%d)ʹ���ǳ�֮��\n", i + 1);
		LOGVARP2(log, "blue", L"(%d)ʹ���ǳ�֮��", i + 1);
		m_pGame->m_pItem->GoShop();         // ȥ�̵��Ա�
		Sleep(500);
		for (; i < 100;) {
			if (i > 0 && (i % 10) == 0) {
				i++;
				goto use_pos_item;
			}
			if (m_pGame->m_pGameData->IsInArea(pos_x, pos_y, 15)) { // �����̵��Ա�
				Sleep(100);
				break;
			}

			i++;
			Sleep(1000);
		}
		m_pGame->m_pItem->SwitchQuickBar(1); // �лؿ����
		Sleep(300);
	}

	m_pGame->m_pMove->RunEnd(pos_x, pos_y, m_pGame->m_pBig); // �ƶ����̶���õ��
	//Sleep(500);
	//MouseWheel(-260);
	//m_pGame->m_pMove->RunEnd(pos_x, pos_y, m_pGame->m_pBig); // �ƶ����̶���õ��
	Sleep(1000);

	int rand_v = GetTickCount() % 2;
	int clk_x, clk_y, clk_x2, clk_y2;
	if (rand_v == 0) { // װ����
		clk_x = 563, clk_y = 545;
		clk_x2 = 565, clk_y2 = 546;
	}
	else { // ������
		clk_x = 306, clk_y = 450;
		clk_x2 = 335, clk_y2 = 500;
	}

	m_pGame->m_pEmulator->Tap(MyRand(clk_x, clk_x2), MyRand(clk_y, clk_y2));
	//Click(clk_x, clk_y, clk_x2, clk_y2);      // �Ի��̵�����
	m_pGame->m_pTalk->WaitTalkOpen(0x00);
	Sleep(1000);
	m_pGame->m_pTalk->Select(0x00); // ������Ʒ
	Sleep(1500);

	m_pGame->m_pItem->SellItem(m_pGame->m_pGameConf->m_stSell.Sells, m_pGame->m_pGameConf->m_stSell.Length);
	Sleep(500);
	m_pGame->m_pItem->SetBag();
	Sleep(500);
	m_pGame->m_pItem->CloseShop();
	Sleep(500);
	if (m_pGame->m_pItem->CheckOut(m_pGame->m_pGameConf->m_stSell.Sells, m_pGame->m_pGameConf->m_stSell.Length)) {
		Sleep(500);
		m_pGame->m_pMove->RunEnd(pos_x, pos_y, m_pGame->m_pBig); // �ƶ����̶���õ��
		Sleep(500);
		m_pGame->m_pEmulator->Tap(MyRand(clk_x, clk_x2), MyRand(clk_y, clk_y2));
		//Click(clk_x, clk_y, clk_x2, clk_y2);      // �Ի��̵�����
		Sleep(1000);
		m_pGame->m_pTalk->Select(0x00); // ������Ʒ
		m_pGame->m_pTalk->WaitTalkOpen(0x00);
		Sleep(1000);
		m_pGame->m_pItem->SellItem(m_pGame->m_pGameConf->m_stSell.Sells, m_pGame->m_pGameConf->m_stSell.Length);
		Sleep(500);
#if 1
		DbgPrint("����������װ��\n");
		LOG2(L"����������װ��", "c0");
		Click(8, 260, 35, 358); // �������װ��
		Sleep(1000);
		DbgPrint("���ȫ������\n");
		LOG2(L"���ȫ������", "c0");
		Click(390, 645, 500, 666); // ��ťȫ������
		if (m_pGame->m_pTalk->WaitForConfirmBtnOpen(1500)) {
			Sleep(150);
			DbgPrint("ȷ������װ��\n");
			LOG2(L"ȷ������װ��", "c0");
			m_pGame->m_pTalk->ClickConfirmBtn(1); // ȷ��
			Sleep(500);
			Click(16, 150, 20, 186); // չ�������б�
			Sleep(350);
		}
		else {
			DbgPrint("��������װ��\n");
			LOG2(L"��������װ��", "c0");
		}
#endif
		m_pGame->m_pItem->CloseShop();
		Sleep(500);
	}

	m_pGame->m_pTalk->CloseAllBox();

	_tm = GetTickCount() - _tm;
	DbgPrint("��������ʱ%.2f��, %d����\n", (float)_tm / 1000.0f, _tm);
	LOGVARP2(log, "green b", L"��������ʱ%d��, %d����", _tm / 1000, _tm);
	DbgPrint("\n-----------------------------\n");
	LOG2(L"\n-----------------------------", "c0 b");
}

// ��ť
void GameProc::Button()
{
	// ȷ����ť 597,445 606,450
	CloseTipBox();
}

// �رյ���
bool GameProc::CloseTipBox()
{
	// ��ȡ����ȷ����ťͼƬ
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_hWndGame, 585, 450, 600, 465, 0, true);
	if (m_pGame->m_pPrintScreen->ComparePixel("��ʾ��", nullptr, 1)) {
		DbgPrint("�ر���ʾ��\n");
		LOG2(L"�ر���ʾ��", "orange b");
		Click(576, 440, 695, 468);
		Sleep(100);

		return true;
	}

	return false;
}

// �ر���Ʒʹ����ʾ��
bool GameProc::CloseItemUseTipBox()
{
	// ��ȡ��Ʒʹ����ʾ��ťǰ��ͼƬ
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_hWndGame, 855, 376, 865, 386, 0, true);
	if (m_pGame->m_pPrintScreen->ComparePixel("��Ʒʹ����ʾ��", nullptr, 1)) {
		DbgPrint("�ر���Ʒʹ����ʾ��\n");
		LOG2(L"�ر���Ʒʹ����ʾ��", "orange b");
		Click(1032, 180);
		Sleep(300);

		return true;
	}

	return false;
}

// �ر�ϵͳ������ʾ��
bool GameProc::CloseSystemViteBox()
{
	// ��ȡ��������������ɫ
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_hWndGame, 765, 187, 775, 197, 0, true);
	if (m_pGame->m_pPrintScreen->ComparePixel("����ȷ��", nullptr, 1)) {
		DbgPrint("�ر�����ȷ�Ͽ�\n");
		LOG2(L"�ر�����ȷ�Ͽ�", "orange b");
		Click(450, 435, 550, 450);
		Sleep(300);

		return true;
	}

	return false;
}

// ������
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
		DbgPrint("������:%d,%d\n", x, y);
		Sleep(MyRand(300, 1000, i * 20));
	}
}

// �񱩵��
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
		DbgPrint("�񱩵��:%d,%d ��ʣ%d��\n", x, y, m_ClickCrazy.Count - 1);
		LOGVARN2(64, "c6", L"�񱩵��:%d,%d ��ʣ%d��", x, y, m_ClickCrazy.Count - 1);
		Sleep(MyRand(300, 500, m_ClickCrazy.Count));
	}
	m_ClickCrazy.Count--;
}

// �ȴ�
void GameProc::Wait()
{
#if 0
	if (m_stLast.OpCode != OP_NPC && m_stLast.OpCode != OP_SELECT) {
		Click(1150, 352, 1180, 372); // ��������Ʒ��
	}
#endif
	
	Wait(m_pStep->WaitMs, m_pStep->Extra[0]);
}

// �ȴ�
void GameProc::Wait(DWORD ms, int no_open)
{
	wchar_t log[64];
	if (ms < 1000) {
		DbgPrint("�ȴ�%d����\n", ms);
		LOGVARP2(log, "c6", L"�ȴ�%d����", ms);
		Sleep(ms);
		return;
	}

	if (ms >= 2000 && !no_open) {
		DWORD start_ms = GetTickCount();
		if ((start_ms & 0x0f) < 3) {
			Click(16, 26, 66, 80); // ���ͷ��
			Sleep(1500);
		
			int click_count = MyRand(0, 2 + (ms / 1000 / 3), ms);
			for (int i = 0; i < click_count; i++) { // ���Ҳ���һ��
				Click(315, 186, 1160, 500);
				Sleep(MyRand(500, 1000, i));
			}

			Click(1155, 55, 1160, 60); // �ر�
			Sleep(500);

			DWORD use_ms = GetTickCount() - start_ms;
			if (use_ms > ms)
				ms = 0;
			else
				ms -= use_ms;
		}
	}

	if (ms >= 12000 && IsNeedAddLife() != -1) {
		DbgPrint("�ȴ�ʱ��ﵽ%d��(%d����), ��������\n", ms / 1000, ms);
		LOGVARP2(log, "c0", L"�ȴ�ʱ��ﵽ%d��(%d����), ��������", ms / 1000, ms, ms);
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
			DbgPrint("�ȴ�%02d�룬��ʣ%02d��.\n", ms / 1000, ls);
			LOGVARP2(log, "c6", L"�ȴ�%02d�룬��ʣ%02d��.", ms / 1000, ls);
		}
		n = ls;
		Sleep(100);
	}
}

// С��
void GameProc::Small()
{
#if 1
	if (m_pStep->SmallV == 0) { // ������
		m_pGame->m_pServer->SmallOutFB(m_pGame->m_pBig, nullptr, 0);
	}
	if (m_pStep->SmallV == 1) { // ������
		m_pGame->m_pServer->SmallInFB(m_pGame->m_pBig, nullptr, 0);
	}
#else
	if (m_pStep->SmallV == 0) { // ������
		Click(597, 445, 606, 450); // �ر���ʾ��
		Sleep(500);
		Click(597, 445, 606, 450); // �����������
		Sleep(2000);

		int count = m_pGame->m_pEmulator->List2();
		for (int i = 0; i < count; i++) {
			MNQ* m = m_pGame->m_pEmulator->GetMNQ(i);
			if (m && m->Account && !m->Account->IsBig) {
				SwitchGameWnd(m->Wnd);
				SetForegroundWindow(m->WndTop);
				Sleep(100);
				m_pGame->m_pMove->Run(890, 1100, 0, 0, false, m->Account); // �ƶ����̶��ص�
			}
		}
		Sleep(1000);

		ClickOther(190, 503, 195, 505, m_pGame->m_pBig); // ���NPC
		Sleep(2000);
		ClickOther(67, 360, 260, 393, m_pGame->m_pBig);  // ѡ��0 ������
		Sleep(1000);
		ClickOther(67, 360, 260, 393, m_pGame->m_pBig);  // ѡ��0 ��Ҫ�뿪����

		SwitchGameWnd(m_pGame->m_pBig->Mnq->Wnd);
		SetForegroundWindow(m_pGame->m_pBig->Mnq->WndTop);
	}
#endif
}

// ����
void GameProc::ReBorn()
{
	LOG2(L"\n�ȴ����︴��...", "red b");
	Wait(26 * 1000);
	CloseTipBox();
	m_pGame->m_pTalk->CloseAllBox();

	for (int i = 1; true; i++) {
		LOGVARN2(32, "c0 b", L"�������(%d)", i);
		Click(537, 415, 550, 425); // �㸴��
		Sleep(2000);

		if (IsNeedAddLife() != -1)
			break;

		CloseTipBox();
	}

	LOG2(L"�ȴ�������Ϸ����...", "red b");
	Wait(2000);
	do {
		Sleep(1000);
	} while (!m_pGame->m_pTalk->IsInGamePic());

	LOG2(L"����ȫ������", "red b");
	m_pGame->m_pPet->PetOut(-1);
	Sleep(1000);
}

// �Ƿ����NPC�Ի����
bool GameProc::IsCheckNPC(const char* name)
{
	return strcmp(name, "ħ���ϱ�") == 0 || strcmp(name, "�������ϱ�") == 0 || strcmp(name, "�����ϱ�") == 0
		|| strcmp(name, "��֮������") == 0 || strcmp(name, "Թ֮������") == 0 || strcmp(name, "��Ѫ��ʿ") == 0
		|| strcmp(name, "����ʿ��̳") == 0 || strcmp(name, "Ů������̳") == 0
		|| strcmp(name, "����֮��") == 0
		|| strstr(name, "Ԫ���ϱ�") != nullptr;
}

// �Ƿ����NPC�Ի����
bool GameProc::IsCheckNPCTipBox(const char* name)
{
	return strstr(name, "��ӡ����") != nullptr
		|| strstr(name, "ͼ��") != nullptr
		|| strstr(name, "ӡ��") != nullptr
		|| strcmp(name, "��ħ������̳") == 0
		|| strcmp(name, "���ȵ����") == 0
		|| strcmp(name, "����֮��") == 0
		|| strcmp(name, "����֮��") == 0
		|| strcmp(name, "�������Թ��") == 0;
}


// ����ƶ�[�����x��y�ƶ�rx��ry����]
void GameProc::MouseMove(int x, int y, int rx, int ry, HWND hwnd)
{
	int dx = abs(rx), dy = abs(ry);
	for (int i = 1; true;) {
		if (i > dx && i > dy)
			break;

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

		Sleep(50);
		MouseMove(x + mv_x, y + mv_y, hwnd);

		DbgPrint("MouseMove:%d,%d\n", x + mv_x, y + mv_y);
		i += MyRand(1, 2);
	}
}

// ����ƶ�
void GameProc::MouseMove(int x, int y, HWND hwnd)
{
	if (!hwnd)
		hwnd = m_hWndGame;

	::SendMessage(hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x, y));
	//DbgPrint("����ƶ�:%d,%d\n", x, y);
}

// ������
void GameProc::MouseWheel(int x, int y, int z, HWND hwnd)
{
	if (!hwnd)
		hwnd = m_hWndGame;

	::SendMessage(hwnd, WM_MOUSEWHEEL, MAKEWPARAM(MK_CONTROL, z), MAKELPARAM(x, y));
}

// ������
void GameProc::MouseWheel(int z, HWND hwnd)
{
	if (!hwnd)
		hwnd = m_hWndGame;

	RECT rect;
	::GetWindowRect(hwnd, &rect);
	MouseWheel(MyRand(rect.left+500, rect.left+800), MyRand(rect.top+236, rect.top+500), z, hwnd);

}

// ���������
void GameProc::Click(int x, int y, int ex, int ey, int flag, HWND hwnd)
{
	Click(MyRand(x, ex), MyRand(y, ey), flag, hwnd);
}

// ���������
void GameProc::Click(int x, int y, int flag, HWND hwnd)
{
#if 0
	m_pGame->m_pEmulator->Tap(x, y);
	return;
#endif

	if (!hwnd)
		hwnd = m_hWndGame;

	if (flag & 0x01) {
		::PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
	}
	if (flag & 0x02) {
		::PostMessage(hwnd, WM_LBUTTONUP, 0, MAKELPARAM(x, y));
	}
	Sleep(15);
	//DbgPrint("===���:%d,%d===\n", x, y);
}

// ���������[���������ʺ�]
void GameProc::ClickOther(int x, int y, int ex, int ey, _account_* account_no)
{
	int count = m_pGame->m_pEmulator->List2();
	for (int i = 0; i < count; i++) {
		MNQ* m = m_pGame->m_pEmulator->GetMNQ(i);
		if (!m || !m->Account) // û���໥��
			continue;
		if (m->Account == account_no) // ��������ʺ�
			continue;

		DbgPrint("GameProc::ClickOther %s\n", m->Account->Name);
		Click(x, y, ex, ey, 0xff, m->Wnd);
	}
}

// ������˫��
void GameProc::DBClick(int x, int y, HWND hwnd)
{
	if (!hwnd)
		hwnd = m_hWndGame;

	//::PostMessage(hwnd, WM_LBUTTONDBLCLK, MK_LBUTTON, MAKELPARAM(x, y));
	Click(x, y, 0xff, hwnd);
	Click(x, y, 0xff, hwnd);
}

// ����
void GameProc::Keyboard(char key, int flag, HWND hwnd)
{
	if (!hwnd)
		hwnd = m_hWndGame;

	if (flag & 0x01)
		::PostMessage(hwnd, WM_KEYDOWN, key, 0);
	if (flag & 0x02)
		::PostMessage(hwnd, WM_KEYUP, key, 0);
}

int GameProc::IsNeedAddLife()
{
	m_pGame->m_pGameData->ReadLife();
	if (m_pGame->m_pGameData->m_dwLife == 0)
		return -1;
	if (m_pGame->m_pGameData->m_dwLife < 8000)
		return 1;
	return 0;
}

// �Ƿ���ǰ����
bool GameProc::IsForegroundWindow()
{
	if (!m_pGame->m_pBig->Mnq)
		return false;

	HWND hWndTop = ::GetForegroundWindow();
	return hWndTop == m_pGame->m_pBig->Mnq->WndTop || hWndTop == m_pGame->m_pBig->Mnq->Wnd;
}
