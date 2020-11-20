#include "Game.h"
#include "GameData.h"
#include "Emulator.h"
#include "Move.h"
#include "Talk.h"
#include "GameProc.h"
#include "PrintScreen.h"
#include <My/Common/func.h>
#include <My/Win32/Peb.h>
#include <time.h>

#define MNQ_TITLE "�׵�ģ����"
#define MNQ_PNAME L"LdBoxHeadless.exe"

GameData::GameData(Game* p)
{
	m_pGame = p;
	m_pReadBuffer = new BYTE[0x10000];

	ZeroMemory(&m_DataAddr, sizeof(m_DataAddr));

	CreateShareMemory();
}

// ������Ϸ
void GameData::WatchGame()
{
	wchar_t log[128];
	while (true) {
		int length = m_pGame->m_pEmulator->List2();
		//DbgPrint("ģ��������:%d\n", length);
		for (int i = 0; i < length; i++) {
			MNQ* m = m_pGame->m_pEmulator->GetMNQ(i);
			if (!m || m->VBoxPid <= 0) {
				//DbgPrint("ģ����δ����\n");
				continue;
			}
				
			if (!m->Account || !m->Account->IsLogin || m->Account->IsGetAddr) {
				//DbgPrint("�ʺ�δ��¼:%p %d %d\n", m->Account, m->Account->IsLogin, m->Account->IsGetAddr);
				continue;
			}

			// ������Ϸ����
			m_pGame->m_pEmulator->ReGameWndSize(m->Index);
				
			ZeroMemory(&m_DataAddr, sizeof(m_DataAddr));
			m_hGameProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m->VBoxPid);
			DbgPrint("%hs��ȡ[%hs(%d)]����\n", m->Account->Name, m->Name, m->VBoxPid);
			LOGVARP2(log, "c0", L"%hs��ȡ[%hs(%d)]����", m->Account->Name, m->Name, m->VBoxPid);
			ReadGameMemory();

			if (m_DataAddr.Player) {
				m->Process = m_hGameProcess;
				m_pGame->SetGameAddr(m->Account, &m_DataAddr);

				while (m_dwX == 0) {
					ReadCoor(nullptr, nullptr, m->Account);
					Sleep(100);
				}

				m->Account->PlayTime = time(nullptr);

				char name[16] = { 0 };
				if (ReadName(name, m->Account)) {
					if (m_dwX && m_dwY) {
						while (true) {
							Sleep(5000);
							if (m_DataAddr.MoveX)
								break;

							m_pGame->m_pGameProc->SwitchGameWnd(m->Wnd);
							m_pGame->m_pTalk->CloseLGCBox();
							m_pGame->m_pMove->OpenMap(m->Account);
							Sleep(100);
							m_pGame->m_pGameProc->Click(552, 300, 600, 350, 0xff, m->Wnd); // �����ͼ�ſ���������Ŀ�ĵ�������
							Sleep(100);
							DbgPrint("%hsѰ��Ŀ�ĵص�ַ..................\n", m->Account->Name);
							LOGVARP2(log, "c0", L"%hsѰ��Ŀ�ĵص�ַ...", m->Account->Name);
							ReadGameMemory();
							m_pGame->m_pTalk->CloseLGCBox();
							m_pGame->m_pMove->CloseMap(m->Wnd);
						}
						
					}
					if (m->Account->IsBig) {
						m_hWndBig = m->Wnd;
						m_hProcessBig = m_hGameProcess;

						m_pGame->m_hWndBig = m->Wnd;
						m_pGame->m_hProcessBig = m_hGameProcess;

						m_pAccoutBig = m->Account;
						m_pGame->m_pPrintScreen->InjectVBox(m_pGame->m_chPath, m->UiPid);
					}

					if (!m_DataAddr.PicScale)
						ReadGameMemory();

					m_pGame->SetGameAddr(m->Account, &m_DataAddr);
					strcpy(m->Account->Role, name);
					ReadLife(nullptr, nullptr, m->Account);
					DbgPrint("%hs ��ɫ����:%hs ����:%d,%d Ѫ��:%d/%d %hs\n",
						m->Account->Name, m->Account->Role, m_dwX, m_dwY,
						m_dwLife, m_dwLifeMax,
						m->Account->IsBig ? "���" : "С��");
					LOGVARP2(log, "green", L"%hs ��ɫ����:%hs ����:%d,%d Ѫ��:%d/%d %hs\n",
						m->Account->Name, m->Account->Role, m_dwX, m_dwY,
						m_dwLife, m_dwLifeMax,
						m->Account->IsBig ? "���" : "С��");
					
					if (m->Account->IsBig)
						goto _end_;
				}
			}
			else {
				CloseHandle(m_hGameProcess);
			}	
		}
		Sleep(8000);
	}
