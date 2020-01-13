#pragma once
#include "GameStep.h"
#include <My/Common/Link.hpp>
#include <Windows.h>
#include <vector>
#include <fstream>

using namespace std;

#define ONE_COOR_PIX     50   // һ����ռ������
#define ONE_MAX_MOV_COOR 8    // һ�������ƶ�8��������
#define ONE_MOV_MAX_MS   1000 // һ���ƶ��������ʱ��(����)

#define BAG_NUM         40       // ��������
#define WUPIN_YAOBAO    0x0B5593 // ҩ��
#define WUPIN_XIANGLIAN 0x0B5B24 // ����������
#define WUPIN_YAO       0x0F6982 // ҩ
#define WUPIN_LINGYAO   0x0F943E // ��Чʥ����ҩ

#define RUNRUN 1

struct _step_;
struct _account_;

enum ComImgIndex;

class Game;
class GameStep;
class GameProc
{
public:
	// !!!
	GameProc(Game* p);

	// ��ʼ������
	void InitData();
	// ��ʼ������
	bool InitSteps();
	// �л���Ϸ����
	void SwitchGameWnd(HWND);
	// �л���Ϸ�ʺ�
	void SwitchGameAccount(_account_* account);
	// ������ǰ
	void SetForegroundWindow(HWND hwnd);
	// ִ��
	void Exec(_account_* account);
	// ����
	void Run(_account_* account);
	// ȥ�������
	void GoInTeamPos(_account_* account);
	// ��������
	void CreateTeam();
	// �������
	void ViteInTeam();
	// ���
	void InTeam(_account_* account);
	// ������
	void InFB(_account_* account);
	// �����˽�����
	void AllInFB(_account_* account_open);
	// ͬ��ϵͳ��Ϣ
	void AgreenMsg(const char* name, HWND hwnd=NULL);
	// ͬ��ϵͳ��Ϣ
	int AgreenMsg(const char* name, int icon_index, bool click=false, HWND hwnd = NULL);
	// ���ȥ������½����
	void GoLeiMing();
	// ȥ��ȡ����
	void GoGetXiangLian();
	// ѯ����������
	_account_*  AskXiangLian();
	// ȥ�����ſ�
	void GoFBDoor(_account_* account);
	// ��������
	_account_* OpenFB();
	// ������
	void OutFB(_account_* account);
	// ��ȡ���������ʺ�
	_account_* GetOpenFBAccount();
	// �Ƿ��ɴ�ſ�������
	bool IsBigOpenFB();
	// ��������
	void ContinueInFB();
	// �Ƿ��ڸ���
	bool IsInFB(_account_* account=nullptr);
	// ִ�и�������
	void ExecInFB();
	// ִ������
	bool ExecStep(Link<_step_*>& link, bool isfb=false);
	// �����Ƿ���ִ�����
	bool StepIsComplete();
	// �ƶ�
	void Move(bool rand_click=false);
	// NCP
	void NPC();
	// NPC
	void NPC(const char* name, int x, int y, int x2 = 0, int y2 = 0);
	// ���һ���Ի���NPC
	bool NPCLast(bool check_pos=true, DWORD mov_sleep_ms=0);
	// ѡ��
	void Select();
	// ����
	void Magic();
	// ��λ
	void KaWei();
	// ���
	void Clear();
	// ����
	void KaiRui();
	// ����
	void PickUp();
	// ��Ǯ
	void SaveMoney();
	// ����
	DWORD CheckIn(bool in=true);
	// ʹ����Ʒ
	void UseItem();
	// ����Ʒ
	void DropItem();
	// ������Ʒ
	void SellItem();
	// ��ť
	void Button();
	// ��ť
	bool Button(int button_id, DWORD sleep_ms=0, const char* name=nullptr);
	// �رյ���
	bool CloseTipBox();
	// �ر���Ʒʹ����ʾ��
	bool CloseItemUseTipBox();
	// �ر�ϵͳ������ʾ��
	bool CloseSystemViteBox();
	// ����
	void Find(Link<_step_*>& link);
	// ������
	void ClickRand();
	// �񱩵��
	void ClickCrazy();
	// �ȴ�
	void Wait();
	// �ȴ�
	void Wait(DWORD ms, int no_open=1);
	// С��
	void Small();
	// ����
	void ReBorn();
	// �Ƿ����NPC�Ի����
	bool IsCheckNPC(const char* name);
	// �Ƿ����NPC�������
	bool IsCheckNPCTipBox(const char* name);
	// ����ƶ�[�����x��y�ƶ�rx��ry����]
	void MouseMove(int x, int y, int rx, int ry, HWND hwnd=NULL);
	// ����ƶ�
	void MouseMove(int x, int y, HWND hwnd=NULL);
	// ������
	void MouseWheel(int x, int y, int z, HWND hwnd=NULL);
	// ������
	void MouseWheel(int z, HWND hwnd=NULL);
	// ���������
	void Click(int x, int y, int ex, int ey, int flag = 0xff, HWND hwnd=NULL);
	// ���������
	void Click(int x, int y, int flag = 0xff, HWND hwnd=NULL);
	// ���������[���������ʺ�]
	void ClickOther(int x, int y, int ex, int ey, _account_* account_no);
	// ������˫��
	void DBClick(int x, int y, HWND hwnd=NULL);
	// ����
	void Keyboard(char key, int flag=0xff, HWND hwnd=NULL);
	// ��ȡ��������
	bool ReadCoor();
	// ��ȡ����Ѫ��
	bool ReadLife();
	// ��ȡ��ݼ�������Ʒ����
	bool ReadQuickKey2Num();
	// ��ȡ������Ʒ
	bool ReadBag();
	// �Ƿ���Ҫ��Ѫ��
	int  IsNeedAddLife();
	// �Ƿ���ǰ����
	bool IsForegroundWindow();
	// ��Ѫ
	void AddLife();
	// ������Ϣ�������
	void SendMsg(const char* v, const char* v2=nullptr);
	// ֹͣ
	void Stop(bool v=true);
	// ���ռ��ļ�
	void OpenLogFile();
	// д���ռ�
	void WriteLog(const char* log, bool flush=true);
public:
	// Ҫ��������Ϸ����
	HWND m_hWndGame;
	// ��ǰ�������ʺ�
	_account_* m_pAccount;
	// ��Ϸָ��
	Game*     m_pGame;
	// ��Ϸ����ָ��
	GameStep* m_pGameStep;
	// ����ִ�еĲ���
	_step_* m_pStep;
	// ����ִ�еĲ���[����]
	_step_* m_pStepCopy;
	// �ϴ�ִ�еĲ���
	_step_* m_pStepLast;
	// �Ѽ�¼����[��ס�ػش˲���]
	_step_* m_pStepRecord;
	// �Ƿ��¼����
	bool m_bIsRecordStep;
	// �Ƿ�ֹͣ
	bool  m_bStop = false;
	// �Ƿ���ͣ
	bool  m_bPause = false;
	// �Ƿ����¿�ʼ
	bool  m_bReStart = false;
	// �Ƿ��˦
	bool m_bIsCrazy = false;
	// ��˦����
	char m_CrazyMagic[32];

