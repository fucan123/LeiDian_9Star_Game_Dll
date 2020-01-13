#include "Game.h"
#include "Item.h"
#include "Talk.h"
#include "PrintScreen.h"
#include "GameProc.h"
#include <My/Common/C.h>

Item::Item(Game* p)
{
	m_pGame = p;
}

// �򿪱���
bool Item::OpenBag()
{
	int x = MyRand(1120, 1130), y = MyRand(190, 200);
	DbgPrint("�򿪱���:(%d,%d)\n", x, y);
	LOGVARN2(32, "c0", L"�򿪱���:(%d,%d)", x, y);
	m_pGame->m_pGameProc->Click(x, y);
	for (int i = 0; i < 2000; i += 150) {
		if (BagIsOpen())
			return true;

		Sleep(150);
	}

	m_pGame->m_pPrintScreen->GetPixelCount(0xFFB83700, 0x100, true);;
	DbgPrint("��ⱳ����ʧ��\n");
	LOG2(L"��ⱳ����ʧ��", "red");
	Sleep(150);
	CloseBag();
	return false;
}

// �رձ���
void Item::CloseBag()
{
	DbgPrint("�رձ���\n");
	LOG2(L"�رձ���", "orange b");
	m_pGame->m_pGameProc->Click(1236, 75, 1250, 80);
}

// ������
void Item::SetBag()
{
	DbgPrint("������\n");
	LOG2(L"������", "c0");
	m_pGame->m_pGameProc->Click(1025, 666, 1125, 686);
}

// ��������
bool Item::SlideBag(int page)
{
	if (page <= 0)
		return false;

	int slide_y = -400;
	int x = MyRand(700, 900), y = MyRand(350, 600);
	DbgPrint("��������(%d):(%d,%d)\n", page, x, y);
	LOGVARN2(64, "c0", L"��������(%d):(%d,%d)", page, x, y);
#if 1
	m_pGame->m_pEmulator->Swipe(x, y, x, y + slide_y, m_pGame->m_pBig->Mnq->Index, 800);
	Sleep(800);
#else
	int slide_y = -487;
	m_pGame->m_pGameProc->Click(x, y, 0x01);
	m_pGame->m_pGameProc->MouseMove(x, y, 0, slide_y);
	m_pGame->m_pGameProc->Click(x, y + slide_y, 0x02);
#endif
	return true;
}

// �򿪲ֿ�
void Item::OpenStorage()
{
	OpenBag();
	Sleep(1000);
	DbgPrint("�򿪲ֿ�\n");
	LOG2(L"�򿪲ֿ�", "c0");
	m_pGame->m_pGameProc->Click(710, 670, 725, 686);
}

// �����ֿ�
bool Item::SlideStorge(int page)
{
	if (page <= 0)
		return false;

	int slide_y = -400;
	int x = MyRand(260, 360), y = MyRand(350, 600);

#if 1
	m_pGame->m_pEmulator->Swipe(x, y, x, y + slide_y, m_pGame->m_pBig->Mnq->Index, 800);
	Sleep(800);
#else
	int slide_y = -487;
	m_pGame->m_pGameProc->Click(x, y, 0x01);
	m_pGame->m_pGameProc->MouseMove(x, y, 0, slide_y);
	m_pGame->m_pGameProc->Click(x, y + slide_y, 0x02);
#endif
	return true;
}

// �رղֿ�
void Item::CloseStorage()
{
	CloseBag();
}

// ����Ѫ
void Item::AddFullLife()
{
	for (int i = 0; i < 6; i++) {
		m_pGame->m_pGameData->ReadLife();
		if ((m_pGame->m_pGameData->m_dwLife + 1000) >= m_pGame->m_pGameData->m_dwLifeMax)
			return;

		UseYao(1);
	}
	
}

// ʹ��ҩ
void Item::UseYao(int num, bool use_yaobao, int sleep_ms)
{
	//DbgPrint("ʹ��ҩ\n");
	for (int i = 0; i < num; i++) {
		m_pGame->m_pGameProc->Click(893, 590, 935, 627);
		Sleep(sleep_ms);
	}
	if (use_yaobao && IsNeedUseYaoBao())
		UseYaoBao();
}