_end_:
	return;
}

// ��ȡ��Ϸ����
void GameData::FindGameWnd()
{
	::EnumWindows(EnumProc, (LPARAM)this);
}

// ö�ٴ���
BOOL GameData::EnumProc(HWND hWnd, LPARAM lParam)
{
	char name[32] = { 0 };
	::GetWindowTextA(hWnd, name, sizeof(name));
	if (strcmp(name, MNQ_TITLE) == 0) {
		HWND children = ::FindWindowEx(hWnd, NULL, L"RenderWindow", NULL);
		printf("%hs:%08X.%08X\n", name, hWnd, children);
		bool isbig = false;
		bool set = ((GameData*)lParam)->m_pGame->SetGameWnd(children, &isbig);
		if (isbig) {
			((GameData*)lParam)->m_hWndBig = children;
		}
		return set ? FALSE : TRUE;
	}
	return TRUE;
}

// �Ƿ���ָ���������� allow=���
bool GameData::IsInArea(int x, int y, int allow, _account_* account)
{
	DWORD pos_x = 0, pos_y = 0;
	ReadCoor(&pos_x, &pos_y, account);
	if (allow == 0)
		return x == pos_x && y == pos_y;

	int cx = (int)pos_x - x;
	int cy = (int)pos_y - y;

	//printf("IsInArea:%d,%d(%d,%d) %d,%d\n", pos_x, pos_y, x, y, cx, cy);

	return abs(cx) <= allow && abs(cy) <= allow;
}

// �Ƿ���ָ���������� allow=���
bool GameData::IsNotInArea(int x, int y, int allow, _account_* account)
{
	ReadCoor(nullptr, nullptr, account);
	int cx = (int)m_dwX - x;
	int cy = (int)m_dwY - y;

	return abs(cx) > allow || abs(cy) > allow;
}

// �Ƿ������
bool GameData::IsInShenDian(_account_* account)
{
	return IsInArea(60, 60, 30, account);
}

// �Ƿ�����������
bool GameData::IsInShenYu(_account_ * account)
{
	return IsInArea(600, 300, 30, account);
}

// �Ƿ��ڸ����ſ�
bool GameData::IsInFBDoor(_account_* account)
{
	return IsInArea(865, 500, 50, account);
}

