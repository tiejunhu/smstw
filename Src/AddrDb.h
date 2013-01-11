#ifndef _SMTW_ADDRDB
#define _SMTW_ADDRDB

#define firstAddressField			name

typedef union
{
	struct
	{
		unsigned reserved	:13;
		unsigned note		:1;	// set if record contains a note handle
		unsigned custom4	:1;	// set if record contains a custom4
		unsigned custom3	:1;	// set if record contains a custom3
		unsigned custom2	:1;	// set if record contains a custom2
		unsigned custom1	:1;	// set if record contains a custom1
		unsigned title		:1;	// set if record contains a title
		unsigned country	:1;	// set if record contains a birthday
		unsigned zipCode	:1;	// set if record contains a birthday
		unsigned state		:1;	// set if record contains a birthday
		unsigned city		:1;	// set if record contains a birthday
		unsigned address	:1;	// set if record contains a address
		unsigned phone5		:1;	// set if record contains a phone5
		unsigned phone4		:1;	// set if record contains a phone4
		unsigned phone3		:1;	// set if record contains a phone3
		unsigned phone2		:1;	// set if record contains a phone2
		unsigned phone1		:1;	// set if record contains a phone1
		unsigned company	:1;	// set if record contains a company
		unsigned firstName	:1;	// set if record contains a firstName
		unsigned name		:1;	// set if record contains a name (bit 0)

	} bits;
	UInt32 allBits;
} AddrDBRecordFlags;

typedef enum
{
	name,
	firstName,
	company,
	phone1,
	phone2,
	phone3,
	phone4,
	phone5,
	address,
	city,
	state,
	zipCode,
	country,
	title,
	custom1,
	custom2,
	custom3,
	custom4,
	note,			// This field is assumed to be < 4K
	addressFieldsCount
} AddressFields;

typedef union
{
	struct
	{
		unsigned reserved:8;
		unsigned displayPhoneForList:4;	// The phone displayed for the list view 0 - 4
		unsigned phone5:4;				// Which phone (home, work, car, ...)
		unsigned phone4:4;
		unsigned phone3:4;
		unsigned phone2:4;
		unsigned phone1:4;
	} phones;
	UInt32 phoneBits;
} AddrOptionsType;

typedef struct
{
	AddrOptionsType	options;        // Display by company or by name
	Char* fields[addressFieldsCount];
} AddrDBRecordType;

typedef AddrDBRecordType *AddrDBRecordPtr;

Err AddrDBGetRecord(DmOpenRef dbP, UInt16 index, AddrDBRecordPtr recordP, MemHandle *recordH);
void lookupNameFromAddressBook(const char* phoneNumber, char* userName);
const char* GetNakedPhoneNumber(const char* phoneNumber);

#define AddressDbName "AddressDB"
#define AddressDbType 'DATA'
#define AddressDbCreatorId 'addr'

#endif
