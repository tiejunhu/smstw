#include "GroupDb.h"
#include "SMSTools.h"
#include "SMSTW.h"

// ---- Globals

static const char* SMSTW_PHONE_GROUP_DB_NAME = "smstwphonegroup";
TDataMgr g_PhoneGroupDb(SMSTW_PHONE_GROUP_DB_NAME, appFileCreator);

// ---- PhoneGroupItem

PhoneGroupItem::PhoneGroupItem()
{
	m_itemType = GI_PHONE;
	m_displayName = NULL;
	m_phone = NULL;
	m_category = 0;	
}

PhoneGroupItem::~PhoneGroupItem()
{
	if (m_displayName) MemPtrFree(m_displayName);
	if (m_phone) MemPtrFree(m_phone);
}

void PhoneGroupItem::SetDisplayName(const char* displayName)
{
	AssignStr(&m_displayName, displayName, 0);
}

const char* PhoneGroupItem::GetDisplayName()
{
	return m_displayName;
}

void PhoneGroupItem::SetPhone(const char* phone)
{
	AssignStr(&m_phone, phone, 0);
}

const char* PhoneGroupItem::GetPhone()
{
	return m_phone;
}

UInt8 PhoneGroupItem::GetItemType()
{
	return m_itemType;
}


// ---- PhoneGroup

PhoneGroup::PhoneGroup(UInt32 uniqId)
{
	m_groupName = NULL;
	m_UniqId = uniqId;
}

PhoneGroup::~PhoneGroup()
{
	if (m_groupName) MemPtrFree(m_groupName);
	PhoneGroupItemVector::iterator i = m_items.begin();
	for (; i != m_items.end(); ++i) {
		delete (*i);
	}
	m_items.clear();
}

const char* PhoneGroup::GetGroupName()
{
	return m_groupName;
}

void PhoneGroup::SetGroupName(const char* groupName)
{
	AssignStr(&m_groupName, groupName, 0);
}

// ---- PhoneGroup end

static UInt32 CalcPhoneGroupSize(PhoneGroupPtr group)
{
	UInt32 size = StrLen(group->GetGroupName()) + 1;
	UInt32 count = group->GetPhoneGroupItemCount();
	for (UInt32 i = 0; i < count; ++i) {
		PhoneGroupItemPtr item = group->GetPhoneGroupItem(i);
		size += sizeof(UInt8); // itemType
		size += StrLen(item->GetDisplayName()) + 1;
		size += StrLen(item->GetPhone()) + 1;
	}
	return size;
}

static void WritePhoneGroup(MemHandle memHandle, UInt32 size, PhoneGroupPtr group)
{
	void* pData = MemHandleLock(memHandle);
	DmSet(pData, 0, size, 0); // set all as zero
	
	UInt32 offset = 0;
	UInt32 bytes = 0;
	
	bytes = StrLen(group->GetGroupName());
	DmWrite(pData, offset, group->GetGroupName(), bytes);
	offset += bytes + 1;

	UInt32 count = group->GetPhoneGroupItemCount();
	for (UInt32 i = 0; i < count; ++i) {
		PhoneGroupItemPtr item = group->GetPhoneGroupItem(i);
		UInt8 itemType = item->GetItemType();
		DmWrite(pData, offset, &itemType, sizeof(UInt8));
		offset += sizeof(UInt8);

		if (itemType == GI_PHONE) {		
			bytes = StrLen(item->GetDisplayName());
			DmWrite(pData, offset, item->GetDisplayName(), bytes);
			offset += bytes + 1;
			
			bytes = StrLen(item->GetPhone());
			DmWrite(pData, offset, item->GetPhone(), bytes);
			offset += bytes + 1;
		}
	}	
	
	MemHandleUnlock(memHandle);
}

// modify a existing phone group
Err ModifyPhoneGroup(PhoneGroupPtr group)
{
	ErrFatalDisplayIf(group == NULL, "NULL group to add");
	
	DmOpenRef db = g_PhoneGroupDb.GetOpenRef();
	UInt32 size = CalcPhoneGroupSize(group);
	UInt16 pos;
	
	Err err = DmFindRecordByID(db, group->GetUniqId(), &pos);
	if (err) return err;
	
	MemHandle memHandle = DmResizeRecord(db, pos, size);
	ErrFatalDisplayIf(memHandle == NULL, "cannot resize record");
	
	memHandle = DmGetRecord(db, pos);
	
	WritePhoneGroup(memHandle, size, group);
	
	DmReleaseRecord(db, pos, true);
	
	return errNone;
}


