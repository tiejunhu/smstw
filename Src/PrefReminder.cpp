#include "PrefReminder.h"
#include "SMSTW_Rsc.h"
#include "SMSTW.h"
#include "SMSTools.h"
#include "Global.h"

// handle to the list containing names and DB info of MIDI tracks.
// Each entry is of type SndMidiListItemType.
static MemHandle	gMidiListH;
// number of entries in the MIDI list
static UInt16	gMidiCount;

#define soundTriggerLabelLen			100

static Char soundTriggerLabelP[soundTriggerLabelLen];

/***********************************************************************
 *
 * FUNCTION:    PlayAlarmSound
 *
 * DESCRIPTION:	Play a MIDI sound given a unique record ID of the MIDI record in the System
 *						MIDI database.  If the sound is not found, then the default alarm sound will
 *						be played.
 *
 * PARAMETERS:  uniqueRecID	-- unique record ID of the MIDI record in the System
 *										   MIDI database.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk	8/10/97	Initial version
 *			trev	08/14/97	Ported to dateBook App
 *			vmk	12/11/97	Prevent call to DmFindRecordByID if DmOpenDatabase failed
 *
 ***********************************************************************/
void PlayAlarmSound(UInt32 uniqueRecID)
{
	Err			err;
	MemHandle	midiH;							// handle of MIDI record
	SndMidiRecHdrType*	midiHdrP;			// pointer to MIDI record header
	UInt8*		midiStreamP;					// pointer to MIDI stream beginning with the 'MThd'
														// SMF header chunk
	UInt16		cardNo;							// card number of System MIDI database
	LocalID		dbID;								// Local ID of System MIDI database
	DmOpenRef	dbP = NULL;						// reference to open database
	UInt16		recIndex;						// record index of the MIDI record to play
	SndSmfOptionsType	smfOpt;					// SMF play options
	DmSearchStateType	searchState;			// search state for finding the System MIDI database
	Boolean		bError = false;				// set to true if we couldn't find the MIDI record
	
		
	// Find the system MIDI database
	err = DmGetNextDatabaseByTypeCreator(true, &searchState,
			 		sysFileTMidi, 0, true, 
			 		&cardNo, &dbID);
	while(true) {
		if (err) return;
		dbP = DmOpenDatabase (cardNo, dbID, dmModeReadOnly);
		if (!dbP) return;
		err = DmFindRecordByID (dbP, uniqueRecID, &recIndex);
		if (err) {
			DmCloseDatabase(dbP);
			err = DmGetNextDatabaseByTypeCreator(false, &searchState,
			 		sysFileTMidi, 0, true, 
			 		&cardNo, &dbID);
			continue;
		}

		// Find the record handle and lock the record
		midiH = DmQueryRecord(dbP, recIndex);
		midiHdrP = (SndMidiRecHdrType*) MemHandleLock(midiH);
		
		// Get a pointer to the SMF stream
		midiStreamP = (UInt8*)midiHdrP + midiHdrP->bDataOffset;
		
		// Play the sound (ignore the error code)
		// The sound can be interrupted by a key/digitizer event
		smfOpt.dwStartMilliSec = 0;
		smfOpt.dwEndMilliSec = sndSmfPlayAllMilliSec;
		smfOpt.amplitude = PrefGetPreference(prefAlarmSoundVolume);
		smfOpt.interruptible = true;
		smfOpt.reserved = 0;
		err = SndPlaySmf (NULL, sndSmfCmdPlay, midiStreamP, &smfOpt, NULL, NULL, false);
		
		// Unlock the record
		MemPtrUnlock (midiHdrP);
		// Close the MIDI database
		if ( dbP )
			DmCloseDatabase (dbP);
			
		return;
	}
	
	if ( dbP )
		DmCloseDatabase (dbP);
}

/***********************************************************************
 *
 * FUNCTION:    MidiPickListDrawItem
 *
 * DESCRIPTION: Draw a midi list item.
 *
 * PARAMETERS:  itemNum - which shortcut to draw
 *					 bounds - bounds in which to draw the text
 *					 unusedP - pointer to data (not used)
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vnk		8/8/97	Initial version
 *			trev		8/14/97	Ported to dateBook App
 *			frigino	8/18/97	Modified to truncate items in list with ...
 *
 ***********************************************************************/
