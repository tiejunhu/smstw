#ifndef _SMSTW_DB
#define _SMSTW_DB

#include "conv.h"

#define SMSTW_DB_FILE "SMSTWDb"
#define SMSTW_DB_TYPE 'data'
#define SMSTW_CREATOR_ID 'SMTW'



typedef struct {
	UInt32 time; // 32 bit datetime
	UInt32 size; // size of content, including end \0, put here for allignment
	unsigned state:8; // 8 bitwise state
	char* phoneNumber; // phone number string
	char* personName; // name string of this person
	char* content; // content of sms
	UInt16 index; // index in db
	void* data; // data pointer original data
} SMSDbRecordType;

/* 8 7 6 5 4 3 2 1
   1: 0 - unread, 1 - read
   2: 0 - not decoded, 1 - decoded
   3: 0 - do not request deliver report, 1 - request
   4: 0 - hasn't gotten deliver report, 1 - gotten
   */

#define IsRead(state) \
	(state & 0x01)

#define IsRecordRead(p) \
	((p)->state & 0x01)

#define ReadRecord(p) \
	{ (p)->state = (p)->state | 0x01; }
	
#define UnReadRecord(p) \
	{ (p)->state = (p)->state & (~0x01); }

#define IsRecordDecoded(p) \
	((p)->state & 0x02)
	
#define DecodeRecord(p) \
	{ (p)->state = (p)->state | 0x02; }

#define UnDecodeRecord(p) \
	{ (p)->state = (p)->state & (~0x02); }

#define IsRecordRequestReport(p) \
	((p)->state & 0x04)
	
#define IsRecordDelivered(p) \
	((p)->state & 0x08)

#define RecordDelivered(p) \
	{ (p)->state = (p)->state | 0x08; }
	
#define DonotRequestReport(s) \
	{ s = s & (~0x04); }

#define RequestReport(s) \
	{ s = s | 0x04; }

DmOpenRef OpenSMSDb();
void CloseSMSDb(DmOpenRef db);
void ClearSMSDb(DmOpenRef db);

Err GetSMSRecord(DmOpenRef db, Int16 category, UInt16 index, 
	SMSDbRecordType& record, MemHandle& memHandle, Boolean readOnly);
	
UInt32 GetSMSCount(DmOpenRef db, Int16 category);
void ReleaseSMSRecord(DmOpenRef db, SMSDbRecordType* pRecord, Boolean dirty);
Int16 AddSMSRecord(DmOpenRef db, SMSDbRecordType* pRecord, Int16 category);
Int16 GetNextUnreadIndex(DmOpenRef db, Int16 category);
void MoveRecordCategory(DmOpenRef db, UInt16 pos, Int16 oldcategory, Int16 category);
void NewRecordInCategory(DmOpenRef db, const char* pszTo, const char* pszContent, Int16 category, UInt8 state);
void DeleteRecordInCategory(DmOpenRef db, UInt16 pos, Int16 category);
UInt16 CountUnread(DmOpenRef db);
//void DecodeSMSRecord(DmOpenRef db, gb2312_table &table, Int16 category, UInt16 index, char* buf);

void FreeSMSRecord(SMSDbRecordType* pRecord);
SMSDbRecordType* AllocSMSRecord();
SMSDbRecordType* CopySMSRecord(SMSDbRecordType* pRecord);
void InitSMSRecord(SMSDbRecordType* pRecord);

void SortSMS(DmOpenRef);
void PurgeCategory(DmOpenRef db, Int16 category);
void MarkRecordDeliveredByPhoneNumber(DmOpenRef db, const char* phoneNumber);

#endif