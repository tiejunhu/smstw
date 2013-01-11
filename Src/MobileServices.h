/******************************************************************************
*
* Copyright (c) 1999-2003 Palm, Inc. or its subsidiaries.
* All rights reserved.
*
* File: MobileServices.h
*
* Release: 
*
* Description:
*		This is the header file for the Telephony Utilities for Palm OS 
*		Wireless Telephony Add-on.
*		It defines the new Telephony Manager public functions.
*
* History:
*		11/01/2001		Ryan Robertson		Initial version.
*
******************************************************************************/

#ifndef __MOBILE_SERVICES_H__
#define __MOBILE_SERVICES_H__

#include "DeviceSystemResources.h"
#include "MobileServicesTypes.h"

#include "TelephonyMgrTypesOem.h"
#include "TelephonyMgrOem.h"
#include "TelephonyMgr.h"

// TO DO IN A FUTURE VERSION OF MOBILE SERVICES
// kMobileSvcProfileUpdate - remove in future
// Delete all the profile change constants
// TO DO IN A FUTURE VERSION OF MOBILE SERVICES

/* sysMakeROMVersion(major, minor, fix, stage, buildNum) */
#define	kMobileSvcVersionMajor				0
#define	kMobileSvcVersionMinor				0
#define	kMobileSvcVersionFix				0
#define	kMobileSvcVersionStage				sysROMStageDevelopment
#define	kMobileSvcVersionBuild 				34

/* tMobileSvc shared lib version */ 
#define	kMobileSvcVersion 					sysMakeROMVersion(kMobileSvcVersionMajor, kMobileSvcVersionMinor, kMobileSvcVersionFix, kMobileSvcVersionStage, kMobileSvcVersionBuild) 

/* MobileSvc shared lib internal name */
#define	kMobileSvcLibName 					"MobileServices"

/* MobileSvc shared lib name and creator */
#define	kMobileSvcDatabaseCreator			sysFileCMobileServices
#define	kMobileSvcDatabaseType				sysFileTApplication

/* MobileSvc notification creator id */
#define kMobileSvcNotification				'msvn'

#define kMobileSvcDefaultEMC				"112"

/*	----------------------------------------------------------------------	*/
/* 	new notification IDs
/*	----------------------------------------------------------------------	*/
typedef enum
{
	kMobileSvcPowerProgressStart,
	kMobileSvcPowerProgressStop,
	kMobileSvcPowerOnComplete,
	kMobileSvcPowerOnFailed,
	kMobileSvcPowerOffComplete,
	kMobileSvcPowerOffFailed,
	kMobileSvcRegistrationUpdate,
	kMobileSvcStatusInfoUpdate,
	kMobileSvcLineStateChange,
	kMobileSvcDialPhoneFailed,
	kMobileSvcProfileUpdate,				// WILL BE REMOVED IN A FUTURE VERSION
	kMobileSvcMuteOff,						// Place holder
	kMobileSvcMuteOn						// Place holder
} MobileSvcNotificationTypeEnum;

/* notification priorities */
#define kMobileSvcNotificationPriority		0x00  /* higher priority */
#define kMobileSvcNotificationHighPriority	0x80 /* highest priority */

/*	----------------------------------------------------------------------	*/
/* 	Error codes: 
	new error format:   mobileSvcErr<service><description>
/*	----------------------------------------------------------------------	*/
#define mobileSvcErrBase					appErrorClass
#define mobileSvcErrDisplayBadRect			(mobileSvcErrBase + 1)
#define mobileSvcErrDisplayBadNetworkID		(mobileSvcErrBase + 2)
#define mobileSvcErrInvalidTuneData			(mobileSvcErrBase + 3)
#define mobileSvcErrInvalidProfileEntry		(mobileSvcErrBase + 4)	
#define mobileSvcErrInvalidVolumeValue		(mobileSvcErrBase + 5)
#define mobileSvcErrPowerPoweringOff		(mobileSvcErrBase + 6)
#define mobileSvcErrPowerPoweringOn			(mobileSvcErrBase + 7)
#define mobileSvcErrPowerAlreadyOff			(mobileSvcErrBase + 8)
#define mobileSvcErrPowerAlreadyOn			(mobileSvcErrBase + 9)
#define mobileSvcErrControlNotValid			(mobileSvcErrBase + 10)
#define mobileSvcErrPowerUINotReady			(mobileSvcErrBase + 11)
#define mobileSvcErrInvalidArgument			(mobileSvcErrBase + 12)
#define mobileSvcErrVolumeControlOpen		(mobileSvcErrBase + 13)

