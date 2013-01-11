#ifndef _SMTW_TOOLS
#define _SMTW_TOOLS

#include "SMSTW_Rsc.h"

#define kPalmOS20Version sysMakeROMVersion(2,0,0,sysROMStageDevelopment,0)

void* GetObjectPtr(FormPtr frm, UInt16 objectID);
void SetFieldText(FieldPtr field, const char* str);
Err RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags);
Err CallPhone(const char* phoneNumber, const char* userName);
void Log(const char* msg);
Boolean IsGPRSConnected();
void UpdateScrollbar(FormPtr form, UInt16 field, UInt16 scrollbar);
void ScrollField(FormPtr form, Int16 linesToScroll, UInt16 field, UInt16 scrollbar);
void ScrollFieldByEvent(FormPtr form, EventType* eventP, UInt16 field, UInt16 scrollbar);
Boolean ScrollFieldPage(FormPtr form, UInt16 field, UInt16 scrollbar, WinDirectionType dir);
void AssignStr(char** ppStr, const char* pSrcStr, UInt32 len);

#if __option(a6frames)
// if debug mode
#define LOG(msg) \
	Log(msg)
	
class MethodLogger
{
private:
	char m_pszMsg[50];
public:
	MethodLogger(const char* msg)
	{
		StrNCopy(m_pszMsg, msg, 49);
		char tmp[60];
		StrCopy(tmp, m_pszMsg);
		StrCat(tmp, "::Starting");
		Log(tmp);
	}
	
	~MethodLogger()
	{
		char tmp[60];
		StrCopy(tmp, m_pszMsg);
		StrCat(tmp, "::Left");
		Log(tmp);
	}
};

#else

#define LOG(msg) \
	{ NULL; }
	
class MethodLogger
{
public:
	MethodLogger(const char* msg) {}	
};

#endif

#endif