// create a new phone group
Err AddPhoneGroup(PhoneGroupPtr group)
{
	ErrFatalDisplayIf(group == NULL, "NULL group to add");
	
	DmOpenRef db = g_PhoneGroupDb.GetOpenRef();
	UInt32 size = CalcPhoneGroupSize(group);
	UInt16 pos = dmMaxRecordIndex;
	
	MemHandle memHandle = DmNewRecord(db, &pos, size);
	ErrFatalDisplayIf(memHandle == NULL, "cannot create new record");
	
	WritePhoneGroup(memHandle, size, group);

	DmReleaseRecord(db, pos, true);
	
	return errNone;
}

static void ClearPhoneGroupVector(PhoneGroupVector& groups)
{
	PhoneGroupVector::iterator i = groups.begin();
	for (; i != groups.end(); ++i) {
		delete (*i);
	}
	groups.clear();
}

Err LoadPhoneGroups(PhoneGroupVector& groups)
{
	ClearPhoneGroupVector(groups); // force clear
	DmOpenRef db = g_PhoneGroupDb.GetOpenRef();
	UInt16 num = DmNumRecords(db);
	for (UInt16 i = 0; i < num; ++i) {
		MemHandle memHandle = DmQueryRecord(db, i);
		UInt32 uniqId = 0;
		DmRecordInfo(db, i, NULL, &uniqId, NULL);
		
		char* pszData = (char *) MemHandleLock(memHandle);
		PhoneGroupPtr pGroup = new PhoneGroup(uniqId);
		pGroup->SetGroupName(pszData);
		groups.push_back(pGroup);
		MemHandleUnlock(memHandle);
	}
	return errNone;
}

Err RemovePhoneGroup(PhoneGroupPtr group)
{
	ErrFatalDisplayIf(group == NULL, "NULL group to remove");
	DmOpenRef db = g_PhoneGroupDb.GetOpenRef();
	
	UInt16 index;
	Err err = DmFindRecordByID(db, group->GetUniqId(), &index);
	if (err) return err;
	
	err = DmRemoveRecord(db, index);
	if (err) return err;
	
	return errNone;
}

PhoneGroupPtr LoadPhoneGroupByUniqId(UInt32 uniqId)
{
	DmOpenRef db = g_PhoneGroupDb.GetOpenRef();

	UInt16 index;
	Err err = DmFindRecordByID(db, uniqId, &index);
	if (err) return NULL;

	MemHandle memHandle = DmQueryRecord(db, index);
	PhoneGroupPtr pGroup = new PhoneGroup(uniqId);
	
	UInt32 size = MemHandleSize(memHandle);
	char* pszData = (char *) MemHandleLock(memHandle);
	char* pszEnd = pszData + size;

	char* pszGroupName = pszData;	
	pGroup->SetGroupName(pszGroupName);
	pszData += StrLen(pszGroupName) + 1;
	
	while (pszData < pszEnd) {
		PhoneGroupItemPtr item = new PhoneGroupItem();
		
		UInt8 itemType = *(UInt8 *)pszData;
		item->SetItemType(itemType);
		pszData += sizeof(UInt8);
		
		if (itemType == GI_PHONE) {
			char* pszDisplayName = pszData;
			item->SetDisplayName(pszDisplayName);
			pszData += StrLen(pszDisplayName) + 1;
			
			char* pszPhone = pszData;
			item->SetPhone(pszPhone);
			pszData += StrLen(pszPhone) + 1;
		}
		
		pGroup->AddPhoneGroupItem(item);
	}
	
	MemHandleUnlock(memHandle);
	
	return pGroup;
}

void ClearPhoneGroups(PhoneGroupVector& groups)
{
	PhoneGroupVector::iterator i = groups.begin();
	for (; i != groups.end(); ++i) {
		delete (*i);
	}
	groups.clear();
}