/*	----------------------------------------------------------------------	*/
/* 	Constants
/*	----------------------------------------------------------------------	*/

/* volume control  */
#define kMobileSvcVolumeControlSpeaker				0
#define kMobileSvcVolumeControlMicrophone			1

/* Profile */
#define kMobileSvcProfileFlagDefault				0x0001
#define kMobileSvcProfileFlagCar					0x0002
#define kMobileSvcProfileFlagCradle					0x0004
#define kMobileSvcProfileFlagReadOnly				0x0008
#define kMobileSvcProfileFlagHidden					0x0010
#define kMobileSvcProfileFlagProtected				0x0020
#define kMobileSvcProfileFlagActivate				0x0040
#define kMobileSvcProfileFlagData					0x0080
#define kMobileSvcProfileFlagUsed					0x8000

#define kMobileSvcProfileDivertOff					0
#define kMobileSvcProfileDivertVoicemail			1		
#define kMobileSvcProfileDivertOn					2

#define kMobileSvcProfileVolumeNone					0
#define kMobileSvcProfileVolumeIncrease				1
#define kMobileSvcProfileVolumeLow					2
#define kMobileSvcProfileVolumeMedium				3
#define kMobileSvcProfileVolumeHigh					4

#define kMobileSvcProfileVibrateOff					0
#define kMobileSvcProfileVibrateOn					1
#define kMobileSvcProfileVibrateThenRing			2

#define kMobileSvcProfileChanged					1
#define kMobileSvcProfileActive						2
#define kMobileSvcProfileDivertActive				3
#define kMobileSvcProfileDivertInactive				4
#define kMobileSvcProfileDivertFailed				5

/* Ringing Tones */
#define kMobileSvcRingingToneEMS					0	

/* Volume resource ids */
#define kMobileSvcProgress1                        	28012
#define kMobileSvcProgress2                        	28013
#define kMobileSvcProgress3                        	28014
#define kMobileSvcProgress4                        	28015
#define kMobileSvcProgress5                        	28016

/* Control Types */
#define kMobileSvcControlTypeDivert					1
#define kMobileSvcControlTypeHeadsetVolume			2

/* Status Flags */
#define kMobileSvcStatusAll							0x0000
#define kMobileSvcStatusHeadsetVolume				0x0001
#define kMobileSvcStatusSignalStrength				0x0002
#define kMobileSvcStatusBatteryLevel				0x0004
#define kMobileSvcStatusDivert						0x0008
#define kMobileSvcStatusRinger						0x0010
#define kMobileSvcStatusVibrator					0x0020
#define kMobileSvcStatusRoaming						0x0040
#define kMobileSvcStatusMessageWaiting				0x0080
#define kMobileSvcStatusTty							0x0100

/* Call State */
#define kMobileSvcCallAllFlag						0x00
#define kMobileSvcCallIncomingFlag					0x01
#define kMobileSvcCallOutgoingFlag					0x02
#define kMobileSvcCallMissedFlag					0x03

