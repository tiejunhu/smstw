#include <SystemMgr.h>
#include <FeatureMgr.h>
#include <StringMgr.h>

#include "FinAddrUtils.h"

Int16 FasSearchAvailable()
{
	DmSearchStateType searchInfo;
	UInt16 cardNo;
	LocalID dbID;

	if(DmGetNextDatabaseByTypeCreator(true,&searchInfo,sysFileTApplication,
		fasFinAddrCreator,true,&cardNo,&dbID)==errNone)
		return 1;		//true
	else
		return 0;		//false
}

Int16 FasRequestSearch(UInt32 Creator)
{
	UInt32* pCreator;
	DmSearchStateType searchInfo;
	UInt16 cardNo;
	LocalID dbID;

	pCreator=(UInt32 *)MemPtrNew(sizeof(UInt32));
	if(!pCreator) return -1;
	if ((MemPtrSetOwner(pCreator, 0) == 0) && 
		(DmGetNextDatabaseByTypeCreator(true, &searchInfo, sysFileTApplication,
		 	 fasFinAddrCreator, true, &cardNo, &dbID) == errNone))
	{
		*pCreator=Creator;
		FtrSet(fasFinAddrCreator, fasFtrFinAddrRequestSearch, fasFtrFinAddrRequestSearchVersion1); 
		SysUIAppSwitch(cardNo, dbID, sysAppLaunchCmdNormalLaunch, (MemPtr)pCreator);
		return 0;
	}
	MemPtrFree(pCreator);
	return -1;
}

Int16 FasGetSearchInfo(FasSearchInfoType *pInfo)
{
	Err error;
	char * pData;
	char * pChrInfo;
	
	error=FtrGet(fasFinAddrCreator, fasFtrFinAddrSearchInfo, (UInt32 *)&pData);
	if(error==0)
	{
		pInfo->recIndex=*(UInt16 *)pData;
		pData+=sizeof(UInt16);
		pInfo->phoneLabel=*(UInt16 *)pData;
		pChrInfo=pData+sizeof(UInt16);
		pInfo->pLastName=pChrInfo;
		pChrInfo=pChrInfo+StrLen(pChrInfo)+1;
		pInfo->pFirstName=pChrInfo;
		pChrInfo=pChrInfo+StrLen(pChrInfo)+1;
		pInfo->pPhoneNumber=pChrInfo;
		FtrUnregister(fasFinAddrCreator, fasFtrFinAddrSearchInfo);
		return 0;
	}
	else
	{
		return fasNoSearchInfo;
	} 
}