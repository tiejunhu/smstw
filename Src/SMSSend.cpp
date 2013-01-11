#include <vector>
#include <SMSLib.h>

#include "SMSTools.h"
#include "SMSSend.h"
#include "MobilePower.h"
#include "SMSDb.h"
#include "PrefReminder.h"
#include "SMSTable.h"

const UInt16 MAX_TRY_COUNT = 499;
const UInt16 CMD_TRY_COUNT = 3;

#define TRY_LOOP_BEGIN(var) \
	for (UInt16 var = 0; var < CMD_TRY_COUNT; ++var) { {}

#define TRY_LOOP_END \
	} {}
	
#define CONTINUE_ON_ERR(err, msg) \
	{ if (err) { LOG(msg); continue; } }
	
#define RETURN_ON_NONE_ERR(err) \
	{ if (err == errNone) return errNone; }

static Err CommSend(UInt16 port, const char* str)
{
	LOG("CommSend::Starting and sending ...");
	LOG(str);
	Err err = errNone;
	
	TRY_LOOP_BEGIN(i);
	
	SrmSend(port, str, StrLen(str), &err);
	CONTINUE_ON_ERR(err, "CommSend::SrmSend Error, Continuing");
	
	err = SrmSendWait(port);
	CONTINUE_ON_ERR(err, "CommSend::SrmSendWait Error, Continuing");
	
	if (err) {
		LOG("CommSend::Error");
	} else {
		LOG("CommSend::Succeeded");
	}
	
	RETURN_ON_NONE_ERR(err);
	
	TRY_LOOP_END;

	return err;
}

/***********************************************************************
 *
 * FUNCTION:    PrvSmsExgLibOpen
 *
 * DESCRIPTION: Initialize the link with SMS Library
 *
 * PARAMETERS:  refNumP - pointer to the lib reference number
 *
 * RETURNED:    An error or 0 if not
 *
 ***********************************************************************/
static Err PrvSmsExgLibOpen(UInt16 *refNumP)
{
	Err	err;

	if (*refNumP == sysInvalidRefNum)
	{
		err = SysLibFind(kSmsLibName, refNumP);
		if (err == sysErrLibNotFound)
		{
			err = SysLibLoad(sysFileTExgLib, sysFileCSmsLib, refNumP);
			if (err)
			{
				if (err != sysErrLibNotFound)
					ErrFatalDisplay("Load SMS Lib Error");
				*refNumP = sysInvalidRefNum;
				// Phone driver not installed
				return err;
			}
		}
	}
	err = ExgLibOpen(*refNumP);
	if (err)
	{
		*refNumP = sysInvalidRefNum;	// Reset the refNum if com can't be open
	}
	return err;
}

/***********************************************************************
 *
 * FUNCTION:    PrvSmsExgOpenCom
 *
 * DESCRIPTION: Open the com
 *
 * PARAMETERS:  socket  - a pointer to store information for the socket
 *              count   - number of record for this socket
 *              refNumP - pointer to the lib reference number
 *
 * RETURNED:    An error or 0 if not
 *
 ***********************************************************************/
static Err PrvSmsExgOpenCom(ExgSocketType* socket, UInt32 count, UInt16 *refNumP)
{
	Err	err;

	if (*refNumP == sysInvalidRefNum)
		err = PrvSmsExgLibOpen(refNumP);

	if (*refNumP != sysInvalidRefNum)
	{
		MemSet(socket, sizeof(ExgSocketType), 0);
		socket->libraryRef = *refNumP;
		socket->count = count;
		err = ExgConnect(socket);
	}

	return err;
}

static char CommReadNextChar(UInt16 port, Err& err)
{
	char buf[1];
	SrmReceive(port, buf, 1, 1500, &err);
	if (err) {
		LOG("CommReadNextChar::Error");
		char buf[12];
		StrPrintF(buf, "Err: %x", err);
		LOG(buf);
		return 0;
	}
	return buf[0];
}

static Boolean IsValidChar(char c)
{
	if ((c >= 32) && (c <= 126)) return true;
	return false;
}

