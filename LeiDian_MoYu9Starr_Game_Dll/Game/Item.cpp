#include "Game.h"
#include "GameConf.h"
#include "Item.h"
#include "Talk.h"
#include "PrintScreen.h"
#include "GameProc.h"
#include <My/Common/C.h>

Item::Item(Game* p)
{
	m_pGame = p;
}

// 打开背包
bool Item::OpenBag()
{
	bool is_try = true;
_open_:
	int x = MyRand(1120, 1130), y = MyRand(190, 200);
	DbgPrint("打开背包:(%d,%d)\n", x, y);
	LOGVARN2(32, "c0", L"打开背包:(%d,%d)", x, y);
	m_pGame->m_pGameProc->Click(x, y);
	for (int i = 0; i < 2000; i += 150) {
		if (BagIsOpen())
			return true;

		Sleep(150);
	}

	m_pGame->m_pPrintScreen->GetPixelCount(0xFFB83700, 0x100, true);;
	DbgPrint("检测背包打开失败\n");
	LOG2(L"检测背包打开失败", "red");
	Sleep(150);
	if (is_try) {
		//m_pGame->m_pTalk->CloseAllBox();
		is_try = false;
		goto _open_;
	}
	//CloseBag();
	return false;
}

// 关闭背包
void Item::CloseBag()
{
	DbgPrint("关闭背包\n");
	LOG2(L"关闭背包", "orange b");
	m_pGame->m_pGameProc->Click(1236, 75, 1250, 80);
}

// 整理背包
void Item::SetBag()
{
	DbgPrint("整理背包\n");
	LOG2(L"整理背包", "c0");
	m_pGame->m_pGameProc->Click(1025, 666, 1125, 686);
}

// 滑动背包
bool Item::SlideBag(int page)
{
	if (page <= 0)
		return false;

	DbgPrint("滑动背包(%d):(%d,%d)\n", page, x, y);
	LOGVARN2(32, "blue_r b", L"滑动背包(%d)", page);

	int slide_y = -400;
	int x = MyRand(700, 900), y = MyRand(350, 600);
#if 0
	m_pGame->m_pEmulator->Swipe(x, y, x, y + slide_y, m_pGame->m_pBig->Mnq->Index, 800);
	Sleep(1000);
#else
	slide_y = -487;
	m_pGame->m_pGameProc->Click_Send(x, y, 0x01);
	m_pGame->m_pGameProc->MouseMove(x, y, 0, slide_y);
	m_pGame->m_pGameProc->Click_Send(x, y + slide_y, 0x02);
#endif
	return true;
}

// 打开仓库
void Item::OpenStorage()
{
	OpenBag();
	Sleep(1000);
	DbgPrint("打开仓库\n");
	LOG2(L"打开仓库", "c0");
	m_pGame->m_pGameProc->Click(710, 670, 725, 686);
}

// 滑动仓库
bool Item::SlideStorge(int page)
{
	if (page <= 0)
		return false;

	LOGVARN2(32, "blue_r b", L"滑动仓库(%d)", page);

	int slide_y = -400;
	int x = MyRand(260, 360), y = MyRand(350, 600);
#if 0
	m_pGame->m_pEmulator->Swipe(x, y, x, y + slide_y, m_pGame->m_pBig->Mnq->Index, 800);
	Sleep(800);
#else
	slide_y = -466;
	m_pGame->m_pGameProc->Click_Send(x, y, 0x01);
	m_pGame->m_pGameProc->MouseMove(x, y, 0, slide_y);
	m_pGame->m_pGameProc->Click_Send(x, y + slide_y, 0x02);
#endif
	return true;
}

// 关闭仓库
void Item::CloseStorage()
{
	CloseBag();
}

// 加满血
void Item::AddFullLife()
{
	for (int i = 0; i < 6; i++) {
		m_pGame->m_pGameData->ReadLife();
		if ((m_pGame->m_pGameData->m_dwLife + 1000) >= m_pGame->m_pGameData->m_dwLifeMax)
			return;

		UseYao(1);
	}
	
}

// 使用药
void Item::UseYao(int num, bool use_yaobao, int sleep_ms)
{
	for (int i = 0; i < num; i++) {
		if (use_yaobao && IsNeedUseYaoBao()) {
			if (!UseYaoBao())
				break;
		}

		m_pGame->m_pGameProc->Click(893, 590, 935, 627);
		Sleep(sleep_ms);
	}
}

// 使用药包包
bool Item::UseYaoBao()
{
	int yaobao = 0, yao = 0;
	GetQuickYaoOrBaoNum(yaobao, yao);
	if (yaobao > 0) {
		m_pGame->m_pGameProc->Click(910, 496, 955, 530);
		Sleep(300);
	}
	
	return yaobao > 0;
}

// 去副本门口
void Item::GoFBDoor()
{
	m_pGame->m_pGameProc->Click(893, 590, 935, 627); // 外围下面那格
}

// 去副本门口
void Item::GoShop()
{
	m_pGame->m_pGameProc->Click(910, 496, 955, 530); // 外围下面倒数第二
}

// 关闭商店
void Item::CloseShop()
{
	DbgPrint("关闭商店");
	LOG2(L"关闭商店", "c0");
	m_pGame->m_pGameProc->Click(553, 85, 558, 90);
}

