#include <PalmNavigator.h>
#include "SMSReply.h"
#include "SMSTW.h"
#include "SMSTW_Rsc.h"
#include "SMSTools.h"
#include "SMSSend.h"
#include "SMSDb.h"
#include "SMSTable.h"
#include "Global.h"
#include "PrefReminder.h"
#include "SMSHelper.h"
#include "FinAddrUtils.h"
#include "GroupDb.h"

static PhoneGroupVector g_PhoneGroups;

static const char* GetDisplayString(UInt16 index)
{
	if (index == 0) return "<NONE>";
	
	if (index <= g_PhoneGroups.size()) {
		PhoneGroupPtr group = g_PhoneGroups[index - 1];
		return group->GetGroupName();
	}
	
	return "";
}

static void ListDrawDataFunc(Int16 itemNum, RectangleType *bounds, Char **itemsText)
{
	char string[20];
	
	StrNCopy(string, GetDisplayString(itemNum), 19);

	WinSetUnderlineMode(noUnderline);
	FntSetFont(stdFont);
	Int16 width = bounds->extent.x;
	Int16 len = StrLen(string);
	Boolean noFit = false;
	FntCharsInWidth(string, &width, &len, &noFit);

	WinEraseRectangle(bounds, 0);
	WinDrawChars(string, len, bounds->topLeft.x, bounds->topLeft.y);
}

static void DrawList()
{
	FormPtr form = FrmGetActiveForm();
	if (FormIsNot(form, FormReply)) return;
	ListPtr list = (ListPtr) GetObjectPtr(form, ListGroups);
	LstSetListChoices(list, NULL, g_PhoneGroups.size() + 1);
	
	UInt16 maxVisible = 8;
	if (g_PhoneGroups.size() < maxVisible - 1) 
		maxVisible = g_PhoneGroups.size() + 1;
	
	LstSetHeight(list, maxVisible);
	LstSetDrawFunction(list, ListDrawDataFunc);
	LstDrawList(list);
}

static void ListAllGroups()
{
	LoadPhoneGroups(g_PhoneGroups);
	DrawList();	
}

static Err ReplyCurrentRecord(DmOpenRef db)
{
	FormPtr form = FrmGetActiveForm();
	if (FormIsNot(form, FormReply)) return frmErrNotTheForm;

	MemHandle memHandle = 0;
	SMSDbRecordType smsRecord;
	
	Err err = GetSMSRecord(db, g_SelectedCategory, GetCurrentIndex(), 
		smsRecord, memHandle, true);
	RETURN_ERR_ON_ERR(err);
	CheckNullPointer(memHandle);
	
	SetFieldText((FieldPtr) GetObjectPtr(form, FieldTo), smsRecord.phoneNumber);
	SetFieldText((FieldPtr) GetObjectPtr(form, FieldReference), smsRecord.content);
	FrmSetFocus(form, FrmGetObjectIndex(form, FieldCompose));

	MemHandleUnlock(memHandle);	
	
	return errNone;
}

static Err ForwardCurrentRecord(DmOpenRef db)
{
	FormPtr form = FrmGetActiveForm();
	if (FormIsNot(form, FormReply)) return frmErrNotTheForm;

	MemHandle memHandle = 0;
	SMSDbRecordType smsRecord;
	
	Err err = GetSMSRecord(db, g_SelectedCategory, GetCurrentIndex(), 
		smsRecord, memHandle, true);
	RETURN_ERR_ON_ERR(err);
	CheckNullPointer(memHandle);
	
	FieldPtr fieldTo = (FieldPtr) GetObjectPtr(form, FieldTo);
	SetFieldText(fieldTo, smsRecord.phoneNumber);
	SetFieldText((FieldPtr) GetObjectPtr(form, FieldCompose), smsRecord.content);

	FldSetSelection(fieldTo, 0, StrLen(FldGetTextPtr(fieldTo)));
	FrmSetFocus(form, FrmGetObjectIndex(form, FieldTo));

	MemHandleUnlock(memHandle);	
	
	return errNone;
}

static void SetupReplyForm(FormPtr frm)
{
	FrmSetTitle(frm , "Reply");
	UpdateScrollbar(frm, FieldReference, ScrollbarReference);
}