static Err CommReadNextLine(UInt16 port, char* str, UInt16 strlen)
{
	int i = 0;
	char c = 0;
	Err err = errNone;
	Int16 tryCount = 0;
	
	MemSet(str, strlen, 0);
	
	do {
		tryCount ++;
		c = CommReadNextChar(port, err);
		if (err) {
			LOG("CommReadNextLine::Error1, Returning");
			return err;
		}
		if (tryCount > MAX_TRY_COUNT) {
			LOG("CommReadNextLine::Try out 1");
			return serErrRead;
		}
	} while (!IsValidChar(c));

	tryCount = 0;
	while(IsValidChar(c)) {
		tryCount ++;
		if (i >= strlen - 1) {
			LOG("CommReadNextLine::Buffer full");
			str[i] = 0;
			LOG(str);
			return serErrRead;
		}
		
		str[i] = c;
		i ++;
		c = CommReadNextChar(port, err);
		if (err) {
			LOG("CommReadNextLine::Error2, Returning");
			return err;
		}
		if (tryCount > MAX_TRY_COUNT) {
			LOG("CommReadNextLine::Try out 2");
			str[i] = 0;
			LOG(str);
			return serErrRead;
		}
	}
	str[i] = 0;
	return errNone;
}

static Err CommReadUntilOk(UInt16 port)
{
	MethodLogger _logger("CommReadUntilOk");
	char buf[500];
	Int16 tryCount = 0;
	do {
		tryCount ++;
		Err err = CommReadNextLine(port, buf, 500);
		if (err) {
			LOG("CommReadUntilOk::Error");
			char buf[12];
			StrPrintF(buf, "Err: %x", err);
			LOG(buf);
			return err;
		}
		if (StrCompare(buf, "OK") == 0) break;
		LOG(buf);
		if (tryCount > MAX_TRY_COUNT) {
			LOG("CommReadUntilOk::Try out");
			return serErrRead;
		}
	} while(true);
	
	return errNone;	
}

static Err CommReadNextMessage(UInt16 port, char* str, UInt16 strlen)
{
	MethodLogger _logger("CommReadNextMessage");
	while (true) {
		Err err = CommReadNextLine(port, str, strlen);
		if (err) {
			LOG("CommReadNextMessage::Error, Returning");
			return err;
		}
		
		if (StrCompare(str, "OK") == 0) {
			StrCopy(str, "");
			LOG("CommReadNextMessage::OK");
			return errNone;
		}
		if (StrNCompare(str, "+CME ERROR", 10) == 0) {
			LOG(str);
			return serErrRead;
		} 
		if (StrNCompare(str, "+CMS ERROR", 10) == 0) {
			LOG(str);
			return serErrRead;
		}
		
		err = CommReadUntilOk(port);
		return err;
	}
}

static Err CommReadUntil(UInt16 port, const char* str)
{
	MethodLogger _logger("CommReadUntil");
	char c = 0;
	int i = 0;
	Err err = errNone;
	
	Int16 tryCount = 0;
	
	while(true) {
		tryCount ++;
		c = CommReadNextChar(port, err);	
		if (err) return err;
		if (c == str[i]) {
			i ++;
			if (str[i] == 0) {
				break;
			}
		} else {
			i = 0;
		}
		if (tryCount > MAX_TRY_COUNT) return serErrRead;
	}
	
	return errNone;
}