	// �Ƿ��������븱��
	bool m_bLockGoFB = false;
	// �Ƿ��ڸ�������
	bool m_bAtFB = false;
	// �Ƿ����忭��
	bool m_bClearKaiRui = false;
	// �Ƿ����õ���¼����
	bool m_bIsResetRecordStep = false;
	// �Ƿ��һ���ƶ�, ��Ҫ��һ�µ�ͼ
	bool m_bIsFirstMove = true;
	// �Ƿ��ؿ�����
	bool m_bReOpenFB = false;

	// �����ƶ�����
	int m_nReMoveCount = 0;
	// ҩ������
	int m_nYaoBao = 0;
	// ҩ����
	int m_nYao = 0;
	// ҩ����������
	int m_nLiveYaoBao = 6;
	// ҩ��������
	int m_nLiveYao = 3;
	// �ؿ���������
	int m_nReOpenFBCount = 0;
	// ˢ��������
	int m_nPlayFBCount = 0;
	// �Ѿ�����ʱ��
	int m_nFBTimeLong = 0;
	// ��ʼ����ʱ��
	int m_nStartFBTime = 0;
	// ���¸���ʱ��
	int m_nUpdateFBTimeLongTime = 0;

	// �ϴε�����
	struct {
		DWORD OpCode;
		DWORD MvX;
		DWORD MvY;
		DWORD NPCOpCode; // �ǶԻ����ǵ��
		char  NPCName[32];  // ����
		DWORD ClickX;
		DWORD ClickY;
		DWORD ClickX2;
		DWORD ClickY2;
	} m_stLast;

	// ��������
	struct {
		int X;
		int Y;
		int X2;
		int Y2;
		int Count;
	} m_ClickCrazy;
};