static void SetupNewForm(FormPtr frm)
{
	FrmSetTitle(frm , "New");
	FrmHideObject(frm, FrmGetObjectIndex(frm, LabelReference));
	FrmHideObject(frm, FrmGetObjectIndex(frm, FieldReference));
	FrmHideObject(frm, FrmGetObjectIndex(frm, ScrollbarReference));
	
	RectangleType rect;
	rect.topLeft.x = 3;
	rect.topLeft.y = 32;
	FrmSetObjectBounds(frm, FrmGetObjectIndex(frm, LabelCompose), &rect);
	
	rect.topLeft.y = 44;
	rect.extent.x = 155;
	rect.extent.y = 95;
	FrmSetObjectBounds(frm, FrmGetObjectIndex(frm, FieldCompose), &rect); 

	FrmSetFocus(frm, FrmGetObjectIndex(frm, FieldTo));
}

static Err SaveMessageIn(DmOpenRef db, Int16 category)
{
	FormPtr form = FrmGetActiveForm();
	if (FormIsNot(form, FormReply)) return frmErrNotTheForm;
	
	FieldPtr fieldTo = (FieldPtr) GetObjectPtr(form, FieldTo);
	FieldPtr fieldCompose = (FieldPtr) GetObjectPtr(form, FieldCompose);
	
	char* pszTo = FldGetTextPtr(fieldTo);
	char* pszCompose = FldGetTextPtr(fieldCompose);
	
	if ((pszTo == NULL) || (StrLen(pszTo) == 0)) {
		ShowMsg("No phone number set.");
		return -1;
	}
	
	if ((pszCompose == NULL) || (StrLen(pszCompose) == 0)) {
		FrmCustomAlert(AlertCustom, "Alert", "No message composed.", "");
		return -1;
	}

	UInt8 state = 0;
	
	if (category == CAT_OUTBOX) {
		SendPref spref;
		ReadSendPreference(spref);
		if (spref.requestReport) RequestReport(state);
	}
	
	NewRecordInCategory(db, pszTo, pszCompose, category, state);
	return errNone;
}

static void DoQueue()
{
	if (!SaveMessageIn(g_SmsDb, CAT_OUTBOX))
		FrmGotoForm(FormMain);
}

static void DoBack()
{
	FrmGotoForm(FormMain);
}

static void DoDraft()
{
	if (!SaveMessageIn(g_SmsDb, CAT_DRAFT))
		FrmGotoForm(FormMain);
}

static Boolean DoSend()
{
	return SendReply(g_SmsDb, g_gbTable);
}

static void DoSendAndBack()
{
	if (DoSend())
		FrmGotoForm(FormMain);
}

static void SetupFeatureNew()
{
	FormPtr form = FrmGetActiveForm();
	if (FormIsNot(form, FormReply)) return;

	char* data;
	Err err = FtrGet(appFileCreator, FEATURE_COMPOSE_NEW, (UInt32 *) &data);
	if (err) return;
	char* pszTo = data;
	SetFieldText((FieldPtr) GetObjectPtr(form, FieldTo), pszTo);
	FtrPtrFree(appFileCreator, FEATURE_COMPOSE_NEW);

	FrmSetFocus(form, FrmGetObjectIndex(form, FieldCompose));
}