static Err InitSerial(UInt16* pPortId, char* buf, UInt16 buflen)
{
	MethodLogger _logger("InitSerial");
	
	UInt16 i = 0;
	SrmOpenConfigType openConfig;
	
	MemSet(&openConfig, sizeof(SrmOpenConfigType), 0);
	openConfig.baud = 9600;
	openConfig.function = serFncTelephony;
	
	Err err = errNone;
	
	*pPortId = 0;
	
	err = SrmExtOpen('JVDR', &openConfig, sizeof(SrmOpenConfigType), pPortId);
	if (err) {
		LOG("InitSerial::SrmExtOpen failed");
		return err;
	}
	
	err = CommSend(*pPortId, "ATE0\r");
	if (err) {
		LOG("InitSerial::CommSend ATE0 failed");
		return err;
	}

	err = CommReadNextMessage(*pPortId, buf, buflen);
	if (err) {
		LOG("InitSerial::CommReadNextMessage after ATE0 failed");
		return err;
	}

	err = CommSend(*pPortId, "AT+CSMS=0\r");
	if (err) {
		LOG("InitSerial::CommSend AT+CSMS=0 failed");
		return err;
	}
	
	err = CommReadNextMessage(*pPortId, buf, buflen);
	if (err) {
		LOG("InitSerial::CommReadNextMessage after AT+CSMS=0 failed");
		return err;
	}
	
	return errNone;
}

static void EncodePhoneNumber(const char* phoneNumber, char* buf, Int16* len)
{
	UInt16 iIndex = 0;
	UInt16 oIndex = 0;
	UInt16 iLen = StrLen(phoneNumber);
	
	if (phoneNumber[0] == '+') {
		iIndex += 1;
	} else {
//		buf[0] = '6';
//		buf[1] = '8';
//		oIndex += 2;
	}
	
	while (iIndex < iLen) {
		buf[oIndex + 1] = phoneNumber[iIndex];
		iIndex ++;
		if (iIndex == iLen) {
			buf[oIndex] = 'F';
		} else {
			buf[oIndex] = phoneNumber[iIndex];
		}
		oIndex += 2;
		iIndex ++;
	}
	
	*len = oIndex;
}

#define RequestStatusReport(c) \
	{ c = c | 0x20; }

#define DontRequestStatusReport(c) \
	{ c = c & ~0x20; }
	
#define SetMessageTypeSMSSubmit(c) \
	{ c = c | 0x01; c = c & ~0x02; }

#define SetMessageValidPeriodRelative(c) \
	{ c = c | 0x10; c = c & ~0x80; }
	
#define SetCodingSchemeUCS2(c) \
	{ c = 0x08; }

#define SetProtocolDefault(c) \
	{ c = 0; }
	
#define SetMaxValidPeriod(c) \
	{ c = 0x45; }

static void FillTwoCharWithHex(char* buf, Int16 value)
{
	char hexbuf[5];
	StrPrintF(hexbuf, "%X", value);
	buf[0] = hexbuf[2];
	buf[1] = hexbuf[3];
}

static void EncodeString(gb2312_table& table, const char* str, char* buf, Int16* encodedLength)
{
	MethodLogger __logger("EncodeString");
	Int16 srcLength = StrLen(str);
	Int16 destLength = srcLength * 2;
	unsigned char* pszTemp = (unsigned char *) MemPtrNew(destLength);
	MemSet(pszTemp, destLength, 0);
	*encodedLength = gb2312_to_ucs2(table, (const unsigned char*) str, pszTemp, srcLength);
	
	/*
	 * turn into string
	 */
	for (UInt16 i = 0; i < *encodedLength; ++i) {
		FillTwoCharWithHex(buf + i * 2, pszTemp[i]);
	}

	MemPtrFree(pszTemp);
}

#define GOTO_ERR_ON_ERR(err) \
	CHECKERR(err); if (err) goto error;
	