/*	----------------------------------------------------------------------	*/
/* 	new messages
/*	----------------------------------------------------------------------	*/
#define kMobileSvcMessage								0
#define kMobileSvcIsControlMessage						(kMobileSvcMessage + 1)
/* function availability */
#define kMobileSvcIsFunctionSupportedMessage			(kMobileSvcMessage + 2)
/* misc */
#define kMobileSvcIsValidCharacterMessage				(kMobileSvcMessage + 3)
#define kMobileSvcFilterPhoneNumberMessage				(kMobileSvcMessage + 4)
#define kMobileSvcVolumeGetMessage						(kMobileSvcMessage + 5)
#define kMobileSvcVolumeControlMessage					(kMobileSvcMessage + 6)
#define kMobileSvcDisplayErrorMessage					(kMobileSvcMessage + 7)
#define kMobileSvcFormatNetworkIDMessage				(kMobileSvcMessage + 8)
/* status */
#define kMobileSvcDisplayStatusMessage					(kMobileSvcMessage + 9)
#define kMobileSvcDisplaySignalMessage					(kMobileSvcMessage + 10)
#define kMobileSvcDisplayBatteryMessage					(kMobileSvcMessage + 11)
/* power */
#define kMobileSvcPowerPhoneMessage						(kMobileSvcMessage + 12)
#define kMobileSvcPowerGetStatusMessage					(kMobileSvcMessage + 13)
/* ringing Tone Services */
#define kMobileSvcRingingToneGetDefaultMessage			(kMobileSvcMessage + 14)
#define kMobileSvcRingingToneCopyMessage				(kMobileSvcMessage + 15)
#define kMobileSvcRingingToneAddMessage					(kMobileSvcMessage + 16)
#define kMobileSvcRingingToneDeleteMessage				(kMobileSvcMessage + 17)
#define kMobileSvcRingingToneConvertMessage				(kMobileSvcMessage + 18)
/* AddressBook Services */
#define kMobileSvcCallerIDLookupMessage					(kMobileSvcMessage + 19)
#define kMobileSvcAddToAddressMessage					(kMobileSvcMessage + 20)
#define kMobileSvcAddToNoteMessage						(kMobileSvcMessage + 21)
/* Emergency Call */
#define kMobileSvcEmergencyCallMessage					(kMobileSvcMessage + 22)
/* Profile Services */
#define kMobileSvcProfileActivateMessage				(kMobileSvcMessage + 23)
#define kMobileSvcProfileGetActiveMessage				(kMobileSvcMessage + 24)
#define kMobileSvcProfileGetDefaultMessage				(kMobileSvcMessage + 25)
#define kMobileSvcProfileDeleteMessage					(kMobileSvcMessage + 26)
#define kMobileSvcProfileAddMessage						(kMobileSvcMessage + 27)
#define kMobileSvcProfileGetMessage						(kMobileSvcMessage + 28)
#define kMobileSvcProfileSaveMessage					(kMobileSvcMessage + 29)
#define kMobileSvcProfileGetCountMessage				(kMobileSvcMessage + 30)
#define kMobileSvcProfileListMessage					(kMobileSvcMessage + 31)
#define kMobileSvcProfileGetIdFromIndexMessage			(kMobileSvcMessage + 32)
#define kMobileSvcProfileDefaultSettingsMessage			(kMobileSvcMessage + 33)
/* Network registration */
#define kMobileSvcNetworkRegistrationMessage			(kMobileSvcMessage + 34)
/* Call State */
#define kMobileSvcLinesGetCountMessage					(kMobileSvcMessage + 35)
#define kMobileSvcLinesGetCountByStateMessage			(kMobileSvcMessage + 36)
#define kMobileSvcLinesGetLineByStateMessage			(kMobileSvcMessage + 37)
#define kMobileSvcLinesGetLineByLineIdMessage			(kMobileSvcMessage + 38)
#define kMobileSvcLinesGetLineMessage					(kMobileSvcMessage + 39)
/* Call HIstory */
#define kMobileSvcHistoryGetCountMessage				(kMobileSvcMessage + 40)
#define kMobileSvcHistoryGetCountByStateMessage			(kMobileSvcMessage + 41)
#define kMobileSvcHistoryGetInfoByStateMessage			(kMobileSvcMessage + 42)
#define kMobileSvcHistoryGetInfoByRecordIdMessage		(kMobileSvcMessage + 43)
#define kMobileSvcHistoryGetInfoMessage					(kMobileSvcMessage + 44)
#define kMobileSvcHistoryDeleteMessage					(kMobileSvcMessage + 45)
#define kMobileSvcHistoryDeleteAllMessage				(kMobileSvcMessage + 46)
/* Call Management */
#define kMobileSvcCallIncomingMessage					(kMobileSvcMessage + 47)
#define kMobileSvcCallDialPhoneMessage					(kMobileSvcMessage + 48)
#define kMobileSvcCallSetTtyModeMessage					(kMobileSvcMessage + 49)
#define kMobileSvcCallGetTtyModeMessage					(kMobileSvcMessage + 50)
/* SIM File Update */
#define kMobileSvcSimGetDivertsMessage					(kMobileSvcMessage + 51)
#define kMobileSvcSimSetDivertsMessage					(kMobileSvcMessage + 52)
	