// 使用复活宠物灵药
void Item::UseLingYao()
{
	DbgPrint("使用速效圣兽灵药\n");
	LOG2(L"使用速效圣兽灵药", "green");
	m_pGame->m_pGameProc->Click(975, 430, 980, 435);
	Sleep(300);
}

// 是否需要使用药包
bool Item::IsNeedUseYaoBao()
{
	//FFB83800 FFB83700 FFA43700 FFA03301 FFA43F09 FF963301 FF8E3606 FFA14D1E
	//FFB33200 FFB83700 FFA03501 FFA93E07 FF943102 FF9C3B0C FFBA6536 FFC97749

	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_hWndBig, 917, 608, 925, 610);
	bool result = m_pGame->m_pPrintScreen->GetPixelCount(0xFFB83700, 0x100) == 0; // 有药的颜色
	m_pGame->m_pPrintScreen->Release();
	return result;
}

// 背包是否打开
bool Item::BagIsOpen()
{
	// 截取的是背包关闭按钮前面一点
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 1150, 75, 1160, 85, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("背包", nullptr, 1) > 0; // 打开会遮住按钮
}

// 物品操作按钮是否打开
bool Item::ItemBtnIsOpen(int index)
{
	// 截取物品操作按钮 
	if (1 || index == 0)      // 第一行
		m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 145, 85, 155, 95, 0, true);
	else if (index == 1) // 第二行
		m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 145, 145, 155, 175, 0, true);
	else                 // 第三行
		m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 145, 215, 155, 245, 0, true);

	return m_pGame->m_pPrintScreen->ComparePixel("物品按钮", nullptr, 1) > 0;
}

// 等待物品操作按钮出现
bool Item::WaitForItemBtnOpen()
{
	for (int i = 0; i < 1000; i += 100) {
		if (ItemBtnIsOpen())
			return true;

		Sleep(100);
	}
	return false;
	//375,260 820,460 第一个BOSS区域扫描是否有宝宝药
}

// 拆分操作按钮是否出现
bool Item::ChaiFeiBtnIsOpen()
{
	// 截取拆分框取消按钮
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 466, 505, 476, 515, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("拆分框", nullptr, 1) > 0;
}

// 关闭拆分框
void Item::CloseChaiFeiBox()
{
	m_pGame->m_pGameProc->Click(466, 505, 566, 515);
}

// 点击按钮捡东西
bool Item::PickUpItemByBtn()
{
	LOG2(L"点击按钮拾取", "c0 b");
	//m_pGame->m_pEmulator->Tap(MyRand(26, 35), MyRand(656, 675));
	m_pGame->m_pGameProc->Click(26, 656, 35, 675);
	return WaitForPickUpItem();
}

// 仓库存钱按钮是否打开
bool Item::SaveMoneyBtnIsOpen()
{
	// 截取存钱确定按钮
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 665, 465, 675, 475, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("仓库存钱确定按钮", nullptr, 1) > 0;
}

// 仓库解锁栏位按钮是否打开
bool Item::StorageAddBtnIsOpen()
{
	// 仓库栏位解锁取消按钮
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 500, 380, 510, 390, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("仓库栏位解锁取消按钮", nullptr, 1) > 0;
}

// 捡物
int Item::PickUpItem(const char* name, int x, int y, int x2, int y2, int pickup_num)
{
	int count = 0;
	for (int i = 1; i <= pickup_num; i++) {
		char pick_name[64] = { 0 };
		int pos_x = 0, pos_y = 0;
		if (!GetGroundItemPos(name, x, y, x2, y2, pos_x, pos_y, pick_name))
			break;

		DbgPrint("%d.捡东西:%s(%d,%d)\n", i, pick_name, pos_x, pos_y);
		LOGVARN2(64, "c0", L"%d.捡东西:%hs(%d,%d)", i, pick_name, pos_x, pos_y);
		m_pGame->m_pGameProc->Click(pos_x, pos_y);
		WaitForPickUpItem();
		if (strcmp(name, ".") == 0) {
			m_pGame->m_pTalk->CloseZuiHouChouJiang();
			if (m_pGame->m_pTalk->ConfirmBtnIsOpen()) {
				m_pGame->m_pTalk->ClickConfirmBtn(1);
			}
		}

		count++;

		Sleep(260);
	}

	return count;
}

// 等待捡起物品
bool Item::WaitForPickUpItem(DWORD wait_ms)
{
	DWORD pos_x = 0, pos_y = 0;
	m_pGame->m_pGameData->ReadCoor(&pos_x, &pos_y, m_pGame->m_pGameProc->m_pAccount);

	DWORD x = 0, y = 0;
	m_pGame->m_pGameData->ReadCoor(&x, &y, m_pGame->m_pGameProc->m_pAccount);
	for (DWORD i = 0; i <= wait_ms; i += 620) {
		Sleep(620);

		DWORD now_x = 0, now_y = 0;
		m_pGame->m_pGameData->ReadCoor(&now_x, &now_y, m_pGame->m_pGameProc->m_pAccount);
		if (x == now_x && y == now_y)
			return now_x != pos_x && now_y != pos_y;

		x = now_x;
		y = now_y;
	}
	return false;
}

