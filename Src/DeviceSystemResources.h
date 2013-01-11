/******************************************************************************
 *
 * Copyright (c) 1996-2003 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: DeviceSystemResources.h
 *
 * Release: 
 *
 * Description:
 *		Include file for both PalmRez and the C Compiler. This file contains
 *  equates used by both tools. When compiling using the C compiler
 *  the variable RESOURCE_COMPILER must be defined.
 *
 * History:
 *		7/24/01		ACo		Copied from Primary
 *
 *****************************************************************************/

#ifndef __DEVICESYSTEMRESOURCES_H__
#define __DEVICESYSTEMRESOURCES_H__

//-----------------------------------------------------------
// File types and creators
// This section is common to both the C and Resource Compiler
// Refer to SystemResources.h for details
//-----------------------------------------------------------

#define sysFileCTelMgrLibOem				'tmge'	// Creator ID for the Telephony Manager Extension shared library.
#define sysFileCSimATDriverOem				'sims'	// Creator ID for the SIM AT Driver.
#define sysFileCSimOfficer					'sats'	// Creator ID for the SIM Application Toolkit client application.
#define sysFileCSimWatcherLib				'simw'	// Creator ID for the SIM Watcher library.
#define sysFileCProvisioning				'prov'	// Creator ID for the provisioning application.
#define sysFileCMobileServices				'msvc'  // Creator ID for the Mobile Service Library
#define sysFileCMyFavorite					'myfv'	// Creator ID for the MyFavorite App.
#define sysFileCNotePad						'npad'	// Creator ID for the Notepad app.

#define sysFileTHidden						'hidn'  // File Type of MyFavorite App.
#define sysFileTSimFileCacheDB				'sefc'	// File Type for the SIM Elementary File Cache daba base.
#define sysFileTDeviceExtension        		'extd'  // File Type for device extensions that need to be loaded after OEM extension

#endif // __DEVICESYSTEMRESOURCES_H__