/*	----------------------------------------------------------------------	*/
/*  TelephonyMgrOem library call ID's
/*	----------------------------------------------------------------------	*/

#define mobileSvcLibTrapControl						(sysLibTrapCustom)
/* function availability */
#define mobileSvcLibTrapIsFunctionSupported			(sysLibTrapCustom+1)
/* misc */
#define mobileSvcLibTrapValidCharacter				(sysLibTrapCustom+2)
#define mobileSvcLibTrapFilterPhone					(sysLibTrapCustom+3)
#define mobileSvcLibTrapVolumeGet					(sysLibTrapCustom+4)
#define mobileSvcLibTrapVolumeControl				(sysLibTrapCustom+5)
#define mobileSvcLibTrapDisplayErrorMessage			(sysLibTrapCustom+6)
#define mobileSvcLibTrapFormatNetworkID				(sysLibTrapCustom+7)
/* status */
#define mobileSvcLibTrapDisplayStatus				(sysLibTrapCustom+8)
#define mobileSvcLibTrapDisplaySignal				(sysLibTrapCustom+9)
#define mobileSvcLibTrapDisplayBattery				(sysLibTrapCustom+10)
/* power */
#define mobileSvcLibTrapPowerPhone					(sysLibTrapCustom+11)
#define mobileSvcLibTrapPowerGetStatus				(sysLibTrapCustom+12)
/* Ringing Tone Services */
#define mobileSvcLibRingingToneGetDefault			(sysLibTrapCustom+13)
#define mobileSvcLibRingingToneCopy					(sysLibTrapCustom+14)
#define mobileSvcLibRingingToneAdd					(sysLibTrapCustom+15)
#define mobileSvcLibRingingToneDelete				(sysLibTrapCustom+16)
#define mobileSvcLibRingingToneConvert				(sysLibTrapCustom+17)
/* Address Book Interaction */
#define mobileSvcLibCallerIDLookup					(sysLibTrapCustom+18)
#define mobileSvcLibAddToAddress					(sysLibTrapCustom+19)
#define mobileSvcLibAddToNote						(sysLibTrapCustom+20)
/* Emergency Call */
#define mobileSvcLibEmergencyCall					(sysLibTrapCustom+21)
/* Profile Services */
#define mobileSvcLibProfileActivate					(sysLibTrapCustom+22)
#define mobileSvcLibProfileGetActive				(sysLibTrapCustom+23)
#define mobileSvcLibProfileGetDefault				(sysLibTrapCustom+24)
#define mobileSvcLibProfileDelete					(sysLibTrapCustom+25)
#define mobileSvcLibProfileAdd						(sysLibTrapCustom+26)
#define mobileSvcLibProfileGet						(sysLibTrapCustom+27)
#define mobileSvcLibProfileSave						(sysLibTrapCustom+28)
#define mobileSvcLibProfileGetCount					(sysLibTrapCustom+29)
#define mobileSvcLibProfileList						(sysLibTrapCustom+30)
#define mobileSvcLibProfileGetIdFromIndex			(sysLibTrapCustom+31)
#define mobileSvcLibProfileDefaultSettings			(sysLibTrapCustom+32)
/* Network registration */
#define mobileSvcLibNetworkRegistration				(sysLibTrapCustom+33)
/* Call State */
#define mobileSvcLibLinesGetCount					(sysLibTrapCustom+34)
#define mobileSvcLibLinesGetCountByState			(sysLibTrapCustom+35)
#define mobileSvcLibLinesGetLineByState				(sysLibTrapCustom+36)
#define mobileSvcLibLinesGetLineByLineId			(sysLibTrapCustom+37)
#define mobileSvcLibLinesGetLine					(sysLibTrapCustom+38)
/* Call HIstory */
#define mobileSvcLibHistoryGetCount					(sysLibTrapCustom+39)
#define mobileSvcLibHistoryGetCountByState			(sysLibTrapCustom+40)
#define mobileSvcLibHistoryGetInfoByState			(sysLibTrapCustom+41)
#define mobileSvcLibHistoryGetInfoByRecordId		(sysLibTrapCustom+42)
#define mobileSvcLibHistoryGetInfo					(sysLibTrapCustom+43)
#define mobileSvcLibHistoryDelete					(sysLibTrapCustom+44)
#define mobileSvcLibHistoryDeleteAll				(sysLibTrapCustom+45)
/* Call Management */
#define mobileSvcLibCallIncoming					(sysLibTrapCustom+46)
#define mobileSvcLibCallDialPhone					(sysLibTrapCustom+47)
#define mobileSvcLibCallSetTtyMode					(sysLibTrapCustom+48)
#define mobileSvcLibCallGetTtyMode					(sysLibTrapCustom+49)
/* SIM File Updates */
#define mobileSvcLibSimGetDiverts					(sysLibTrapCustom+50)
#define mobileSvcLibSimSetDiverts					(sysLibTrapCustom+51)