// ʹ��ҩ����
void Item::UseYaoBao()
{
	//DbgPrint("ʹ��ҩ��\n");
	m_pGame->m_pGameProc->Click(910, 496, 955, 530);
	Sleep(300);
}

// ȥ�����ſ�
void Item::GoFBDoor()
{
	m_pGame->m_pGameProc->Click(893, 590, 935, 627); // ��Χ�����Ǹ�
}

// ȥ�����ſ�
void Item::GoShop()
{
	m_pGame->m_pGameProc->Click(910, 496, 955, 530); // ��Χ���浹���ڶ�
}

// �ر��̵�
void Item::CloseShop()
{
	DbgPrint("�ر��̵�");
	LOG2(L"�ر��̵�", "c0");
	m_pGame->m_pGameProc->Click(553, 85, 558, 90);
}

// ʹ�ø��������ҩ
void Item::UseLingYao()
{
	DbgPrint("ʹ����Чʥ����ҩ\n");
	LOG2(L"ʹ����Чʥ����ҩ", "green");
	m_pGame->m_pGameProc->Click(975, 430, 980, 435);
	Sleep(300);
}

// �Ƿ���Ҫʹ��ҩ��
bool Item::IsNeedUseYaoBao()
{
	//FFB83800 FFB83700 FFA43700 FFA03301 FFA43F09 FF963301 FF8E3606 FFA14D1E
	//FFB33200 FFB83700 FFA03501 FFA93E07 FF943102 FF9C3B0C FFBA6536 FFC97749

	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_hWndBig, 917, 608, 925, 610);
	bool result = m_pGame->m_pPrintScreen->GetPixelCount(0xFFB83700, 0x100) == 0; // ��ҩ����ɫ
	m_pGame->m_pPrintScreen->Release();
	return result;
}

// �����Ƿ��
bool Item::BagIsOpen()
{
	// ��ȡ���Ǳ����رհ�ťǰ��һ��
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 1150, 75, 1160, 85, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("����", nullptr, 1) > 0; // �򿪻���ס��ť
}

// ��Ʒ������ť�Ƿ��
bool Item::ItemBtnIsOpen(int index)
{
	// ��ȡ��Ʒ������ť 
	if (index == 0)      // ��һ��
		m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 145, 85, 155, 95, 0, true);
	else if (index == 1) // �ڶ���
		m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 145, 145, 155, 175, 0, true);
	else                 // ������
		m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 145, 215, 155, 245, 0, true);

	return m_pGame->m_pPrintScreen->ComparePixel("��Ʒ��ť", nullptr, 1) > 0;
}

// �ȴ���Ʒ������ť����
bool Item::WaitForItemBtnOpen()
{
	for (int i = 0; i < 1000; i += 100) {
		if (ItemBtnIsOpen())
			return true;

		Sleep(100);
	}
	return false;
	//375,260 820,460 ��һ��BOSS����ɨ���Ƿ��б���ҩ
}

// ��ֲ�����ť�Ƿ����
bool Item::ChaiFeiBtnIsOpen()
{
	// ��ȡ��ֿ�ȡ����ť
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 466, 505, 476, 515, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("��ֿ�", nullptr, 1) > 0;
}

// �رղ�ֿ�
void Item::CloseChaiFeiBox()
{
	m_pGame->m_pGameProc->Click(466, 505, 566, 515);
}

// ����
int Item::PickUpItem(const char* name, int x, int y, int x2, int y2, int pickup_num)
{
	int count = 0;
	for (int i = 1; i <= pickup_num; i++) {
		int pos_x = 0, pos_y = 0;
		if (!GetGroundItemPos(name, x, y, x2, y2, pos_x, pos_y))
			break;

		DbgPrint("%d.����:%s(%d,%d)\n", i, name, pos_x, pos_y);
		LOGVARN2(64, "c0", L"%d.����:%hs(%d,%d)", i, name, pos_x, pos_y);
		m_pGame->m_pGameProc->Click(pos_x, pos_y);
		WaitForPickUpItem();
		count++;

		Sleep(260);
	}

	return count;
}

