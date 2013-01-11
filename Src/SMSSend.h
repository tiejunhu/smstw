#ifndef _SMTW_SEND
#define _SMTW_SEND

#include "conv.h"

Err SendSMS(gb2312_table& table, const char* center, 
	const char* dest, const char* content);
	
void SendAllInOutbox(DmOpenRef db, gb2312_table& table, const char* center);

#endif