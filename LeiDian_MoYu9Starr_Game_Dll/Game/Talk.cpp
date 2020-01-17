#include "Game.h"
#include "Talk.h"
#include "PrintScreen.h"
#include "GameProc.h"
#include <My/Common/C.h>

Talk::Talk(Game* p)
{
	m_pGame = p;
}

// ���ж��NPC��һ��ʱ, ����NPC�б�ѡ��
bool Talk::IsNeedCheckNPC()
{
	// ���NPC
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 503, 40, 513, 50, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("���NPC", nullptr, 1) > 0;
}

// ѡ��NPC
void Talk::SelectNPC(int no)
{
	int x = 450, x2 = 600, y = 0, y2 = 0;
	if (no == 0)
		y = 90, y2 = 125;
	if (no == 1)
		y = 175, y2 = 205;
	if (no == 2)
		y = 260, y2 = 286;

	m_pGame->m_pGameProc->Click(x, y, x2, y2);
}

// NPC
DWORD Talk::NPC(const char* name)
{
	// �Ի���ť����[875,340 912,388]
	m_pGame->m_pEmulator->Tap(MyRand(875, 912), MyRand(350, 376));
	//m_pGame->m_pGameProc->Click(875, 350, 912, 376);
	return 0;
}

// NPC�Ի�ѡ����
void Talk::Select(DWORD no)
{
	// ��һ�� 67,360 456,393
	// �ڶ��� 67,425 456,460
	// ������ 67,425 456,460
	if (no == 0) {
		m_pGame->m_pEmulator->Tap(MyRand(67, 368), MyRand(360, 393));
		//m_pGame->m_pGameProc->Click(67, 360, 368, 393);
		return;
	}
	if (no == 1) {
		m_pGame->m_pEmulator->Tap(MyRand(67, 368), MyRand(425, 460));
		//m_pGame->m_pGameProc->Click(67, 425, 368, 460);
		return;
	}
	if (no == 2) {
		m_pGame->m_pEmulator->Tap(MyRand(67, 368), MyRand(503, 535));
		//m_pGame->m_pGameProc->Click(67, 503, 368, 535);
		return;
	}
}

// NPC�Ի�״̬[�Ի����Ƿ��]
bool Talk::NPCTalkStatus(int index)
{
	if (index >= 0 && index < 5) {
		int x = 30, y = 372 + (index * 72);
		int x2 = 40, y2 = 382 + (index * 72);
		//DbgPrint("NPCTalkStatus:%d %d,%d %d,%d\n", index, x, y, x2, y2);
		m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, x, y, x2, y2, 0, true);
		//m_pGame->m_pPrintScreen->GetPixelCount(0xFFFF0000, 0x00505050, true);
		return m_pGame->m_pPrintScreen->ComparePixel("�Ի�ѡ��", nullptr, 1) > 0;
	}
	else {
		m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 100, 320, 168, 323, 0, true);
		return m_pGame->m_pPrintScreen->CheckPixel(0xFF161616);
	}
}

// �ȴ��Ի����
bool Talk::WaitTalkOpen(int index, DWORD ms)
{
	if (ms == 0)
		ms = 1500;

	for (int i = 0; i < ms; i+=100) {
		if (NPCTalkStatus(index))
			return true;

		Sleep(100);
	}
	return false;
}

// �ȴ��Ի����
bool Talk::WaitTalkClose(int index, DWORD ms)
{
	if (ms == 0)
		ms = 1500;

	for (int i = 0; i < ms; i += 100) {
		if (!NPCTalkStatus(index))
			return true;

		Sleep(100);
	}
	return false;
}

// �Ի���ť�Ƿ��Ѵ�, ����NPC����ֶԻ���ť
bool Talk::TalkBtnIsOpen()
{
	// ��ȡ���ǶԻ���ť��������
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 886, 355, 896, 365, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("�Ի���ť", nullptr, 1) > 0;
}

// �����������Ƿ��Ѵ�
bool Talk::LianGongChangIsOpen()
{
	// ��ȡ������������������
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 575, 70, 585, 80, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("������", nullptr, 1) > 0;
}

// �ر�����������
void Talk::CloseLianGongChangBox()
{
	m_pGame->m_pGameProc->Click(906, 66, 910, 68);
}

// ���촰���Ƿ��Ѵ�
bool Talk::SpeakIsOpen()
{
	// ��ȡ�������½Ǹ���ͼ��
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 42, 682, 52, 692, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("�����Ѵ�", nullptr, 1) > 0;
}

// �ر����촰��
void Talk::CloseSpeakBox()
{
	LOG2(L"�ر������", "orange b");
	m_pGame->m_pGameProc->Click(563, 327, 580, 350);
}

// ȷ����ť�Ƿ��
bool Talk::SureBtnIsOpen()
{
	// ��ȡ����ȷ����ťͼƬ
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 585, 450, 600, 465, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("��ʾ��", nullptr, 1) > 0;
}

