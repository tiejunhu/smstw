#include "ImportSMS.h"
#include "SMSDb.h"
#include "AddrDb.h"
#include "conv.h"
#include "Global.h"

#define GOTO_ERROR_ON_NULL(p) \
	CheckNullPointer(p); if (p == NULL) goto error;

#define GOTO_ERROR_ON_ERR(e) \
	CHECKERR(e); if (e) goto error;

#define SMSBUF_SIZE 400

static SMSDbRecordType* DecodeRec(DmOpenRef internalSmsDb, gb2312_table &table, 
	int index, UInt16* category, char* smsBuf)
{
	LOG("DecodeRec::Starting");
	MemHandle memHandle = NULL;
	unsigned char* pRecord = NULL;
	unsigned char * basePointer;

	int smsDispLen = 0;
	int smsLen = 0;
	
	
	SMSDbRecordType* pResult = AllocSMSRecord();
	GOTO_ERROR_ON_NULL(pResult);
	
	InitSMSRecord(pResult);
	
	UInt16 attr;
	Err err = DmRecordInfo(internalSmsDb, index, &attr, NULL, NULL);
	GOTO_ERROR_ON_ERR(err);
	
	*category = attr & dmRecAttrCategoryMask;
	LOG("DecodeRec::Got category");
	
	memHandle = DmQueryRecord(internalSmsDb, index);
	GOTO_ERROR_ON_NULL(memHandle);
	
	pRecord = (unsigned char*) MemHandleLock(memHandle);
	GOTO_ERROR_ON_NULL(pRecord);

	LOG("DecodeRec::Record locked");
	
	unsigned char* pointer = pRecord;

	// time	
	pointer = pRecord + 10;
	pResult->time = *((UInt32 *)pointer);
	
	//get phone number
	pointer = pRecord + 18;
	AssignStr(&(pResult->phoneNumber), (char *) pointer, 0);

	// skip phone number and a \0 string end
	pointer += StrLen(pResult->phoneNumber) + 1;
	
	//get phone username
	char* userName = (char *) MemPtrNew(50);
	StrCopy(userName, (const char *)pointer);
	pointer += StrLen(userName);
	
	if (StrLen(userName) == 0) {
		lookupNameFromAddressBook(pResult->phoneNumber, userName);
	}
	AssignStr(&(pResult->personName), userName, 0);
	MemPtrFree(userName);
	LOG("DecodeRec::Got time, phone number, phone username");

	
	pointer += 5;
	smsLen = *pointer;
	
	if(smsLen == 0) {
		AssignStr(&(pResult->content), "", 0);
		goto exit;
	}
	
	basePointer = pointer + 1;
	smsDispLen = buffer_to_gb2312(table, basePointer, (unsigned char *) smsBuf, smsLen, -1);
	AssignStr(&(pResult->content), smsBuf, smsDispLen);
	pResult->size = smsDispLen + 1;
	
	DecodeRecord(pResult);
	ReadRecord(pResult);

	LOG("DecodeRec::Content decoded");
	
exit:	
	MemHandleUnlock(memHandle);
	LOG("DecodeRec::Left");
	return pResult;

error:
	if (pRecord) MemHandleUnlock(memHandle);
	FreeSMSRecord(pResult);
	LOG("DecodeRec::Left with error");
	return NULL;
}

static char CategoryMap[] = {
	0, // Inbox->Inbox
	1, // Sent->Sent
	5, // Trash->Trash
	4, // Archive->Archive
	3, // Draft->Draft
};

void ImportSMS(DmOpenRef smsDb, gb2312_table &table, UInt32 intFrom, UInt32 intTo)
{
	LOG("ImportSMS::Starting");
	DmOpenRef internalSmsDb = DmOpenDatabaseByTypeCreator('data', 'smsm', dmModeReadOnly);
	if (internalSmsDb == NULL) {
		LOG("ImportSMS::Cannot open internal sms db");
		return;
	}

	UInt32 count = DmNumRecords(internalSmsDb);
	if (intFrom < 0) intFrom = 0;
	if (intTo >= count) intTo = count - 1;

	MemHandle memHandle = MemHandleNew(SMSBUF_SIZE);
	if (memHandle == NULL) goto err;
	char* smsBuf = (char *) MemHandleLock(memHandle);
	if (smsBuf == NULL) goto err;
	
	LOG("ImportSMS::decode buffer allocated");

	for (UInt32 i = intFrom; i <= intTo; ++i) {
		UInt16 category = 0;

		MemSet(smsBuf, SMSBUF_SIZE, 0);
		SMSDbRecordType* pRecord = DecodeRec(internalSmsDb, table, i, &category, smsBuf);
		if ((category < 0) || (category > 4)) category = 0;
		if (pRecord) {
			LOG("ImportSMS::Adding decoded sms to db");
			AddSMSRecord(smsDb, pRecord, CategoryMap[category]);
			LOG("ImportSMS::Delocating sms record");
			FreeSMSRecord(pRecord);
		}
	}

err:
	LOG("ImportSMS::Leaving");
	if (smsBuf) MemHandleUnlock(memHandle);
	if (memHandle) MemHandleFree(memHandle);
	if (internalSmsDb) DmCloseDatabase(internalSmsDb);
	LOG("ImportSMS::Left");
}
