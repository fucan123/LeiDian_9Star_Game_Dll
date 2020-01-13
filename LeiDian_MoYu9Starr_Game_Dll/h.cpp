// LeiDian_MoYu9Starr_Game_Dll.cpp : 定义 DLL 应用程序的导出函数。
//

#include "Game/Game.h"
#include "Game/GameProc.h"
#include "Game/Driver.h"
#include "stdafx.h"

#include "h.h"

// 无法识别的标志“-Ot”(在“p2”中) 选择vs2017编译 其他链接不上
// _DllMain@12 已经在 mfcs140u.lib(dllmodul.obj) 中定义 删除_USERDLL

Game game;

// 初始化游戏机
void WINAPI Game_Init(HWND hWnd, const char* conf_path)
{
#if 0
	AllocConsole();
	freopen("CON", "w", stdout);
#endif

	printf("Game_Init\n");
	game.Init(hWnd, conf_path);
	//::MessageBoxA(NULL, "OK", "bbb", MB_OK);

	CreateThread(NULL, NULL, PlayGame, NULL, NULL, NULL);
	CreateThread(NULL, NULL, RunGame, NULL, NULL, NULL);
	CreateThread(NULL, NULL, Listen, NULL, NULL, NULL);
	CreateThread(NULL, NULL, Verify, NULL, NULL, NULL);
}

// 游戏释放
DLLEXPORT void WINAPI Game_Relase()
{
	game.m_pDriver->UnStall();
}

// 游戏暂停
DLLEXPORT void WINAPI Game_Pause(bool v)
{
	HWND hWndTop = ::GetForegroundWindow();
	if (game.m_pBig && game.m_pBig->Mnq) {
		if (hWndTop == game.m_pBig->Mnq->WndTop || hWndTop == game.m_pBig->Mnq->Wnd) {
			game.m_pGameProc->m_bPause = v;

			if (v) {
				DbgPrint("游戏暂停\n");
				game.AddUILog(L"游戏暂停", "red");
			}
			else {
				DbgPrint("游戏继续\n");
				game.AddUILog(L"游戏继续", "green");
			}
		}
	}
}

// 帐号是否登录
DLLEXPORT int WINAPI Game_IsLogin(int index)
{
	return game.IsLogin(game.GetAccount(index));
}

// 是否成功安装dll 0未 1是 -1请激活
DLLEXPORT int WINAPI Game_InstallDll()
{
	return game.InstallDll();
}

// 打开游戏 index[-2=停止 -1=自动登录 大于-1=账号索引]
DLLEXPORT int WINAPI Game_OpenGame(int index, int close_all)
{
	return game.OpenGame(index, close_all);
}

// 关闭游戏机
DLLEXPORT int WINAPI Game_CloseGame(int index)
{
	game.CloseGame(index);
	return 0;
}

// 已经入队了
DLLEXPORT int WINAPI Game_InTeam(int index)
{
	game.SetInTeam(index);
	return 0;
}

// 修改一些设置
DLLEXPORT int WINAPI Game_PutSetting(const wchar_t* name, int v)
{
	game.PutSetting(name, v);
	return 0;
}

// 验证卡号
DLLEXPORT int WINAPI Game_VerifyCard(const wchar_t* card)
{
	game.VerifyCard(card);
	return 0;
}

// 玩游戏
DWORD WINAPI PlayGame(LPVOID param)
{
	game.ReadConf();
	game.UpdateFBCountText(0, false);
	game.UpdateFBTimeLongText(0, 0);

	while (true) {
		game.CheckLoginTimeOut();
		Sleep(8000);
	}
	return 0;
}

// game.run
DWORD WINAPI RunGame(LPVOID param)
{
	game.Run();
	return 0;
}
// game.listen
DWORD WINAPI Listen(LPVOID param)
{
	game.Listen(12379);
	return 0;
}

// 激活
DWORD WINAPI Verify(LPVOID param)
{
	game.AddUILog(L"验证卡号...", "c0 b");
	game.UpdateStatusText(L"正在激活.", 4);
	if (game.m_pHome->Verify()) {
		game.UpdateStatusText(L"激活成功.", 2);
		game.UpdateText("card_date", game.m_pHome->GetExpireTime_S().c_str());
		game.AddUILog(L"验证成功.", "green b");
	}
	else {
		game.UpdateStatusText(L"未激活.", 3);
		game.UpdateText("card_date", L"未激活");
		game.AddUILog(L"未激活.", "red b");
	}
	return 0;
}
