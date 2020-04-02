#include "Game.h"
#include "Driver.h"
#include <My/Common/func.h>
#include <stdio.h>
#include <shellapi.h>

#define FSFILTER_DRIVER_NAME "DriverFs999"

// ...
Driver::Driver(Game * p)
{
	m_pJsCall = p->m_pJsCall;
	m_pGame = p;
}

BOOL Driver::InstallFsFilter(const char* path, const char * lpszDriverPath, const char * lpszAltitude)
{
	char    szTempStr[MAX_PATH];
	HKEY    hKey;
	DWORD    dwData;
	char    szDriverImagePath[MAX_PATH];

	if (NULL == lpszDriverPath)
	{
		return FALSE;
	}
	//得到完整的驱动路径
	//GetFullPathNameA(lpszDriverPath, MAX_PATH, szDriverImagePath, NULL);

	sprintf_s(szDriverImagePath, "%s\\files\\%s", path, lpszDriverPath);
	printf("szDriverImagePath:%s\n", szDriverImagePath);

	SC_HANDLE hServiceMgr = NULL;// SCM管理器的句柄
	SC_HANDLE hService = NULL;// NT驱动程序的服务句柄

	//打开服务控制管理器
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hServiceMgr == NULL)
	{
		// OpenSCManager失败
		printf(" OpenSCManager失败\n");
		CloseServiceHandle(hServiceMgr);
		return FALSE;
	}

	// OpenSCManager成功  

	//创建驱动所对应的服务
	hService = CreateServiceA(hServiceMgr,
		FSFILTER_DRIVER_NAME,             // 驱动程序的在注册表中的名字
		FSFILTER_DRIVER_NAME,             // 注册表驱动程序的DisplayName 值
		SERVICE_ALL_ACCESS,         // 加载驱动程序的访问权限
		SERVICE_FILE_SYSTEM_DRIVER, // 表示加载的服务是文件系统驱动程序
		SERVICE_DEMAND_START,       // 注册表驱动程序的Start 值
		SERVICE_ERROR_IGNORE,       // 注册表驱动程序的ErrorControl 值
		szDriverImagePath,          // 注册表驱动程序的ImagePath 值
		"FSFilter Activity Monitor",// 注册表驱动程序的Group 值
		NULL,
		"FltMgr",                   // 注册表驱动程序的DependOnService 值
		NULL,
		NULL);

	if (hService == NULL)
	{
		if (GetLastError() == ERROR_SERVICE_EXISTS)
		{
			//服务创建失败，是由于服务已经创立过
			printf("服务创建失败，是由于服务已经创立过\n");
			CloseServiceHandle(hService);       // 服务句柄
			CloseServiceHandle(hServiceMgr);    // SCM句柄
			return TRUE;
		}
		else
		{
			printf(" 服务创建失败 %d\n", GetLastError());
			CloseServiceHandle(hService);       // 服务句柄
			CloseServiceHandle(hServiceMgr);    // SCM句柄
			return FALSE;
		}
	}
	CloseServiceHandle(hService);       // 服务句柄
	CloseServiceHandle(hServiceMgr);    // SCM句柄

	//-------------------------------------------------------------------------------------------------------
	// SYSTEM\\CurrentControlSet\\Services\\DriverName\\Instances子健下的键值项 
	//-------------------------------------------------------------------------------------------------------
	strcpy(szTempStr, "SYSTEM\\CurrentControlSet\\Services\\");
	strcat(szTempStr, FSFILTER_DRIVER_NAME);
	strcat(szTempStr, "\\Instances");
	if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, szTempStr, 0, (LPSTR)"", TRUE, KEY_ALL_ACCESS, NULL, &hKey, (LPDWORD)&dwData) != ERROR_SUCCESS)
	{
		printf("HKEY_LOCAL_MACHINE %s创建失败\n", FSFILTER_DRIVER_NAME);
		return FALSE;
	}
	// 注册表驱动程序的DefaultInstance 值 
	strcpy(szTempStr, FSFILTER_DRIVER_NAME);
	strcat(szTempStr, " Instance");
	if (RegSetValueExA(hKey, "DefaultInstance", 0, REG_SZ, (CONST BYTE*)szTempStr, (DWORD)strlen(szTempStr)) != ERROR_SUCCESS)
	{
		printf("HKEY_LOCAL_MACHINE Set失败\n");
		return FALSE;
	}
	RegFlushKey(hKey);//刷新注册表
	RegCloseKey(hKey);
	//-------------------------------------------------------------------------------------------------------

	//-------------------------------------------------------------------------------------------------------
	// SYSTEM\\CurrentControlSet\\Services\\DriverName\\Instances\\DriverName Instance子健下的键值项 
	//-------------------------------------------------------------------------------------------------------
	strcpy(szTempStr, "SYSTEM\\CurrentControlSet\\Services\\");
	strcat(szTempStr, FSFILTER_DRIVER_NAME);
	strcat(szTempStr, "\\Instances\\");
	strcat(szTempStr, FSFILTER_DRIVER_NAME);
	strcat(szTempStr, " Instance");
	if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, szTempStr, 0, (LPSTR)"", TRUE, KEY_ALL_ACCESS, NULL, &hKey, (LPDWORD)&dwData) != ERROR_SUCCESS)
	{
		printf("HKEY_LOCAL_MACHINE %s %s创建失败\n", FSFILTER_DRIVER_NAME, FSFILTER_DRIVER_NAME);
		return FALSE;
	}
	// 注册表驱动程序的Altitude 值
	strcpy(szTempStr, lpszAltitude);
	if (RegSetValueExA(hKey, "Altitude", 0, REG_SZ, (CONST BYTE*)szTempStr, (DWORD)strlen(szTempStr)) != ERROR_SUCCESS)
	{
		printf("Altitude Set失败\n");
		return FALSE;
	}
	// 注册表驱动程序的Flags 值
	dwData = 0x0;
	if (RegSetValueExA(hKey, "Flags", 0, REG_DWORD, (CONST BYTE*)&dwData, sizeof(DWORD)) != ERROR_SUCCESS)
	{
		printf("Altitude Set失败2\n");
		return FALSE;
	}
	RegFlushKey(hKey);//刷新注册表
	RegCloseKey(hKey);
	//-------------------------------------------------------------------------------------------------------

	return TRUE;
}

