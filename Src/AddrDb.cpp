#include "AddrDB.h"
#include "SMSTools.h"

#define BitAtPosition(pos)                ((UInt32)1 << (pos))
#define GetBitMacro(bitfield, index)      ((bitfield) & BitAtPosition(index))

typedef struct {
	AddrOptionsType		options;        // Display by company or by name
	AddrDBRecordFlags	flags;
	UInt8				companyFieldOffset;   // Offset from firstField
	char				firstField;
} PrvAddrPackedDBRecord;

static void	PrvAddrDBUnpack(PrvAddrPackedDBRecord *src, AddrDBRecordPtr dest);

/************************************************************
 *
 *  FUNCTION: AddrDBGetRecord
 *
 *  DESCRIPTION: Get a record from the Address Database
 *
 *  PARAMETERS: database pointer - open db pointer
 *            database index - index of record to lock
 *            address record pointer - pointer address structure
 *            address record - MemHandle to unlock when done
 *
 *  RETURNS: ##0 if successful, errorcode if not
 *    The record's MemHandle is locked so that the pointer to
 *  strings within the record remain pointing to valid chunk
 *  versus the record randomly moving.  Unlock the MemHandle when
 *  AddrDBRecord is destroyed.
 *
 *  CREATED: 1/14/95
 *
 *  BY: Roger Flores
 *
 *************************************************************/
Err AddrDBGetRecord(DmOpenRef dbP, UInt16 index, AddrDBRecordPtr recordP,
				  MemHandle *recordH)
{
	PrvAddrPackedDBRecord *src;

	*recordH = DmQueryRecord(dbP, index);
	UInt32 size = MemHandleSize(*recordH);
	if (size < sizeof(PrvAddrPackedDBRecord)) return dmErrIndexOutOfRange;
	src = (PrvAddrPackedDBRecord *) MemHandleLock(*recordH);
	if (src == NULL)
		return dmErrIndexOutOfRange;

	PrvAddrDBUnpack(src, recordP);

	return 0;
}


/************************************************************
 *
 *  FUNCTION: PrvAddrDBUnpack
 *
 *  DESCRIPTION: Fills in the AddrDBRecord structure
 *
 *  PARAMETERS: address record to unpack
 *                the address record to unpack into
 *
 *  RETURNS: the record unpacked
 *
 *  CREATED: 1/14/95
 *
 *  BY: Roger Flores
 *
 *************************************************************/
void PrvAddrDBUnpack(PrvAddrPackedDBRecord *src, AddrDBRecordPtr dest)
{
	Int16   index;
	UInt32 flags;
	char *p;


	dest->options = src->options;
	flags = src->flags.allBits;
	p = &src->firstField;


	for (index = firstAddressField; index < addressFieldsCount; index++)
	{
		// If the flag is set point to the string else NULL
		if (GetBitMacro(flags, index) != 0)
		{
			dest->fields[index] = p;
			p += StrLen(p) + 1;
		}
		else
			dest->fields[index] = NULL;
	}
}

void lookupNameFromAddressBook(const char* phoneNumber, char* userName)
{
	LOG("lookupNameFromAddressBook::Starting");
	UInt16 totalCount = 0;
	UInt16 i = 0;
	DmOpenRef addrDb = NULL;
	MemHandle hEntryMem = NULL;
	Err err = 0;
	AddrDBRecordType addrRecord;
	int k = firstAddressField;
	
	
	if (userName == NULL) {
		LOG("lookupNameFromAddressBook::userName is NULL");
		goto exit;
	}

	StrCopy(userName, "");

	if (phoneNumber == NULL) {
		LOG("lookupNameFromAddressBook::phoneNumber is NULL");
		goto exit;
	}
	

	addrDb = DmOpenDatabaseByTypeCreator(AddressDbType, AddressDbCreatorId, dmModeReadOnly);
	if (addrDb == NULL) {
		LOG("lookupNameFromAddressBook::Cannot open address db");
		goto exit;
	}
	
	const char* nakedNumber = GetNakedPhoneNumber(phoneNumber);
	if (nakedNumber == NULL) {
		LOG("lookupNameFromAddressBook::Cannot get naked number");
		goto exit;
	}
	
	totalCount = DmNumRecordsInCategory(addrDb, dmAllCategories);
//	totalCount = DmNumRecords(addrDb);
	LOG("lookupNameFromAddressBook::Searching for name");
	for (i = 0; i < totalCount; ++i) {
	
		hEntryMem = NULL;

		err = AddrDBGetRecord(addrDb, i, &addrRecord, &hEntryMem);
		if (err) continue;
		if (hEntryMem == NULL) continue;
		
		for (k = firstAddressField + 1; k < addressFieldsCount; ++k) {
			if (addrRecord.fields[k] == NULL) continue;
			
			if (StrStr(addrRecord.fields[k], nakedNumber)) {
				Boolean firstPart = false;
				if (addrRecord.fields[name] != NULL) {
					if (StrLen(addrRecord.fields[name]) > 0) {
						StrCat(userName, addrRecord.fields[name]);
						firstPart = true;							
					}
				}
				if (addrRecord.fields[firstName] != NULL) {
					if (StrLen(addrRecord.fields[firstName]) > 0) {
						if (firstPart) {
							StrCat(userName,",");
						}
						StrCat(userName, addrRecord.fields[firstName]);
					}
				}
				MemHandleUnlock(hEntryMem);
				LOG("lookupNameFromAddressBook::Got name");
				goto exit;
			}
		}
		MemHandleUnlock(hEntryMem);
	}
exit:
		
	if (addrDb) DmCloseDatabase(addrDb);
	LOG("lookupNameFromAddressBook::Left");
	return;
}

// phone number length: (13800100500) 11 numbers

#define MOBILE_NUMBER_LENGTH 11

const char* GetNakedPhoneNumber(const char* phoneNumber)
{
	if (phoneNumber == NULL) return NULL;
	UInt32 len = StrLen(phoneNumber);
	if (len < MOBILE_NUMBER_LENGTH) return NULL;
	return phoneNumber + len - MOBILE_NUMBER_LENGTH;
}