static Int16 FillBufferBeforeContent(char* buf, const char* dest)
{
	MethodLogger __logger("FillBufferBeforeContent");

	Int16 len;
	Int16 i;
	char c;
	
/*	use sms center parameter

	EncodePhoneNumber(center, buf + 4, &len);
	FillTwoCharWithHex(buf, len / 2 + 1);
	buf[2] = '9';
	buf[3] = '1';
	i = len + 4;*/
	
	// use sim sms center
	buf[0] = '0';
	buf[1] = '0';
	i = 2;

	// Set First Octet
	c = 0;
	
	SendPref spref;
	ReadSendPreference(spref);
	
	if (spref.requestReport) {
		RequestStatusReport(c);
	} else {
		DontRequestStatusReport(c);
	}
	SetMessageTypeSMSSubmit(c);
	SetMessageValidPeriodRelative(c);
	FillTwoCharWithHex(buf + i, c);
	i += 2;

	// TP-Message-Reference
	c = 0;
	FillTwoCharWithHex(buf + i, c);
	i += 2;
	
	// Dest
	len = StrLen(dest);
	if (dest[0] == '+') len --;
	FillTwoCharWithHex(buf + i, len);
	i += 2;

	// 81 format, ok with or without +86	
	buf[i] = '8';
	buf[i + 1] = '1';
	i += 2;
	
	EncodePhoneNumber(dest, buf + i, &len);
	i += len;

	// protocol
	c = 0;
	SetProtocolDefault(c);
	FillTwoCharWithHex(buf + i, c);
	i += 2;

	// coding scheme
	c = 0;
	SetCodingSchemeUCS2(c);
	FillTwoCharWithHex(buf + i, c);
	i += 2;
	
	// valid period
	c = 0;
	SetMaxValidPeriod(c);
	FillTwoCharWithHex(buf + i, c);
	i += 2;
	
	return i;
}

static Err InternalSendEncodedStr(const char* dest, const char* content, Int16 len, FieldPtr fieldProgress)
{
	MethodLogger _logger("InternalSendEncodedStr");
	
	MemHandle bufHandle = MemHandleNew(500);
	CheckNullPointer(bufHandle);
	
	char* buf = (char *) MemHandleLock(bufHandle);
	MemSet(buf, 500, 0);
	
	MemHandle tmpHandle = MemHandleNew(500);
	CheckNullPointer(tmpHandle);
	
	char* tmpbuf = (char *) MemHandleLock(tmpHandle);
	CheckNullPointer(tmpbuf);
	
	Int16 i = FillBufferBeforeContent(buf, dest);
	
	FillTwoCharWithHex(buf + i, len / 2);
	StrNCopy(buf + i + 2, content, len);
	
	i += 2 + len;
	
	len = StrLen(buf);
	
	buf[i] = '\032';
	buf[i + 1] = '\r';
	
	char cmdbuf[30];
	MemSet(cmdbuf, 30, 0);
	StrCopy(cmdbuf, "AT+CMGS=");
	len = len / 2 - 1;
	cmdbuf[StrLen(cmdbuf)] = (len / 100) + '0';
	len = len - len / 100 * 100;
	cmdbuf[StrLen(cmdbuf)] = (len / 10) + '0';
	cmdbuf[StrLen(cmdbuf)] = (len % 10) + '0';
	StrCat(cmdbuf, "\r");
	
	SetFieldText(fieldProgress, "Initializing...");
	FldDrawField(fieldProgress);
	
	UInt16 portId = NULL;
	Err err = errNone;

/*	UInt16 refNum = sysInvalidRefNum;
	ExgSocketType socket;
	PrvSmsExgOpenCom(&socket, 1, &refNum);*/

TRY_LOOP_BEGIN(loopvar);

	err = InitSerial(&portId, tmpbuf, 500);
	if (err == serErrConfigurationFailed) {
		TurnRadioOnOff(true);
		if (portId) SrmClose(portId);
		ShowMsg("Phone turned on.");
		err = InitSerial(&portId, tmpbuf, 500);
	}
	CONTINUE_ON_ERR(err, "InternalSendEncodedStr::InitSerial Failed, Continuing");

	SetFieldText(fieldProgress, "Sending...");
	FldDrawField(fieldProgress);

	err = CommSend(portId, cmdbuf);
	if (err) SrmClose(portId);
	CONTINUE_ON_ERR(err, "InternalSendEncodedStr::CommSend Cmd Failed, Continuing");
	
	err = CommReadUntil(portId, "> ");
	if (err) SrmClose(portId);
	CONTINUE_ON_ERR(err, "InternalSendEncodedStr::CommReadUntil > Failed, Continuing");
	
	err = CommSend(portId, buf);
	if (err) SrmClose(portId);
	CONTINUE_ON_ERR(err, "InternalSendEncodedStr::CommSend SMS Failed, Continuing");

	err = CommReadNextMessage(portId, tmpbuf, 500);
	if (err) SrmClose(portId);
	CONTINUE_ON_ERR(err, "InternalSendEncodedStr::CommReadNextMessage Failed, Continuing");
	
	if (err == errNone) goto ok;

TRY_LOOP_END;
	
	goto error;
	
ok:
	SetFieldText(fieldProgress, "Sent successfully.");
	err = errNone;
	goto end;

error:
	SetFieldText(fieldProgress, "Error while sending.");

end:
	FldDrawField(fieldProgress);
	SrmClose(portId);
	MemHandleUnlock(tmpHandle);
	MemHandleUnlock(bufHandle);
	MemHandleFree(tmpHandle);
	MemHandleFree(bufHandle);
	return err;

}

