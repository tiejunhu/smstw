#ifndef FINADDRUTILS_H_INCLUDED
#define FINADDRUTILS_H_INCLUDED

#define fasFtrFinAddrRequestSearch 1
#define fasFtrFinAddrSearchInfo fasFtrFinAddrRequestSearch+1
#define fasFinAddrCreator 'wFAS'
#define fasFtrFinAddrRequestSearchVersion1 1

#define fasNoSearchInfo -1

typedef struct{
	UInt16 recIndex;
	UInt16 phoneLabel;
	char * pLastName;
	char * pFirstName;
	char * pPhoneNumber;
}FasSearchInfoType;

Int16 FasRequestSearch(UInt32 Creator);
Int16 FasGetSearchInfo(FasSearchInfoType *pInfo);
Int16 FasSearchAvailable();

#endif
