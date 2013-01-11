/*****************************************************************************
* 
* Copyright (c) 1999-2002 Palm, Inc. or its subsidiaries. All rights reserved.
*
* File: 			NetPicker.h
*
* Release: 
*
* Description:		Public header for the NetPicker library containing the 
*					public API and definitions.
*
* History:
*	11/01/2002		Mark Kruger	Initial revision
*
*****************************************************************************/

#ifndef _NETPICKER_H
#define _NETPICKER_H

//----------------------------------------------------------------------------
// Defines for includes.
//----------------------------------------------------------------------------
#define ALLOW_ACCESS_TO_INTERNALS_OF_PROGRESS


//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------
#include <PalmTypes.h>



//----------------------------------------------------------------------------
// Defines
//----------------------------------------------------------------------------
/* sysMakeROMVersion(major, minor, fix, stage, buildNum) */
#define	kNetPickerVersionMajor				1
#define	kNetPickerVersionMinor				0
#define	kNetPickerVersionFix				0
#define	kNetPickerVersionStage				sysROMStageDevelopment
#define	kNetPickerVersionBuild 				1

/* NetPicker shared lib version */ 
#define	kNetPickerVersion 					sysMakeROMVersion(kNetPickerVersionMajor, kNetPickerVersionMinor, kNetPickerVersionFix, kNetPickerVersionStage, kNetPickerVersionBuild) 
  
#define kNetPickerLibName					"NetPicker.lib"

#define kNetPickerLibId						'npkr'
#define kNetPickerLibType					'libr'

// Error Codes
#define netPickerErrBase					appErrorClass
#define netPickerVersionError				(netPickerErrBase | 1)
#define netPickerMemoryError				(netPickerErrBase | 2)


// Library call ID's
#define trapNetPickerIsProgressPatched			(sysLibTrapCustom)
#define trapNetPickerInstallProgressPatch		(sysLibTrapCustom+1)
#define trapNetPickerUninstallProgressPatch		(sysLibTrapCustom+2)
#define trapNetPickerIsActivated				(sysLibTrapCustom+3)
#define trapNetPickerDoDialog					(sysLibTrapCustom+4)


#ifdef BUILDING_NET_PICKER
	#define NET_PICKER_TRAP(trapNum)
#else
	#define NET_PICKER_TRAP(trapNum) SYS_TRAP(trapNum)
#endif


//----------------------------------------------------------------------------
// API
//----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

Err 	NetPickerOpen(UInt16 refnum, UInt32 versionNum)
		NET_PICKER_TRAP(sysLibTrapOpen);

Err 	NetPickerClose(UInt16 refnum)
		NET_PICKER_TRAP(sysLibTrapClose);

Err 	NetPickerSleep(UInt16 /*refnum*/)
		NET_PICKER_TRAP(sysLibTrapSleep);

Err 	NetPickerWake(UInt16 /*refnum*/)
		NET_PICKER_TRAP(sysLibTrapWake);

Boolean	NetPickerIsProgressPatched(UInt16 refNum)
		NET_PICKER_TRAP(trapNetPickerIsProgressPatched);

void 	NetPickerInstallProgressPatch(UInt16 refNum)
		NET_PICKER_TRAP(trapNetPickerInstallProgressPatch);

void 	NetPickerUninstallProgressPatch(UInt16 refNum)
		NET_PICKER_TRAP(trapNetPickerUninstallProgressPatch);

Boolean	NetPickerIsActivated(UInt16 refNum)
		NET_PICKER_TRAP(trapNetPickerIsActivated);

Boolean	NetPickerDoDialog(UInt16 refNum, UInt16* selection)
		NET_PICKER_TRAP(trapNetPickerDoDialog);
		


#ifdef __cplusplus 
}
#endif

#endif //_NETPICKER_H