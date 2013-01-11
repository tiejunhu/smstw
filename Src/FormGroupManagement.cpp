#include "FormGroupManagement.h"
#include "SMSTW_Rsc.h"
#include "GroupDb.h"
#include "Global.h"
#include "FormNewGroup.h"

static PhoneGroupVector g_PhoneGroups;

static void ListDrawDataFunc(Int16 itemNum, RectangleType *bounds, Char **itemsText)
{
	char string[50];
	
	if (itemNum < g_PhoneGroups.size()) {
		PhoneGroupPtr group = g_PhoneGroups[itemNum];
		StrNCopy(string, group->GetGroupName(), 49);
	} else {
		StrCopy(string, "");
	}

	WinSetUnderlineMode(noUnderline);
	FntSetFont(stdFont);
	Int16 width = bounds->extent.x;
	Int16 len = StrLen(string);
	Boolean noFit = false;
	FntCharsInWidth(string, &width, &len, &noFit);

	WinEraseRectangle(bounds, 0);
	WinDrawChars(string, len, bounds->topLeft.x, bounds->topLeft.y);
}

static void DrawList()
{
	FormPtr form = FrmGetActiveForm();
	if (FormIsNot(form, FormGroupManagement)) return;
	ListPtr list = (ListPtr) GetObjectPtr(form, ListGroups);
	LstSetListChoices(list, NULL, g_PhoneGroups.size());
	LstSetDrawFunction(list, ListDrawDataFunc);
	LstDrawList(list);
}

static void ListAllGroups()
{
	LoadPhoneGroups(g_PhoneGroups);
	DrawList();	
}

static void RemoveCurrentSelection()
{
	FormPtr form = FrmGetActiveForm();
	if (FormIsNot(form, FormGroupManagement)) return;
	ListPtr list = (ListPtr) GetObjectPtr(form, ListGroups);
	Int16 sel = LstGetSelection(list);
	if (sel == noListSelection) {
		ShowMsg("Select a group first.");
		return;
	}

	if (FrmCustomAlert(AlertQuestion, "Do you really wanna remove current selection ?", "", "") == 1) return;
	
	RemovePhoneGroup(g_PhoneGroups[sel]);
	PhoneGroupPtr group = *(g_PhoneGroups.begin() + sel);
	g_PhoneGroups.erase(g_PhoneGroups.begin() + sel);
	delete group;
	DrawList();
}

static void EditCurrentSelection()
{
	FormPtr form = FrmGetActiveForm();
	if (FormIsNot(form, FormGroupManagement)) return;
	ListPtr list = (ListPtr) GetObjectPtr(form, ListGroups);
	Int16 sel = LstGetSelection(list);
	if (sel == noListSelection) {
		ShowMsg("Select a group first.");
		return;
	}
	
	EditGroup(g_PhoneGroups[sel]);
}

Boolean FormGroupManagementHandleEvent(EventType* eventP)
{
	Boolean handled = false;
	
	switch (eventP->eType) 
	{
		case frmOpenEvent:
			FormPtr frmP = FrmGetActiveForm();
			FrmDrawForm(frmP);
			ListAllGroups();
			handled = true;
			break;
		case frmCloseEvent:
			ClearPhoneGroups(g_PhoneGroups);
			break;
		case ctlSelectEvent:
			switch(eventP->data.ctlSelect.controlID) {
				case ButtonOk:
					FrmGotoForm(FormMain);
					break;
				case ButtonNew:
					NewGroup();
					break;
				case ButtonRemove:
					RemoveCurrentSelection();
					break;
				case ButtonModify:
					EditCurrentSelection();
					break;
			}
			break;
	}
	return handled;
}