// 获取地面物品坐标
int Item::GetGroundItemPos(const char* name, int x, int y, int x2, int y2, int& pos_x, int& pos_y, char* pick_name)
{
	if (!x || !y || !x2 || !y2) {
		x = 360;
		y = 150;
		x2 = 880;
		y2 = 566;
	}

	wchar_t log[128];
	//LOGVARP2(log, "orange b", L"捡东西数量:%d (%d,%d)-(%d,%d)", 0, x, y, x2, y2);
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, x, y, x2, y2, 0, true);

	ConfItemInfo* items = m_pGame->m_pGameConf->m_stPickUp.PickUps; // 配置捡东西物品
	int length = m_pGame->m_pGameConf->m_stPickUp.Length; // 捡东西物品数量
	
	for (int i = 0; i < length; i++) {
		if (m_pGame->m_pGameProc->m_bPause)
			return 0;

		if (!strstr(name, ".") && strcmp(name, items[i].Name) != 0)
			continue;
		if (strcmp(name, "...") == 0 && strcmp("30星神兽碎片+3", items[i].Name) == 0)
			continue;
		if (strcmp(name, ".") == 0 || strcmp(name, ".nowait") == 0) { // 最后一关
			if (strcmp("30星神兽碎片+2", items[i].Name) != 0 && strcmp("30星神兽碎片+3", items[i].Name) != 0)
				continue;
		}
		if (strcmp(name, "速效治疗包") == 0) {
#if 0
			int yaobao = 0, yao = 0;
			GetQuickYaoOrBaoNum(yaobao, yao);
			if (yaobao >= 7) {
				LOGVARP2(log, "blue_r b", L"无须再俭药包, 数量:%d\n", yaobao);
				return 0;
			}	
#endif
		}

		strcpy(pick_name, items[i].Name);
		if (0 && strcmp(name, "速效治疗包") == 0 && !strstr(name, "."))
			LOGVARP2(log, "orange b", L"识别物品:%hs(%hs)", pick_name, items[i].Name);

		DWORD color = 0x00000000, diff = 0x00000000;
		DWORD color2 = 0x00000000, diff2 = 0x00000000;
		int tmp_x = 0, tmp_y = 0, tmp_x2 = 0, tmp_y2 = 0;
		if (strcmp("速效圣兽灵药", pick_name) == 0) {
			color = 0xFF72AA23;
			diff = 0x000B0B0B;
		}
		else if (strcmp("6星O礼包", pick_name) == 0) {
			//color = 0xFFBDA392;
			diff = 0x00080808;
		}
		else if (strcmp("12星XO礼包", pick_name) == 0) {
			//color = 0xFF5AB6DF;
			diff = 0x00080808;
		}
		else if (strcmp("25星XO礼包", pick_name) == 0) {
			color = 0xFF667EBE;
			diff = 0x00010101;
		}
		else if (strcmp("30星神兽碎片+1", pick_name) == 0) {
			color = 0xFFB6936B;
			diff = 0x00010101;
		}
		else if (strcmp("30星神兽碎片+2", pick_name) == 0) {
			color = 0xFF6278B2;
			diff = 0x00030303;
		}
		else if (strcmp("30星神兽碎片+3", pick_name) == 0) {
			color = 0xFFB885CA;
			diff = 0x00080808;
		}
		else if (strcmp("幻魔晶石", pick_name) == 0) {
			color = 0xFFE352ED;
			diff = 0x00020202;
		}
		else if (strcmp("魔魂晶石", pick_name) == 0) {
			color = 0xFFF5E880;
			diff = 0x00020202;
		}
		else if (strcmp("灵魂晶石", pick_name) == 0) {
			color = 0xFFF7C68E;
			diff = 0x00030303;
		}
		else if (strcmp("速效治疗包", pick_name) == 0) {
			if (!strstr(name, ".")) {
				color = 0xFFF66A6D;
				diff = 0x00151515;

				//color2 = 0xFFFA8ADE;
				//diff2 = 0x00202020;
			}
			else {
				color = 0xFFF66A6D;
				diff = 0x00030303;
			}
		}
		if (color == 0x00000000)
			continue;

		m_pGame->m_pPrintScreen->GetPixelPos(color, tmp_x, tmp_y, diff);
		if (color2) {
			//m_pGame->m_pPrintScreen->GetPixelPos(color2, tmp_x2, tmp_y2, diff2);
		}
		if ((tmp_x && tmp_y) || (tmp_x2 && tmp_y2)) {
			//LOGVARP2(log, "c0", L"发现物品:%hs, 坐标:%d,%d 坐标2:%d,%d", name, tmp_x, tmp_y, tmp_x2, tmp_y2);
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
	}
	//DbgPrint("捡物截图:%s(%d,%d %d,%d)\n", name, x, y, x2, y2);
	
	return 0;
}