static void SelectUsingFingerAddr()
{
	FormPtr form = FrmGetActiveForm();
	if (FormIsNot(form, FormReply)) return;
	
	UInt16 mode = g_ComposeMode;
	
	char* pszTo = FldGetTextPtr((FieldPtr) GetObjectPtr(form, FieldTo));
	char* pszCompose = FldGetTextPtr((FieldPtr) GetObjectPtr(form, FieldCompose));
	char* pszReference = FldGetTextPtr((FieldPtr) GetObjectPtr(form, FieldReference));
	
	UInt16 lenTo = 0;
	if (pszTo) lenTo = StrLen(pszTo);
	
	UInt16 lenCompose = 0;
	if (pszCompose) lenCompose = StrLen(pszCompose);
	
	UInt16 lenReference = 0;
	if (pszReference) lenReference = StrLen(pszReference);
	
	UInt16 size = sizeof(UInt16) + lenTo + 1 + lenCompose + 1 + lenReference + 1;

	char* ftrBuf = NULL;
	Err err = FtrPtrNew(appFileCreator, (UInt16) FEATURE_FINGER_ADDR, size, (void**) &ftrBuf);
	if (err) return;
	DmSet(ftrBuf, 0, size, 0);
	
	UInt16 offset = 0;
	
	DmWrite(ftrBuf, offset, &mode, sizeof(UInt16));
	offset += sizeof(UInt16);
	
	if (lenTo) {
		DmWrite(ftrBuf, offset, pszTo, lenTo);
	}
	offset += lenTo + 1;
	
	if (lenCompose) {
		DmWrite(ftrBuf, offset, pszCompose, lenCompose);
	}
	offset += lenCompose + 1;
	
	if (lenReference) {
		DmWrite(ftrBuf, offset, pszReference, lenReference);
	}
	
	FasRequestSearch(appFileCreator);
}

static void RestoreReplyForm()
{
	FormPtr form = FrmGetActiveForm();
	if (FormIsNot(form, FormReply)) return;

	FasSearchInfoType FasSearchInfo;
	if (FasGetSearchInfo(&FasSearchInfo) == fasNoSearchInfo) return;

	char* data;
	Err err = FtrGet(appFileCreator, FEATURE_FINGER_ADDR, (UInt32 *) &data);
	if (err) return;

	UInt16 mode = *(UInt16*)data;
	data += sizeof(UInt16);
	
	char* pszTo = data;
	data += StrLen(pszTo) + 1;
	
	char* pszCompose = data;
	data += StrLen(pszCompose) + 1;
	
	char* pszReference = data;
	
	FieldPtr fieldTo = (FieldPtr) GetObjectPtr(form, FieldTo);
	FieldPtr fieldCompose = (FieldPtr) GetObjectPtr(form, FieldCompose);
	FieldPtr fieldReference = (FieldPtr) GetObjectPtr(form, FieldReference);

	if (FasSearchInfo.pPhoneNumber) {
		SetFieldText(fieldTo, FasSearchInfo.pPhoneNumber);
	} else {
		SetFieldText(fieldTo, pszTo);
	}

	SetFieldText(fieldCompose, pszCompose);
	SetFieldText(fieldReference, pszReference);
	
	if (mode == COMPOSE_REPLY) {
		SetupReplyForm(form);
	} else {
		SetupNewForm(form);
	}
	FrmSetFocus(form, FrmGetObjectIndex(form, FieldCompose));
	g_ComposeMode = (ComposeMode) mode;
		
	FtrPtrFree(appFileCreator, FEATURE_FINGER_ADDR);
}

static void SelectTo()
{
	FormPtr frmP = FrmGetActiveForm();
	if (FormIsNot(frmP, FormReply)) return;
	
	SendPref pref;
	ReadSendPreference(pref);
	
	if (pref.useFingerAddress) {
		if (FasSearchAvailable()) {
			SelectUsingFingerAddr();
			return;
		}
	}

	FieldPtr fieldTo = (FieldPtr) GetObjectPtr(frmP, FieldTo);
	FldSetSelection(fieldTo, 0, StrLen(FldGetTextPtr(fieldTo)));

	AddrLookupParamsType params;
	MemSet(&params, sizeof(AddrLookupParamsType), 0);
	params.formatStringP = "^mobile";
	params.field1 = addrLookupSortField;
	params.field2 = addrLookupMobile;
	params.field2Optional = false;
	params.userShouldInteract = true;

	PhoneNumberLookupCustom (fieldTo, &params, true);
	FrmSetFocus(frmP, FrmGetObjectIndex(frmP, FieldCompose));
}

static void SetTriggerText(UInt16 selected)
{
	FormPtr frmP = FrmGetActiveForm();
	if (FormIsNot(frmP, FormReply)) return;
	char* str = (char *)CtlGetLabel ((ControlPtr) GetObjectPtr (frmP, TriggerGroup));	
	StrNCopy(str, GetDisplayString(selected), 19);
	CtlDrawControl((ControlPtr) GetObjectPtr (frmP, TriggerGroup));
}

