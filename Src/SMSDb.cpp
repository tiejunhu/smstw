#include "SMSDb.h"
#include "SMSTools.h"
#include "conv.h"
#include "AddrDb.h"
#include "Global.h"

static DmOpenRef PrivOpenSMSDb()
{
	return DmOpenDatabaseByTypeCreator(SMSTW_DB_TYPE, SMSTW_CREATOR_ID, dmModeReadWrite);
}

static UInt32 PrivReadTime(DmOpenRef db, int index)
{
	MemHandle memHandle = DmQueryRecord(db, index);
	void* pData = MemHandleLock(memHandle);
	UInt32 time = *(UInt32 *)pData;
	MemHandleUnlock(memHandle);
	return time;
}

static UInt8 PrivReadState(DmOpenRef db, int index)
{
	MemHandle memHandle = DmQueryRecord(db, index);
	char* pData = (char *) MemHandleLock(memHandle);
	UInt8 state = *(UInt8 *)(pData + 8);
	MemHandleUnlock(memHandle);
	
	return state;
}

static Int16 SeekRecordInCategory(DmOpenRef db, Int16 category, UInt16 offset, UInt16 startIndex)
{
//	static UInt16 lastCategory = -1;
	if (category < 0) {
		return startIndex + offset;
	}
	
	UInt16 total = DmNumRecords(db);
	for (UInt16 i = startIndex; i < total; ++i) {
		UInt16 attr;
		DmRecordInfo(db, i, &attr, NULL, NULL);
		if (category == (attr & dmRecAttrCategoryMask)) {
			if (offset == 0) return i;
			offset --;
		}
	}
	return -1;
}

static UInt16 PrivFindIndexToInsert(DmOpenRef smsDb, SMSDbRecordType* pRecord)
{
	return dmMaxRecordIndex;
}

#define localizedAppInfoStrID 1000

static Err PrivInitAppInfo(DmOpenRef db)
{
	LocalID dbID;
	UInt16 cardNo;
	LocalID appInfoID;

	if (DmOpenDatabaseInfo(db, &dbID, NULL, NULL, &cardNo, NULL))
		return dmErrInvalidParam;

	if (DmDatabaseInfo(cardNo, dbID, NULL, NULL, NULL, NULL, NULL, NULL, 
		 NULL, &appInfoID, NULL, NULL, NULL))
		return dmErrInvalidParam;

	if (appInfoID == 0) {
		MemHandle h = DmNewHandle(db, sizeof (AppInfoType));
		if (! h) return dmErrMemError;

		appInfoID = MemHandleToLocalID (h);
		DmSetDatabaseInfo(cardNo, dbID, NULL, NULL, NULL, NULL, NULL, NULL, 
			NULL, &appInfoID, NULL, NULL, NULL);
	}
		
	AppInfoPtr appInfoP = (AppInfoPtr) MemLocalIDToLockedPtr(appInfoID, cardNo);
	DmSet (appInfoP, 0, sizeof(AppInfoType), 0);
	CategoryInitialize(appInfoP, localizedAppInfoStrID);
	MemPtrUnlock (appInfoP);

	return errNone;
}

DmOpenRef OpenSMSDb()
{
	DmOpenRef db = PrivOpenSMSDb();
	if (db == NULL) {
		Err err = DmGetLastErr();
		if (err == dmErrCantFind) {
			err = DmCreateDatabase (0, SMSTW_DB_FILE, SMSTW_CREATOR_ID, SMSTW_DB_TYPE, false);
			if (err) {
				return NULL;
			}
			db = PrivOpenSMSDb();
			PrivInitAppInfo(db);
		}
	}
	return db;
}

void CloseSMSDb(DmOpenRef db)
{
	if (db) {
		DmCloseDatabase(db);
	}
}

static inline UInt32 SizeOfRecord(SMSDbRecordType* pRecord)
{
	CheckNullPointer(pRecord);
	return sizeof(pRecord->time) + sizeof(pRecord->state) + 
		StrLen(pRecord->phoneNumber) + 1 + StrLen(pRecord->personName) + 1 +
		sizeof(pRecord->size) + pRecord->size;
}

static void WriteRecord(MemHandle memHandle, SMSDbRecordType* pRecord)
{
	LOG("WriteRecord::Starting");
	void* pData = MemHandleLock(memHandle);
	CheckNullPointer(pData);
	
	UInt32 offset = 0;

	UInt32 bytes = sizeof(pRecord->time);
	DmWrite(pData, offset, &(pRecord->time), bytes);
	offset += bytes;
	
	bytes = sizeof(pRecord->size);
	DmWrite(pData, offset, &(pRecord->size), bytes);
	offset += bytes;

	bytes = sizeof(pRecord->state);
	DmWrite(pData, offset, &(pRecord->state), bytes);
	offset += bytes;
	
	bytes = StrLen(pRecord->phoneNumber) + 1;
	DmWrite(pData, offset, pRecord->phoneNumber, bytes);
	offset += bytes;

	bytes = StrLen(pRecord->personName) + 1;
	DmWrite(pData, offset, pRecord->personName, bytes);
	offset += bytes;

	bytes = pRecord->size;
	DmWrite(pData, offset, pRecord->content, bytes);
	offset += bytes;
		
	MemHandleUnlock(memHandle);

	LOG("WriteRecord::Left");
}