// 丢弃药包
int Item::DropItem(ComImgIndex index, int live_count, DWORD* ms)
{
	DWORD _tm = GetTickCount();

	if (!OpenBag())
		Sleep(500);

	Sleep(300);

	bool is_use_item = false;
	int count = 0; // 数量
	for (int i = 0; i <= 3; i++) {
		if (m_pGame->m_pGameProc->m_bPause)
			break;

		if (!BagIsOpen()) {
			LOGVARN2(64, "red", L"背包没有打开, Wait Bag Open!");
			Sleep(100);
			if (!BagIsOpen()) {
				LOGVARN2(32, "red", L"背包没有打开");
				if (i > 1) {
					break;
				}
				else {
					Sleep(100);
					continue;
				}
			}
		}

		if (SlideBag(i)) { // 滑动背包
			Sleep(200);
		}

		ConfItemInfo* items = m_pGame->m_pGameConf->m_stDrop.Drops;
		int length = m_pGame->m_pGameConf->m_stDrop.Length;
		int idx = 0;
		for (idx = 0; idx < length; idx++) {
			if (i > 0 && strcmp("紫色祝福碎片", items[idx].Name) == 0)
				continue;

			ComPoint cp[32];
			PrintBagImg(true); // 截取背包图片
			int num = m_pGame->m_pPrintScreen->ComparePixel(items[idx].Name, cp, sizeof(cp) / sizeof(ComPoint));
			if (num == 0)
				continue;
			if (num < 3 && strcmp(items[idx].Name, "钥匙") == 0) // 钥匙只有一把不能丢
				continue;
			if (num <= 6 && strcmp(items[idx].Name, "速效治疗包") == 0)
				continue;
			//m_pGame->m_pPrintScreen->SaveBitmapToFile(bitmap, L"C:\\Users\\fucan\\Desktop\\MNQ-9Star\\jt.bmp");
			//DbgPrint("%d.丢.%s.背包物品数量:%d\n", i + 1, drops[idx], num);

			int drop_num = 0;

			if (count >= live_count) { // 需要全部丢
				drop_num = num;
			}
			else {
				drop_num = count + num - live_count;
			}
			drop_num = strcmp(items[idx].Name, "钥匙") != 0 ? num : 1; // 钥匙只丢最前面那一把
			if (strcmp(items[idx].Name, "速效治疗包") == 0)
				drop_num = num - 6;

			int btn_index = 1;
			if (strcmp("钥匙", items[idx].Name) == 0 || strcmp("清凉的圣水", items[idx].Name) == 0) // 只有一个操作按钮
				btn_index = 0;

			for (int j = 0; j < drop_num; j++) {
				if (j == 0) {
					DropItem(items[idx].Name, cp[0].x, cp[0].y, btn_index);
				}
				else {
					ComPoint item[1];
					PrintBagImg(true); // 截取背包图片
					if (!m_pGame->m_pPrintScreen->ComparePixel(items[idx].Name, item, sizeof(item) / sizeof(ComPoint)))
						break;

						DropItem(items[idx].Name, item[0].x, item[0].y, btn_index);
				}
				Sleep(600);

				if (ChaiFeiBtnIsOpen()) {
					CloseChaiFeiBox();
					Sleep(300);
				}
			}
			count += num;

			m_pGame->m_pGameProc->CloseTipBox();
		}

		if (1 || !is_use_item) {
			items = m_pGame->m_pGameConf->m_stUse.Uses;
			length = m_pGame->m_pGameConf->m_stUse.Length;
			for (idx = 0; idx < length; idx++) {
				ComPoint cp[32];
				PrintBagImg(true); // 截取背包图片
				int num = m_pGame->m_pPrintScreen->ComparePixel(items[idx].Name, cp, sizeof(cp) / sizeof(ComPoint));
				if (num == 0)
					continue;

				is_use_item = true;
				for (int j = 0; j < num; j++) {
					if (j == 0) {
						UseItem(items[idx].Name, cp[0].x, cp[0].y);
					}
					else {
						ComPoint item[1];
						PrintBagImg(true); // 截取背包图片
						if (!m_pGame->m_pPrintScreen->ComparePixel(items[idx].Name, item, sizeof(item) / sizeof(ComPoint)))
							break;

						UseItem(items[idx].Name, item[0].x, item[0].y);
					}
					Sleep(600);

					m_pGame->m_pGameProc->CloseTipBox();
					if (ChaiFeiBtnIsOpen()) {
						CloseChaiFeiBox();
						Sleep(300);
					}
				}
				count += num;

				m_pGame->m_pGameProc->CloseTipBox();
			}
		}
		
		if (i > 1) {
			if (m_pGame->m_pPrintScreen->ComparePixel("没有物品", nullptr, 1))
				break;
			if (m_pGame->m_pPrintScreen->ComparePixel("锁定格子", nullptr, 1))
				break;
		}
	}

	if (ChaiFeiBtnIsOpen()) {
		CloseChaiFeiBox();
		Sleep(300);
	}

	Sleep(200);
	CloseBag();
	Sleep(100);

	_tm = GetTickCount() - _tm;
	if (ms) {
		*ms = _tm;
	}
	
	DbgPrint("丢弃物品用时:%.2f秒, %d毫秒\n", (float)_tm/1000.0f, _tm);
	LOGVARN2(64, "c0", L"丢弃物品用时:%d秒, %d毫秒", _tm / 1000, _tm);
	return count;
}

