#pragma once
#include <Windows.h>

#define DLLEXPORT __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif

	// ��ʼ�����п�
	DLLEXPORT void WINAPI Game_Init(HWND hWnd, const char* conf_path);
	// ��Ϸ�ͷ�
	DLLEXPORT void WINAPI Game_Relase();
	// ��Ϸ��ͣ
	DLLEXPORT void WINAPI Game_Pause(bool v);
	// �ʺ��Ƿ��¼
	DLLEXPORT int WINAPI Game_IsLogin(int index);
	// �Ƿ�ɹ���װdll 0δ 1�� -1�뼤��
	DLLEXPORT int WINAPI Game_InstallDll();
	// ����Ϸ index[-2=ֹͣ -1=�Զ���¼ ����-1=�˺�����]
	DLLEXPORT int WINAPI Game_OpenGame(int index, int close_all);
	// �ر���Ϸ��
	DLLEXPORT int WINAPI Game_CloseGame(int index);
	// �Ѿ������
	DLLEXPORT int WINAPI Game_InTeam(int index);
	// �޸�һЩ����
	DLLEXPORT int WINAPI Game_PutSetting(const wchar_t* name, int v);
	// ��֤����
	DLLEXPORT int WINAPI Game_VerifyCard(const wchar_t* card);

#ifdef __cplusplus
}
#endif

// ����Ϸ
DWORD WINAPI PlayGame(LPVOID param);
// game.run
DWORD WINAPI RunGame(LPVOID param);
// game.listen
DWORD WINAPI Listen(LPVOID param);
// ����
DWORD WINAPI Verify(LPVOID param);
