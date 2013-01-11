#ifndef _SMSTW_GROUPDB
#define _SMSTW_GROUPDB
#include <vector>
#include "DataMgr.h"

extern TDataMgr g_PhoneGroupDb;

enum GroupItemType {
	GI_PHONE, // 电话
	GI_CATEGORY // 分类
};

class PhoneGroupItem
{
private:
	UInt8 m_itemType;
	char* m_displayName;
	char* m_phone;
	UInt16 m_category;
	
public:
	PhoneGroupItem();
	~PhoneGroupItem();
	void SetDisplayName(const char* displayName);
	const char* GetDisplayName();
	void SetPhone(const char* phone);
	const char* GetPhone();
	void SetItemType(UInt8 itemType) { m_itemType = itemType; }
	UInt8 GetItemType();
};

typedef PhoneGroupItem* PhoneGroupItemPtr;
typedef std::vector<PhoneGroupItemPtr> PhoneGroupItemVector;

class PhoneGroup
{
private:
	char* m_groupName;
	UInt32 m_UniqId;
	PhoneGroupItemVector m_items;
public:
	PhoneGroup(UInt32 uniqId);
	~PhoneGroup();
	const char* GetGroupName();
	void SetGroupName(const char* groupName);
	UInt32 GetUniqId() { return m_UniqId; }
	void AddPhoneGroupItem(PhoneGroupItemPtr item) { m_items.push_back(item); }
	PhoneGroupItemPtr GetPhoneGroupItem(Int16 index) { return m_items[index]; }
	UInt32 GetPhoneGroupItemCount() { return m_items.size(); }
	void RemovePhoneGroupItem(UInt32 index) { m_items.erase(m_items.begin() + index); }
};

typedef PhoneGroup* PhoneGroupPtr;
typedef std::vector<PhoneGroupPtr> PhoneGroupVector;

Err AddPhoneGroup(PhoneGroupPtr group);
Err LoadPhoneGroups(PhoneGroupVector& groups);
Err RemovePhoneGroup(PhoneGroupPtr group);
Err ModifyPhoneGroup(PhoneGroupPtr group);
PhoneGroupPtr LoadPhoneGroupByUniqId(UInt32 uniqId);
void ClearPhoneGroups(PhoneGroupVector& groups);

#endif