// 使用物品
void Item::UseItem(const char* name, int x, int y)
{
	DbgPrint("使用物品:%s(%d,%d)\n", name, x, y);
	LOGVARN2(64, "c0", L"使用物品:%hs(%d,%d)", name, x, y);
	m_pGame->m_pGameProc->Click(x, y);
	if (WaitForItemBtnOpen()) {
		Sleep(150);
		// 点击物品出来操作按钮旁边的图标
		m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 320, 82, 375, 145, 0, true);
		if (m_pGame->m_pPrintScreen->ComparePixel("30星神兽碎片+3", nullptr, 1) > 0) // 不要去使用30星神兽碎片+3
			return;

		if (!ItemBtnIsOpen()) {
			LOGVARN2(64, "red", L"物品操作按钮未打开", name, x, y);
			Sleep(300);
		}

		GetItemBtnPos(x, y, 0);
		m_pGame->m_pGameProc->Click(x, y); // 点击使用按钮
		LOGVARN2(64, "c0", L"使用物品完成", name, x, y);

		if (0 && strstr(name, "30") != nullptr) {    // 合成活动选择
			//if (m_pGame->m_pTalk->WaitTalkOpen(0x00)) {
			    Sleep(1500);
				m_pGame->m_pTalk->Select(0x03);
				Sleep(800);
				m_pGame->m_pTalk->Select(0x00);
				Sleep(500);
			//}
		}
	}
	else {
		LOGVARN2(64, "c0", L"使用物品:%hs(%d,%d)失败", name, x, y);
	}
}

// 丢弃物品
void Item::DropItem(const char* name, int x, int y, int index)
{
	DbgPrint("丢物:%s(%d,%d)\n", name, x, y);
	LOGVARN2(64, "c0", L"丢物:%hs(%d,%d)", name, x, y);
	m_pGame->m_pGameProc->Click(x, y);
	if (WaitForItemBtnOpen()) {
		Sleep(100);
		if (strcmp("紫色祝福碎片", name) == 0 && ItemBtnIsOpen(2)) // 有第三个按钮, 那么是30星神兽碎片+3
			return;

		if (!ItemBtnIsOpen()) {
			LOGVARN2(64, "red", L"物品操作按钮未打开", name, x, y);
			Sleep(300);
		}

#if 1
		if (strcmp("钥匙", name) != 0) {
			// 点击物品出来操作按钮旁边的图标
			for (int n = 1; n <= 2; n++) { // 一共检查两次
				m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 320, 82, 375, 145, 0, true);
				if (m_pGame->m_pPrintScreen->ComparePixel("钥匙", nullptr, 1) > 0) // 丢到了钥匙
					return;

				Sleep(n < 2 ? 100 : 50);
			}
		}
		else {
			// 点击物品出来操作按钮旁边的图标
			for (int n = 1; n <= 2; n++) { // 一共检查两次
				// 点击物品出来物品名称(取钥匙字那里)
				m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 500, 76, 510, 86, 0, true);
				if (m_pGame->m_pPrintScreen->ComparePixel("卡利亚堡钥匙(丢弃名称)", nullptr, 1) > 0) // 丢到了钥匙
					return;

				Sleep(n < 2 ? 100 : 50);
			}
		}

		// 点击物品出来操作按钮旁边的图标
		m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 320, 82, 375, 145, 0, true);
		if (m_pGame->m_pPrintScreen->ComparePixel("勇气符石", nullptr, 1) > 0) // 勇气浮石
			return;
		
#else
		// 点击物品出来物品名称(取钥匙字那里)
		m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 500, 76, 510, 86, 0, true);
		if (strcmp("钥匙", name) != 0) {
			if (m_pGame->m_pPrintScreen->ComparePixel("卡利亚堡钥匙(丢弃名称)", nullptr, 1) > 0) // 丢到了钥匙
				return;
		}

		// 点击物品出来物品名称
		m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 415, 76, 425, 86, 0, true);
		if (m_pGame->m_pPrintScreen->ComparePixel("勇气符石(丢弃名称)", nullptr, 1) > 0) // 勇气浮石
			return;
#endif

		GetItemBtnPos(x, y, index > -1 ? index : 1);
		m_pGame->m_pGameProc->Click(x, y); // 点击丢弃按钮
		if (m_pGame->m_pTalk->WaitForConfirmBtnOpen()) {
			Sleep(100);
			m_pGame->m_pTalk->ClickConfirmBtn(1); // 确定按钮
		}
	}
	else {
		LOGVARN2(64, "c0", L"丢物:%hs(%d,%d)失败", name, x, y);
	}
}