#ifdef BUILDING_MOBILE_SERVICES
	#define MOBILE_SERVICES_TRAP(trapNum)
#else
	#define MOBILE_SERVICES_TRAP(trapNum) SYS_TRAP(trapNum)
#endif

/*	----------------------------------------------------------------------	*/
/* 	standard functions
/*	----------------------------------------------------------------------	*/
/* function traps */
Err MobileSvcOpen( UInt16 refnum,
					  UInt32 iVersnum)
		MOBILE_SERVICES_TRAP(sysLibTrapOpen);

Err MobileSvcClose( UInt16 refnum)
		MOBILE_SERVICES_TRAP(sysLibTrapClose);

Err MobileSvcSleep( UInt16 refnum )
		MOBILE_SERVICES_TRAP(sysLibTrapSleep);

Err MobileSvcWake( UInt16 refnum )
		MOBILE_SERVICES_TRAP(sysLibTrapWake);
		
Err MobileSvcControl( UInt16 refnum, MobileSvcControlType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibTrapControl);

/* function availability */
Err	MobileSvcIsFunctionSupported( UInt16 iRefnum, UInt16 functionId)
		MOBILE_SERVICES_TRAP(mobileSvcLibTrapIsFunctionSupported);
		
/*  miscellaneous */
Err MobileSvcIsValidCharacter( UInt16 refnum, MobileSvcIsValidCharacterType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibTrapValidCharacter);
		
Err	MobileSvcFilterPhoneNumber( UInt16 refnum, MobileSvcFilterPhoneType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibTrapFilterPhone);

Err	MobileSvcVolumeGet( UInt16 refnum,  MobileSvcVolumeControlType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibTrapVolumeGet);
		
Err	MobileSvcVolumeControl( UInt16 refnum,  MobileSvcVolumeControlType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibTrapVolumeControl);

Err MobileSvcDisplayErrorMessage( UInt16 refnum, MobileSvcDisplayErrorType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibTrapDisplayErrorMessage);
		
Err MobileSvcFormatNetworkID( UInt16 refnum, MobileSvcFormatNetworkIDType *data)
		MOBILE_SERVICES_TRAP(mobileSvcLibTrapFormatNetworkID);	
		
/*  status */
Err MobileSvcDisplayStatus( UInt16 refnum, MobileSvcDisplayStatusType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibTrapDisplayStatus);
		