// ��ȡ�����׵�ַ
bool GameData::FindPlayerAddr()
{
	//LOG2(L"FindPlayerAddr\n", "c6");
	// 4:0x073B1190 4:0x00 4:0xFFFFFFFF 4:0x3F800000 4:0x00010001 4:0x000 4:0x072C4678 4:0x9A078100

	// 4:0x073B1190 4:0x0000DECE 4:0x00000000 4:0x00000001 4:0x00000000 4:0x00000030 4:0x00000000 4:0x0000DECE 4:0x00000000 4:0x00000001 4:0x00000000 4:0x00000030
	DWORD codes[] = {
		0x079A1E88, 0x00000000, 0xFFFFFFFF, 0x3F800000,
		0x00010001, 0x00000000, 0x079A2028, 0x00000011,
	}; // 073AE2C8

	DWORD address = 0;
	DWORD result = SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address);
	if (!result) {
		codes[4] = 0x00010000;
		result = SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address);
	}
	
	if (result) {
		DWORD data = 0;
		bool result = ReadProcessMemory(m_hGameProcess, (LPVOID)address, &data, sizeof(data), NULL);
		//printf("�����׵�ַ:%08X Data:%08X\n", address, data);
		//LOGVARN2(32, "blue", L"�����׵�ַ:%08X Data:%08X", m_DataAddr.Player, data);
		if ((address & 0x0fff) == 0x0000) {
			m_DataAddr.Player = address;
			m_DataAddr.Name = m_DataAddr.Player + NAME_OFFSET;
			m_DataAddr.CoorX = m_DataAddr.Player + X_OFFSET;
			m_DataAddr.CoorY = m_DataAddr.Player + Y_OFFSET;
			m_DataAddr.Life = m_DataAddr.Player + LIFE_OFFSET;
			m_DataAddr.LifeMax = m_DataAddr.Player + LIFE_MAX_OFFSET;

			LOGVARN2(32, "blue", L"�����׵�ַ:%08X", m_DataAddr.Player);
			DbgPrint("�����׵�ַ:%08X\n", m_DataAddr.Player);
		}
	}

	return address != 0;
}

// ��ȡĿ�ĵ������ַ
bool GameData::FindMoveCoorAddr()
{
	// 4:0x00 4:0x00 4:0x00 4:0x00 4:0x00 4:0x00 4:0x00 4:0x000014DB
	DWORD codes[] = {
		0x00000011, 0x00000011, 0x00000011, 0x00000011,
		0x00000011, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000022,
		0x00000022, 0x00000022, 0x00000022, 0x0000110E,
		0x00000022, 0x00000011, 0x00000011, 0x00000000,
	};
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		DWORD data[sizeof(codes)/sizeof(DWORD)];
		bool result = ReadProcessMemory(m_hGameProcess, (LPVOID)address, &data, sizeof(data), NULL);
		//LOGVARN2(32, "blue", L"Ŀ�ĵ������ַ:%08X %08X", address, address&0x0f);
		if (data[0x0B] > 0x100 && data[0x0B] < 0x100000
			&& data[0x0C] > 30 && data[0x0C] < 1000 && data[0x0D] > 30 && data[0x0D] < 1060 && data[0x10] < 0x10
			&& address > 0x10000000 && (address&0x0f) == 0x08) {
			m_DataAddr.MoveX = address + 0x30;
			m_DataAddr.MoveY = m_DataAddr.MoveX + 4;

			LOGVARN2(32, "blue", L"Ŀ�ĵ������ַ:%08X", m_DataAddr.MoveX);
			DbgPrint("Ŀ�ĵ������ַ:%08X\n", m_DataAddr.MoveX);
		}
	}

	return address != 0;
}

// ��ȡ����������ֵ��ַ
bool GameData::FindPicScale()
{
	// 0x00000022, 0x00000600, 0x00000360, 0x12340000,
	DWORD codes[] = {
		0x00000000, 0x00000000, 0x00000000, 0x3F800000,
		0x00000022, 0x00001234, 0x00001234, 0x12340000,
	};
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		DWORD data[4] = { 0 };
		bool result = ReadProcessMemory(m_hGameProcess, LPVOID(address + 0x10), &data, sizeof(data), NULL);
		//LOGVARN2(32, "blue", L"�������ŵ�ַf:%08X", address + 0x10);
		if (0 && address > 0x4DEC9500 && address < 0x4DEC9600) {
			printf("�������ŵ�ַf:%08X %d %d %d\n", address + 0x10, data[0], data[1], data[2]);
		}
		
		if (data[0] >= 128 && data[0] <= 256 
			&& data[1] >= 0x300 && data[1] <= 0x600 
			&& data[2] >= 0x1B0 && data[2] <= 0x360) {
			m_DataAddr.PicScale = address + 0x10;

			LOGVARN2(32, "blue", L"�������ŵ�ַ:%08X", m_DataAddr.PicScale);
		}
	}

	return address != 0;
}