// �ȴ�������Ʒ
int Item::WaitForPickUpItem(DWORD wait_ms)
{
	DWORD x = 0, y = 0;
	m_pGame->m_pGameData->ReadCoor(&x, &y, m_pGame->m_pGameProc->m_pAccount);
	for (DWORD i = 0; i <= wait_ms; i += 800) {
		Sleep(800);

		DWORD now_x = 0, now_y = 0;
		m_pGame->m_pGameData->ReadCoor(&now_x, &now_y, m_pGame->m_pGameProc->m_pAccount);
		if (x == now_x && y == now_y)
			return 1;

		x = now_x;
		y = now_y;
	}
	return 0;
}

// ��ȡ������Ʒ����
int Item::GetGroundItemPos(const char* name, int x, int y, int x2, int y2, int& pos_x, int& pos_y)
{
	if (!x || !y || !x2 || !y2)
		return 0;

	//DbgPrint("�����ͼ:%s(%d,%d %d,%d)\n", name, x, y, x2, y2);
	int tmp_x = 0, tmp_y = 0, tmp_x2 = 0, tmp_y2 = 0;
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, x, y, x2, y2, 0, true);
	if (strcmp("��Чʥ����ҩ", name) == 0) {
		if (m_pGame->m_pPrintScreen->GetPixelPos(0xFF72AA23, tmp_x, tmp_y, 0x00101010)) {
			m_pGame->m_pPrintScreen->GetPixelPos(0xFF528A03, tmp_x2, tmp_y2, 0x00101010);
		}
		//DbgPrint("%s:%d,%d %d,%d\n", name, tmp_x, tmp_y, tmp_x2, tmp_y2);
	}
	else if (strcmp("30��������Ƭ+3", name) == 0) {
		if (m_pGame->m_pPrintScreen->GetPixelPos(0xB885CA, tmp_x, tmp_y, 0x00080808)) {
			//m_pGame->m_pPrintScreen->GetPixelPos(0xF5C5FB, tmp_x2, tmp_y2, 0x00101010);
		}
		//DbgPrint("%s:%d,%d %d,%d\n", name, tmp_x, tmp_y, tmp_x2, tmp_y2);
	}
	if ((tmp_x && tmp_y) || (tmp_x2 && tmp_y2)) {
		//DbgPrint("������Ʒ:%s, ����:%d,%d ����2:%d,%d\n", name, tmp_x, tmp_y, tmp_x2, tmp_y2);

		if (tmp_x) {
			pos_x = tmp_x;
			pos_y = tmp_y;
		}
		else {
			pos_x = tmp_x2;
			pos_y = tmp_y2;
		}
			
		return 1;
	}

	return 0;
}