Err MobileSvcDisplaySignalMeterDeprecated( UInt16 refnum, MobileSvcDisplaySignalMeterType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibTrapDisplaySignal);

Err MobileSvcDisplayBatteryMeterDeprecated( UInt16 refnum, MobileSvcDisplayBatteryMeterType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibTrapDisplayBattery);

/* power */
Err MobileSvcPowerPhone( UInt16 refnum, MobileSvcPowerPhoneType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibTrapPowerPhone);
		
Err MobileSvcPowerGetStatus( UInt16 refnum, MobileSvcPowerGetStatusType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibTrapPowerGetStatus);		

/* Ringing Tone Services */
Err MobileSvcRingingToneGetDefault( UInt16 refnum, UInt32 *uniqueId )
		MOBILE_SERVICES_TRAP(mobileSvcLibRingingToneGetDefault);
		
Err MobileSvcRingingToneCopy( UInt16 refnum, MobileSvcRingingToneCopyType *data )
		MOBILE_SERVICES_TRAP(mobileSvcLibRingingToneCopy);
		
Err MobileSvcRingingToneAdd( UInt16 refnum, MobileSvcRingingToneAddType *data )
		MOBILE_SERVICES_TRAP(mobileSvcLibRingingToneAdd);

Err MobileSvcRingingToneDelete( UInt16 refnum, UInt32 uniqueId )
		MOBILE_SERVICES_TRAP(mobileSvcLibRingingToneDelete);

Err MobileSvcRingingToneConvert( UInt16 refnum, MobileSvcRingingToneConvertType *data )
		MOBILE_SERVICES_TRAP(mobileSvcLibRingingToneConvert);
		
/* Address Book Lookup */
Err MobileSvcCallerIDLookup( UInt16 refnum, MobileSvcCallerIDLookupType *data )
		MOBILE_SERVICES_TRAP(mobileSvcLibCallerIDLookup);
		
Err MobileSvcAddToAddress( UInt16 refnum, MobileSvcAddToAddressType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibAddToAddress);
		
Err MobileSvcAddToNote( UInt16 refnum, MobileSvcAddToNoteType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibAddToNote);
		
/* Emergency Call */

Err MobileSvcEmergencyCall( UInt16 refnum, MobileSvcEmergencyCallType *data )
		MOBILE_SERVICES_TRAP(mobileSvcLibEmergencyCall);
		
/* Profile Services */
Err MobileSvcProfileActivate( UInt16 refnum, UInt32 id )
		MOBILE_SERVICES_TRAP(mobileSvcLibProfileActivate);
Err MobileSvcProfileGetActive( UInt16 refnum, UInt32 *id )
		MOBILE_SERVICES_TRAP(mobileSvcLibProfileGetActive);
Err MobileSvcProfileGetDefault( UInt16 refnum, UInt32 *id )
		MOBILE_SERVICES_TRAP(mobileSvcLibProfileGetDefault);
Err MobileSvcProfileDelete( UInt16 refnum, UInt32 id )
		MOBILE_SERVICES_TRAP(mobileSvcLibProfileDelete);
Err MobileSvcProfileAdd( UInt16 refnum, MobileSvcProfileType *data )
		MOBILE_SERVICES_TRAP(mobileSvcLibProfileAdd);
Err MobileSvcProfileGet( UInt16 refnum, MobileSvcProfileType *data )
		MOBILE_SERVICES_TRAP(mobileSvcLibProfileGet);
Err MobileSvcProfileSave( UInt16 refnum, MobileSvcProfileType *data )
		MOBILE_SERVICES_TRAP(mobileSvcLibProfileSave);
Err MobileSvcProfileGetCount( UInt16 refnum, MobileSvcCountProfilesType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibProfileGetCount);
Err MobileSvcProfileList( UInt16 refnum, MobileSvcGetProfilesType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibProfileList);		
Err MobileSvcProfileGetIdFromIndex( UInt16 refnum, MobileSvcProfileIndexType *data )
		MOBILE_SERVICES_TRAP(mobileSvcLibProfileGetIdFromIndex);	
