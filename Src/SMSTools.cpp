#include <PalmOS.h>
#include <Helper.h>
#include <HelperServiceClass.h>


#include "SMSTools.h"
#include "conv.h"
#include "SMSTW_Rsc.h"
#include "SMSTW.h"
#include "Global.h"
#include "MobileServices.h"

void* GetObjectPtr(FormPtr frm, UInt16 objectID)
{
	return FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, objectID));
}

void SetFieldText(FieldPtr field, const char* str)
{
	MemHandle textHandle = FldGetTextHandle(field);
	FldSetTextHandle(field, NULL);
	if (textHandle == NULL) {
		textHandle = MemHandleNew(StrLen(str) + 1);
	} else {
		MemHandleResize(textHandle, StrLen(str) + 1);
	}
	char* pStr = (char*) MemHandleLock(textHandle);
	StrCopy(pStr, str);
	MemHandleUnlock(textHandle);
	FldSetTextHandle(field, textHandle);
}

void ScrollField(FormPtr form, Int16 linesToScroll, UInt16 field, UInt16 scrollbar)
{
	UInt16				blankLines;
	FieldPtr			fld;

	fld = (FieldPtr) GetObjectPtr(form, field);
	blankLines = FldGetNumberOfBlankLines (fld);

	if (linesToScroll < 0)
		FldScrollField (fld, -linesToScroll, winUp);
	else if (linesToScroll > 0)
		FldScrollField (fld, linesToScroll, winDown);
		
	UpdateScrollbar(form, field, scrollbar);
}

void UpdateScrollbar(FormPtr form, UInt16 field, UInt16 scrollbar)
{
	UInt16 scrollPos;
	UInt16 textHeight;
	UInt16 fieldHeight;
	Int16 maxValue;
	FieldPtr fld;
	ScrollBarPtr bar;

	fld = (FieldPtr) GetObjectPtr(form, field);
	bar = (ScrollBarPtr) GetObjectPtr(form, scrollbar);

	FldGetScrollValues (fld, &scrollPos, &textHeight,  &fieldHeight);

	if (textHeight > fieldHeight)
	{
		// On occasion, such as after deleting a multi-line selection of text,
		// the display might be the last few lines of a field followed by some
		// blank lines.  To keep the current position in place and allow the user
		// to "gracefully" scroll out of the blank area, the number of blank lines
		// visible needs to be added to max value.  Otherwise the scroll position
		// may be greater than maxValue, get pinned to maxvalue in SclSetScrollBar
		// resulting in the scroll bar and the display being out of sync.
		maxValue = (textHeight - fieldHeight) + FldGetNumberOfBlankLines (fld);
	}
	else if (scrollPos)
		maxValue = scrollPos;
	else
		maxValue = 0;

	SclSetScrollBar (bar, scrollPos, 0, maxValue, fieldHeight-1);
}

void ScrollFieldByEvent(FormPtr form, EventType* eventP, UInt16 field, UInt16 scrollbar)
{
	if (eventP->eType != sclRepeatEvent) return;
	Int16 linesToScroll = eventP->data.sclRepeat.newValue - eventP->data.sclRepeat.value;
	ScrollField(form, linesToScroll, field, scrollbar);
}

Boolean ScrollFieldPage(FormPtr form, UInt16 field, UInt16 scrollbar, WinDirectionType dir)
{
	FieldPtr fld = (FieldPtr) GetObjectPtr(form, field);
	if (FldScrollable (fld, dir)) {
		Int16 linesToScroll = FldGetVisibleLines (fld) - 1;

		if (dir == winUp)
			linesToScroll = -linesToScroll;
		
		ScrollField(form, linesToScroll, field, scrollbar);
		return true;
	}
	
	return false;
}

/*
 * FUNCTION: RomVersionCompatible
 *
 * DESCRIPTION: 
 *
 * This routine checks that a ROM version is meet your minimum 
 * requirement.
 *
 * PARAMETERS:
 *
 * requiredVersion
 *     minimum rom version required
 *     (see sysFtrNumROMVersion in SystemMgr.h for format)
 *
 * launchFlags
 *     flags that indicate if the application UI is initialized
 *     These flags are one of the parameters to your app's PilotMain
 *
 * RETURNED:
 *     error code or zero if ROM version is compatible
 */

