/*
 * SMSTW.c
 *
 * main file for SMSTW
 *
 * This wizard-generated code is based on code adapted from the
 * stationery files distributed as part of the Palm OS SDK 4.0.
 *
 * Copyright (c) 1999-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 */
 
#include <PalmOS.h>
#include <PalmOSGlue.h>
#include <SMSLib.h>
#include <Helper.h>
#include <PalmNavigator.h>

#include "SMSTW_Rsc.h"
#include "AddrDb.h"

#include "ImportSMS.h"

#include "SMSDb.h"
#include "SMSTW.h"
#include "SMSTable.h"
#include "SMSTools.h"
#include "SMSReply.h"
#include "SMSReceive.h"
#include "SMSAttention.h"
#include "SMSNew.h"
#include "SMSSend.h"
#include "SMSHelper.h"
#include "PrefReminder.h"

#include "conv.h"
#include "Global.h"

#include "FinAddrUtils.h"

#include "TelephonyMgrOem.h"
#include "TelephonyMgrTypesOem.h"

// Forms

#include "FormMain.h"
#include "FormProgress.h"
#include "FormDetail.h"
#include "FormGroupManagement.h"
#include "FormNewGroup.h"

/* Define the minimum OS version we support */
#define ourMinVersion    sysMakeROMVersion(3,0,0,sysROMStageDevelopment,0)


/*
 * FUNCTION: AppHandleEvent
 *
 * DESCRIPTION: 
 *
 * This routine loads form resources and set the event handler for
 * the form loaded.
 *
 * PARAMETERS:
 *
 * event
 *     a pointer to an EventType structure
 *
 * RETURNED:
 *     true if the event was handled and should not be passed
 *     to a higher level handler.
 */

static Boolean AppHandleEvent(EventType * eventP)
{
	UInt16 formId;
	FormType * frmP;

	if (eventP->eType == frmLoadEvent)
	{
		/* Load the form resource. */
		formId = eventP->data.frmLoad.formID;
		frmP = FrmInitForm(formId);
		FrmSetActiveForm(frmP);

		/* 
		 * Set the event handler for the form.  The handler of the
		 * currently active form is called by FrmHandleEvent each
		 * time is receives an event. 
		 */
		switch (formId)
		{
			case FormMain:
				FrmSetEventHandler(frmP, FormMainHandleEvent);
				break;
			case FormDetail:
				FrmSetEventHandler(frmP, FormDetailHandleEvent);
				break;
			case FormReply:
				FrmSetEventHandler(frmP, ReplyFormHandleEvent);
				break;			
			case FormProgress:
				FrmSetEventHandler(frmP, FormProgressHandleEvent);
				break;			
			case FormPreference:
				FrmSetEventHandler(frmP, ReminderPrefFormHandleEvent);
				break;
			case FormGroupManagement:
				FrmSetEventHandler(frmP, FormGroupManagementHandleEvent);
				break;
			case FormNewGroup:
				FrmSetEventHandler(frmP, FormNewGroupHandleEvent);
				break;
		}
		return true;
	}

	return false;
}

/*
 * FUNCTION: AppEventLoop
 *
 * DESCRIPTION: This routine is the event loop for the application.
 */

static void AppEventLoop(void)
{
	EventType event;
	Err error;
	
	do
	{
		EvtGetEvent(&event, evtWaitForever);
		
		// If the event is a five way event, pass it to the form
		// event handler.
		if(IsFiveWayNavEvent(&event) || !SysHandleEvent(&event))
		{
			if (! MenuHandleEvent(0, &event, &error))
			{
				if(!AppHandleEvent(&event))
				{
					FrmDispatchEvent(&event);
				}
			}
		}
	} while(event.eType != appStopEvent);
}

/*
 * FUNCTION: AppStart
 *
 * DESCRIPTION:  Get the current application's preferences.
 *
 * RETURNED:
 *     errNone - if nothing went wrong
 */

static Err AppStart(void)
{
	/* addr lookup test */

/*	char tmp[50];
	lookupNameFromAddressBook("13611202096", tmp);*/
	
	g_GbTableDb = init_gb2312_table(g_gbTable);
	g_SmsDb = OpenSMSDb();
	return errNone;
}

/*
 * FUNCTION: AppStop
 *
 * DESCRIPTION: Save the current state of the application.
 */

static void AppStop(void)
{
	CloseSMSDb(g_SmsDb);
	fini_gb2312_table(g_GbTableDb, g_gbTable);
	/* Close all the open forms. */
	FrmCloseAllForms();

}


/**
 * Register as default SMS receiver of system
 */
static void RegisterToSystem()
{
	Err error;
	error = ExgRegisterDatatype(appFileCreator, exgRegExtensionID,
		kSmsMessageRegExtensionType"\t"kSmsReportRegExtensionType,
		"SMS", 0);

	if (!error)
	{
		error = ExgSetDefaultApplication (appFileCreator, exgRegExtensionID,
				kSmsMessageRegExtensionType);
		if (!error)
			error = ExgSetDefaultApplication (appFileCreator, exgRegExtensionID,
					kSmsReportRegExtensionType);
	}
}

