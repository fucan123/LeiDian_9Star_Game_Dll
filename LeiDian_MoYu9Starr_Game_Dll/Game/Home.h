#pragma once

#include "HttpClient.h"
#include <string>

#define HOME_HOST      L"137.59.149.38"
//#define HOME_HOST      L"www.fuzhu.com"
#define HOME_GAME_FLAG "1"
#define JIAOYAN_V      16899

class Game;
class Home;

typedef bool (Home::*func_valid)();
#ifdef x64
typedef union func_addr {
	func_valid f;
	unsigned __int64 v;
} FuncAddr;
#else 
typedef union func_addr {
	func f;
	unsigned __int v;
} FuncAddr;
#endif

class Home
{
public:
	Home(Game* p);
	// 设置是否免费
	void SetFree(bool v);
	// 是否有效
	bool IsValid();
	bool IsValidS();
	// 转移
	bool GetInCard(const char* card);
	// 充值
	bool Recharge(const char* card);
	// 验证
	bool Verify();
	// 获得此机器有效时间（秒）
	int  GetExpire();
	// 解析返回结果
	void Parse(const char* msg);
	// 获得返回结果中的值
	bool GetValue(char* key, char value[], int length);
	// 获得返回结果中的值
	int  GetValue(char* key, int length, int default_value=0);
	// 获得返回结果中的值
	int  GetValue(const char* key, int length, int default_value = 0);
	// 设置验证时间
	int  SetVerifyTime();
	// 设置到期时间
	int  SetEndTime();
	// 设置到期时间字符串
	void SetExpireTime_S(time_t expire_time);
	// 设置过期时间
	void SetExpire(int t);
	// 获得DES密钥
	void GetDesKey(char save[17]);
	// 获得到期时间字符串
	std::string& GetExpireTime_S();
	void SetError(int code, const char* str);
	void SetError(int code, wchar_t* str);
	void SetError(int code, CString str);
	void SetError(int code, const char* str, DWORD status);
	void SetError(int code, wchar_t* str, DWORD status);
	void SetErrorCode(int code);
	void SetMsgStr(wchar_t* str);
	void SetMsgStr(CString str);
	void SetMsgStr(wchar_t* str, DWORD status);
	void SetMsgStr(const char* str, DWORD status);
	const char* GetMsgStr();

	// 是否有效
	bool IsValid2();
public:
	Game* m_pGame;

	char m_NoUse[128];

	int m_nVerifyNum = 0;
	// 返回结果指针
	char m_pRepsone[256];
	// 验证时间（秒）
	int   m_iVerifyTime;
	// 过期时间
	int m_iExpire;
	// 到期时间
	int   m_iEndTime;

	char m_MachineId[33];
	int  m_Status;
	int  m_Error;

	// 是否免费
	bool m_bFree = false;

	// 网站返回内容
	std::string m_sResult;
	// 提示内容
	char m_MsgStr[128];
	// 过期时间
	std::string m_sExpireTime;
};