void MoveRecordCategory(DmOpenRef db, UInt16 pos, Int16 oldcategory, Int16 category)
{
	CheckNullPointer(db);
	Int16 realIndex = SeekRecordInCategory(db, oldcategory, pos, 0);
	UInt16 attr;
	DmRecordInfo(db, realIndex, &attr, NULL, NULL);
	attr &= ~dmRecAttrCategoryMask;
	attr |= category;
	DmSetRecordInfo(db, realIndex, &attr, NULL);
}

Int16 AddSMSRecord(DmOpenRef db, SMSDbRecordType* pRecord, Int16 category)
{
	LOG("AddSMSRecord::Starting");
	CheckNullPointer(db);

	UInt16 pos = PrivFindIndexToInsert(db, pRecord);
	MemHandle memHandle = DmNewRecord(db, &pos, SizeOfRecord(pRecord));
	WriteRecord(memHandle, pRecord);
	DmReleaseRecord(db, pos, true);

	MoveRecordCategory(db, pos, -1, category);

	pRecord->index = pos;
	
	LOG("AddSMSRecord::Left");
	return pos;
}

void ReleaseSMSRecord(DmOpenRef db, SMSDbRecordType* pRecord, Boolean dirty)
{
	DmReleaseRecord(db, pRecord->index, dirty);
}

/**
 * category: 分类id，若传入-1，则不区分分类
 * index: 在指定分类下的序号，从0开始
 * record: 返回的记录结果
 * memHandle: 字符串对应的memHandle，使用完record后要Unlock这个handle
 * readOnly: 是否只读？
 */
Err GetSMSRecord(DmOpenRef db, Int16 category, UInt16 index, 
	SMSDbRecordType& record, MemHandle& memHandle, Boolean readOnly)
{
	CheckNullPointer(db);
	
	UInt32 count = GetSMSCount(db, category);
	if (index >= count) return dmErrCantFind;
	
	Int16 realIndex = SeekRecordInCategory(db, category, index, 0);
	if (realIndex < 0) return dmErrCantFind;
	
	record.index = realIndex;
	
	if (readOnly)
		memHandle = DmQueryRecord(db, realIndex);
	else 
		memHandle = DmGetRecord(db, realIndex);
	
	if (memHandle == NULL) return dmErrCantFind;
		
	unsigned char* pData = (unsigned char*) MemHandleLock(memHandle);
	
	CheckNullPointer(pData);
	
	record.data = pData;
	
	record.time = *((UInt32*) pData);
	pData += sizeof(UInt32);
	
	record.size = *((UInt32*) pData);
	pData += sizeof(UInt32);

	record.state = *pData;
	pData += 1;
	
	char* pStr = (char *) pData;
	UInt32 uStrLen = StrLen(pStr) + 1;
	record.phoneNumber = pStr;
	pData += uStrLen;
	
	pStr = (char *) pData;
	uStrLen = StrLen(pStr) + 1;
	record.personName = pStr;
	pData += uStrLen;
	
	pStr = (char *) pData;
	record.content = pStr;
	
	return errNone;
}

void FreeSMSRecord(SMSDbRecordType* pRecord)
{
	if (pRecord) {
		if (pRecord->phoneNumber) MemPtrFree(pRecord->phoneNumber);
		if (pRecord->personName) MemPtrFree(pRecord->personName);
		if (pRecord->content) MemPtrFree(pRecord->content);
		MemPtrFree(pRecord);
	}
}

SMSDbRecordType* AllocSMSRecord()
{
	return (SMSDbRecordType*) MemPtrNew(sizeof(SMSDbRecordType));
}

void InitSMSRecord(SMSDbRecordType* pRecord)
{
	MemSet(pRecord, sizeof(SMSDbRecordType), 0);
}


void ClearSMSDb(DmOpenRef db)
{
	if (db == NULL) return;
	while(DmNumRecords(db) > 0) DmRemoveRecord(db, 0);
}

UInt32 GetSMSCount(DmOpenRef db, Int16 category)
{
	if (db == NULL) return 0;
	if (category < 0) return DmNumRecords(db);
	return DmNumRecordsInCategory(db, category);
}

static Int16 CompareSMS(void *rec1, void *rec2, Int16 other, 
						SortRecordInfoPtr rec1SortInfo,
						SortRecordInfoPtr rec2SortInfo,
				 		MemHandle appInfoH) 
{
	UInt32 time1 = *(UInt32 *)rec1;
	UInt32 time2 = *(UInt32 *)rec2;
	
	if (time1 > time2) return -1;
	if (time1 == time2) return 0;
	return 1;
}

void SortSMS(DmOpenRef db)
{
	CheckNullPointer(db);
	DmQuickSort(db, CompareSMS, 0);
}