static void MidiPickListDrawItem (Int16 itemNum, RectanglePtr bounds, 
	Char **unusedP)
{
#pragma unused (unusedP)

	Char *	itemTextP;
	Int16		itemTextLen;
	Int16		itemWidth;
	SndMidiListItemType*	listP;
	
	ErrNonFatalDisplayIf(itemNum >= gMidiCount, "index out of bounds");
	
	// Bail out if MIDI sound list is empty
	if (gMidiListH == NULL)
		return;
	
	listP = (SndMidiListItemType *) MemHandleLock(gMidiListH);

	itemTextP = listP[itemNum].name;

	// Truncate the item with an ellipsis if it doesnt fit in the list width.
	// Get the item text length
	itemTextLen = StrLen(itemTextP);
	// Get the width of the text
	itemWidth = FntCharsWidth(itemTextP, itemTextLen);
	// Does it fit?
	if (itemWidth <= bounds->extent.x)
		{
		// Draw entire item text as is
		WinDrawChars(itemTextP, itemTextLen, bounds->topLeft.x, bounds->topLeft.y);
		}
	else
		{
		// We're going to truncate the item text
		Boolean	ignored;
		char		ellipsisChar = chrEllipsis;
		// Set the new max item width
		itemWidth = bounds->extent.x - FntCharWidth(ellipsisChar);
		// Find the item length that fits in the bounds minus the ellipsis
		FntCharsInWidth(itemTextP, &itemWidth, &itemTextLen, &ignored);
		// Draw item text that fits
		WinDrawChars(itemTextP, itemTextLen, bounds->topLeft.x, bounds->topLeft.y);
		// Draw ellipsis char
		WinDrawChars(&ellipsisChar, 1, bounds->topLeft.x + itemWidth, bounds->topLeft.y);
		}

	// Unlock list items
	MemPtrUnlock(listP);
}


/***********************************************************************
 *
 * FUNCTION:    CreateMidiPickList
 *
 * DESCRIPTION: Create a list of midi sounds available.
 *
 * PARAMETERS:	formP	-- the form that owns the list to contain the panel list
 *					objIndex -- the index of the list within the form
 *					funcP	-- item draw function
 *
 * RETURNED:    panelCount and panelIDsP are set
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			vmk		8/8/97	Initial version
 *			trev		8/14/97	Ported to dateBook App
 *			frigino	8/20/97	Added maximum widening of MIDI sound list
 *
 ***********************************************************************/
static void CreateMidiPickList(FormType *formP, UInt16 objIndex, ListDrawDataFuncPtr funcP)
{
	SndMidiListItemType*	midiListP;
	UInt16		i;
	UInt16		listWidth;
	UInt16		maxListWidth;
	Boolean	bSuccess;
	RectangleType formBounds, listBounds;
	ListPtr		listP;
	
	if (FormIsNot(formP, FormPreference)) return;
		
	// Load list of midi record entries
	bSuccess = SndCreateMidiList(0, true, &gMidiCount, &gMidiListH);
	if ( !bSuccess )
		{
		gMidiListH = 0;
		gMidiCount = 0;
		return;
		}
	
	listP = (ListPtr) FrmGetObjectPtr(formP, objIndex);
		
	// Now set the list to hold the number of sounds found.  There
	// is no array of text to use.
	LstSetListChoices(listP, NULL, gMidiCount);
	
	// Now resize the list to the number of panel found
	LstSetHeight (listP, gMidiCount);

	// Because there is no array of text to use, we need a function
	// to interpret the panelIDsP list and draw the list items.
	LstSetDrawFunction(listP, funcP);

	// Make the list as wide as possible to display the full sound names
	// when it is popped winUp.

	// Lock MIDI sound list
	midiListP = (SndMidiListItemType *) MemHandleLock(gMidiListH);
	// Initialize max width
	maxListWidth = 0;
	// Iterate through each item and get its width
	for (i = 0; i < gMidiCount; i++)
		{
			// Get the width of this item
			listWidth = FntCharsWidth(midiListP[i].name, StrLen(midiListP[i].name));
			// If item width is greater that max, swap it
			if (listWidth > maxListWidth)
				{
				maxListWidth = listWidth;
				}
		}
	// Unlock MIDI sound list
	MemPtrUnlock(midiListP);

	// Set list width to max width + left margin
	FrmGetObjectBounds(formP, objIndex, &listBounds);
	listBounds.extent.x = maxListWidth + 2;

	// Get pref dialog window extent
	FrmGetFormBounds(FrmGetActiveForm(), &formBounds);
	// Make sure width is not more than window extent
	if (listBounds.extent.x > formBounds.extent.x)
		{
		listBounds.extent.x = formBounds.extent.x;
		}
	// Move list left if it doesnt fit in window
	if (listBounds.topLeft.x + listBounds.extent.x > formBounds.extent.x)
		{
		listBounds.topLeft.x = formBounds.extent.x - listBounds.extent.x;
		}
		
	FrmSetObjectBounds(formP, objIndex, &listBounds);
}