// ����ҩ��
int Item::DropItem(ComImgIndex index, int live_count, DWORD* ms)
{
	DWORD _tm = GetTickCount();

	if (!OpenBag())
		Sleep(500);

	Sleep(300);

	// 30��������Ƭ+2��ʹ�ò��ᶪ�� ��ɫף����Ƭ��30��������Ƭ+2һ���� ��ɫף����30��������Ƭ+3һ����
	char drops[][32] = {
		"�İٵ�ͼ����", "������ʿ�Ĺ�׹", "Կ��",
		"�������ּ�", "������ʥˮ", "�������緢�Ͱ�", "��Ӣ��ʿ����",
		"30��������Ƭ+1", "30��������Ƭ+2", "��ɫף����Ƭ"
	};
	int drops_num = sizeof(drops)/ sizeof(drops[0]);
	int count = 0; // ����
	for (int i = 0; i <= 5; i++) {
		if (!BagIsOpen() || m_pGame->m_pGameProc->m_bPause)
			break;

		if (SlideBag(i)) { // ��������
			Sleep(200);
		}

		for (int idx = 0; idx < drops_num; idx++) {
			if (i > 0 && strcmp("��ɫף����Ƭ", drops[idx]) == 0)
				continue;

			ComPoint cp[32];
			PrintBagImg(true); // ��ȡ����ͼƬ
			int num = m_pGame->m_pPrintScreen->ComparePixel(drops[idx], cp, sizeof(cp) / sizeof(ComPoint));
			if (num == 0)
				continue;
			if (num == 1 && strcmp(drops[idx], "Կ��") == 0) // Կ��ֻ��һ�Ѳ��ܶ�
				continue;
			//m_pGame->m_pPrintScreen->SaveBitmapToFile(bitmap, L"C:\\Users\\fucan\\Desktop\\MNQ-9Star\\jt.bmp");
			//DbgPrint("%d.��.%s.������Ʒ����:%d\n", i + 1, drops[idx], num);

			int drop_num = 0;

			if (count >= live_count) { // ��Ҫȫ����
				drop_num = num;
			}
			else {
				drop_num = count + num - live_count;
			}
			drop_num = strcmp(drops[idx], "Կ��") != 0 ? num : 1; // Կ��ֻ����ǰ����һ��

			int btn_index = 1;
			if (strcmp("Կ��", drops[idx]) == 0 || strcmp("������ʥˮ", drops[idx]) == 0) // ֻ��һ��������ť
				btn_index = 0;
			else if (strstr(drops[idx], "30��������Ƭ")) // ���ʹ��
				btn_index = 0;

			for (int j = 0; j < drop_num; j++) {
				if (j == 0) {
					if (strstr(drops[idx], "30��������Ƭ")) {
						UseItem(drops[idx], cp[0].x, cp[0].y);
					}
					else {
						DropItem(drops[idx], cp[0].x, cp[0].y, btn_index);
					}
				}
				else {
					ComPoint item[1];
					PrintBagImg(true); // ��ȡ����ͼƬ
					if (!m_pGame->m_pPrintScreen->ComparePixel(drops[idx], item, sizeof(item) / sizeof(ComPoint)))
						break;

					if (strstr(drops[idx], "30��������Ƭ")) {
						UseItem(drops[idx], item[0].x, item[0].y);
					}
					else {
						DropItem(drops[idx],item[0].x, item[0].y, btn_index);
					}
				}
				Sleep(600);

				if (strstr(drops[idx], "30��������Ƭ")) {
					m_pGame->m_pGameProc->CloseTipBox();
				}
				if (ChaiFeiBtnIsOpen()) {
					CloseChaiFeiBox();
					Sleep(300);
				}
			}
			count += num;

			m_pGame->m_pGameProc->CloseTipBox();
		}
		
		if (m_pGame->m_pPrintScreen->CompareImage(CIN_NoItem, nullptr, 1)) 
			break;
		if (m_pGame->m_pPrintScreen->CompareImage(CIN_LockItem, nullptr, 1)) 
			break;
	}

	if (ChaiFeiBtnIsOpen()) {
		CloseChaiFeiBox();
		Sleep(300);
	}

	Sleep(150);
	CloseBag();

	_tm = GetTickCount() - _tm;
	if (ms) {
		*ms = _tm;
	}
	
	DbgPrint("������Ʒ��ʱ:%.2f��, %d����\n", (float)_tm/1000.0f, _tm);
	LOGVARN2(64, "c0", L"������Ʒ��ʱ:%.2f��, %d����", (float)_tm / 1000.0f, _tm);
	return count;
}

// ʹ����Ʒ
void Item::UseItem(const char* name, int x, int y)
{
	DbgPrint("ʹ����Ʒ:%s(%d,%d)\n", name, x, y);
	LOGVARN2(64, "c0", L"ʹ����Ʒ:%hs(%d,%d)", name, x, y);
	m_pGame->m_pGameProc->Click(x, y);
	if (WaitForItemBtnOpen()) {
		Sleep(100);
		GetItemBtnPos(x, y, 0);
		m_pGame->m_pGameProc->Click(x, y); // ���ʹ�ð�ť
	}
}