SMSDbRecordType* CopySMSRecord(SMSDbRecordType* pRecord)
{
	SMSDbRecordType* pResult = AllocSMSRecord();
	InitSMSRecord(pResult);
	MemMove(pResult, pRecord, sizeof(SMSDbRecordType));
	AssignStr(&(pResult->phoneNumber), pRecord->phoneNumber, 0);
	AssignStr(&(pResult->personName), pRecord->personName, 0);
	AssignStr(&(pResult->content), pRecord->content, 0);
	return pResult;
}

/*
void DecodeSMSRecord(DmOpenRef db, gb2312_table &table, Int16 category, UInt16 index, char* buf)
{
	CheckNullPointer(db);

	SMSDbRecordType record;
	MemHandle memHandle;

	GetSMSRecord(db, category, index, record, memHandle, true);

//	UInt32 size = buffer_to_gb2312(table, (unsigned char *) record.content, (unsigned char *) buf, record.size);
//	buf[size] = 0;

	MemHandleUnlock(memHandle);
}
*/

Int16 GetNextUnreadIndex(DmOpenRef db, Int16 category)
{
	UInt16 total = DmNumRecords(db);
	for (UInt16 i = 0; i < total; ++i) {
		UInt16 attr;
		DmRecordInfo(db, i, &attr, NULL, NULL);
		if (category == (attr & dmRecAttrCategoryMask)) {
			UInt8 state = PrivReadState(db, i);
			if (!IsRead(state)) {
				return i;
			}
		}
	}
	return -1;
}

void PurgeCategory(DmOpenRef db, Int16 category)
{
	if (category == -1) {
		while(DmNumRecords(db) > 0) DmRemoveRecord(db, 0);
		return;
	}
	
	while(GetSMSCount(db, category) > 0) {
		Int16 index = SeekRecordInCategory(db, category, 0, 0);
		DmRemoveRecord(db, index);
	}
}

void NewRecordInCategory(DmOpenRef db, const char* pszTo, const char* pszContent, Int16 category, UInt8 state)
{
	MethodLogger logger("NewRecordInCategory");
	SMSDbRecordType* sms = AllocSMSRecord();
	InitSMSRecord(sms);
	
	char* userName = (char *) MemPtrNew(100);
	lookupNameFromAddressBook(GetNakedPhoneNumber(pszTo), userName);

	sms->state = state;
	sms->phoneNumber = (char *) pszTo;
	sms->content = (char *) pszContent;
	sms->personName = userName;
	
	sms->size = StrLen(pszContent) + 1;
	sms->time = TimGetSeconds();
	
	ReadRecord(sms);
	
	AddSMSRecord(db, sms, category);
	
	sms->phoneNumber = NULL;
	sms->content = NULL;	
	FreeSMSRecord(sms);
}

void DeleteRecordInCategory(DmOpenRef db, UInt16 pos, Int16 category)
{
	Int16 index = SeekRecordInCategory(db, category, pos, 0);
	DmRemoveRecord(db, index);
}

void MarkRecordDeliveredByPhoneNumber(DmOpenRef db, const char* phoneNumber)
{
	LOG("MarkRecordDeliveredByPhoneNumber::Starting");
	CheckNullPointer(db);
	const char* nakedNumber = GetNakedPhoneNumber(phoneNumber);
	UInt16 total = DmNumRecords(db);
	if (total == 0) goto exit;
	for (Int16 i = 0; i < total; i++) {
		UInt16 attr;
		DmRecordInfo(db, i, &attr, NULL, NULL);
		if (CAT_SENT == (attr & dmRecAttrCategoryMask)) {
			MemHandle memHandle = 0;
			SMSDbRecordType smsRecord;
			Err err = GetSMSRecord(db, -1, i, smsRecord, memHandle, false);
			if (err) continue;
			Boolean dirty = false;
			if (StrStr(smsRecord.phoneNumber, nakedNumber)) {
				if (IsRecordRequestReport(&smsRecord)) {
					if (!IsRecordDelivered(&smsRecord)) {
						RecordDelivered(&smsRecord);
						DmSet(smsRecord.data, (UInt32) &(((SMSDbRecordType *)NULL)->state), 1, smsRecord.state);
						dirty = true;
					}
				}
			}
			ReleaseSMSRecord(db, &smsRecord, dirty);
			MemHandleUnlock(memHandle);
			if (dirty) goto exit;
		}
	}
exit:
	LOG("MarkRecordDeliveredByPhoneNumber::Left");
}

UInt16 CountUnread(DmOpenRef db)
{
	UInt16 count = 0;
	UInt16 total = DmNumRecords(db);
	for (UInt16 i = 0; i < total; ++i) {
		UInt16 attr;
		DmRecordInfo(db, i, &attr, NULL, NULL);
		if (CAT_INBOX == (attr & dmRecAttrCategoryMask)) {
			UInt8 state = PrivReadState(db, i);
			if (!IsRead(state)) {
				count ++;
			}
		}
	}
	return count;
}