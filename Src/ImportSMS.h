#ifndef _SMSTW_IMPORT
#define _SMSTW_IMPORT

#include "conv.h"

#define OLD_CAT_INBOX 0
#define OLD_CAT_SENT 1
#define OLD_CAT_TRASH 2
#define OLD_CAT_ARCHIVE 3
#define OLD_CAT_DRAFT 4

void ImportSMS(DmOpenRef smsDb, gb2312_table &table, UInt32 intFrom, UInt32 intTo);

#endif