// ������Ʒ
void Item::DropItem(const char* name, int x, int y, int index)
{
	DbgPrint("����:%s(%d,%d)\n", name, x, y);
	LOGVARN2(64, "c0", L"����:%hs(%d,%d)", name, x, y);
	m_pGame->m_pGameProc->Click(x, y);
	if (WaitForItemBtnOpen()) {
		Sleep(150);
		if (strcmp("��ɫף����Ƭ", name) == 0 && ItemBtnIsOpen(2)) // �е�������ť, ��ô��30��������Ƭ+3
			return;
		if (strcmp("Կ��", name) != 0) {
			Sleep(100);
			// �����Ʒ����������ť�Աߵ�ͼ��
			m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 320, 82, 375, 145, 0, true); 
			if (m_pGame->m_pPrintScreen->ComparePixel("Կ��", nullptr, 1) > 0) // ������Կ��
				return;
		}

		GetItemBtnPos(x, y, index > -1 ? index : 1);
		m_pGame->m_pGameProc->Click(x, y); // ���������ť
		if (m_pGame->m_pTalk->WaitForConfirmBtnOpen()) {
			Sleep(100);
			m_pGame->m_pTalk->ClickConfirmBtn(1); // ȷ����ť
		}
	}	
}

// ������Ʒ
int Item::SellItem(ConfItemInfo* items, DWORD length)
{
	for (int i = 0; i <= 5; i++) {
		if (!BagIsOpen() || m_pGame->m_pGameProc->m_bPause)
			break;

		if (SlideBag(i)) { // ��������
			Sleep(100);
		}

		for (DWORD iti = 0; iti < length; iti++) {
			for (int sell_i = 0; sell_i < 10; sell_i++) {
				ComPoint cp[32];
				PrintBagImg(true); // ��ȡ����ͼƬ
				int num = m_pGame->m_pPrintScreen->ComparePixel(items[iti].Name, cp, sizeof(cp) / sizeof(ComPoint));
				//DbgPrint("%d.����.������Ʒ����:%d\n", i + 1, num);
				if (num == 0)
					break;

				for (int j = 0; j < num; j++) {
					if (j == 0) {
						SellItem(cp[0].x, cp[0].y);
						DbgPrint("%d.����:%hs(%d,%d)\n", j + 1, items[iti].Name, cp[j].x, cp[j].y);
						LOGVARN2(64, "c0", L"%d.����:%hs(%d,%d)", j + 1, items[iti].Name, cp[j].x, cp[j].y);
					}
					else {
						ComPoint item[1];
						PrintBagImg(true); // ��ȡ����ͼƬ
						if (!m_pGame->m_pPrintScreen->ComparePixel(items[iti].Name, item, sizeof(item) / sizeof(ComPoint)))
							break;

						SellItem(item[0].x, item[0].y);
						DbgPrint("%d.����:%hs(%d,%d)\n", j + 1, items[iti].Name, item[0].x, item[0].y);
						LOGVARN2(64, "c0", L"%d.����:%hs(%d,%d)", j + 1, items[iti].Name, item[0].x, item[0].y);
					}
					Sleep(600);
				}
			}
		}

		if (m_pGame->m_pPrintScreen->CompareImage(CIN_NoItem, nullptr, 1))
			break;
		if (m_pGame->m_pPrintScreen->CompareImage(CIN_LockItem, nullptr, 1))
			break;
	}

	return 0;
}

// ������Ʒ
void Item::SellItem(int x, int y)
{
	//DbgPrint("����Ʒ:%d,%d\n", x, y);
	m_pGame->m_pGameProc->Click(x, y);
	if (WaitForItemBtnOpen()) {
		Sleep(100);
		GetItemBtnPos(x, y, 0);
		m_pGame->m_pGameProc->Click(x, y); // ���������ť
		if (m_pGame->m_pTalk->WaitForConfirmBtnOpen()) {
			Sleep(100);
			m_pGame->m_pTalk->ClickConfirmBtn(1); // ȷ����ť
		}
	}
}

// ��ȡ��Ʒ������ťλ��
void Item::GetItemBtnPos(int& x, int& y, int index)
{
	if (index == 0) {
		x = MyRand(180, 200);
		y = MyRand(86, 100);
		return;
	}
	if (index == 1) {
		x = MyRand(180, 200);
		y = MyRand(155, 168);
		return;
	}
}