/**
 * content ×î¶à280¸ö×Ö·û¡£
 */
static Err InternalSendSMS(gb2312_table& table, const char* center, 
	const char* dest, const char* content, FieldPtr fieldProgress)
{
	MethodLogger _logger("InternalSendSMS");
	Int16 len = StrLen(content);

	MemHandle contentHandle = MemHandleNew(len * 4); // one char turn into four char
	char* contentbuf = (char *) MemHandleLock(contentHandle); 
	MemSet(contentbuf, len * 4, 0);
	EncodeString(table, content, contentbuf, &len);
	
	Int16 left = len * 2;
	Err err = errNone;
	while(true) {
		if (left > 280) {
			err = InternalSendEncodedStr(dest, contentbuf + len * 2 - left, 280, fieldProgress);
			if (err) break;
			left -= 280;
		} else {
			err = InternalSendEncodedStr(dest, contentbuf + len * 2 - left, left, fieldProgress);
			break;
		}
	}

	MemHandleUnlock(contentHandle);
	MemHandleFree(contentHandle);	
	return err;
}

Err SendSMS(gb2312_table& table, const char* center, const char* dest, const char* content)
{
	MethodLogger _logger("SendSMS");
	
	if (IsGPRSConnected()) {
		ShowMsg("GPRS is connected, cannot send SMS.");
		return -1;
	}
		
	FormPtr oldfrm = FrmGetActiveForm();

	FormPtr frm = FrmInitForm(FormProgress);
	FrmDrawForm(frm);
	
	char msg[100];
	StrPrintF(msg, "Sending SMS to %s ...", dest);
	FieldPtr fieldDest = (FieldPtr) GetObjectPtr(frm, FieldDest);
	SetFieldText(fieldDest, msg);
	FldDrawField(fieldDest);
	
	FieldPtr fieldProgress = (FieldPtr) GetObjectPtr(frm, FieldProgress);
	Err err = InternalSendSMS(table, center, dest, content, fieldProgress);
	
	FrmEraseForm(frm);
	FrmDeleteForm(frm);
	FrmDrawForm(oldfrm);
	
	return err;
}

void SendAllInOutbox(DmOpenRef db, gb2312_table& table, const char* center)
{
	FormPtr oldFrm = FrmGetActiveForm();
	UInt16 count = 	GetSMSCount(db, CAT_OUTBOX);
	std::vector<UInt16> sentSMS;
	for (int i = 0; i < count; ++i) {
		SMSDbRecordType record;
		MemHandle memHandle;
		Err err = GetSMSRecord(db, CAT_OUTBOX, i, record, memHandle, true);
		CHECKERR(err);
		if (err) continue;
		err = SendSMS(table, center, record.phoneNumber, record.content);
		MemHandleUnlock(memHandle);
		if (err == errNone) sentSMS.push_back(record.index);
	}
	
	std::vector<UInt16>::iterator i = sentSMS.begin();
	for(; i != sentSMS.end(); ++i) {
		UInt16 index = *i;
		MoveRecordCategory(db, index, -1, CAT_SENT);
	}
	
	SortSMS(db);
	RefreshPage(db);
	
	char buf[30];
	StrPrintF(buf, "%d messages sent.", (UInt16) sentSMS.size());
	ShowMsg(buf);
	FrmDrawForm(oldFrm);
}