// ��ȡ��ɫ
bool GameData::ReadName(char* name, _account_* account)
{
	account = account ? account : m_pAccoutBig;
	if (!ReadMemory((PVOID)account->Addr.Name, name, 16, account)) {
		DbgPrint("�޷���ȡ��ɫ����(%d)\n", GetLastError());
		LOGVARN2(32, "red", L"�޷���ȡ��ɫ����(%d)", GetLastError());
		return false;
	}
	return true;
}

// ��ȡ����
bool GameData::ReadCoor(DWORD * x, DWORD * y, _account_* account)
{
	account = account ? account : m_pAccoutBig;
	if (!account->Addr.CoorX || !account->Addr.CoorY)
		return false;

	DWORD pos_x = 0, pos_y = 0;
	if (!ReadDwordMemory(account->Addr.CoorX, pos_x, account)) {
		::printf("�޷���ȡ����X(%d) %08X\n", GetLastError(), account->Addr.CoorX);
		return false;
	}
	if (!ReadDwordMemory(account->Addr.CoorY, pos_y, account)) {
		::printf("�޷���ȡ����Y(%d) %08X\n", GetLastError(), account->Addr.CoorY);
		return false;
	}

	m_dwX = pos_x;
	m_dwY = pos_y;
	if (x) {
		*x = pos_x;
	}
	if (y) {
		*y = pos_y;
	}
	return true;
}

// ��ȡ����ֵ
DWORD GameData::ReadLife(DWORD* life, DWORD* life_max, _account_* account)
{
	account = account ? account : m_pAccoutBig;
	ReadDwordMemory(account->Addr.Life, m_dwLife, account);
	ReadDwordMemory(account->Addr.LifeMax, m_dwLifeMax, account);

	if (life) {
		*life = m_dwLife;
	}
	if (life_max) {
		*life_max = m_dwLifeMax;
	}
	return m_DataAddr.Life;
}

// ���������ڴ�
void GameData::CreateShareMemory()
{
	m_hShareMap = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 1024, L"Share_Write_XY");
	if (!m_hShareMap) {
		m_pShareBuffer = nullptr;
		DbgPrint("CreateFileMappingʧ��\n");
		LOGVARN2(32, "red", L"CreateFileMappingʧ��(%d)", GetLastError());
		return;
	}
	// ӳ������һ����ͼ���õ�ָ�����ڴ��ָ�룬�������������
	m_pShareBuffer = (ShareWriteXYData*)::MapViewOfFile(m_hShareMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	// ��ʼ��
	ZeroMemory(m_pShareBuffer, 1024);
}