// ����ֿ�
void Item::CheckIn(ConfItemInfo* items, DWORD length)
{
	DbgPrint("����\n");
	LOG2(L"����", "c0");
	DWORD _tm = GetTickCount();

	OpenStorage();
	Sleep(1000);

	int pet_yao_1 = 0, pet_yao_2 = 1;
	for (int i = 0; i <= 5; i++) {
		if (!BagIsOpen() || m_pGame->m_pGameProc->m_bPause)
			break;

		m_pGame->m_pGameProc->CloseTipBox();
		if (SlideBag(i)) { // ��������
			Sleep(300);
		}

		if (!pet_yao_1) {
			PrintBagImg(true); // ��ȡ����ͼƬ
			pet_yao_1 = m_pGame->m_pPrintScreen->ComparePixel("��Чʥ����ҩ", nullptr, 1) > 0;
		}
		
		bool is_save = false; // �Ƿ���˶���
		for (DWORD iti = 0; iti < length; iti++) {
			ComPoint cp[32];
			PrintBagImg(true); // ��ȡ����ͼƬ
			int num = m_pGame->m_pPrintScreen->ComparePixel(items[iti].Name, cp, sizeof(cp) / sizeof(ComPoint));
			//DbgPrint("%d.����.������Ʒ����:%d\n", i + 1, num);
			if (num > 0) {
				for (int j = 0; j < num; j++) {
					if (j == 0) {
						m_pGame->m_pGameProc->Click(cp[0].x, cp[0].y);
						DbgPrint("%d.����.%s:(%d,%d)\n", j + 1, items[iti].Name, cp[j].x, cp[j].y);
						LOGVARN2(64, "c0", L"%d.����:%hs(%d,%d)", j + 1, items[iti].Name, cp[j].x, cp[j].y);
					}
					else {
						ComPoint item[1];
						PrintBagImg(true); // ��ȡ����ͼƬ
						if (!m_pGame->m_pPrintScreen->ComparePixel(items[iti].Name, item, sizeof(item) / sizeof(ComPoint)))
							break;

						m_pGame->m_pGameProc->Click(item[0].x, item[0].y);
						DbgPrint("%d.����.%s:(%d,%d)\n", j + 1, items[iti].Name, item[0].x, item[0].y);
						LOGVARN2(64, "c0", L"%d.����:%hs(%d,%d)", j + 1, items[iti].Name, item[0].x, item[0].y);
					}
					Sleep(800);
				}
				is_save = true;
			}
		}

		if (is_save && pet_yao_1) {
			PrintBagImg(true); // ��ȡ����ͼƬ
			pet_yao_2 = m_pGame->m_pPrintScreen->ComparePixel("��Чʥ����ҩ", nullptr, 1);
		}

		if (m_pGame->m_pPrintScreen->CompareImage(CIN_NoItem, nullptr, 1))
			break;
		if (m_pGame->m_pPrintScreen->CompareImage(CIN_LockItem, nullptr, 1))
			break;
	}
	if (pet_yao_1 && !pet_yao_2) {
		DbgPrint("\n======��Чʥ����ҩ������, ��Ҫȡ����======\n\n");
		LOG2(L"\n======��Чʥ����ҩ������, ��Ҫȡ����======", "blue_r");
		CheckOutOne("��Чʥ����ҩ", false, false);
	}

	Sleep(150);
	CloseStorage();

	_tm = GetTickCount() - _tm;
	DbgPrint("������ʱ:%.2f��, %d����\n", (float)_tm / 1000.0f, _tm);
	LOGVARN2(64, "c0", L"������ʱ:%.2f��, %d����", (float)_tm / 1000.0f, _tm);
}