/***********************************************************************
 *
 * FUNCTION:    FreeMidiPickList
 *
 * DESCRIPTION: Free the list of midi sounds available.
 *
 * PARAMETERS:	none
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk	8/11/97	Initial version
 *			trev	08/14/97	Ported to dateBook App
 *
 ***********************************************************************/
static void FreeMidiPickList(void)
{
	if ( gMidiListH )
		{
		MemHandleFree(gMidiListH);
		gMidiListH = 0;
		gMidiCount = 0;
		}
}

/***********************************************************************
 *
 * FUNCTION:    SetSoundLabel
 *
 * DESCRIPTION: Sets the sound trigger label, using truncation
 *
 * PARAMETERS:  formP - the form ptr
 *              labelP - ptr to original label text
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			frigino	8/18/97	Initial Revision
 *
 ***********************************************************************/

static void SetSoundLabel(FormPtr formP, const char* labelP)
{
	ControlPtr	triggerP;
	UInt16			triggerIdx;

	// Copy the label, winUp to the max into the ptr
	StrNCopy(soundTriggerLabelP, labelP, soundTriggerLabelLen);
	// Terminate string at max len
	soundTriggerLabelP[soundTriggerLabelLen - 1] = '\0';
	// Get trigger idx
	triggerIdx = FrmGetObjectIndex(formP, SoundTrigger);
	// Get trigger control ptr
	triggerP = (ControlPtr) FrmGetObjectPtr(formP, triggerIdx);
	// Use category routines to truncate it
	CategoryTruncateName(soundTriggerLabelP, 90);
	// Set the label
	CtlSetLabel(triggerP, soundTriggerLabelP);
}

void ReadSendPreference(SendPref& pref)
{
	UInt16 prefSize = sizeof(SendPref);
	PrefGetAppPreferences(appFileCreator, PREF_SEND, &pref, &prefSize, true);
}

void ReadReminderPreference(ReminderPref& pref)
{
	UInt16 prefSize = sizeof(ReminderPref);
	PrefGetAppPreferences(appFileCreator, PREF_REMINDER, &pref, &prefSize, true);
}

static void WriteReminderPreference(const ReminderPref& pref)
{
	UInt16 prefSize = sizeof(ReminderPref);
	PrefSetAppPreferences(appFileCreator, PREF_REMINDER, 0, &pref, prefSize, true);
}

static void WriteSendPreference(const SendPref& pref)
{
	UInt16 prefSize = sizeof(SendPref);
	PrefSetAppPreferences(appFileCreator, PREF_SEND, 0, &pref, prefSize, true);
}

static void UpdateSoundTrigger(UInt32 sndId)
{
	FormType* frmP = FrmGetActiveForm();
	if (FormIsNot(frmP, FormPreference)) return;

	ListPtr listP = (ListPtr) GetObjectPtr(frmP, SoundList);

	UInt16 item = 0;
	// Lock MIDI sound list
	if ( gMidiListH ) {
		SndMidiListItemType* midiListP = (SndMidiListItemType*) MemHandleLock(gMidiListH);

		// Iterate through each item and get its unique ID
		for (UInt16 i = 0; i < gMidiCount; i++) {
			if (midiListP[i].uniqueRecID == sndId) {
				item = i;
				break;		// exit for loop
			}
		}
		
		// Set the list selection
		LstSetSelection (listP, item);
		SetSoundLabel(frmP, midiListP[item].name);

		MemPtrUnlock(midiListP);
	}

}

