#pragma once
#include <Windows.h>

#define DLLEXPORT __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif

	// 初始化运行库
	DLLEXPORT void WINAPI Game_Init(HWND hWnd, const char* conf_path);
	// 游戏是否激活
	bool WINAPI Game_IsValid();
	// 游戏释放
	void WINAPI Game_Relase();
	// 游戏暂停
	void WINAPI Game_Pause(bool v);
	// 帐号是否登录
	int WINAPI Game_IsLogin(int index);
	// 是否成功安装dll 0未 1是 -1请激活
	int WINAPI Game_InstallDll();
	// 打开游戏 index[-2=停止 -1=自动登录 大于-1=账号索引]
	int WINAPI Game_OpenGame(int index, int close_all);
	// 关闭游戏机
	int WINAPI Game_CloseGame(int index);
	// 已经入队了
	int WINAPI Game_InTeam(int index);
	// 修改一些设置
	int WINAPI Game_PutSetting(const wchar_t* name, int v);
	// 转移卡号本机
	int WINAPI Game_GetInCard(const wchar_t* card);
	// 验证卡号
	int WINAPI Game_VerifyCard(const wchar_t* card);
	// 查询副本记录
	int WINAPI Game_SelectFBRecord(char*** result, int* col);
	// 喊话
	DLLEXPORT void WINAPI Talk(const char* text, int type);
#ifdef __cplusplus
}
#endif

// 玩游戏
DWORD WINAPI PlayGame(LPVOID param);
// game.run
DWORD WINAPI RunGame(LPVOID param);
// game.listen
DWORD WINAPI Listen(LPVOID param);
// 激活
DWORD WINAPI Verify(LPVOID param);