// ȡ���ֿ�
int Item::CheckOut(ConfItemInfo* items, DWORD length)
{
	DbgPrint("ȡ��\n");
	LOG2(L"ȡ��", "c0");
	DWORD _tm = GetTickCount();

	OpenStorage();
	Sleep(1000);

	int count = 0;

	for (int i = 0; i <= 5; i++) {
		if (SlideStorge(i)) { // �����ֿ�
			Sleep(300);
		}

		for (DWORD iti = 0; iti < length; iti++) {
			ComPoint cp[32];
			PrintStorageImg(true); // ��ȡ�ֿ�ͼƬ
			int num = m_pGame->m_pPrintScreen->ComparePixel(items[iti].Name, cp, sizeof(cp) / sizeof(ComPoint));
			count += num;
			//DbgPrint("%d.ȡ��.�ֿ���Ʒ����:%d\n", i + 1, num);
			if (num > 0) {
				for (int j = 0; j < num; j++) {
					if (j == 0) {
						m_pGame->m_pGameProc->Click(cp[0].x, cp[0].y);
						DbgPrint("%d.ȡ��.(%s:%d,%d)\n", j + 1, items[iti].Name, cp[j].x, cp[j].y);
						LOGVARN2(64, "c0", L"%d.ȡ��:%hs(%d,%d)", j + 1, items[iti].Name, cp[j].x, cp[j].y);
					}
					else {
						ComPoint item[1];
						PrintStorageImg(true); // ��ȡ�ֿ�ͼƬ
						if (!m_pGame->m_pPrintScreen->ComparePixel(items[iti].Name, item, sizeof(item) / sizeof(ComPoint)))
							break;

						m_pGame->m_pGameProc->Click(item[0].x, item[0].y);
						DbgPrint("%d.ȡ��.%s:(%d,%d)\n", j + 1, items[iti].Name, item[0].x, item[0].y);
						LOGVARN2(64, "c0", L"%d.ȡ��:%hs(%d,%d)", j + 1, items[iti].Name, item[0].x, item[0].y);
					}
					Sleep(800);
				}
			}
		}

		if (m_pGame->m_pPrintScreen->CompareImage(CIN_NoItem, nullptr, 1))
			break;
		if (m_pGame->m_pPrintScreen->CompareImage(CIN_LockItem, nullptr, 1))
			break;
	}

	Sleep(150);
	CloseStorage();

	_tm = GetTickCount() - _tm;
	DbgPrint("ȡ����ʱ:%.2f��, %d����\n", (float)_tm / 1000.0f, _tm);
	LOGVARN2(64, "c0", L"ȡ����ʱ:%.2f��, %d����", (float)_tm / 1000.0f, _tm);

	return count;
}

// ȡ��һ���ֿ���Ʒ
int Item::CheckOutOne(const char * name, bool open, bool close)
{
	if (open) {
		OpenStorage();
		Sleep(1000);
	}
	
	int count = 0;
	for (int i = 0; i <= 5; i++) {
		if (SlideStorge(i)) { // �����ֿ�
			Sleep(300);
		}

		ComPoint cp[1];
		PrintStorageImg(true); // ��ȡ�ֿ�ͼƬ
		if (m_pGame->m_pPrintScreen->ComparePixel(name, cp, 1)) {
			DbgPrint("ȡ��.%s:(%d,%d)\n", name, cp[0].x, cp[0].y);
			LOGVARN2(64, "c0", L"ȡ��.%hs:(%d,%d)\n", name, cp[0].x, cp[0].y);
			m_pGame->m_pGameProc->Click(cp[0].x, cp[0].y);
			count = 1;
		}

		if (m_pGame->m_pPrintScreen->CompareImage(CIN_NoItem, nullptr, 1))
			break;
		if (m_pGame->m_pPrintScreen->CompareImage(CIN_LockItem, nullptr, 1))
			break;
	}

	if (close) {
		Sleep(150);
		CloseStorage();
	}

	return count;
}

// ��ȡ������Ʒ����
int Item::GetBagCount(ComImgIndex index)
{
	DWORD _tm = GetTickCount();

	OpenBag();
	Sleep(1000);

	int count = 0; // ����
	for (int i = 0; i <= 3; i++) {
		if (SlideBag(i)) { // ��������
			Sleep(300);
		}

		PrintBagImg(true); // ��ȡ����ͼƬ
		int num = m_pGame->m_pPrintScreen->CompareImage(index);
		DbgPrint("%d.������Ʒ����:%d\n", i + 1, num);
		LOGVARN2(64, "c0", L"%d.������Ʒ����:%d", i + 1, num);

		count += num;
		if (count > 0 && num == 0) // ������û��Ʒ
			break;

		if (m_pGame->m_pPrintScreen->CompareImage(CIN_NoItem, nullptr, 1))
			break;
		if (m_pGame->m_pPrintScreen->CompareImage(CIN_LockItem, nullptr, 1))
			break;
	}

	Sleep(500);
	CloseBag();

	_tm = GetTickCount() - _tm;
	DbgPrint("��ȡ��Ʒ��ʱ:%.2f��, %d����\n", (float)_tm / 1000.0f, _tm);
	LOGVARN2(64, "c0", L"��ȡ��Ʒ��ʱ:%.2f��, %d����", (float)_tm / 1000.0f, _tm);
	return count;
}