Err MobileSvcProfileDefaultSettings( UInt16 refnum, MobileSvcProfileType *data )
		MOBILE_SERVICES_TRAP(mobileSvcLibProfileDefaultSettings);

/* Network Registration */
Err MobileSvcNetworkRegistrationInfo( UInt16 refnum, MobileSvcNetworkRegistrationType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibNetworkRegistration);	

/* Call State */
Err MobileSvcLinesGetCount( UInt16 refnum, MobileSvcCountLinesType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibLinesGetCount);	
	
Err MobileSvcLinesGetCountByState( UInt16 refnum, MobileSvcCountLinesType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibLinesGetCountByState);	
		
Err MobileSvcLinesGetLineByState( UInt16 refnum, MobileSvcGetLineType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibLinesGetLineByState);	
		
Err MobileSvcLinesGetLineByLineId( UInt16 refnum, MobileSvcGetLineType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibLinesGetLineByLineId);	
		
Err MobileSvcLinesGetLine( UInt16 refnum, MobileSvcGetLineType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibLinesGetLine);	

/* Call HIstory */
Err MobileSvcHistoryGetCount( UInt16 refnum, MobileSvcCountHistoryType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibHistoryGetCount);	
	
Err MobileSvcHistoryGetCountByState( UInt16 refnum, MobileSvcCountHistoryType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibHistoryGetCountByState);	
		
Err MobileSvcHistoryGetInfoByState( UInt16 refnum, MobileSvcGetHistoryType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibHistoryGetInfoByState);	
		
Err MobileSvcHistoryGetInfoByRecordId( UInt16 refnum, MobileSvcGetHistoryType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibHistoryGetInfoByRecordId);	
		
Err MobileSvcHistoryGetInfo( UInt16 refnum, MobileSvcGetHistoryType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibHistoryGetInfo);	
	
Err MobileSvcHistoryDelete( UInt16 refnum, MobileSvcDeleteHistoryType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibHistoryDelete);	
		
Err MobileSvcHistoryDeleteAll( UInt16 refnum )
		MOBILE_SERVICES_TRAP(mobileSvcLibHistoryDeleteAll);	
		
/* Call management */

Err MobileSvcCallIncoming( UInt16 refnum, MobileSvcCallIncomingType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibCallIncoming);	

Err MobileSvcCallDialPhone( UInt16 refnum, MobileSvcCallDialPhoneType* data )
		MOBILE_SERVICES_TRAP(mobileSvcLibCallDialPhone);	

Err MobileSvcCallSetTtyMode( UInt16 refnum, UInt32 ttyMode )
		MOBILE_SERVICES_TRAP(mobileSvcLibCallSetTtyMode);	

Err MobileSvcCallGetTtyMode( UInt16 refnum, UInt32* ttyMode )
		MOBILE_SERVICES_TRAP(mobileSvcLibCallGetTtyMode);	
		
/* SIM file update */
Err MobileSvcSimGetDiverts (UInt16 refnum, MobileSvcSimGetDivertsType *data)
		MOBILE_SERVICES_TRAP(mobileSvcLibSimGetDiverts);	

Err MobileSvcSimSetDiverts (UInt16 refnum, MobileSvcSimSetDivertsType *data)
		MOBILE_SERVICES_TRAP(mobileSvcLibSimSetDiverts);	

