#ifndef _SMTW_HELPER
#define _SMTW_HELPER

#include "SMSTW.h"
#include <Helper.h>
#include <HelperServiceClass.h>

#define FEATURE_COMPOSE_NEW 1
#define FEATURE_FINGER_ADDR 2

static Boolean PrvAppEnumerate(HelperNotifyEventType *helperNotifyEventP) 
{ 
   HelperNotifyEnumerateListType* newNodeP; 
    
   newNodeP = (HelperNotifyEnumerateListType*) MemPtrNew (sizeof(HelperNotifyEnumerateListType)); 
  
   // Get name to display in user interface.  
   StrCopy(newNodeP->helperAppName, "SMSTW"); 
  
   // Get name of service to display in UI.  
   StrCopy(newNodeP->actionName, "SMS"); 
  
   newNodeP->serviceClassID = kHelperServiceClassIDSMS; 
   newNodeP->helperAppID = appFileCreator; 
   newNodeP->nextP = 0; 
  
   // Add the new node. 
   if (helperNotifyEventP->data.enumerateP == 0) { 
      helperNotifyEventP->data.enumerateP = newNodeP; 
   } else { 
      HelperNotifyEnumerateListType* nodeP; 
      nodeP = helperNotifyEventP->data.enumerateP; 
      newNodeP->nextP = nodeP;
      helperNotifyEventP->data.enumerateP = newNodeP;

      //Look for the end of the list.  
/*      while ( nodeP->nextP != 0 ) 
         nodeP = nodeP->nextP; 
      nodeP->nextP = newNodeP; */
   } 
  
   return true; 
} 

static Boolean PrvAppValidate (SysNotifyParamType *sysNotifyParamP) 
{ 
   HelperNotifyEventType* helperNotifyEvent; 
    
   helperNotifyEvent = (HelperNotifyEventType*) sysNotifyParamP->notifyDetailsP; 
       
   // Check service 
   if (helperNotifyEvent-> data.validateP->serviceClassID  
         != kHelperServiceClassIDSMS) 
      return false; 
       
   // check appId (either null or me) 
   if ((helperNotifyEvent->data.validateP->helperAppID != 0)  
      && (helperNotifyEvent->data.validateP->helperAppID !=  
         appFileCreator)) 
      return false; 
    
   sysNotifyParamP->handled = true; 
   return true; 
} 

static void PrvAppExecute(HelperNotifyExecuteType* pExec)
{
	if (pExec->serviceClassID != kHelperServiceClassIDSMS) return;
	const char* phoneNumber = pExec->dataP;
	if (phoneNumber == NULL) return;
	Int16 phoneLen = StrLen(phoneNumber);
	if (phoneLen == 0) return; 
	
	const char* displayName = pExec->displayedName;
	Int16 displayLen = 0;
	
	if (displayName != NULL) displayLen = StrLen(displayName);
	
	Int16 size = phoneLen + 1 + displayLen + 1;
	
	char* ftrBuf = NULL;
	Err err = FtrPtrNew(appFileCreator, (UInt16) FEATURE_COMPOSE_NEW, size, (void**) &ftrBuf);
	if (err) return;
	DmSet(ftrBuf, 0, size, 0);
	
	DmWrite(ftrBuf, 0, phoneNumber, phoneLen);
	if (displayLen) {
		DmWrite(ftrBuf, phoneLen + 1, displayName, displayLen);
	}
	
	UInt16 cardNo;
	LocalID dbID;
	DmSearchStateType searchInfo;
	DmGetNextDatabaseByTypeCreator (true, &searchInfo, sysFileTApplication, appFileCreator, true, &cardNo, &dbID);

	SysUIAppSwitch(cardNo, dbID, sysAppLaunchCmdNormalLaunch, NULL);
}

#endif