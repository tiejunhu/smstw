#ifndef _SMTW_GLOBAL
#define _SMTW_GLOBAL

#include "conv.h"
#include "SMSTools.h"
#include <ErrorBase.h>

extern DmOpenRef g_SmsDb;
extern DmOpenRef g_GbTableDb;
extern gb2312_table g_gbTable;
extern UInt16 g_SelectedCategory;
extern UInt32 g_DisplayUniqId;
extern const char* DayOfWeekInChinese[];

enum CategoryFolder {
	CAT_INBOX = 0, CAT_SENT, CAT_OUTBOX, CAT_DRAFT, CAT_ARCHIVE, CAT_TRASH
};

enum ComposeMode {
	COMPOSE_REPLY, COMPOSE_NEW, COMPOSE_FORWARD, COMPOSE_FEATURE, COMPOSE_RESTORE
};

extern ComposeMode g_ComposeMode;

#define TUNGSTEN_W_FEATURE_GPRS_STATUS			( 2 )


enum {
	PREF_REMINDER = 0,
	PREF_SEND,
	PREF_OTHER
};

#define	serErrRead (serErrorClass | 50)
#define exgErrComplete (exgErrorClass | 50)
#define frmErrNotTheForm (appErrorClass | 50)

enum {
	appLaunchCmdAlarmEventGoto = sysAppLaunchCmdCustomBase
};

#define ShowMsg(msg) \
	{ FrmCustomAlert(AlertCustom, "", msg, ""); }

static void CheckErr(Err err, const char* file, int line)
{
	if (err) {
		char buf[50];
		StrPrintF(buf, "Err: %x at %s, %d", err, file, line);
		ShowMsg(buf);
	}
}

#define CHECKERR(err) \
	CheckErr(err, __FILE__, __LINE__)
	
#define RETURN_ERR_ON_ERR(err) \
	CHECKERR(err); if (err) return err;
	
#define RETURN_ERR(err) \
	if (err) return err;

static void ShowNum(const char* msg, UInt16 num)
{
	char buf[50];
	StrCopy(buf, msg);
	StrCat(buf, ", ");
	StrIToA(buf + StrLen(buf), num);
	ShowMsg(buf);
}

#define CheckPointer(p, msg) \
	{ if ((p) == NULL) FrmCustomAlert(AlertCustom, "", msg, ""); }
	
#define CheckNullPointer(p) \
	{ if ((p) == NULL) { char msg[50]; StrPrintF(msg, "NULL pointer at file %s, line %d", __FILE__, __LINE__); FrmCustomAlert(AlertCustom, "", msg, ""); } }
	
#define FormIsNot(frm, id) \
	(FrmGetFormId(frm) != (id))
	
#endif