// 售卖物品
int Item::SellItem(ConfItemInfo* items, DWORD length)
{
	for (int i = 0; i <= 3; i++) {
		if (m_pGame->m_pGameProc->m_bPause)
			break;

		if (!BagIsOpen()) {
			LOGVARN2(64, "red", L"背包没有打开, Wait Bag Open!");
			Sleep(100);
			if (!BagIsOpen()) {
				LOGVARN2(32, "red", L"背包没有打开");
				if (i > 1) {
					break;
				}
				else {
					Sleep(100);
					continue;
				}
			}
		}

		if (SlideBag(i)) { // 滑动背包
			Sleep(100);
		}

		for (DWORD iti = 0; iti < length; iti++) {
			for (int sell_i = 0; sell_i < 10; sell_i++) {
				ComPoint cp[32];
				PrintBagImg(true); // 截取背包图片
				int num = m_pGame->m_pPrintScreen->ComparePixel(items[iti].Name, cp, sizeof(cp) / sizeof(ComPoint));
				//DbgPrint("%d.售卖.背包物品数量:%d\n", i + 1, num);
				if (num == 0)
					break;

				for (int j = 0; j < num; j++) {
					if (j == 0) {
						SellItem(cp[0].x, cp[0].y);
						DbgPrint("%d.售卖:%hs(%d,%d)\n", j + 1, items[iti].Name, cp[j].x, cp[j].y);
						LOGVARN2(64, "c0", L"%d.售卖:%hs(%d,%d)", j + 1, items[iti].Name, cp[j].x, cp[j].y);
					}
					else {
						ComPoint item[1];
						PrintBagImg(true); // 截取背包图片
						if (!m_pGame->m_pPrintScreen->ComparePixel(items[iti].Name, item, sizeof(item) / sizeof(ComPoint)))
							break;

						SellItem(item[0].x, item[0].y);
						DbgPrint("%d.售卖:%hs(%d,%d)\n", j + 1, items[iti].Name, item[0].x, item[0].y);
						LOGVARN2(64, "c0", L"%d.售卖:%hs(%d,%d)", j + 1, items[iti].Name, item[0].x, item[0].y);
					}
					Sleep(600);
				}
			}
		}

		if (i > 1) {
			if (m_pGame->m_pPrintScreen->ComparePixel("没有物品", nullptr, 1))
				break;
			if (m_pGame->m_pPrintScreen->ComparePixel("锁定格子", nullptr, 1))
				break;
		}
	}

	return 0;
}

// 售卖物品
void Item::SellItem(int x, int y)
{
	//DbgPrint("卖物品:%d,%d\n", x, y);
	m_pGame->m_pGameProc->Click(x, y);
	if (WaitForItemBtnOpen()) {
		Sleep(100);
		GetItemBtnPos(x, y, 0);
		m_pGame->m_pGameProc->Click(x, y); // 点击丢弃按钮
		if (m_pGame->m_pTalk->WaitForConfirmBtnOpen()) {
			Sleep(100);
			m_pGame->m_pTalk->ClickConfirmBtn(1); // 确定按钮
		}
	}
}

// 获取物品操作按钮位置
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

// 存入仓库
void Item::CheckIn(ConfItemInfo* items, DWORD length)
{
	DbgPrint("存物\n");
	LOG2(L"存物", "c0");
	DWORD _tm = GetTickCount();

	OpenStorage();
	Sleep(1000);

	int pet_yao_1 = 0, pet_yao_2 = 1;
	for (int i = 0; i <= 3; i++) {
		if (m_pGame->m_pGameProc->m_bPause)
			break;

		if (!BagIsOpen()) {
			LOGVARN2(64, "red", L"背包没有打开, Wait Bag Open!");
			Sleep(100);
			if (!BagIsOpen()) {
				LOGVARN2(32, "red", L"背包没有打开");
				if (i > 1) {
					break;
				}
				else {
					m_pGame->m_pTalk->CloseAllBox();
					Sleep(100);
					continue;
				}
			}
		}

		m_pGame->m_pGameProc->CloseTipBox();
		if (SlideBag(i)) { // 滑动背包
			Sleep(300);
		}

		int pet_yao = 0;
		if (!pet_yao_1) {
			PrintBagImg(true); // 截取背包图片
			pet_yao_1 = m_pGame->m_pPrintScreen->ComparePixel("速效圣兽灵药", nullptr, 1) > 0;
			pet_yao = pet_yao_1;
		}
		else {
			pet_yao = m_pGame->m_pPrintScreen->ComparePixel("速效圣兽灵药", nullptr, 1) > 0;
		}

		bool is_save = false; // 是否存了东西
		for (DWORD iti = 0; iti < length; iti++) {
			ComPoint cp[32];
			PrintBagImg(true); // 截取背包图片
			int num = m_pGame->m_pPrintScreen->ComparePixel(items[iti].Name, cp, sizeof(cp) / sizeof(ComPoint));
			//DbgPrint("%d.存物.背包物品数量:%d\n", i + 1, num);
			if (num > 0) {
				for (int j = 0; j < num; j++) {
					if (j == 0) {
						m_pGame->m_pGameProc->Click(cp[0].x, cp[0].y);
						DbgPrint("%d.存物.%s:(%d,%d)\n", j + 1, items[iti].Name, cp[j].x, cp[j].y);
						LOGVARN2(64, "c0", L"%d.存物:%hs(%d,%d)", j + 1, items[iti].Name, cp[j].x, cp[j].y);
					}
					else {
						ComPoint item[1];
						PrintBagImg(true); // 截取背包图片
						if (!m_pGame->m_pPrintScreen->ComparePixel(items[iti].Name, item, sizeof(item) / sizeof(ComPoint)))
							break;

						m_pGame->m_pGameProc->Click(item[0].x, item[0].y);
						DbgPrint("%d.存物.%s:(%d,%d)\n", j + 1, items[iti].Name, item[0].x, item[0].y);
						LOGVARN2(64, "c0", L"%d.存物:%hs(%d,%d)", j + 1, items[iti].Name, item[0].x, item[0].y);
					}
					Sleep(800);
				}
				is_save = true;
			}
		}

		if (is_save && pet_yao_1 && pet_yao) {
			PrintBagImg(true); // 截取背包图片
			pet_yao_2 = m_pGame->m_pPrintScreen->ComparePixel("速效圣兽灵药", nullptr, 1);
		}

		if (i > 2) {
			if (m_pGame->m_pPrintScreen->ComparePixel("没有物品", nullptr, 1))
				break;
			if (m_pGame->m_pPrintScreen->ComparePixel("锁定格子", nullptr, 1))
				break;
		}
	}
	if (pet_yao_1 && !pet_yao_2) {
		DbgPrint("\n======速效圣兽灵药不见了, 需要取出来======\n\n");
		LOG2(L"\n======速效圣兽灵药不见了, 需要取出来======", "blue_r");
		CheckOutOne("速效圣兽灵药", false, false);
	}

	Sleep(150);
	CloseStorage();
	Sleep(100);

	_tm = GetTickCount() - _tm;
	DbgPrint("存物用时:%.2f秒, %d毫秒\n", (float)_tm / 1000.0f, _tm);
	LOGVARN2(64, "c0", L"存物用时:%d秒, %d毫秒", _tm / 1000, _tm);
}