Err RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags)
{
	UInt32 romVersion;

	/* See if we're on in minimum required version of the ROM or later. */
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
	if (romVersion < requiredVersion)
	{
		if ((launchFlags & 
			(sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
			(sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
		{
			FrmAlert (RomIncompatibleAlert);

			/* Palm OS versions before 2.0 will continuously relaunch this
			 * app unless we switch to another safe one. */
			if (romVersion < kPalmOS20Version)
			{
				AppLaunchWithCommand(
					sysFileCDefaultApp, 
					sysAppLaunchCmdNormalLaunch, NULL);
			}
		}

		return sysErrRomIncompatible;
	}

	return errNone;
}

Err CallPhone(const char* phoneNumber, const char* userName)
{
   SysNotifyParamType param; 
   HelperNotifyEventType details; 
   HelperNotifyExecuteType execute; 
  
   param.notifyType = sysNotifyHelperEvent; 
   param.broadcaster = sysFileCAddress; 
   param.notifyDetailsP = &details; 
   param.handled = false; 
  
   details.version = kHelperNotifyCurrentVersion; 
   details.actionCode = kHelperNotifyActionCodeExecute; 
   details.data.executeP = &execute; 
  
   execute.serviceClassID = kHelperServiceClassIDVoiceDial; 
   execute.helperAppID = 0; 
   execute.dataP = (char *) phoneNumber; 
   execute.displayedName = (char *) userName; 
   execute.detailsP = 0; 
   execute.err = errNone; 
  
   SysNotifyBroadcast(&param); 
  
   // Check error code 
   if (!param.handled) 
   // Not handled so exit the list - Unexpected error 
      return true; 
   else 
      return (execute.err == errNone); 
}

void Log(const char* msg)
{
	DmCreateDatabase(0, "SMSTWLog", 'SMTW', 'LOG', false);
	LocalID dbId = DmFindDatabase(0, "SMSTWLog");
	if (dbId == 0) return;
	DmOpenRef db = DmOpenDatabase(0, dbId, dmModeReadWrite);
	if (db == NULL) return;

	DateTimeType now;
	TimSecondsToDateTime(TimGetSeconds(), &now);
	UInt32 size = sizeof(DateTimeType) + StrLen(msg);
	
	UInt16 pos = dmMaxRecordIndex;
	MemHandle memHandle = DmNewRecord(db, &pos, size);
	if (memHandle == NULL) goto err;
	
	void* pRecord = MemHandleLock(memHandle);
	if (pRecord == NULL) goto err;
	
	Err error = DmWrite(pRecord, 0, &(now), sizeof(DateTimeType));
	if (error) goto err;

	error = DmWrite(pRecord, sizeof(DateTimeType), msg, StrLen(msg));
	
err:
	if (pRecord) MemHandleUnlock(memHandle);
	if (memHandle) DmReleaseRecord(db, pos, true);
	if (db) {
		if (pos > 800) DmRemoveRecord(db, 0);
		DmCloseDatabase(db);
	}
}

Boolean IsGPRSConnected()
{
	UInt32 notify;
	if (FtrGet(appFileCreator, TUNGSTEN_W_FEATURE_GPRS_STATUS, &notify)) {
		return false;
	}
	if (notify == kTelOemGPRSLaunchCmdSessionStarted) {
		return true;
	}
	return false;
}

void AssignStr(char** ppStr, const char* pSrcStr, UInt32 len)
{
	if (pSrcStr == NULL) return;
	
	if (len == 0) {
		len = StrLen(pSrcStr);
	}
	
	if (*ppStr) MemPtrFree(*ppStr);
	
	*ppStr = (char *) MemPtrNew(len + 1);
	MemSet(*ppStr, len + 1, 0);
	StrNCopy(*ppStr, pSrcStr, len);
}