static void RegisterHelperNotification()
{
	UInt16 cardNo;
	LocalID dbID;
	DmSearchStateType searchInfo;

	DmGetNextDatabaseByTypeCreator (true, &searchInfo, sysFileTApplication, appFileCreator, true, &cardNo, &dbID);
	SysNotifyRegister(cardNo, dbID, sysNotifyHelperEvent, 0, sysNotifyNormalPriority, 0);
}

static void RegisterTelNotification()
{
	UInt16 cardNo;
	LocalID dbID;
	DmSearchStateType searchInfo;

	DmGetNextDatabaseByTypeCreator (true, &searchInfo, sysFileTApplication, appFileCreator, true, &cardNo, &dbID);
	SysNotifyRegister(cardNo, dbID, kTelTelephonyNotification, 0, sysNotifyNormalPriority, 0);
}

static Err ReceiveSystemSMS(ExgSocketPtr socket, UInt16 launchFlags)
{
	DmOpenRef db = NULL;
	DmOpenRef gbDb = NULL;
	gb2312_table* pTable = NULL;
	gb2312_table gbTable;
	bool byUs = false;

	if (!(launchFlags & sysAppLaunchFlagSubCall)) {
		LOG("ReceiveSystemSMS::Starting while not active");
		db = OpenSMSDb();
		gbDb = init_gb2312_table(gbTable);
		pTable = &gbTable;
		byUs = true;
	} else {
		LOG("ReceiveSystemSMS::Starting while active");
		db = g_SmsDb;
		pTable = &g_gbTable;
	}
	
	CheckNullPointer(db);
	CheckNullPointer(pTable);

	MemHandle numberHandle = NULL;
	MemHandle textHandle = NULL;
	UInt32 len = 0;
	Int8 coding = 4;
	Err err = ReceiveSMS(db, socket, numberHandle, textHandle, &len, &coding);

	if (err == exgErrComplete) {
		if (byUs) {
			CloseSMSDb(db);
			fini_gb2312_table(gbDb, gbTable);
		} else {
			RefreshPage(db);
		}
		LOG("ReceiveSystemSMS::Left early");
		return errNone;
	}
	if (err) return err;	

	SMSDbRecordType* pRecord = AllocSMSRecord();
	CheckNullPointer(pRecord);
	
	char* pszPhone = (char *) MemHandleLock(numberHandle);
	char* pszText = (char *) MemHandleLock(textHandle);
	char pszName[100];
	lookupNameFromAddressBook(pszPhone, pszName);

	InitSMSRecord(pRecord);

#define BUF_SIZE 500

	MemHandle bufHandle = MemHandleNew(BUF_SIZE);	
	CheckNullPointer(bufHandle);
	
	char* buf = (char *) MemHandleLock(bufHandle);
	CheckNullPointer(buf);
	
	MemSet(buf, BUF_SIZE, 0);
	len = buffer_to_gb2312(*pTable, (unsigned char *) pszText, (unsigned char *) buf, len, coding);

	pRecord->phoneNumber = pszPhone;
	pRecord->content = buf;
	pRecord->size = len + 1;
	pRecord->personName = pszName;
	pRecord->time = TimGetSeconds();
	UnReadRecord(pRecord);
	DecodeRecord(pRecord);
	
	UInt16 pos = AddSMSRecord(db, pRecord, CAT_INBOX);
	
	MemHandleUnlock(bufHandle);
	MemHandleFree(bufHandle);

	pRecord->phoneNumber = NULL;
	pRecord->content = NULL;
	pRecord->personName = NULL;
	FreeSMSRecord(pRecord);

	MemHandleUnlock(numberHandle);
	MemHandleUnlock(textHandle);
	MemHandleFree(numberHandle);
	MemHandleFree(textHandle);
	
	UInt32 uniqId;
	DmRecordInfo(db, pos, NULL, &uniqId, NULL);

	SortSMS(db);
	
	if (byUs) {
		CloseSMSDb(db);
		fini_gb2312_table(gbDb, gbTable);
	} else {
		RefreshPage(db);
	}
	
	ReminderPref pref;
	ReadReminderPreference(pref);
	if (pref.useReminder) {
		GetAttention(uniqId, pref.vibration, pref.playSound); // notice ourself
	}
	
	LOG("ReceiveSystemSMS::Left");
	return errNone;
}

static Boolean CheckComposeFeature()
{
	char* data;
	Err err = FtrGet(appFileCreator, FEATURE_COMPOSE_NEW, (UInt32 *) &data);
	if (err) return false;
	g_ComposeMode = COMPOSE_FEATURE;
	return true;
}