static Boolean IsEventInControl(FormPtr frmP, EventPtr eventP, UInt16 control)
{
	RectangleType rect;
	FrmGetObjectBounds(frmP, FrmGetObjectIndex(frmP, control), &rect);
	if ((eventP->screenX >= rect.topLeft.x) &&
		(eventP->screenX <= rect.topLeft.x + rect.extent.x) &&
		(eventP->screenY >= rect.topLeft.y) &&
		(eventP->screenY <= rect.topLeft.y + rect.extent.y))
		return true;
	return false;
}

static void SelectNextInList(Int16 step)
{
	FormPtr frmP = FrmGetActiveForm();
	if (FormIsNot(frmP, FormReply)) return;
	ListPtr list = (ListPtr) GetObjectPtr(frmP, ListGroups);
	Int16 sel = LstGetSelection(list);
	if (sel == noListSelection) return;
	
	sel += step;
	
	if (sel < 0) sel = 0;
	if (sel >= LstGetNumberOfItems(list)) sel = LstGetNumberOfItems(list) - 1;
	
	LstSetSelection(list, sel);
}

Boolean ReplyFormHandleEvent(EventType * eventP)
{
	Boolean handled = false;
	FormType * frmP;
	static Boolean listShown = false;
	
	frmP = FrmGetActiveForm();

	switch (eventP->eType) 
	{
		case frmOpenEvent:
			switch (g_ComposeMode) {
				case COMPOSE_REPLY:
					ReplyCurrentRecord(g_SmsDb);
					SetupReplyForm(frmP);
					break;
				case COMPOSE_NEW:
					SetupNewForm(frmP);
					break;
				case COMPOSE_FORWARD:
					ForwardCurrentRecord(g_SmsDb);
					SetupNewForm(frmP);
					break;
				case COMPOSE_FEATURE:
					SetupFeatureNew();
					SetupNewForm(frmP);
					break;
				case COMPOSE_RESTORE:
					RestoreReplyForm();
					break;
			}
			FrmDrawForm(frmP);
			ListAllGroups();
			SetTriggerText(0);
			handled = true;
			break;
		case frmCloseEvent:
			ClearPhoneGroups(g_PhoneGroups);
			break;
		case menuEvent:
			switch(eventP->data.menu.itemID) {
				case MenuItemQueue:
					DoQueue();
					break;
				case MenuItemBack:
					DoBack();
					break;
				case MenuItemDraft:
					DoDraft();
					break;
				case MenuItemSend:
					DoSend();
					break;
				case MenuItemSendAndBack:
					DoSendAndBack();
					break;
				case MenuItemSelectTo:
					SelectTo();
					break;
				case MenuItemSelectGroup:
					CtlHitControl((ControlPtr) GetObjectPtr(frmP, TriggerGroup));
					break;
			}
			break;
		case ctlSelectEvent:
			switch(eventP->data.ctlSelect.controlID) {
				case ButtonSendAndBack:
					DoSendAndBack();
					break;
				case ButtonBack:
					DoBack();
					break;
				case ButtonGo:
					DoSend();
					break;
				case ButtonSelectTo:
					SelectTo();
					break;
				case TriggerGroup:
					if (listShown) {
						FrmHideObject(frmP, FrmGetObjectIndex(frmP, ListGroups));
						FrmDrawForm(frmP);
						listShown = false;
					} else {
						FrmShowObject(frmP, FrmGetObjectIndex(frmP, ListGroups));
						listShown = true;
					}
					handled = true;
					break;
			}
			break;
		case penDownEvent:
			if (IsEventInControl(frmP, eventP, TriggerGroup)) break;
			if (IsEventInControl(frmP, eventP, ListGroups)) break;
			FrmHideObject(frmP, FrmGetObjectIndex(frmP, ListGroups));
			FrmDrawForm(frmP);
			listShown = false;
			break;
		case lstSelectEvent:
			if (eventP->data.lstSelect.listID == ListGroups) {
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, ListGroups));
				FrmDrawForm(frmP);
				listShown = false;
				
				ListPtr list = (ListPtr) GetObjectPtr(frmP, ListGroups);
				Int16 sel = LstGetSelection(list);
				if (sel == noListSelection) {
					ShowMsg("Select a group.");
					break;
				}
				SetTriggerText(sel);
			}
			break;
		case keyDownEvent:
			WChar chr = eventP->data.keyDown.chr;
			UInt16 		keyCode		= eventP->data.keyDown.keyCode;
			switch(chr)
			{
				case vchrPageUp:
					if (listShown)
						SelectNextInList(-1);
					else
						ScrollFieldPage(frmP, FieldReference, ScrollbarReference, winUp);
					break;
				case vchrPageDown:
					if (listShown)
						SelectNextInList(1);
					else
						ScrollFieldPage(frmP, FieldReference, ScrollbarReference, winDown);
					break;
				case vchrNavChange:
					if(keyCode & navBitSelect) {
						FrmHideObject(frmP, FrmGetObjectIndex(frmP, ListGroups));
						FrmDrawForm(frmP);
						listShown = false;
						
						ListPtr list = (ListPtr) GetObjectPtr(frmP, ListGroups);
						Int16 sel = LstGetSelection(list);
						if (sel == noListSelection) {
							ShowMsg("Select a group.");
							break;
						}
						SetTriggerText(sel);
					}
					break;
			}
			break;
		case sclRepeatEvent:
			ScrollFieldByEvent(frmP, eventP, FieldReference, ScrollbarReference);
			break;
	}
	return handled;
}

