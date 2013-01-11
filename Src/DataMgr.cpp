#include "DataMgr.h"

TDataMgr::TDataMgr(const char* dbName, UInt32 creator) :
	m_db(NULL)
{
	LocalID dbId = DmFindDatabase(0, dbName);
	if (dbId == 0) {
		Err err = DmCreateDatabase(0, dbName, creator, 'DATA', false);
		if (err == errNone) {
			dbId = DmFindDatabase(0, dbName);
		} 
	}
	
	if (dbId != 0) {
		m_db = DmOpenDatabase(0, dbId, dmModeReadWrite);
	}
	
	ErrFatalDisplayIf(m_db == NULL, "cannot open db");
}

TDataMgr::~TDataMgr()
{
	ErrFatalDisplayIf(m_db == NULL, "No open db");
	DmCloseDatabase(m_db);
}

DmOpenRef TDataMgr::GetOpenRef()
{
	ErrFatalDisplayIf(m_db == NULL, "No open db");
	return m_db;
}
