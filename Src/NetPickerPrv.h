/*****************************************************************************
* 
* Copyright (c) 1999-2002 Palm, Inc. or its subsidiaries. All rights reserved.
*
* File: 			NetPickerPrv.h
*
* Release: 
*
* Description:		Header for the NetPicker library that maintains private 
*					definitions used in the library.
*
* History:
*	11/01/2002		Mark Kruger	Initial revision
*
*****************************************************************************/

#ifndef _NETPICKERPRV_H
#define _NETPICKERPRV_H

//----------------------------------------------------------------------------
// Defines for includes.
//----------------------------------------------------------------------------
#define ALLOW_ACCESS_TO_INTERNALS_OF_PROGRESS


//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------
#include <PalmTypes.h>
#include <DataMgr.h>
#include <Progress.h>


//----------------------------------------------------------------------------
// Defines
//----------------------------------------------------------------------------

// NetPicker Progress Dialog UI
#define netPickerProgressCounterGadget    31604	// resource ID of gadget used as opened counter
#define netPickerProgressBtnCancel        31603	// Cancel button ID
#define netPickerProgressBtnSelectNet     31601	// Trigger button ID
#define netPickerProgressFrm              31600	// progress form ID

#define HelpID31500String                 31500
#define OldInitString                     31501	// "Initializing"
#define NewInitTemplateString             31502	// "Initializing ^0"

#define PickerForm                        31500
#define pickerOkButton                    31501
#define pickerCancelButton                31502
#define pickerInstructionLabel            31503
#define pickerNetworksList                31505
#define pickerNetworksPopTrigger          31504

/*
// Generic Progress dialog UI
#define prgStringList						11600	// String list resource ID
#define prgProgressLabelStage				11602	// Label ID
#define prgPictPhone						11620	// resource ID of phone picture
#define prgPictHandshake					11621	// resource ID of handshake picture
#define prgPictBook							11622	// resource ID of book picture
#define prgPictError						11623	// resource ID of error picture
*/

// UI Constants
#define	prvStrIndexOK						1		// "OK"
#define	prvStrIndexCancelling				2		// "Cancelling"
#define	prvStrIndexError					3		// "Error: "
#define	prvStrIndexUnnamed					4		// "Unnamed"

#define	prvProgressIconLeft					4		// Left coordinate for icon
#define	prvProgressIconTop					12		// Top coordinate for icon 12
#define	prvProgressIconWidth				33		// Width of dial progress icons
#define	prvProgressIconHeight				41		// Height of dial progress icons	

#define	prvProgressTextLeft					40		// Left coordinate for text 40
#define	prvProgressTextTop					12		// Top coordinate for text
#define	prvProgressTextHeight				52		// height of text
#define	prvProgressTextWidth				110		// width of text 110

// Misc defines used in the NetPicker Library
#define maxSizeForServiceName    			32		//Maximum size for the service name
#define	prvPrefServiceNameID				11	
#define netMaxConfigs 						64		// value copied from NetPrv.h
#define noServiceSelected	-1						// index for no current record.


//----------------------------------------------------------------------------
// Typedefs
//----------------------------------------------------------------------------


// Structure for holding network service name and index mappings
typedef struct {
	MemHandle nameH;
	UInt16 index;
} NetServicesInfoType;

// Structure for the NetPicker Dialog Box globals data.
typedef struct {
//	DmOpenRef 	netServiceDB;
	UInt16  			 	origSelection;
	UInt16					currentSelection;
	Char					triggerLabelStr[maxSizeForServiceName];
	UInt16 					serviceListCount;
	NetServicesInfoType 	serviceList[netMaxConfigs]; 
	
} NetPickerDialogGlobalsType;


// Typedef to hold the function pointers to the Progress Manager functions
// being patched.
typedef struct PatchedProgressFunctionsStruct {
	void* trapPrgUpdateDialog;
	void* trapPrgHandleEvent;
	void* trapPrgStartDialogV31;
	void* trapPrgStartDialog;
	void* trapPrgStopDialog;	
} PatchedProgressFunctionsType;



//----------------------------------------------------------------------------
// Globals
//----------------------------------------------------------------------------
typedef struct NetPickerGlobalsStruct {
	UInt16 							openCount;
	PatchedProgressFunctionsType	savedTraps;
	Boolean							progressSelectNetTapped;
	DmOpenRef						progressResourceP;
	NetPickerDialogGlobalsType		picker;
} NetPickerGlobalsType;


//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
//#define PrvUnlockGlobals(gP)	MemPtrUnlock(gP)


#define TraceFunction(X)	//TraceOutput(X)
#define TraceDebug(X)		//TraceOutput(X)


//----------------------------------------------------------------------------
// Private functions
//----------------------------------------------------------------------------
extern UInt16		DoPickerDialog(void);
extern void			PrvInstallProgressMgrPatches(UInt16 refNum);
extern void			PrvUnInstallProgressMgrPatches(UInt16 refNum);
extern Boolean		PrvIsProgressAlreadyPatched(UInt16 refNum);
void				PrvUpdateDialog(ProgressPtr prgP);

extern void			NetPickerPrgUpdateDialog(ProgressPtr prgP, UInt16 err, UInt16 stage, const Char * messageP,Boolean updateNow);
extern Boolean		NetPickerPrgHandleEvent(ProgressPtr prgP,EventType * eventP);
extern ProgressPtr	NetPickerPrgStartDialogV31(const Char * titleP,PrgCallbackFunc textCallback);
extern ProgressPtr	NetPickerPrgStartDialog(const Char * titleP,PrgCallbackFunc textCallback, void* userDataP);
extern void 		NetPickerPrgStopDialog(ProgressPtr prgP,Boolean force);

NetPickerGlobalsType*	PrvMakeGlobals(UInt16 refNum);
void 					PrvFreeGlobals(UInt16 refNum);
NetPickerGlobalsType* 	PrvLockGlobals(UInt16 refNum);
Err						PrvUnlockGlobals(MemPtr p);
UInt16					PrvFindRefNum(void);
NetPickerGlobalsType*	PrvFindGlobals(void);

// NetPickerDialog prototypes
void		PrvInitServicePickList(FormPtr formP, NetPickerDialogGlobalsType* pickerP);
void		PrvServicePickListDrawItem (Int16 itemNum, RectanglePtr bounds, Char ** textH);
Boolean		NetPickerFormHandleEvent(EventPtr eventP);
UInt16		NetPickerFrmDoDialog (FormType * formP);
Boolean		PrvHandleModalEvent(FormType* formP, EventType* eventP);
Err			PrvGetNetServicesList(UInt16 netLibRefNum, NetServicesInfoType serviceList[], UInt16* indexCount);
Int16		ServiceListCompare(void* svc1, void* svc2, Int32 extra);
UInt16		PrvGetCurrentActiveIndex(UInt16 netLibRefnum, NetServicesInfoType services[], UInt16 indexCount);
Err			PrvSetDefaultIndex(UInt16 netLibRefNum, UInt16 index);
MemHandle	CloneStr(const Char * srcStr);
void 		PrvFreeServiceList(NetServicesInfoType* services, UInt16* servicesCount);


#endif //_NETPICKERPRV_H