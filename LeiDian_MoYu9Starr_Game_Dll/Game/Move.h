#pragma once
#include <Windows.h>

struct _account_;
class Game;
class Move
{
public:
	// ...
	Move(Game* p);

	// ��ʼ������
	void InitData();
	// �ƶ����յ�[�ﵽ�յ㷵��]
	bool RunEnd(DWORD x, DWORD y, _account_* account, bool click=true, DWORD ms=0);
	// �ƶ�
	int  Run(DWORD x, DWORD y, _account_* account, DWORD click_x, DWORD click_y, bool is_click=false, bool rand_click=false);
	// ����Ϸ��ͼ
	void OpenMap(_account_* account);
	// �ر���Ϸ��ͼ
	void CloseMap(HWND hwnd=NULL);
	// ��ͼ�Ƿ��
	bool IsOpenMap();
	// �ȴ���ͼ��
	void WaitMapOpen(DWORD ms=1500);
	// ����Զ�Ѱ·
    void GoWay();
	// �����ƶ�λ��
	void SetMove(DWORD x, DWORD y, _account_* account);
	// ����ƶ�����
	void ClearMove();
	// �Ƿ�ﵽ�յ�
	bool IsMoveEnd(_account_* account);
	// �Ƿ��ƶ�
	bool IsMove(_account_* account);
public:
	// ��Ϸ��
	Game* m_pGame;
	// ��ǰλ��X
	DWORD m_dwX = 0;
	// ��ǰλ��Y
	DWORD m_dwY = 0;
	// �ϴ�λ��X
	DWORD m_dwLastX = 0;
	// �ϴ�λ��Y
	DWORD m_dwLastY = 0;
	// �ƶ�λ��X
	DWORD m_dwMvX;
	// �ƶ�λ��Y
	DWORD m_dwMvY;
	// �ϴ��ƶ�λ��X
	DWORD m_dwLastMvX = 0;
	// �ϴ��ƶ�λ��Y
	DWORD m_dwLastMvY = 0;
	// �ж��ƶ�����ʱ��
	DWORD m_dwIsEndTime;
	// �ж��Ƿ��ƶ�ʱ��
	DWORD m_dwIsMvTime;
	// ���ڱȽϵ�X
	DWORD m_dwCmpX;
	// ���ڱȽϵ�Y
	DWORD m_dwCmpY;
	// �ƶ�ʱ��
	DWORD m_dwMvTime;
	// ��ȡ����ʱ��
	int   m_iGetPosTime;
	// ��ȡ������ʱ��
	int   m_iFlagPosTime;
};