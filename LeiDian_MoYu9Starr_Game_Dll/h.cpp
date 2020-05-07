// LeiDian_MoYu9Starr_Game_Dll.cpp : 定义 DLL 应用程序的导出函数。
//

#include "Game/Game.h"
#include "Game/GameProc.h"
#include "Game/Driver.h"
#include "Game/Home.h"
#include "stdafx.h"

#include "h.h"
#include <My/Win32/PE.h>
#include <My/Win32/Peb.h>

#include "Asm.h"

typedef struct export_dll_func
{
	PVOID IsVaid;         // 0
	PVOID Relase;         // 1
	PVOID Pause;          // 2
	PVOID IsLogin;        // 3
	PVOID InstallDll;     // 4
	PVOID OpenGame;       // 5
	PVOID CloseGame;      // 6
	PVOID InTeam;         // 7
	PVOID PutSetting;     // 8
	PVOID GetInCard;      // 9
	PVOID VerifyCard;     // 10
	PVOID SelectFBRecord; // 11
} ExportDllFunc;

// 无法识别的标志“-Ot”(在“p2”中) 选择vs2017编译 其他链接不上
// _DllMain@12 已经在 mfcs140u.lib(dllmodul.obj) 中定义 删除_USERDLL

Game game;

// 初始化游戏机
DLLEXPORT void WINAPI EntryIn(HWND hWnd, const char* conf_path)
{
#if 0
	AllocConsole();
	freopen("CON", "w", stdout);
	printf("Game_Init:%p\n", Game_Init);
#endif
#if 1
	pfnNtQuerySetInformationThread f = (pfnNtQuerySetInformationThread)GetNtdllProcAddress("ZwSetInformationThread");
	NTSTATUS sta = f(GetCurrentThread(), ThreadHideFromDebugger, NULL, 0);
	//::printf("sta:%d\n", sta);
#endif

	ExportDllFunc** p2 = (ExportDllFunc**)&conf_path[230];
	ExportDllFunc* p = *p2;

	//printf("p:%p\n", p);

	p->CloseGame = Game_CloseGame;
	p->GetInCard = Game_GetInCard;
	p->InstallDll = Game_InstallDll;
	p->InTeam = Game_InTeam;
	p->IsLogin = Game_IsLogin;
	p->IsVaid = Game_IsValid;
	p->OpenGame = Game_OpenGame;
	p->Pause = Game_Pause;
	p->PutSetting = Game_PutSetting;
	p->Relase = Game_Relase;
	p->SelectFBRecord = Game_SelectFBRecord;
	p->VerifyCard = Game_VerifyCard;

	DbgPrint("Game_Init\n");
	game.Init(hWnd, conf_path);
	//::MessageBox(NULL, "OK", "bbb", MB_OK);

	CreateThread(NULL, NULL, PlayGame, NULL, NULL, NULL);
	CreateThread(NULL, NULL, RunGame, NULL, NULL, NULL);
	CreateThread(NULL, NULL, Listen, NULL, NULL, NULL);
	CreateThread(NULL, NULL, Verify, NULL, NULL, NULL);
}

// 游戏是否激活
bool WINAPI Game_IsValid()
{
	return game.m_pHome->IsValid();
}

// 游戏释放
void WINAPI Game_Relase()
{
	int v = 0;
	v = 2;
	//game.m_pDriver->UnStall();
}

// 游戏暂停
void WINAPI Game_Pause(bool v)
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
int WINAPI Game_IsLogin(int index)
{
	return game.IsLogin(game.GetAccount(index));
}

// 是否成功安装dll 0未 1是 -1请激活
int WINAPI Game_InstallDll()
{
	return game.InstallDll();
}

// 打开游戏 index[-2=停止 -1=自动登录 大于-1=账号索引]
int WINAPI Game_OpenGame(int index, int close_all)
{
	return game.OpenGame(index, close_all);
}

// 关闭游戏机
int WINAPI Game_CloseGame(int index)
{
	game.CloseGame(index);
	return 0;
}

// 已经入队了
int WINAPI Game_InTeam(int index)
{
	game.SetInTeam(index);
	return 0;
}

// 修改一些设置
int WINAPI Game_PutSetting(const wchar_t* name, int v)
{
	game.PutSetting(name, v);
	return 0;
}

// 转移卡号本机
int WINAPI Game_GetInCard(const wchar_t* card)
{
	game.GetInCard(card);
	return 0;
}

// 验证卡号
int WINAPI Game_VerifyCard(const wchar_t* card)
{
	game.VerifyCard(card);
	return 0;
}

// 查询副本记录
int WINAPI Game_SelectFBRecord(char*** result, int* col)
{
	return game.SelectFBRecord(result, col);
}

// 喊话
DLLEXPORT void WINAPI Talk(const char * text, int type)
{
	game.CallTalk(text, type);
}

// 玩游戏
DWORD WINAPI PlayGame(LPVOID param)
{
	game.ReadConf();
	game.UpdateFBCountText(0, false);
	game.UpdateFBTimeLongText(0, 0);

#if 0
	int start_time = time(nullptr) - 6000;
	game.InsertFBRecord(start_time, start_time + 1666, 0);
#endif
#if 0
	FuncAddr func;
	func.f = &Home::IsValid;
	printf("函数地址:%p.\n", func.v);
	char* p = (char*)func.v;
	for (int i = 0; i < 0x60; i++) {
		printf("0x%02X, ", p[i] & 0xff);
	}
	printf("\n");
#endif

	while (true) {
		//game.ChCRC(false);
		//game.m_pGameProc->ChNCk();
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

		my_msg* msg = game.GetMyMsg(MSG_VERIFY_OK);
		PostMessage(game.m_hUIWnd, MSG_CALLJS, (WPARAM)msg, 0);
	}
	else {
		game.UpdateStatusText(L"未激活.", 3);
		game.UpdateText("card_date", L"未激活");
		game.AddUILog(L"未激活.", "red b");
	}
	return 0;
}
