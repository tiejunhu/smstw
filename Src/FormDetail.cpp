#include "FormDetail.h"
#include "SMSTW_Rsc.h"
#include "Global.h"
#include "SMSTable.h"
#include "SMSDb.h"
#include <PalmNavigator.h>

static void DoBack()
{
	FrmGotoForm(FormMain);
}

static void DoReply()
{
	g_ComposeMode = COMPOSE_REPLY;
	FrmGotoForm(FormReply);
}

static void DoForward()
{
	g_ComposeMode = COMPOSE_FORWARD;
	FrmGotoForm(FormReply);
}

Boolean FormDetailHandleEvent(EventType* eventP)
{
	Boolean handled = false;
	FormType * frmP;

	switch (eventP->eType) 
	{
		case frmOpenEvent:
			Err err;
			if (g_DisplayUniqId == (UInt32)(-1))
				err = DisplayCurrentRecord(g_SmsDb);
			else {
				UInt16 index = 0;
				err = DmFindRecordByID(g_SmsDb, g_DisplayUniqId, &index);
				if (err == errNone)
					err = DisplayRecord(g_SmsDb, -1, index);
			}
			if (err) DoBack();	
			frmP = FrmGetActiveForm();
			UpdateScrollbar(frmP, FieldContent, ScrollbarContent);
			FrmDrawForm(frmP);
			handled = true;
			break;
		case ctlSelectEvent:
			switch(eventP->data.ctlSelect.controlID) {
				case ButtonOk:
					DoBack();
					break;
				case ButtonReply:
					DoReply();
					break;
				case ButtonForward:
					DoForward();
					break;
			}
			break;
		case menuEvent:
			switch(eventP->data.menu.itemID) {
				case MenuItemDelete:
					if (g_SelectedCategory == CAT_TRASH)
						DeleteCurrentRecord(g_SmsDb);
					else
						SetCurrentRecordCategory(g_SmsDb, CAT_TRASH);
				case MenuItemBack:
					DoBack();
					break;
				case MenuItemReply:
					DoReply();
					break;
				case MenuItemForward:
					DoForward();
					break;
				case MenuItemNext:
					DisplayNextRecord(g_SmsDb, 1);
					break;
				case MenuItemNextUnread:
					DisplayNextUnreadRecord(g_SmsDb);
					break;
				case MenuItemPrevious:
					DisplayNextRecord(g_SmsDb, -1);
					break;
				case MenuItemCallback:
				{
					frmP = FrmGetActiveForm();
					const char* phone = FrmGetLabel(frmP, LabelPhone);
					const char* userName = FrmGetLabel(frmP, LabelName);
					CallPhone(phone, userName);
					break;
				}
			}
			break;
		case keyDownEvent:
			WChar 		chr			= eventP->data.keyDown.chr;
			UInt16 		keyCode		= eventP->data.keyDown.keyCode;
			frmP = FrmGetActiveForm();
			switch(chr)
			{
				case vchrPageUp:
					if (ScrollFieldPage(frmP, FieldContent, ScrollbarContent, winUp)) break;
					DisplayNextRecord(g_SmsDb, -1);
					break;
				case vchrPageDown:
					if (ScrollFieldPage(frmP, FieldContent, ScrollbarContent, winDown)) break;
					DisplayNextUnreadRecord(g_SmsDb);
					break;
				case vchrNavChange:
					if(keyCode & navBitRight) {
						DoForward();
					} else if(keyCode & navBitLeft) {
						DoBack();
					} else if(keyCode & navBitSelect) {
						DoReply();
					}
					break;
			}
			break;
		case sclRepeatEvent:
			frmP = FrmGetActiveForm();
			ScrollFieldByEvent(frmP, eventP, FieldContent, ScrollbarContent);
			break;
	}
	return handled;
}

Err DisplayRecord(DmOpenRef db, Int16 category, UInt16 index)
{
	FormPtr form = FrmGetActiveForm();
	if (FormIsNot(form, FormDetail)) return frmErrNotTheForm;

	MemHandle memHandle = 0;
	SMSDbRecordType smsRecord;
	
	Err err = GetSMSRecord(db, category, index, smsRecord, memHandle, false);
	if (err) return err;

	FrmCopyLabel(form, LabelPhone, smsRecord.phoneNumber);
	FrmCopyLabel(form, LabelName, smsRecord.personName);

	char string[50];
	DateTimeType smsDatetime;
	TimSecondsToDateTime(smsRecord.time, &smsDatetime);
	DateTemplateToAscii("^4r-^3z-^0z ", smsDatetime.month, smsDatetime.day, smsDatetime.year, string, 49);
	TimeToAscii(smsDatetime.hour, smsDatetime.minute, tfColon24h, string + StrLen(string));
	StrCat(string, " ");
	StrCat(string, DayOfWeekInChinese[DayOfWeek(smsDatetime.month, smsDatetime.day, smsDatetime.year)]);
	FrmCopyLabel(form, LabelTime, string);
	
	FieldPtr fieldContent = (FieldPtr) GetObjectPtr(form, FieldContent);
	SetFieldText(fieldContent, smsRecord.content);

	Boolean dirty = false;
	if (!IsRecordRead(&smsRecord)) {
		ReadRecord(&smsRecord);
		DmSet(smsRecord.data, (UInt32) &(((SMSDbRecordType *)NULL)->state), 1, smsRecord.state);
		dirty = true;
	}
	
	ReleaseSMSRecord(db, &smsRecord, dirty);
	MemHandleUnlock(memHandle);
	
	return errNone;
}