// ȷ�ϰ�ť�Ƿ��
bool Talk::ConfirmBtnIsOpen()
{
	// ��ȡȷ�Ͽ�ȡ����ť
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 466, 430, 476, 440, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("ȷ�Ͽ�", nullptr, 1) > 0;
}

// ����ȷ�ϰ�ťflag=0ȡ�� flag=1ȷ��
void Talk::ClickConfirmBtn(int flag)
{
	for (int i = 0; i < 5; i++) {
		if (flag == 0)
			m_pGame->m_pGameProc->Click(700, 430, 750, 450); // ȡ����ť
		if (flag == 1)
			m_pGame->m_pGameProc->Click(700, 430, 750, 450); // ȷ����ť

		for (int j = 0; j < 1000; j += 100) {
			Sleep(100);
			if (!ConfirmBtnIsOpen())
				return;
		}
		DbgPrint("%d.�����ٴε��ȷ�Ͽ�ť\n", i + 1);
		LOGVARN2(54, "c6", L"%d.�����ٴε��ȷ�Ͽ�ť", i + 1);
	}
}

// �ȴ�ȷ�ϰ�ť��
bool Talk::WaitForConfirmBtnOpen(DWORD ms)
{
	for (int i = 0; i < ms; i += 100) {
		if (ConfirmBtnIsOpen())
			return true;

		Sleep(100);
	}
	return false;
}

// �ر���������ȡ�����
void Talk::CloseLGCBox()
{
	// ��ȡȷ�Ͽ�ȡ����ť
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 575, 70, 585, 80, 0, true);
	if (m_pGame->m_pPrintScreen->ComparePixel("������", nullptr, 1) > 0) {
		m_pGame->m_pGameProc->Click(908, 65, 912, 68);
		Sleep(500);
	}
}

// �罻��Ϣ��򿪷�
bool Talk::SheJiaoBoxIsOpen()
{
	// ��ȡ�罻��
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 366, 466, 376, 476, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("�罻��Ϣ��", nullptr, 1) > 0;
}

// �ȴ��罻��Ϣ���
bool Talk::WaitForSheJiaoBox(DWORD ms)
{
	for (DWORD i = 0; i < ms; i += 100) {
		if (SheJiaoBoxIsOpen())
			return true;

		Sleep(100);
	}
	return false;
}

// �ر��罻��Ϣ��
void Talk::CloseSheJiaoBox(bool is_check)
{
	if (is_check && !SheJiaoBoxIsOpen())
		return;

	m_pGame->m_pGameProc->Click(933, 166, 935, 170); // �ر�
	Sleep(500);
}

// �Ƿ��ڵ�¼����
bool Talk::IsInLoginPic(HWND hwnd)
{
	if (!hwnd)
		hwnd = m_pGame->m_pGameProc->m_hWndGame;

	// �Ҳ��ʺ�ͼ��
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(hwnd, 1205, 140, 1220, 155, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("��¼��ť", nullptr, 1) > 0;
}

// �ȴ������¼����
bool Talk::WaitForInLoginPic(HWND hwnd, DWORD ms)
{
	if (!hwnd)
		hwnd = m_pGame->m_pGameProc->m_hWndGame;
	for (DWORD i = 0; i < ms; i += 100) {
		if (IsInLoginPic(hwnd))
			return true;

		Sleep(200);
	}
	return false;
}

// �Ƿ�����Ϸ���� û����������
bool Talk::IsInGamePic()
{
	// ���½�ʱ�����
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 60, 690, 186, 706, 0, true);
	return m_pGame->m_pPrintScreen->GetPixelCount(0xFFFFFFFF, 0x00101010) >= 100;
}

// �ȴ�������Ϸ����
bool Talk::WaitForInGamePic(DWORD ms)
{
	for (DWORD i = 0; i < ms; i += 100) {
		if (IsInGamePic())
			return true;

		Sleep(200);
	}
	return false;
}

// �������Ƿ��
bool Talk::CommonBoxIsOpen()
{
	// ������
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 460, 50, 470, 60, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("������", nullptr, 1) > 0
		|| m_pGame->m_pPrintScreen->ComparePixel("������2", nullptr, 1) > 0;
}

// �ر�ϵͳ������ʾ��
void Talk::CloseCommonBox()
{
	DbgPrint("\n--------------�ر��ڵ���--------------\n\n");
	LOG2(L"�ر��ڵ���", "orange b");
	m_pGame->m_pGameProc->Click(1150, 50, 1175, 68);
}

// �ر������ڵ���
void Talk::CloseAllBox()
{
	while (!IsInGamePic()) { // �������ڵ���
		if (CommonBoxIsOpen()) {
			CloseCommonBox();
		}
		else if (SpeakIsOpen()) {
			CloseSpeakBox();
		}
		else {
			// �رհ�ť
			m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 350, 0, 1280, 350, 0, true);
			ComPoint cp[32];
			if (m_pGame->m_pPrintScreen->ComparePixel("�رհ�ť", cp, sizeof(cp)/sizeof(ComPoint))) {
				m_pGame->m_pGameProc->Click(cp[0].x, cp[0].y);
			}
			else {
				break;
			}
		}
		Sleep(800);
	}
}