static Boolean CheckFingerAddr()
{
	char* pData;
	
	Err err = FtrGet(fasFinAddrCreator, fasFtrFinAddrSearchInfo, (UInt32 *)&pData);
	if (err) return false;
	g_ComposeMode = COMPOSE_RESTORE;
	return true;
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	Err error;

	error = RomVersionCompatible (ourMinVersion, launchFlags);
	if (error) return (error);

	switch (cmd)
	{
		case sysAppLaunchCmdNormalLaunch:
			error = AppStart();
			if (error) 
				return error;
			if (CheckComposeFeature()) {
				FrmGotoForm(FormReply);
			} else if (CheckFingerAddr()) {
				FrmGotoForm(FormReply);
			} else {
				FrmGotoForm(FormMain);
			}
			AppEventLoop();
			AppStop();
			break;
		case appLaunchCmdAlarmEventGoto:
			error = AppStart();
			if (error) 
				return error;
			g_DisplayUniqId = *((UInt32*)cmdPBP);
			FrmGotoForm(FormDetail);
			AppEventLoop();
			AppStop();
			break;
		case sysAppLaunchCmdExgReceiveData:
			ReceiveSystemSMS((ExgSocketPtr)cmdPBP, launchFlags);
			break;
		case sysAppLaunchCmdAttention:
		{
			DmOpenRef db = NULL;
			AttnLaunchCodeArgsType* pAttn = (AttnLaunchCodeArgsType*) cmdPBP;
			if (!(launchFlags & sysAppLaunchFlagSubCall)) {
				if (pAttn->command == kAttnCommandGoThere) {
					UInt16 cardNo;
					LocalID dbID;
					DmSearchStateType searchInfo;
					DmGetNextDatabaseByTypeCreator (true, &searchInfo, sysFileTApplication, appFileCreator, true, &cardNo, &dbID);
					AttnForgetIt(cardNo, dbID, pAttn->userData);
					UInt32* gotoInfoP = (UInt32*)MemPtrNew (sizeof(UInt32));
					ErrFatalDisplayIf ((!gotoInfoP), "Out of memory");
					MemPtrSetOwner(gotoInfoP, 0);

					// initialize the goto params structure so that datebook will open day view 
					// with the specified item selected
					*gotoInfoP = pAttn->userData;
					SysUIAppSwitch(cardNo, dbID, appLaunchCmdAlarmEventGoto, gotoInfoP);
				} else {
					DmOpenRef db = OpenSMSDb();
					ProcessAttention(pAttn, db);
					CloseSMSDb(db);
				}
			} else {
				if (pAttn->command == kAttnCommandGoThere) {
					UInt16 cardNo;
					LocalID dbID;
					DmSearchStateType searchInfo;
					DmGetNextDatabaseByTypeCreator (true, &searchInfo, sysFileTApplication, appFileCreator, true, &cardNo, &dbID);
					AttnForgetIt(cardNo, dbID, pAttn->userData);
					g_DisplayUniqId = pAttn->userData;
					FrmGotoForm(FormDetail);
				} else {
					db = g_SmsDb;
					ProcessAttention((AttnLaunchCodeArgsType*) cmdPBP, db);
				}
			}
			break;
		}
		case sysAppLaunchCmdSystemReset:
		case sysAppLaunchCmdSyncNotify:
			RegisterToSystem();
			RegisterHelperNotification();
			RegisterTelNotification();
			FtrSet(appFileCreator, TUNGSTEN_W_FEATURE_GPRS_STATUS, kTelOemGPRSLaunchCmdSessionClosed);
			break;
		case sysAppLaunchCmdNotify:

			SysNotifyParamType* notifyP = (SysNotifyParamType *)( cmdPBP );
			switch (notifyP->notifyType)
			{
				case sysNotifyHelperEvent:
					HelperNotifyEventType* pHelper = (HelperNotifyEventType *)notifyP->notifyDetailsP;
					switch (pHelper->actionCode) {
						case kHelperNotifyActionCodeEnumerate:
							PrvAppEnumerate(pHelper);
							break;
						case kHelperNotifyActionCodeValidate:
							PrvAppValidate(notifyP);
							break;
						case kHelperNotifyActionCodeExecute:
							HelperNotifyExecuteType* pExec = pHelper->data.executeP;
							PrvAppExecute(pExec);
							break;
					}
					break;
				case kTelTelephonyNotification:
					TelNotificationType* notifyTypeP = (TelNotificationType *)( notifyP->notifyDetailsP );
					switch ( notifyTypeP->notificationId  )
					{
						case kTelOemGPRSLaunchCmdSessionStarted:
							FtrSet(appFileCreator, TUNGSTEN_W_FEATURE_GPRS_STATUS, (UInt32)notifyTypeP->notificationId);
							break;
						case kTelOemGPRSLaunchCmdSessionClosed:
						    FtrSet(appFileCreator, TUNGSTEN_W_FEATURE_GPRS_STATUS, (UInt32)notifyTypeP->notificationId);
							break;
						default:
							break;
					}
					break;
			}
			break;
	}

	return errNone;
}