// ��ȡ�������Ʒ����
int Item::GetQuickYaoOrBaoNum(int& yaobao, int& yao)
{
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_hWndBig, 915, 520, 970, 640, 0, true);
	yaobao = m_pGame->m_pPrintScreen->LookNum(25, 0, 0, 25, 0xffffffff, 0x003A3A3A);
	yao = m_pGame->m_pPrintScreen->LookNum(0, 0, 95, 0, 0xffffffff, 0x003A3A3A);
	//DbgPrint("ҩ������:%d ҩ����:%d\n", yaobao, yao);

	return 0;
}

// �л������ܿ����
void Item::SwitchMagicQuickBar()
{
#if 0
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_hWndBig, 1235, 350, 1245, 360, 0, true);
	if (m_pGame->m_pPrintScreen->ComparePixel("������л���ť", nullptr, 1) == 0) { // ������XP������
		DbgPrint("������л���ť\n");
		SwitchQuickBar(1);
		Sleep(500);
	}
#endif

	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_hWndBig, 926, 510, 936, 520, 0, true);
	if (m_pGame->m_pPrintScreen->ComparePixel("�ǳ�֮�ۿ����", nullptr, 1)) { // �л�
		SwitchQuickBar(1);
		Sleep(500);
	}
}

// ������Ƿ����ǳ�֮��
bool Item::QuickBarIsXingChen()
{
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_hWndBig, 908, 602, 923, 617, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("�ǳ�֮��") > 0;
}

// �л������
void Item::SwitchQuickBar(int page)
{
	DbgPrint("�л��������:%d\n", page);
	m_pGame->m_pGameProc->Click(1235, 360, 1250, 370);
}

// ��ȡ��Ʒ��ComImgIndex
ComImgIndex Item::GetItemComImgIndex(const char * name)
{
	if (strcmp(name, "˫����������") == 0)
		return CIN_QingChi;
	if (strcmp(name, "˫������ڤ���") == 0)
		return CIN_ZiMing;
	if (strcmp(name, "�����ǿ������") == 0)
		return CIN_KaDi;
	if (strcmp(name, "�������°����") == 0)
		return CIN_MuBa;
	if (strcmp(name, "��ţ���Ƕ����") == 0)
		return CIN_YaEr;
	if (strcmp(name, "��ţ��³�����") == 0)
		return CIN_LuDi;
	if (strcmp(name, "��ħ��ʯ") == 0)
		return CIN_HuanMo;
	if (strcmp(name, "ħ�꾧ʯ") == 0)
		return CIN_MoHun;
	if (strcmp(name, "��꾧ʯ") == 0)
		return CIN_LingHun;
	if (strcmp(name, "���ȵ�����") == 0)
		return CIN_XiangLian;

	return CIN_NoItem;
}

// ��ȡ����ͼƬ
HBITMAP Item::PrintBagImg(bool del)
{
	HBITMAP bitmap = m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 672, 158, 1170, 620);
	if (del) {
		m_pGame->m_pPrintScreen->GetPixel(0, 0);
		DeleteBagImg();
	}
	return bitmap;
}

// ɾ������ͼƬ
void Item::DeleteBagImg()
{
	m_pGame->m_pPrintScreen->Release();
}

// ��ȡ�ֿ�ͼƬ
HBITMAP Item::PrintStorageImg(bool del)
{
	HBITMAP bitmap = m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_hWndBig, 113, 158, 620, 620);
	if (del) {
		m_pGame->m_pPrintScreen->GetPixel(0, 0);
		DeleteBagImg();
	}
	return bitmap;
}

// ɾ���ֿ�ͼƬ
void Item::DeleteStorageImg()
{
	m_pGame->m_pPrintScreen->Release();
}