//const char* testStr = "一二三四五六七八九十一二三四五六七八九十一二三四五六七八九十一二三四五六七八九十一二三四五六七八九十一二三四五六七八九十一二三四五六七八九十一二三四五六七八九十一二三四五六七八九十一二三四五六七八九十一二三四五六七八九十";

static void SendTheSMS(DmOpenRef db, gb2312_table& table, const char* pszTo, const char* pszCompose)
{
	Err err = SendSMS(table, "+8613800100500", pszTo, pszCompose);
//	err = SendSMS(table, "+8613800100500", pszTo, testStr);
	if (!err) {
		UInt8 state = 0;
		SendPref spref;
		ReadSendPreference(spref);
		if (spref.requestReport) RequestReport(state);
		NewRecordInCategory(db, pszTo, pszCompose, CAT_SENT, state);
		SortSMS(db);
	} else {
		FrmCustomAlert(AlertCustom, "Alert", "Message send failed.", "");
	}
}

Err SendReply(DmOpenRef db, gb2312_table& table)
{
	MethodLogger log("SendReply");
	FormPtr form = FrmGetActiveForm();
	if (FormIsNot(form, FormReply)) return frmErrNotTheForm;
	
	FieldPtr fieldTo = (FieldPtr) GetObjectPtr(form, FieldTo);
	FieldPtr fieldCompose = (FieldPtr) GetObjectPtr(form, FieldCompose);
	ListPtr list = (ListPtr) GetObjectPtr(form, ListGroups);
	Int16 sel = LstGetSelection(list);
	
	char* pszTo = FldGetTextPtr(fieldTo);
	char* pszCompose = FldGetTextPtr(fieldCompose);
	
	Err err = -1;

	Boolean emptyTo = false;	
	if ((pszTo == NULL) || (StrLen(pszTo) == 0)) {
		emptyTo = true;
	}
	
	Boolean emptyGroup = false;
	if ((sel == noListSelection) || (sel == 0)) {
		emptyGroup = true;
	}
	
	if (emptyTo && emptyGroup) {
		ShowMsg("Please set To or Group.");
		goto exit;
	}
	
	if ((pszCompose == NULL) || (StrLen(pszCompose) == 0)) {
		FrmCustomAlert(AlertCustom, "Alert", "No message composed.", "");
		goto exit;
	}
	
	if (emptyGroup)
		SendTheSMS(db, table, pszTo, pszCompose);
	else {
		PhoneGroupPtr group = LoadPhoneGroupByUniqId(g_PhoneGroups[sel - 1]->GetUniqId());
		UInt32 count = group->GetPhoneGroupItemCount();
		for (UInt32 i = 0; i < count; ++i) {
			PhoneGroupItemPtr item = group->GetPhoneGroupItem(i);
			SendTheSMS(db, table, item->GetPhone(), pszCompose);
		}
		delete group;
	}
	
exit:
	return err;
}
