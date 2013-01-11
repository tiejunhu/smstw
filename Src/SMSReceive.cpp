#include "SMSReceive.h"
#include <SMSLib.h>
#include "conv.h"
#include "SMSTW_Rsc.h"
#include "SMSTools.h"
#include "Global.h"
#include "SMSDb.h"

#define kTextSize 500
#define kNumberSize 32

#pragma warn_a5_access on

Err ReceiveSMS(DmOpenRef db, ExgSocketPtr exgSocketP, MemHandle& numberH, MemHandle& textH, 
	UInt32 *lengthP, Int8* coding)
{
	SmsParamsPtr	smsParamP;
	UInt32			dataSize = kTextSize;	// Default size
	UInt32			dataLen = 0;
	UInt32			len;
	Err				err = errNone;
	
	LOG("ReceiveSMS::Starting");

	if (numberH == NULL)
		numberH = MemHandleNew(kNumberSize);
	if (textH == NULL)
		textH = MemHandleNew(kTextSize);
		
	// Accept will open a progress dialog and wait for your receive commands
	err = ExgAccept(exgSocketP);

	if (err)
		return err;

	// Get all application specific info
	smsParamP = (SmsParamsPtr)exgSocketP->socketRef;
	
	*coding = smsParamP->dataCodingScheme;
	
	err = exgErrBadParam;
	
	if ((smsParamP == NULL)
			|| (smsParamP->networkType != kSmsNetworkGSM))
		return err;

	// Useless switch, just to be able to make a few comments...
	switch(smsParamP->dataType)
	{
		case kSmsMessageType:
			/*-------------------------------------------------*/
			/* Standard SMS message of no more than 160 bytes  */
			/*-------------------------------------------------*/
			break;
			
		case kSmsIncompleteType:
			/*-------------------------------------------------*/
			/* A part of a multipart SMS message: you don't    */
			/* really have to manage this temporary parts, but */
			/* it can be a good idea to retrieve them (see     */
			/* ExgSampleRequestSMS()) and to let the user      */
			/* decides if they should be deleted (for example, */
			/* if the other parts are definitively lost)       */
			/*-------------------------------------------------*/
			return err;
						
		case kSmsReportType:
			/*-------------------------------------------------*/
			/* A confirmation, indicating that an SMS message  */
			/* was successfully sent.                          */
			/* In this sample, the reports are not managed...  */
			/* Retrieving a report is nearly the same as       */
			/* retrieving a message, but you have to match the */
			/* report to the corresponding message, using      */
			/* smsParamP->smsID. And smsParamP->data.report    */
			/* is the specific report struncture...            */
			/*-------------------------------------------------*/
			MarkRecordDeliveredByPhoneNumber(db, smsParamP->data.report.originatingAddress);
			return exgErrComplete;
	}

	UInt8* textP = (UInt8 *) MemHandleLock(textH);
	MemSet(textP, kTextSize, 0);

	if (exgSocketP->length)
		dataSize = exgSocketP->length;

	do
	{
		len = ExgReceive(exgSocketP, &textP[dataLen], dataSize - dataLen, &err);
		
		if (len && !err)
		{
			dataLen += len;
			
			/*------------------------------------------------------*/
			/* In this sample, the memory management is simplified, */
			/* but you should have something more efficient than    */
			/* reserving a large and fixed block of memory...       */
			/* Something more or less like this, for example:       */
			/*------------------------------------------------------*/
			/*
			// resize block when we reach the limit of this one...
			if (dataLen == dataSize)
			{
				MemHandleUnlock(textH);
				err = MemHandleResize(textH, dataSize + 50);
				textP = MemHandleLock(textH);
				
				if (err)
					return err;
				
				dataSize += 50;
			}
			*/
		}
	}
	while (len && !err);   // reading 0 bytes means end of file....

	if (err) {
		goto exit;
	}

	/*------------------------------------------*/
	/* Copy message in the parameters to return */
	/*------------------------------------------*/
	*lengthP = dataLen;
	
	UInt8* numberP = (UInt8 *) MemHandleLock(numberH);
	MemSet(numberP, kNumberSize, 0);
	StrCopy((char *)numberP, smsParamP->data.receive.originatingAddress);
	MemHandleUnlock(numberH);
	LOG("ReceiveSMS::Got MSG");
	

exit:
	MemHandleUnlock(textH);
	/*-------------------*/
	/* Disconnection     */
	/*-------------------*/
	err = ExgDisconnect(exgSocketP, err);
	LOG("ReceiveSMS::Left");

	return errNone;
}

#pragma warn_a5_access reset