// 取出仓库
int Item::CheckOut(ConfItemInfo* items, DWORD length)
{
	DbgPrint("取物\n");
	LOG2(L"取物", "c0");
	DWORD _tm = GetTickCount();

	OpenStorage();
	Sleep(1000);

	int count = 0;

	for (int i = 0; i <= 5; i++) {
		if (SlideStorge(i)) { // 滑动仓库
			Sleep(300);
		}

		for (DWORD iti = 0; iti < length; iti++) {
			ComPoint cp[32];
			PrintStorageImg(true); // 截取仓库图片
			int num = m_pGame->m_pPrintScreen->ComparePixel(items[iti].Name, cp, sizeof(cp) / sizeof(ComPoint));
			count += num;
			//DbgPrint("%d.取物.仓库物品数量:%d\n", i + 1, num);
			if (num > 0) {
				for (int j = 0; j < num; j++) {
					if (j == 0) {
						m_pGame->m_pGameProc->Click(cp[0].x, cp[0].y);
						DbgPrint("%d.取物.(%s:%d,%d)\n", j + 1, items[iti].Name, cp[j].x, cp[j].y);
						LOGVARN2(64, "c0", L"%d.取物:%hs(%d,%d)", j + 1, items[iti].Name, cp[j].x, cp[j].y);
					}
					else {
						ComPoint item[1];
						PrintStorageImg(true); // 截取仓库图片
						if (!m_pGame->m_pPrintScreen->ComparePixel(items[iti].Name, item, sizeof(item) / sizeof(ComPoint)))
							break;

						m_pGame->m_pGameProc->Click(item[0].x, item[0].y);
						DbgPrint("%d.取物.%s:(%d,%d)\n", j + 1, items[iti].Name, item[0].x, item[0].y);
						LOGVARN2(64, "c0", L"%d.取物:%hs(%d,%d)", j + 1, items[iti].Name, item[0].x, item[0].y);
					}
					Sleep(800);
				}
			}
		}

		if (0) {
			if (m_pGame->m_pPrintScreen->ComparePixel("没有物品", nullptr, 1))
				break;
			if (m_pGame->m_pPrintScreen->ComparePixel("锁定格子", nullptr, 1))
				break;
		}
	}

	// 仓库格子解锁是否打开
	while (StorageAddBtnIsOpen()) {
		m_pGame->m_pGameProc->Click(510, 380, 602, 400); // 取消
		Sleep(500);
	}

#if 1
	LOG2(L"存钱.", "orange");
	m_pGame->m_pGameProc->Click(150, 666, 235, 682); // 仓库存钱按钮
	for (int n = 0; n < 2000; n += 100) {
		Sleep(100);
		if (SaveMoneyBtnIsOpen()) {
			Sleep(500);
			m_pGame->m_pGameProc->Click(770, 382, 776, 385); // 滑块最右边
			Sleep(500);
			m_pGame->m_pGameProc->Click(750, 300, 760, 320); // +号, 以免0存不起
			Sleep(500);
			m_pGame->m_pGameProc->Click(680, 468, 750, 490); // 确定
			Sleep(500);
			break;
		}
	}

	while (SaveMoneyBtnIsOpen()) { // 检查是否还有按钮
		m_pGame->m_pGameProc->Click(460, 468, 550, 490); // 取消
		Sleep(300);
	}
	LOG2(L"存钱完成.", "orange");
#endif

	CloseStorage();

	for (int j = 0; j < 5; j++) {
		Sleep(500);
		if (!BagIsOpen())
			break;

		CloseStorage();
	}

	_tm = GetTickCount() - _tm;
	DbgPrint("取物用时:%.2f秒, %d毫秒\n", (float)_tm / 1000.0f, _tm);
	LOGVARN2(64, "c0", L"取物用时:%d秒, %d毫秒", _tm / 1000, _tm);

	return count;
}