static void InitPref()
{
	FormType* frmP = FrmGetActiveForm();
	if (FormIsNot(frmP, FormPreference)) return;

	ReminderPref rpref;
	ReadReminderPreference(rpref);
	
	CtlSetValue((ControlPtr) GetObjectPtr(frmP, CheckboxUseReminder), rpref.useReminder);
	CtlSetValue((ControlPtr) GetObjectPtr(frmP, CheckboxVibration), rpref.vibration);
	CtlSetValue((ControlPtr) GetObjectPtr(frmP, CheckboxPlaySound), rpref.playSound);
	
	UpdateSoundTrigger(rpref.soundId);

	if (rpref.playSound) {
		FrmShowObject(frmP, FrmGetObjectIndex(frmP, SoundTrigger));
	} else {
		FrmHideObject(frmP, FrmGetObjectIndex(frmP, SoundTrigger));
	}
	
	SendPref spref;
	ReadSendPreference(spref);

	CtlSetValue((ControlPtr) GetObjectPtr(frmP, CheckboxRequestReport), spref.requestReport);
	CtlSetValue((ControlPtr) GetObjectPtr(frmP, CheckboxUseFingerAddress), spref.useFingerAddress);
}

static void SavePref()
{
	FormType* frmP = FrmGetActiveForm();
	if (FormIsNot(frmP, FormPreference)) return;

	ReminderPref rpref;
	
	rpref.useReminder = CtlGetValue((ControlPtr) GetObjectPtr(frmP, CheckboxUseReminder));
	rpref.vibration = CtlGetValue((ControlPtr) GetObjectPtr(frmP, CheckboxVibration));
	rpref.playSound = CtlGetValue((ControlPtr) GetObjectPtr(frmP, CheckboxPlaySound));

	if ( gMidiListH ) {
		SndMidiListItemType* midiListP = (SndMidiListItemType*) MemHandleLock(gMidiListH);
		ListPtr listP = (ListPtr) GetObjectPtr(frmP, SoundList);
		Int16 item = LstGetSelection(listP);
		rpref.soundId = midiListP[item].uniqueRecID;
		MemPtrUnlock(midiListP);
	}
	
	WriteReminderPreference(rpref);
	
	SendPref spref;
	spref.requestReport = CtlGetValue((ControlPtr) GetObjectPtr(frmP, CheckboxRequestReport));
	spref.useFingerAddress = CtlGetValue((ControlPtr) GetObjectPtr(frmP, CheckboxUseFingerAddress));
	WriteSendPreference(spref);
}

Boolean ReminderPrefFormHandleEvent(EventType* eventP)
{
	Boolean handled = false;
	FormType * frmP;

	switch (eventP->eType) 
	{
		case frmOpenEvent:
			frmP = FrmGetActiveForm();
			CreateMidiPickList(frmP, FrmGetObjectIndex(frmP, SoundList), MidiPickListDrawItem);
			InitPref();
			FrmDrawForm(frmP);
			handled = true;
			break;
		case ctlSelectEvent:
			switch(eventP->data.ctlSelect.controlID) {
				case CheckboxPlaySound:
					frmP = FrmGetActiveForm();
					if (FormIsNot(frmP, FormPreference)) break;
					Boolean checked = CtlGetValue(eventP->data.ctlSelect.pControl);
					if (checked) {
						FrmShowObject(frmP, FrmGetObjectIndex(frmP, SoundTrigger));
					} else {
						FrmHideObject(frmP, FrmGetObjectIndex(frmP, SoundTrigger));
					}
					break;
				case ButtonOk:
					FrmGotoForm(FormMain);
					break;
			}
			break;
		case frmCloseEvent:
			SavePref();
			FreeMidiPickList();
			break;
		case popSelectEvent:
			switch (eventP->data.popSelect.listID) {
				case SoundList:
					// Get new selected item
					UInt16 item = eventP->data.popSelect.selection;
					// Get active form
					frmP = FrmGetActiveForm ();
					if (FormIsNot(frmP, FormPreference)) break;
					// Lock MIDI list
					SndMidiListItemType* listP = (SndMidiListItemType *) MemHandleLock(gMidiListH);
					// Save alarm sound unique rec ID
					UInt32 sndId = listP[item].uniqueRecID;
					// Erase control
					CtlEraseControl (eventP->data.popSelect.controlP);
					// Set new trigger label
					SetSoundLabel(frmP, listP[item].name);
					// Redraw control
					CtlDrawControl (eventP->data.popSelect.controlP);
					// Unlock MIDI list
					MemPtrUnlock(listP);
					// Play new alarm sound
					PlayAlarmSound (sndId);
					// Mark event as handled
					handled = true;
					break;
			}
	}
	return handled;
}