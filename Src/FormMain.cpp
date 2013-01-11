#include "FormMain.h"
#include "SMSTW_Rsc.h"
#include "Global.h"
#include "ImportSMS.h"
#include "SMSDb.h"
#include "SMSTable.h"
#include "SMSSend.h"
#include "SMSNew.h"

#include <PalmNavigator.h>

Boolean MainFormDoCommand(UInt16 command)
{
	Boolean handled = false;

	switch (command)
	{
		case MenuItemImport:
			if (FrmCustomAlert(AlertQuestion, "Do you really wanna import all ?", "", "") == 1) break;
			ImportSMS(g_SmsDb, g_gbTable, 0, -1);
			SortSMS(g_SmsDb);
			DrawTable(g_SmsDb, -1);
			break;
		case MenuItemPurgeAll:
			if (FrmCustomAlert(AlertQuestion, "Do you really wanna delete all in this folder?", "", "") == 1) break;
			PurgeCategory(g_SmsDb, g_SelectedCategory);
			FirstPage(g_SmsDb);
			break;
		case MenuItemSendAll:
			SendAllInOutbox(g_SmsDb, g_gbTable, "+8613800100500");
			RefreshPage(g_SmsDb);
			break;
		case MenuItemNewSMS:
			NewSMS();
			break;
		case MenuItemPreference:
			FrmGotoForm(FormPreference);
			break;
		case MenuItemDeleteAll:
			if (FrmCustomAlert(AlertQuestion, "Do you really wanna delete ALL SMS ?", "", "") == 1) break;
			PurgeCategory(g_SmsDb, -1);
			FirstPage(g_SmsDb);
			break;
		case MenuItemAbout:
			FormPtr about = FrmInitForm(FormAbout);
			FrmDoDialog(about);
			FrmDeleteForm(about);
			break;
		case MenuItemGroupManagement:
			FrmGotoForm(FormGroupManagement);
			break;
		case MenuItemChangeFolder:
			FormType* frmP = FrmGetActiveForm();
			CtlHitControl((ControlPtr) GetObjectPtr(frmP, TriggerFolder));
			break;
	}

	return handled;
}

Boolean FormMainHandleEvent(EventType * eventP)
{
	Boolean handled = false;
	FormType * frmP;

	switch (eventP->eType) 
	{
		case menuEvent:
			return MainFormDoCommand(eventP->data.menu.itemID);

		case frmOpenEvent:
		{
			frmP = FrmGetActiveForm();
			FrmDrawForm(frmP);
			CategoryGetName(g_SmsDb, g_SelectedCategory, (Char *)CtlGetLabel ((ControlPtr) GetObjectPtr (frmP, TriggerFolder)));
			CtlDrawControl((ControlPtr) GetObjectPtr (frmP, TriggerFolder));
			DrawTable(g_SmsDb, g_SelectedCategory);
			handled = true;
			break;
		}
			
		case tblSelectEvent:
			SelectRow(g_SmsDb, eventP->data.tblSelect.row);
			g_DisplayUniqId = -1;
			FrmGotoForm(FormDetail);
			handled = true;
			break;
            
   		case sclRepeatEvent:
   			GotoPage(g_SmsDb, eventP->data.sclExit.newValue);
   			break;

		case ctlSelectEvent:
		{
			switch(eventP->data.ctlSelect.controlID) {
				case TriggerFolder:
				{
					char* str;
					frmP = FrmGetActiveForm();
					str = (Char *)CtlGetLabel ((ControlPtr) GetObjectPtr (frmP, TriggerFolder));	
					CategorySelect(g_SmsDb, frmP, TriggerFolder, ListFolder, false, 
						&g_SelectedCategory, str, 6, categoryHideEditCategory);
					GotoPageOf(g_SmsDb, 0, g_SelectedCategory);
					handled = true;
					break;
				}
				case ButtonNew:
					NewSMS();
					break;
			}
			break;
		}
		case keyDownEvent:
			WChar 		chr			= eventP->data.keyDown.chr;
			UInt16 		keyCode		= eventP->data.keyDown.keyCode;
			switch(chr)
			{
				case vchrPageUp:
					SelectNextRow(g_SmsDb, -1);
					handled = true;
					break;
				case vchrPageDown:
					SelectNextRow(g_SmsDb, 1);
					handled = true;
					break;
				case vchrNavChange:
					if(keyCode & navBitRight) {
						NextPage(g_SmsDb);
					} else if(keyCode & navBitLeft)
						PrevPage(g_SmsDb);
					else if(keyCode & navBitSelect) {
						g_DisplayUniqId = -1;
						FrmGotoForm(FormDetail);
					}
					break;
			}
			
			break;
	}
    
	return handled;
}
