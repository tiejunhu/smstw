#ifndef _SMTW_REPLY
#define _SMTW_REPLY

#include "conv.h"

Boolean ReplyFormHandleEvent(EventType * eventP);
Err SendReply(DmOpenRef db, gb2312_table& table);

#endif
