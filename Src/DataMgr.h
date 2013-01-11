#ifndef _SMSTW_DATA_MGR
#define _SMSTW_DATA_MGR

class TDataMgr
{
private:
	DmOpenRef m_db;
public:
	TDataMgr(const char* dbName, UInt32 creator);
	DmOpenRef GetOpenRef();
	~TDataMgr();
};

#endif