// д��Ŀ�ĵ�
void GameData::WriteMoveCoor(DWORD x, DWORD y, _account_* account)
{
	account = account ? account : m_pAccoutBig;
	if (!account || !account->Mnq)
		return;

	DWORD old[2] = { 0, 0 };
	if (ReadMemory((PVOID)account->Addr.MoveX, old, sizeof(old))) {
		//printf("old:%d,%d\n", old[0], old[1]);
		if (old[0] == x && old[1] == y)
			return;
	}

	SIZE_T write_len = 0;
	DWORD data[] = { x, y };

	DWORD old_p;
	//VirtualProtect((PVOID)m_DataAddr.MoveX, sizeof(data), PAGE_READWRITE, &old_p);
	
	if (0 && !m_bInDll && WriteProcessMemory(account->Mnq->Process, (PVOID)account->Addr.MoveX, data, sizeof(data), &write_len)) {
		//printf("WriteMoveCoor����:%d\n", write_len);
	}
	else {
		wchar_t log[64];
		//::printf("�޷�д��Ŀ������(%d) %08X\n", GetLastError(), account->Addr.MoveX);
		if (!m_pShareBuffer) {
			DbgPrint("�޷���������\n");
			LOGVARP2(log, "red", L"�޷���������(!m_pShareBuffer.%d)", GetLastError());
			return;
		}

		if (!m_bInDll) {
#if 0
			char cmd[128];
			sprintf_s(cmd, "%s\\win7\\\WriteXY.exe %d", m_pGame->m_chPath, account->Mnq->VBoxPid);
			printf("����WriteXY����ע��DLL:%hs\n", cmd);
			system(cmd);
#else
			wchar_t dll[256];
			wsprintfW(dll, L"%hs\\files\\wxy.dll", m_pGame->m_chPath);
			InjectDll(account->Mnq->VBoxPid, dll, NULL, FALSE);
#endif
			m_bInDll = true;
			Sleep(100);
		}

		// ӳ������һ����ͼ���õ�ָ�����ڴ��ָ�룬�������������
		m_pShareBuffer = (ShareWriteXYData*)::MapViewOfFile(m_hShareMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

		if (x < 500 && !m_pGame->CheckGameOtherModule()) {
			m_pShareBuffer->Value = 0xff;
		}

		m_pShareBuffer->AddrX = account->Addr.MoveX;
		m_pShareBuffer->AddrY = account->Addr.MoveY;
		m_pShareBuffer->X = x;
		m_pShareBuffer->Y = y;
		m_pShareBuffer->Flag = 1; // Dllд��

		DbgPrint("�ȴ�д�����...\n");
		//LOGVARP2(log, "c6", L"�ȴ�д�����...");
		while (m_pShareBuffer->Flag == 1) { // �ȴ�Dllд�����
			//printf("m_pShareBuffer->Flag:%d\n", m_pShareBuffer->Flag);
			Sleep(10);
		}
		DbgPrint("�Ѿ����д��!!!\n");
		//LOGVARP2(log, "c6", L"�Ѿ����д��!!!");
	}
}

// д�뻭����ֵ
bool GameData::WritePicScale(DWORD v, _account_* account)
{
	account = account ? account : m_pAccoutBig;
	if (!account || !account->Mnq || !account->Addr.PicScale)
		return false;

	if (!m_bInDll && WriteProcessMemory(account->Mnq->Process, (PVOID)account->Addr.PicScale, &v, sizeof(v), NULL)) {
		//printf("WriteMoveCoor����:%d\n", write_len);
	}
	else {
		wchar_t log[64];
		//::printf("�޷�д��Ŀ������(%d) %08X\n", GetLastError(), account->Addr.MoveX);
		if (!m_pShareBuffer) {
			DbgPrint("�޷���������\n");
			LOGVARP2(log, "red", L"�޷���������(!WritePicScale.%d)", GetLastError());
			return false;
		}

		if (!m_bInDll) {
			wchar_t dll[256];
			wsprintfW(dll, L"%hs\\files\\wxy.dll", m_pGame->m_chPath);
			InjectDll(account->Mnq->VBoxPid, dll, NULL, FALSE);
			m_bInDll = true;
		}

		// ӳ������һ����ͼ���õ�ָ�����ڴ��ָ�룬�������������
		m_pShareBuffer = (ShareWriteXYData*)::MapViewOfFile(m_hShareMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

		m_pShareBuffer->AddrPic = account->Addr.PicScale;
		m_pShareBuffer->PicScale = v;
		m_pShareBuffer->Flag = 2; // Dllд��
	}

	return true;
}

// ����������
DWORD GameData::SearchCode(DWORD* codes, DWORD length, DWORD* save, DWORD save_length, DWORD step)
{
	if (length == 0 || save_length == 0)
		return 0;


	DWORD count = 0;
	for (DWORD i = 0; i < m_dwReadSize; i += step) {
		if ((i + length) > m_dwReadSize)
			break;

		DWORD addr = m_dwReadBase + i;
		if (addr == (DWORD)codes) { // �����Լ�
			//::printf("���������Լ�:%08X %08X\n", codes, codes[0]);
			//return 0;
		}

		DWORD* dw = (DWORD*)(m_pReadBuffer + i);
		bool result = true;
		for (DWORD j = 0; j < length; j++) {
			if (codes[j] == 0x11) { // �����
				result = true;
			}
			else if (codes[j] == 0x22) { // ��Ҫ��ֵ��Ϊ0
				if (dw[j] == 0) {
					result = false;
					break;
				}
			}
			else if (((codes[j] & 0xffff0000) == 0x12340000)) { // ��2�ֽ����
				if ((dw[j] & 0x0000ffff) != (codes[j] & 0x0000ffff)) {
					result = false;
					break;
				}
				else {
					//::printf("%08X\n", dw[j]);
				}
			}
			else if (((codes[j] & 0x0000ffff) == 0x00001234)) { // ��2�ֽ����
				if ((dw[j] & 0xffff0000) != (codes[j] & 0xffff0000)) {
					result = false;
					break;
				}
			}
			else {
				if ((codes[j] & 0xffffff00) == 0x00001100) { // �Ƚ�������ַ��ֵ��� ���8λΪ�Ƚ�����
					//::printf("%X:%X %08X:%08X\n", j, codes[j] & 0xff, dw[j], dw[codes[j] & 0xff]);
					if (dw[j] != dw[codes[j] & 0xff]) {
						result = false;
						break;
					}
				}
				else if (dw[j] != codes[j]) { // ������ֵ�����
					if (0 && m_dwReadBase >= 0x247DD000 && m_dwReadBase < 0x247DE000 && i == 0) {
						printf("%d.m_dwReadBase!=0x4F67C000 %08X=%08X\n", j, dw[j], codes[j]);
					}
					result = false;
					break;
				}
			}
		}

		if (result) {
			save[count++] = addr;
			//::printf("��ַ:%08X   %08X\n", addr, codes);
			if (count == save_length)
				break;
		}
	}

	return count;
}

// ����������
DWORD GameData::SearchByteCode(BYTE * codes, DWORD length)
{
	if (length == 0)
		return 0;

	for (DWORD i = 0; i < m_dwReadSize; i++) {
		if ((i + length) > m_dwReadSize)
			break;

		DWORD addr = m_dwReadBase + i;
		if (addr == (DWORD)codes) { // �����Լ�
			//::printf("���������Լ�:%08X\n", codes);
			return 0;
		}

		BYTE* data = (m_pReadBuffer + i);
		bool result = true;
		for (DWORD j = 0; j < length; j++) {
			if (codes[j] == 0x11) { // �����
				result = true;
			}
			else if (codes[j] == 0x22) { // ��Ҫ��ֵ��Ϊ0
				if (data[j] == 0) {
					result = false;
					break;
				}
			}
			else if (codes[j] != data[j]) {
				result = false;
				break;
			}
		}

		if (result)
			return addr;
	}

	return 0;
}

// ��ȡ���ֽ�����
bool GameData::ReadDwordMemory(DWORD addr, DWORD& v, _account_* account)
{
	return ReadMemory((PVOID)addr, &v, 4, account);
}

// ��ȡ�ڴ�
bool GameData::ReadMemory(PVOID addr, PVOID save, DWORD length, _account_* account)
{
	if (!account)
		account =  m_pGame->m_pBig;
	if (!account || !account->Mnq)
		return false;

	SIZE_T dwRead = 0;
	bool result = ReadProcessMemory(account->Mnq->Process, addr, save, length, &dwRead);
	//::printf("ReadProcessMemory:%d %08X %d Read:%d ��ֵ:%d(%08X)\n", GetLastError(), addr, result, dwRead, *(DWORD*)save, *(DWORD*)save);

	if (!result || dwRead != length) {
		::printf("ReadProcessMemory����:%d %08X %d\n", GetLastError(), addr, result);
		if (GetLastError() == 6) {
			m_hProcessBig = GetCurrentProcess();
			return ReadProcessMemory(m_hProcessBig, addr, save, length, NULL);
		}
	}

	return result;
}


// ��ȡ��Ϸ�ڴ�
bool GameData::ReadGameMemory(DWORD flag)
{
	m_bIsReadEnd = false;

	MEMORY_BASIC_INFORMATION mbi;
	memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION));
	DWORD_PTR MaxPtr = 0xf0000000; // ����ȡ�ڴ��ַ
	DWORD_PTR max = 0;


	DWORD ms = GetTickCount();
	DWORD_PTR ReadAddress = 0x00000000;
	ULONG count = 0, failed = 0;
	//::printf("fuck\n");
	while (ReadAddress < MaxPtr)
	{
		SIZE_T r = VirtualQueryEx(m_hGameProcess, (LPCVOID)ReadAddress, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
		SIZE_T rSize = 0;

		//::printf("r:%d\n", r);
		//::printf("ReadAddress:%08X - %08X-----%X\n", ReadAddress, ReadAddress + mbi.RegionSize, mbi.RegionSize);
		if (r == 0) {
			::printf("r:%d -- %p\n", (int)r, ReadAddress);
			break;
		}

		if (mbi.Type != MEM_PRIVATE && mbi.Protect != PAGE_READWRITE) {
			//::printf("%p-%p ===����, ��С:%d %08X %08X\n", ReadAddress, ReadAddress + mbi.RegionSize, mbi.RegionSize, mbi.Type, mbi.Protect);
			goto _c;
		}
		else {
			DWORD pTmpReadAddress = ReadAddress;
			DWORD dwOneReadSize = 0x1000; // ÿ�ζ�ȡ����
			DWORD dwReadSize = 0x00;      // �Ѷ�ȡ����
			while (dwReadSize < mbi.RegionSize) {
				m_dwReadBase = pTmpReadAddress;
				m_dwReadSize = (dwReadSize + dwOneReadSize) <= mbi.RegionSize
					? dwOneReadSize : mbi.RegionSize - dwReadSize;

				SIZE_T write_len = 0;
				if (ReadProcessMemory(m_hGameProcess, (LPVOID)pTmpReadAddress, m_pReadBuffer, m_dwReadSize, &write_len)) {
					//::printf("flag:%08X %p-%p\n", flag, ReadAddress, ReadAddress + mbi.RegionSize);

					if (flag & 0x01) {
						DWORD find_num = 0;
						if (!m_DataAddr.Player) {
							if (FindPlayerAddr())
								find_num++;
						}
						if (!m_DataAddr.MoveX) {
							if (FindMoveCoorAddr())
								find_num++;
						}
						if (!m_DataAddr.PicScale) {
							if (FindPicScale())
								find_num++;
						}
						if (find_num == 3)
							flag = 0;
					}
					if (!flag) {
						DbgPrint("������ȫ���ҵ�\n");
						goto end;
					}
				}
				else {
					//::printf("%p-%p === ��ȡʧ��, ��С:%d, ������:%d\n", pTmpReadAddress, pTmpReadAddress + m_dwReadSize, (int)m_dwReadSize, GetLastError());
				}

				dwReadSize += m_dwReadSize;
				pTmpReadAddress += m_dwReadSize;
			}

			count++;
		}
	_c:
		// ��ȡ��ַ����
		ReadAddress += mbi.RegionSize;
		//Sleep(8);
		//::printf("%016X---�ڴ��С2:%08X\n", ReadAddress, mbi.RegionSize);
		// ɨ0x10000000�ֽ��ڴ� ����100����
	}
end:
	DWORD ms2 = GetTickCount();
	DbgPrint("��ȡ���ڴ���ʱ:%d����\n", ms2 - ms);
	LOGVARN2(64, "c0", L"��ȡ���ڴ���ʱ:%d����", ms2 - ms);
	m_bIsReadEnd = true;
	return true;
}