#include "Global.h"

DmOpenRef g_SmsDb = NULL;
DmOpenRef g_GbTableDb = NULL;
gb2312_table g_gbTable;
UInt16 g_SelectedCategory = 0;
ComposeMode g_ComposeMode = COMPOSE_NEW;
UInt32 g_DisplayUniqId = -1;

const char* DayOfWeekInChinese[] = {
	"��", "һ", "��", "��", "��", "��", "��"
};