// 取出一个仓库物品
int Item::CheckOutOne(const char * name, bool open, bool close)
{
	if (open) {
		OpenStorage();
		Sleep(1000);
	}
	
	int count = 0;
	for (int i = 0; i <= 5; i++) {
		if (SlideStorge(i)) { // 滑动仓库
			Sleep(300);
		}

		ComPoint cp[1];
		PrintStorageImg(true); // 截取仓库图片
		if (m_pGame->m_pPrintScreen->ComparePixel(name, cp, 1)) {
			DbgPrint("取物.%s:(%d,%d)\n", name, cp[0].x, cp[0].y);
			LOGVARN2(64, "c0", L"取物.%hs:(%d,%d)\n", name, cp[0].x, cp[0].y);
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

// 获取背包物品数量
int Item::GetBagCount(ComImgIndex index)
{
	DWORD _tm = GetTickCount();

	OpenBag();
	Sleep(1000);

	int count = 0; // 数量
	for (int i = 0; i <= 3; i++) {
		if (SlideBag(i)) { // 滑动背包
			Sleep(300);
		}

		PrintBagImg(true); // 截取背包图片
		int num = m_pGame->m_pPrintScreen->CompareImage(index);
		DbgPrint("%d.背包物品数量:%d\n", i + 1, num);
		LOGVARN2(64, "c0", L"%d.背包物品数量:%d", i + 1, num);

		count += num;
		if (count > 0 && num == 0) // 后面已没物品
			break;

		if (m_pGame->m_pPrintScreen->CompareImage(CIN_NoItem, nullptr, 1))
			break;
		if (m_pGame->m_pPrintScreen->CompareImage(CIN_LockItem, nullptr, 1))
			break;
	}

	Sleep(500);
	CloseBag();

	_tm = GetTickCount() - _tm;
	DbgPrint("获取物品用时:%.2f秒, %d毫秒\n", (float)_tm / 1000.0f, _tm);
	LOGVARN2(64, "c0", L"获取物品用时:%d秒, %d毫秒", _tm / 1000, _tm);
	return count;
}

// 获取快捷栏物品数量
int Item::GetQuickYaoOrBaoNum(int& yaobao, int& yao)
{
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 915, 520, 970, 640, 0, true);
	yaobao = m_pGame->m_pPrintScreen->LookNum(25, 0, 0, 25, 0xffffffff, 0x003A3A3A);
	yao = m_pGame->m_pPrintScreen->LookNum(0, 0, 95, 0, 0xffffffff, 0x003A3A3A);
	//DbgPrint("药包数量:%d 药数量:%d\n", yaobao, yao);

	return 0;
}

// 切换到技能快捷栏
void Item::SwitchMagicQuickBar()
{
#if 0
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_hWndBig, 1235, 350, 1245, 360, 0, true);
	if (m_pGame->m_pPrintScreen->ComparePixel("快捷栏切换按钮", nullptr, 1) == 0) { // 可能是XP技能栏
		DbgPrint("快捷栏切换按钮\n");
		SwitchQuickBar(1);
		Sleep(500);
	}
#endif

	if (QuickBarIsXingChen()) { // 切换
		SwitchQuickBar(1);
		Sleep(500);
	}
}

// 快捷栏是否有星辰之眼
bool Item::QuickBarIsXingChen()
{
	m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 926, 510, 936, 520, 0, true);
	return m_pGame->m_pPrintScreen->ComparePixel("星辰之眼快捷栏", nullptr, 1) > 0;
}

// 切换快捷栏
void Item::SwitchQuickBar(int page)
{
	DbgPrint("切换到快捷栏:%d\n", page);
	m_pGame->m_pGameProc->Click(1235, 360, 1250, 370);
}

// 获取物品的ComImgIndex
ComImgIndex Item::GetItemComImgIndex(const char * name)
{
	if (strcmp(name, "双子星青螭礼包") == 0)
		return CIN_QingChi;
	if (strcmp(name, "双子星紫冥礼包") == 0)
		return CIN_ZiMing;
	if (strcmp(name, "白羊星卡迪礼包") == 0)
		return CIN_KaDi;
	if (strcmp(name, "白羊星穆巴礼包") == 0)
		return CIN_MuBa;
	if (strcmp(name, "金牛星亚尔礼包") == 0)
		return CIN_YaEr;
	if (strcmp(name, "金牛星鲁迪礼包") == 0)
		return CIN_LuDi;
	if (strcmp(name, "幻魔晶石") == 0)
		return CIN_HuanMo;
	if (strcmp(name, "魔魂晶石") == 0)
		return CIN_MoHun;
	if (strcmp(name, "灵魂晶石") == 0)
		return CIN_LingHun;
	if (strcmp(name, "爱娜的项链") == 0)
		return CIN_XiangLian;

	return CIN_NoItem;
}

// 截取背包图片
HBITMAP Item::PrintBagImg(bool del)
{
	HBITMAP bitmap = m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_pGameProc->m_hWndGame, 672, 158, 1170, 620);
	if (del) {
		m_pGame->m_pPrintScreen->GetPixel(0, 0);
		DeleteBagImg();
	}
	return bitmap;
}

// 删除背包图片
void Item::DeleteBagImg()
{
	m_pGame->m_pPrintScreen->Release();
}

// 截取仓库图片
HBITMAP Item::PrintStorageImg(bool del)
{
	HBITMAP bitmap = m_pGame->m_pPrintScreen->CopyScreenToBitmap(m_pGame->m_hWndBig, 113, 158, 620, 620);
	if (del) {
		m_pGame->m_pPrintScreen->GetPixel(0, 0);
		DeleteBagImg();
	}
	return bitmap;
}

// 删除仓库图片
void Item::DeleteStorageImg()
{
	m_pGame->m_pPrintScreen->Release();
}