// 启动文件过滤保护
BOOL Driver::StartFsFilter()
{
	SC_HANDLE        schManager;
	SC_HANDLE        schService;
	SERVICE_STATUS    svcStatus;

	schManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == schManager)
	{
		printf("schManager=NULL %d\n", GetLastError());
		CloseServiceHandle(schManager);
		return FALSE;
	}
	schService = OpenServiceA(schManager, FSFILTER_DRIVER_NAME, SERVICE_ALL_ACCESS);
	if (NULL == schService)
	{
		printf("schService=NULL %d\n", GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return FALSE;
	}

	if (!StartService(schService, 0, NULL))
	{
		printf("!StartService %d\n", GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		if (GetLastError() == ERROR_SERVICE_ALREADY_RUNNING)
		{
			// 服务已经开启
			return TRUE;
		}
		return FALSE;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);

	return TRUE;
}

// 停止文件保护驱动
BOOL Driver::StopFsFilter()
{
	SC_HANDLE        schManager;
	SC_HANDLE        schService;
	SERVICE_STATUS    svcStatus;
	bool            bStopped = false;

	schManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == schManager)
	{
		return FALSE;
	}
	schService = OpenServiceA(schManager, FSFILTER_DRIVER_NAME, SERVICE_ALL_ACCESS);
	if (NULL == schService)
	{
		CloseServiceHandle(schManager);
		return FALSE;
	}
	if (!ControlService(schService, SERVICE_CONTROL_STOP, &svcStatus) && (svcStatus.dwCurrentState != SERVICE_STOPPED))
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return FALSE;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);

	return TRUE;
}

// 删除文件保护驱动
BOOL Driver::DeleteFsFilter()
{
	SC_HANDLE        schManager;
	SC_HANDLE        schService;
	SERVICE_STATUS    svcStatus;

	schManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == schManager)
	{
		return FALSE;
	}
	schService = OpenServiceA(schManager, FSFILTER_DRIVER_NAME, SERVICE_ALL_ACCESS);
	if (NULL == schService)
	{
		CloseServiceHandle(schManager);
		return FALSE;
	}
	ControlService(schService, SERVICE_CONTROL_STOP, &svcStatus);
	if (!DeleteService(schService))
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return FALSE;
	}
	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);

	return TRUE;
}

