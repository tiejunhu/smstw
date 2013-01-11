#ifndef _SMSTW_ATTENTION
#define _SMSTW_ATTENTION

#include "SMSTW.h"
#include "SMSDb.h"
#include "PrefReminder.h"

static void GetAttention(UInt32 data, Boolean vibration, Boolean playSound)
{
	LOG("GetAttention::Starting");
	UInt16 cardNo;
	LocalID dbID;
	DmSearchStateType searchInfo;

	AttnFlagsType flag = kAttnFlagsUseUserSettings;
	
	if (vibration)
		flag |= kAttnFlagsVibrateBit;
	else
		flag |= kAttnFlagsNoVibrate;
		
	if (playSound) 
		flag |= kAttnFlagsCustomEffectBit;
	else
		flag |= kAttnFlagsNoCustomEffect;
		
	
	DmGetNextDatabaseByTypeCreator (true, &searchInfo, sysFileTApplication, appFileCreator, true, &cardNo, &dbID);
	AttnGetAttention(cardNo, dbID, data, NULL, kAttnLevelInsistent, 
		flag, 60, 4);
	LOG("GetAttention::Left");
}

static void DrawSMS (UInt32 data, AttnCommandArgsType *paramsPtr, DmOpenRef smsDb, Boolean drawDetail)
{
	Int16 x, y;
	x = paramsPtr->drawDetail.bounds.topLeft.x;
	y = paramsPtr->drawDetail.bounds.topLeft.y;
	
	UInt16 index;
	Err err = DmFindRecordByID(smsDb, data, &index);

	if (err) {
		const char* str = "No unread SMS.";
		WinDrawChars(str, StrLen(str), x + 5, y + 9);
		return;
	}

	SMSDbRecordType record;
	MemHandle memHandle;
	err = GetSMSRecord(smsDb, -1, index, record, memHandle, true);
	if (err) {
		const char* str = "No unread SMS.";
		WinDrawChars(str, StrLen(str), x + 5, y + 9);
		return;
	}

	if (drawDetail) {
		MemHandle resH = DmGetResource(iconType, IconSMS);
		BitmapPtr bitmap = (BitmapPtr) MemHandleLock(resH);
		WinDrawBitmap(bitmap, x + 100, y + 5);
		MemHandleUnlock(resH);
		DmReleaseResource(resH);
		
		FntSetFont(largeBoldFont);
		WinDrawChars("New SMS", 7, x + 5, y + 9);

		FntSetFont(boldFont);
		WinDrawChars("From:", 5, x + 6, y + 30);
		WinDrawChars("Name:", 5, x + 6, y + 42);
		

		FntSetFont(stdFont);
		WinDrawChars(record.phoneNumber, StrLen(record.phoneNumber), x + 42, y + 30);
		if (StrLen(record.personName) == 0) {
			WinDrawChars("<NOT FOUND>", 11, x + 42, y + 42);
		} else {
			WinDrawChars(record.personName, StrLen(record.personName), x + 42, y + 42);
		}

		char* text = record.content;
		char* end = text + StrLen(text);	
		for (int i = 0; i < 4; ++i) {
			Int16 pixelWidth = 134;
			Int16 charLength = StrLen(text);
			Boolean fit = false;
			FntCharsInWidth(text, &pixelWidth, &charLength, &fit);
			WinDrawChars(text, charLength, x + 6, y + 56 + i * 12);
			text += charLength;
			if (text >= end) break;
		}
	} else {
		MemHandle resH = DmGetResource(iconType, IconSMSSmall);
		BitmapPtr bitmap = (BitmapPtr) MemHandleLock(resH);
		WinDrawBitmap(bitmap, x, y);
		MemHandleUnlock(resH);
		FntSetFont(stdFont);
		WinDrawChars(record.phoneNumber, StrLen(record.phoneNumber), x + 14, y);
		
		Int16 width = FntCharsWidth(record.phoneNumber, StrLen(record.phoneNumber));
		WinDrawChars(record.personName, StrLen(record.personName), x + 18 + width, y);
		
		char* text = record.content;
		Int16 charLength = StrLen(text);
		WinDrawTruncChars(text, charLength, x, y + 12, 132);
		
	}

	MemHandleUnlock(memHandle);
}

static void ProcessAttention(AttnLaunchCodeArgsType* pAttn, DmOpenRef smsDb)
{
	AttnCommandArgsType* argsP = pAttn->commandArgsP;

	switch (pAttn->command) {
		case kAttnCommandDrawDetail:
			DrawSMS(pAttn->userData, pAttn->commandArgsP, smsDb, true);
			break;			

		case kAttnCommandDrawList:
			DrawSMS(pAttn->userData, pAttn->commandArgsP, smsDb, false);
			break;
		
		case kAttnCommandCustomEffect:
			ReminderPref pref;
			ReadReminderPreference(pref);
			if (pref.useReminder) {
				if (pref.playSound) {
					PlayAlarmSound(pref.soundId);
				}
			}
			break;
		
	}
}

#endif