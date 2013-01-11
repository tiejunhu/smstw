#ifndef _SMSTW_REMINDER_PREF
#define _SMSTW_REMINDER_PREF

struct ReminderPref {
	Boolean useReminder;
	Boolean vibration;
	Boolean playSound;
	UInt32 soundId;
	
	ReminderPref()
	{
		useReminder = true;
		vibration = true;
		playSound = false;
		soundId = 0;
	}
};

struct SendPref {
	Boolean requestReport;
	Boolean useFingerAddress;
	
	SendPref()
	{
		requestReport = false;
		useFingerAddress = false;
	}
};

void PlayAlarmSound(UInt32 uniqueRecID);
void ReadReminderPreference(ReminderPref& pref);
void ReadSendPreference(SendPref& pref);
Boolean ReminderPrefFormHandleEvent(EventType* eventP);

#endif