// 安装驱动
bool Driver::InstallDriver(const char* path)
{
	CString file;
	file = path;
	file += L"\\files\\firenet.sys";

	if (!IsFileExist(file)) {
		//m_pJsCall->ShowMsg("缺少必需文件:firenet.sys", "文件不存在", 2);
		LOG2(L"firenet.sys不存在", "red");
		m_bIsInstallDll = false;
		char kill[32];
		sprintf_s(kill, "taskkill /f /t /pid %d", GetCurrentProcessId());
		system(kill);
		TerminateProcess(GetCurrentProcess(), 4);
		return false;
	}


	bool is_try = false;
_try_install_:
	if (m_SysDll.Install(L"firenet_safe", L"safe fire", file)) {
		LOG2(L"安装驱动成功", "green");
		return true;
	}
	else {
		if (!is_try) {
			is_try = true;
			LOG2(L"安装驱动失败, 准备重新尝试.", "red");
#if 1
			system("sc stop firenet_safe");
			system("sc delete firenet_safe");
#else
			ShellExecuteA(NULL, "open", "cmd", "/C sc stop firenet_safe", NULL, SW_HIDE);
			ShellExecuteA(NULL, "open", "cmd", "/C sc delete firenet_safe", NULL, SW_HIDE);
#endif
			goto _try_install_;
		}
		else {
			m_SysDll.UnInstall();
			char kill[32];
			sprintf_s(kill, "taskkill /f /t /pid %d", GetCurrentProcessId());
			system(kill);
			TerminateProcess(GetCurrentProcess(), 4);
			//LOG2(L"安装驱动失败, 请重启本程序再尝试.", "red");
		}

		//MessageBox(NULL, "安装驱动失败", "提示", MB_OK);
		return false;
	}
}

// 安装Dll驱动
bool Driver::InstallDll(const char* path)
{
	if (!m_bIsInstallDll) {
		SetDll(path);
		m_bIsInstallDll = true;
	}
	else {
		if (DelDll()) {
			LOG2(L"删除dll成功", "green");
			m_bIsInstallDll = false;
			return true;
		}
		else {
#if 0
			system("sc stop firenet_safe");
			system("sc delete firenet_safe");
#else
			Delete(L"firenet_safe");
#endif
			InstallDriver(path);
			if (!DelDll()) {
				LOGVARN2(32, "red", L"删除dll失败:%d", GetLastError());
				return false;
			}
			//MessageBox(NULL, "卸载驱动失败", "提示", MB_OK);
			return true;
		}
	}

}

// 卸载
bool Driver::UnStall()
{
	//StopFsFilter();
	//DeleteFsFilter();

	return m_SysDll.UnInstall();
}

// 设置要注入的DLL
bool Driver::SetDll(const char* path)
{
	BOOL	result;
	DWORD	returnLen;
	char	output;

	HANDLE	hDevice = NULL;

	PVOID	dllx64Ptr = NULL;
	PVOID	dllx86Ptr = NULL;

	ULONG	dllx64Size = 0;
	ULONG	dllx86Size = 0;

	hDevice = CreateFileA("\\\\.\\CrashDumpUpload",
		NULL,
		NULL,
		NULL,
		OPEN_EXISTING,
		NULL,
		NULL);

	if (hDevice == INVALID_HANDLE_VALUE) {
		//m_pJsCall->ShowMsg("连接驱动失败.", "提示", 2);
		return false;
	}

	DWORD pid = GetCurrentProcessId();
	result = DeviceIoControl(
		hDevice,
		IOCTL_SET_PROTECT_PID,
		&pid,
		4,
		&output,
		sizeof(char),
		&returnLen,
		NULL);
	printf("保护进程ID:%d %d\n", pid, result);

	// 设置隐藏进程
	SetHidePid(pid);

	char file[255];
	sprintf_s(file, "%s\\files\\9Star.dll", path);
	dllx86Ptr = MyReadFile(file, &dllx86Size);
	if (dllx86Ptr == NULL) {
		LOG2(L"找不到文件9Star.dll", "red");
		return false;
	}
	// 1234
	// 1 2 3 4 5 6 7 8 9 10
	// 11 12 13 14 15

	result = DeviceIoControl(
		hDevice,
		IOCTL_SET_INJECT_X86DLL,
		dllx86Ptr,
		dllx86Size,
		&output,
		sizeof(char),
		&returnLen,
		NULL);

	if (dllx86Ptr)
	{
		free(dllx86Ptr);
	}
	if (result) {
		LOG2(L"设置dll成功", "green");
	}
	else {
		//LOGVARN2(32, "red", L"GetLastError:%d.\n", GetLastError());
		LOG2(L"设置dll失败", "red");
	}

	if (hDevice != NULL) {
		CloseHandle(hDevice);
	}
	return true;
}

// 删除要注入dll
bool Driver::DelDll()
{
	HANDLE hDevice = CreateFileA("\\\\.\\CrashDumpUpload",
		NULL,
		NULL,
		NULL,
		OPEN_EXISTING,
		NULL,
		NULL);

	if (hDevice == INVALID_HANDLE_VALUE) {
		return false;
	}

	int     inv = 0;
	char	output;
	DWORD	returnLen;
	BOOL result = DeviceIoControl(
		hDevice,
		IOCTL_DEL_INJECT_X86DLL,
		&inv,
		sizeof(int),
		&output,
		sizeof(char),
		&returnLen,
		NULL);

	CloseHandle(hDevice);
	return result;
}

