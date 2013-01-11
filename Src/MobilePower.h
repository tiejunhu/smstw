#ifndef _MOBILE_POWER
#define _MOBILE_POWER

#include <PalmOS.h>
#include <TelephonyMgr.h>
#include "SMSTW.h"
#include "TelephonyMgrOem.h"
#include "MobileServices.h"
#include "Global.h"


static Boolean IsRadioOn()
{
	Err							 err				= errNone;
	UInt16						 telRefNum			= 0;
	TelAppID                     telAppId			= 0;
	UInt32						 powerLevel			= 0xFFFFFFFF;


	err = SysLibFind( kTelMgrLibName, &telRefNum );
	if (err == sysErrLibNotFound)
	{
		err = SysLibLoad( sysFileTLibrary, sysFileCTelMgrLib, &telRefNum );
		if (err)
		{
			return false;
		}
	}
	
	err = TelOpen( telRefNum, kTelMgrVersion, &telAppId );
	if (err)
	{
		return false;
	}
															
//	FtrGet( appFileCreator, TUNGSTEN_W_FEATURE_NUM_POWER_STATUS, &powerLevel );
	// Check if the radio is on
	switch ( powerLevel )
	{
		// Radio/Phone is Off
		case kTelOemLaunchCmdPhoneIsOff:
			return false;
			break;
		// Radio/Phone is On
		case kTelOemLaunchCmdPhoneIsOn:
			return true;
			break;
		default:
			return false;
			break;
	}
}

static Err TurnRadioOnOff( Boolean	turnRadioOn )
{
	MethodLogger _logger("TurnRadioOnOff");
	MobileSvcPowerPhoneType		param;
	UInt16						mobileSvcRefNum;
	Err							error				= errNone;


	error = SysLibFind( kMobileSvcLibName, &mobileSvcRefNum );
	if( error )
		error =
		   SysLibLoad( kMobileSvcDatabaseType,
		                        kMobileSvcDatabaseCreator,
		                        &mobileSvcRefNum );
	if ( error )
	{
		return ( error );
	}

	error = MobileSvcOpen( mobileSvcRefNum, kMobileSvcVersion );
	if( error )
	{
		return ( error );
	}

	if ( turnRadioOn == true )
	{
		param.state = MobileSvcPowerPhoneOn;
	}
	else
	{
		param.state = MobileSvcPowerPhoneOff;
	}
	param.ask    = false;  	// Just do it
	param.urgent = false;

	error = MobileSvcPowerPhone( mobileSvcRefNum, &param );
	
	// Close the Mobile Services library
	MobileSvcClose( mobileSvcRefNum );

	return ( errNone );
}

#endif