/*	----------------------------------------------------------------------	*/
/*	MACROS to check function availability									*/
/*	----------------------------------------------------------------------	*/
#define UtilsIsControlSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcIsControlMessage)	
#define UtilsIsFilterPhoneNumberSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcFilterPhoneNumberMessage)
/*  miscellaneous */
#define UtilsIsValidCharacterSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcIsValidCharacterMessage)
#define UtilsIsVolumeGetSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcVolumeGetMessage)
#define UtilsIsVolumeControlSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcVolumeControlMessage)
#define UtilsIsDisplayErrorMessageSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcDisplayErrorMessage)
#define UtilsIsFormatNetworkIdSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcFormatNetworkId)
/*  status */
#define UtilsIsDisplayStatusSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcDisplayStatusMessage)
#define UtilsIsDisplaySignalDeprecatedSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcDisplaySignalMessage)
#define UtilsIsDisplayBatteryDeprecatedSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcDisplayBatteryMessage)
/*  power  */	
#define UtilsIsPhonePowerSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcPowerPhoneMessage)
#define UtilsIsPowerGetStatusSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, mobileSvcLibTrapPowerGetStatus)	
/* Ringing Tone Services */	
#define UtilsIsRingingToneGetDefaultSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcRingingToneGetDefaultMessage)
#define UtilsIsRingingToneCopySupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcRingingToneCopyMessage)
#define UtilsIsRingingToneAddSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcRingingToneAddMessage)
#define UtilsIsRingingToneDeleteSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcRingingToneDeleteMessage)
#define UtilsIsRingingToneConvertSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcRingingToneConvertMessage)
/* Address Book Lookup */		
#define UtilsIsCallerIDLookupSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcCallerIDLookupMessage)
#define UtilsIsAddToAddressSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcAddToAddressMessage)
#define UtilsIsAddToNoteSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcAddToNoteMessage)
/* Emergency Call */	
#define UtilsIsEmergencyCallSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcEmergencyCallMessage)
/* Profile Services */
#define UtilsIsProfileActivateSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcProfileActivateMessage)
#define UtilsIsProfileGetActiveSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcProfileGetActiveMessage)
#define UtilsIsProfileGetDefaultSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcProfileGetDefaultMessage)
#define UtilsIsProfileDeleteSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcProfileDeleteMessage)
#define UtilsIsProfileAddSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcProfileAddMessage)
#define UtilsIsProfileGetSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcProfileGetMessage)
#define UtilsIsProfileSaveSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcProfileSaveMessage)
#define UtilsIsProfileGetCountSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcProfileGetCountMessage)
#define UtilsIsProfileListSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcProfileListMessage)	
#define UtilsIsProfileGetIdFromIndexSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcProfileGetIdFromIndexMessage)				
#define UtilsIsProfileDefaultSettingsSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcProfileDefaultSettingsMessage)				
/* Network Registration */
#define UtilsIsNetworkRegistrationSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcNetworkRegistrationMessage)
/* Call State */
#define UtilsIsLinesGetCountSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcLinesGetCountMessage)
#define UtilsIsLinesGetCountByStateSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcLinesGetCountByStateMessage)
#define UtilsIsLinesGetLineByStateSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcLinesGetLineByStateMessage)
#define UtilsIsLinesGetLineByLineIdSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcLinesGetLineByLineIdMessage)
#define UtilsIsLinesGetLineSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcLinesGetLineMessage)
/* Call HIstory */
#define UtilsIsHistoryGetCountSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcHistoryGetCountMessage)
#define UtilsIsHistoryGetCountByStateSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcHistoryGetCountByStateMessage)
#define UtilsIsHistoryGetInfoByStateSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcHistoryGetInfoByStateMessage)
#define UtilsIsHistoryGetInfoByRecordIdSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcHistoryGetInfoByRecordIdMessage)
#define UtilsIsHistoryGetInfoSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcHistoryGetInfoMessage)
#define UtilsIsHistoryDeleteSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcHistoryDeleteMessage)
#define UtilsIsHistoryDeleteAllSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcHistoryDeleteAllMessage)
/* Call Management */
#define UtilsIsCallIncomingSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcCallIncomingMessage)
#define UtilsIsCallDialPhoneSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcCallDialPhoneMessage)
#define UtilsIsCallSetTtyModeSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcCallSetTtyModeMessage)
#define UtilsIsCallGetTtyModeSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcCallGetTtyModeMessage)
/* SIM File Update */
#define UtilsIsSimGetDivertsSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcSimGetDivertsMessage)
#define UtilsIsSimSetDivertsSupported(refnum) \
	MobileSvcIsFunctionSupported(refnum, kMobileSvcSimSetDivertsMessage)

#endif