// 读取文件
PVOID Driver::MyReadFile(const CHAR* fileName, PULONG fileSize)
{
	HANDLE fileHandle = NULL;
	DWORD readd = 0;
	PVOID fileBufPtr = NULL;

	fileHandle = CreateFileA(
		fileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		*fileSize = 0;
		return NULL;
	}

	*fileSize = GetFileSize(fileHandle, NULL);

	fileBufPtr = calloc(1, *fileSize);

	if (!ReadFile(fileHandle, fileBufPtr, *fileSize, &readd, NULL))
	{
		free(fileBufPtr);
		fileBufPtr = NULL;
		*fileSize = 0;
	}

	CloseHandle(fileHandle);
	return fileBufPtr;
}

// 设置保护进程ID
void Driver::SetProtectPid(DWORD pid)
{
	pid = pid ? pid : GetCurrentProcessId();
	HANDLE hDevice = CreateFileA("\\\\.\\CrashDumpUpload",
		NULL,
		NULL,
		NULL,
		OPEN_EXISTING,
		NULL,
		NULL);

	if (hDevice == INVALID_HANDLE_VALUE) {
		LOGVARN2(32, "red b", L"连接驱动失败Protect:%d", pid);
		return;
	}

	char	output;
	DWORD	returnLen;
	BOOL result = DeviceIoControl(
		hDevice,
		IOCTL_SET_PROTECT_PID,
		&pid,
		4,
		&output,
		sizeof(char),
		&returnLen,
		NULL);
	printf("保护进程ID:%d %d\n", pid, result);
	LOGVARN2(32, "green b", L"保护进程:%d", pid);

	CloseHandle(hDevice);
}


// 设置模拟器进程ID
void Driver::SetProtectVBoxPid(DWORD pid)
{
	HANDLE hDevice = CreateFileA("\\\\.\\CrashDumpUpload",
		NULL,
		NULL,
		NULL,
		OPEN_EXISTING,
		NULL,
		NULL);

	if (hDevice == INVALID_HANDLE_VALUE) {
		LOGVARN2(32, "red b", L"连接驱动失败VBox:%d", pid);
		return;
	}

	char	output;
	DWORD	returnLen;
	BOOL result = DeviceIoControl(
		hDevice,
		IOCTL_SET_PROTECT_VBOX_PID,
		&pid,
		4,
		&output,
		sizeof(char),
		&returnLen,
		NULL);
	printf("保护模拟器进程ID:%d %d\n", pid, result);
	LOGVARN2(32, "green b", L"保护模拟器进程:%d", pid);

	CloseHandle(hDevice);
}

// 设置隐藏进程ID
void Driver::SetHidePid(DWORD pid)
{
	return;
	DWORD dwWinLogonId = SGetProcessId(L"winlogon.exe");
	if (!dwWinLogonId) {
		dwWinLogonId = SGetProcessId(L"WINLOGON.exe");
	}
	if (!dwWinLogonId)
		return;

	if (m_pGame->m_Setting.NoHideProc)
		return;
	if (0 && pid == GetCurrentProcessId())
		return;

	wchar_t log[64];
	HANDLE hDevice = CreateFileA("\\\\.\\CrashDumpUpload",
		NULL,
		NULL,
		NULL,
		OPEN_EXISTING,
		NULL,
		NULL);

	if (hDevice == INVALID_HANDLE_VALUE) {
		LOGVARP2(log, "red b", L"连接驱动失败Hide:%d", pid);
		return;
	}

	DWORD pids[2] = { pid, dwWinLogonId };
	char	output;
	DWORD	returnLen;
	BOOL result = DeviceIoControl(
		hDevice,
		IOCTL_SET_HIDE_PID,
		pids,
		sizeof(pids),
		&output,
		sizeof(char),
		&returnLen,
		NULL);

	printf("隐藏进程ID:%d-%d(%d)\n", pid, dwWinLogonId, result);
	LOGVARP2(log, "green b", L"隐藏进程:%d", pid);

	CloseHandle(hDevice);
}

// 删除驱动服务
bool Driver::Delete(const wchar_t* name)
{
	SC_HANDLE        schManager;
	SC_HANDLE        schService;
	SERVICE_STATUS    svcStatus;

	schManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == schManager)
		return false;

	schService = OpenService(schManager, name, SERVICE_ALL_ACCESS);
	if (NULL == schService) {
		LOG2(L"NULL == schService", "red");
		CloseServiceHandle(schManager);
		return false;
	}
	if (!ControlService(schService, SERVICE_CONTROL_STOP, &svcStatus)) {
		//LOG2(L"!ControlService", "red");
		//return false;
	}	

	if (!DeleteService(schService)) {
		LOG2(L"!DeleteServic", "red");
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return false;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);
	return true;
}
