#pragma once
#include <Windows.h>

#define NAME_OFFSET     0x0FC // ���������
#define X_OFFSET        0x028 // X��������������׵�ַ��ƫ��
#define Y_OFFSET        0x02C // Y��������������׵�ַ��ƫ��
#define LIFE_OFFSET     0xF60 // Ѫ������������׵�ַ��ƫ��
#define LIFE_MAX_OFFSET 0xD40 // Ѫ�����ֵ����������׵�ַ��ƫ��

#define CLICK_X_OPEN_MAP  1120 // �򿪵�ͼ����X
#define CLICK_Y_OPEN_MAP  66   // �򿪵�ͼ����Y
#define CLICK_X2_OPEN_MAP 1220 // �򿪵�ͼ����X2
#define CLICK_Y2_OPEN_MAP 125  // �򿪵�ͼ����Y2

#define CLICK_X_GOWAY     725  // �Զ�Ѱ·����X
#define CLICK_Y_GOWAY     620  // �Զ�Ѱ·����Y
#define CLICK_X2_GOWAY    790  // �Զ�Ѱ·����X2
#define CLICK_Y2_GOWAY    630  // �Զ�Ѱ·����Y2

// ��Ϸ��Ʒ����
enum ITEM_TYPE
{
	δ֪��Ʒ = 0x00,       // ��֪��ʲô��Ʒ
	��Ч���ư� = 0x000B5593, // ���Կ�����ƿ��Ч����ҩˮ
	�������ҩˮ = 0x000B783C,
	��Ч����ҩˮ = 0x000F6982, // +2500����ֵ
	��Чʥ����ҩ = 0x000F943E, // �������
	��ʮ��������Ƭ��1 = 0x000F90E4,
	��ʮ��������Ƭ��2 = 0x000F90E5,
	ħ�꾧ʯ = 0x000FD35E,
	��꾧ʯ = 0x000FD368,
	��ħ��ʯ = 0x000FD372,
};

// �Զ���ʰ��Ʒ��Ϣ
typedef struct conf_item_info
{
	CHAR      Name[32];
	DWORD     Type;
	DWORD     Extra[2];   
} ConfItemInfo;

typedef struct game_data_addr
{
	DWORD Player;  // �����׵�ַ
	DWORD Name;    // ��ɫ������
	DWORD CoorX;   // X����
	DWORD CoorY;   // Y����
	DWORD MoveX;   // X����(Ŀ�ĵ�)
	DWORD MoveY;   // Y����(Ŀ�ĵ�)
	DWORD Life;    // Ѫ��
	DWORD LifeMax; // Ѫ�����ֵ
} GameDataAddr;

// ����д��Ŀ�ĵ�xy��ַ����
typedef struct share_write_xy_data
{
	DWORD InDll;   // �Ƿ���ע��DLL
	DWORD AddrX;   // X��ַ
	DWORD AddrY;   // Y��ַ
	DWORD X;       // X��ֵ
	DWORD Y;       // Y��ֵ
	DWORD Flag;    // 0-������д������ 1-Dll����д������
} ShareWriteXYData;

class Game;
class GameData
{
public:
	GameData(Game* p);

	
	// ������Ϸ
	void WatchGame();
	// ��ȡ��Ϸ����
	void FindGameWnd();
	// ö�ٴ���
	static BOOL CALLBACK EnumProc(HWND hWnd, LPARAM lParam);

	// �Ƿ���ָ���������� allow=���
	bool IsInArea(int x, int y, int allow, _account_* account=nullptr);
	// �Ƿ���ָ���������� allow=���
	bool IsNotInArea(int x, int y, int allow, _account_* account=nullptr);

	// �Ƿ������
	bool IsInShenDian(_account_* account=nullptr);
	// �Ƿ��ڸ����ſ�
	bool IsInFBDoor(_account_* account=nullptr);

	// ��ȡ�����׵�ַ
	bool FindPlayerAddr();
	// ��ȡĿ�ĵ������ַ
	bool FindMoveCoorAddr();

	// ��ȡ��ɫ
	bool ReadName(char* name, _account_* account=nullptr);
	// ��ȡ����
	bool ReadCoor(DWORD* x=nullptr, DWORD* y=nullptr, _account_* account=nullptr);
	// ��ȡ����ֵ
	DWORD ReadLife(DWORD* life=nullptr, DWORD* life_max=nullptr, _account_* account=nullptr);

	// ���������ڴ�
	void CreateShareMemory();
	// д��Ŀ�ĵ�
	void WriteMoveCoor(DWORD x, DWORD y, _account_* account=nullptr);

	// ����������
	DWORD SearchCode(DWORD* codes, DWORD length, DWORD* save, DWORD save_length = 1, DWORD step = 4);
	// ����������
	DWORD SearchByteCode(BYTE* codes, DWORD length);
	// ��ȡ���ֽ�����
	bool ReadDwordMemory(DWORD addr, DWORD& v, _account_* account=nullptr);
	// ��ȡ�ڴ�
	bool ReadMemory(PVOID addr, PVOID save, DWORD length, _account_* account=nullptr);
	// ��ȡ��Ϸ�ڴ�
	bool ReadGameMemory(DWORD flag = 0x01);

public:
	Game* m_pGame;

	// ��ǰX����
	DWORD m_dwX = 0;
	// ��ǰY����
	DWORD m_dwY = 0;
	// ��ǰѪ��
	DWORD m_dwLife = 0;
	// Ѫ�����ֵ
	DWORD m_dwLifeMax = 0;

	// ����
	GameDataAddr m_DataAddr;
	// ���ݴ��
	GameDataAddr m_DataAddrBig;
public:
	// ��Ϸ��Ŵ���
	HWND m_hWndBig = NULL;
	// ��Ϸ���
	HANDLE m_hProcessBig = NULL;
	// ���
	_account_* m_pAccoutBig = NULL;

	// ��ϷȨ�޾��
	HANDLE   m_hGameProcess = NULL;
	// �Ƿ��ȡ���
	bool  m_bIsReadEnd;
	// ��ȡ�ڴ��Ĵ�С
	DWORD m_dwReadSize;
	// ��ȡ����ַ
	DWORD m_dwReadBase;
	// ��ȡ������ʱ�ڴ�
	BYTE* m_pReadBuffer;

	HANDLE m_hShareMap;    // �����ڴ�
	ShareWriteXYData* m_pShareBuffer; // �����ڴ�
	bool m_bInDll = false;
};