#include "conv.h"
#include "ucs2.h"
#include "euc_cn.h"
#include <TelephonyMgr.h>

#include "SMSTools.h"
#include "Global.h"

UInt32 ucs2_to_gb2312(gb2312_table& table, const unsigned char* ucs2_buf, unsigned char* gb_buf, UInt32 len)
{
	UInt32 i = 0;
	UInt32 o = 0;
	UInt32 ucs4;
	struct conv_struct conv;
	int r;
	
	MemSet(&conv, sizeof(struct conv_struct), 0);
	
	while (i < len - 1) {
	/*
		if (*(ucs2_buf + i) == 0) {
			*(gb_buf + o) = *(ucs2_buf + i + 1);
			i += 2;
			o ++;
			continue;
		}*/
		r = ucs2_mbtowc(&conv, &ucs4, ucs2_buf + i, 2);
		i += 2;
		if (r != 2) {
			continue;
		}

		r = euc_cn_wctomb(&conv, table, gb_buf + o, ucs4, 2);
		if (r < 0) {
			continue;
		}
		o += r;
	}
	
	return o;
}

UInt32 gb2312_to_ucs2(gb2312_table& table, const unsigned char* gb_buf, unsigned char* ucs2_buf, UInt32 len)
{
	UInt32 i = 0;
	UInt32 o = 0;
	UInt32 ucs4;
	struct conv_struct conv;
	int r;
	
	MemSet(&conv, sizeof(struct conv_struct), 0);
	while (i < len) {
		r = euc_cn_mbtowc(&conv, table, &ucs4, gb_buf + i, len - i);
		if (r < 0) {
			i++;
			continue;
		}
		i += r;
		r  = ucs2_wctomb(&conv, ucs2_buf + o, ucs4, 2);
		if (r < 0) continue;
		o += 2;
	}
	
	return o;
}

UInt32 buffer_to_gb2312(gb2312_table& table, const unsigned char* buf, unsigned char* gb_buf, UInt32 len, Int8 coding)
{
	LOG("buffer_to_gb2312::starting");
	
	int k = 0;
	Boolean isNeedDecode = true;
	char* basePointer = (char *) buf;
	UInt32 resultLen = len;
	
	if (coding == -1) {
		for(k = 0; k < len; k ++) {
			if((*basePointer > 127) || *basePointer < 32)
				break;
			basePointer++;
		}
		if (k == len) isNeedDecode = false;
	} else if (coding == kTelSmsUCS2Encoding) {
		isNeedDecode = true;
	} else {
		isNeedDecode = false;
	}


	if (isNeedDecode) {
		resultLen = ucs2_to_gb2312(table, buf, gb_buf, len);
	} else {
		StrNCopy((char *) gb_buf, (const char *) buf, len);
	}

	LOG("buffer_to_gb2312::left");
	
	return resultLen;
}

#define LibName "gb2312table"

#define set_var_to_ushort_record(var, i) \
	table.memHandle[i] = DmQueryRecord(libRef, i); \
	table.var = (unsigned short *) MemHandleLock(table.memHandle[i]);

#define set_var_to_summary_record(var, i) \
	table.memHandle[i] = DmQueryRecord(libRef, i); \
	table.var = (Summary16 *) MemHandleLock(table.memHandle[i]);

DmOpenRef init_gb2312_table(gb2312_table& table)
{
	DmOpenRef libRef = NULL;
	LocalID dbId = DmFindDatabase(0, LibName);
	if (dbId == NULL) {
		libRef = NULL;
		ShowMsg("Please install gb2312table.");
		return NULL;
	} else {
		libRef = DmOpenDatabase(0, dbId, dmModeReadOnly);
	}
	
	set_var_to_ushort_record(gb2312_2uni_page21,     0);
	set_var_to_ushort_record(gb2312_2uni_page30,     1);
	set_var_to_ushort_record(gb2312_2charset,        2);
	set_var_to_summary_record(gb2312_uni2indx_page00, 3);
	set_var_to_summary_record(gb2312_uni2indx_page20, 4);
	set_var_to_summary_record(gb2312_uni2indx_page30, 5);
	set_var_to_summary_record(gb2312_uni2indx_page4e, 6);
	set_var_to_summary_record(gb2312_uni2indx_page9e, 7);
	set_var_to_summary_record(gb2312_uni2indx_pageff, 8);
	
	return libRef;
}

void fini_gb2312_table(DmOpenRef db, gb2312_table& table)
{
	int i = 0;
	for (i = 0; i < 9; ++i) MemHandleUnlock(table.memHandle[i]);
	DmCloseDatabase(db);
}
