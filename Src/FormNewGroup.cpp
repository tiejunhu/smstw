#include "FormNewGroup.h"
#include "SMSTW_Rsc.h"
#include "Global.h"
#include "GroupDb.h"

static PhoneGroupPtr g_PhoneGroupToEdit = NULL;

enum FormMode {
	FM_NEW, FM_EDIT
};

FormMode g_FormMode = FM_NEW;

static void ListDrawDataFunc(Int16 itemNum, RectangleType *bounds, Char **itemsText)
{
	char string[50];

	if (itemNum < g_PhoneGroupToEdit->GetPhoneGroupItemCount()) {
		PhoneGroupItemPtr item = g_PhoneGroupToEdit->GetPhoneGroupItem(itemNum);
		StrPrintF(string, "%-10s   %-20s", item->GetDisplayName(), item->GetPhone());
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
	if (FormIsNot(form, FormNewGroup)) return;
	ListPtr list = (ListPtr) GetObjectPtr(form, ListGroupItems);
	LstSetListChoices(list, NULL, g_PhoneGroupToEdit->GetPhoneGroupItemCount());
	LstSetDrawFunction(list, ListDrawDataFunc);
	LstDrawList(list);
}

static void AppendAddress()
{
	FormPtr frmP = FrmGetActiveForm();
	if (FormIsNot(frmP, FormNewGroup)) return;

	FieldPtr fieldHidden = (FieldPtr) GetObjectPtr(frmP, FieldHidden);
	if (FldGetTextPtr(fieldHidden)) {
		FldSetSelection(fieldHidden, 0, StrLen(FldGetTextPtr(fieldHidden)));
	}
	FldSetDirty(fieldHidden, false);
	
	AddrLookupParamsType params;
	MemSet(&params, sizeof(AddrLookupParamsType), 0);
	params.formatStringP = "^name$$$$^first$$$$^mobile";
	params.field1 = addrLookupSortField;
	params.field2 = addrLookupMobile;
	params.field2Optional = false;
	params.userShouldInteract = true;

	PhoneNumberLookupCustom (fieldHidden, &params, false);
	
	if (!FldDirty(fieldHidden)) return;
	
	char* addrstr = FldGetTextPtr(fieldHidden);
	if (addrstr == NULL) return;
	
	char* tmpstr = NULL;
	AssignStr(&tmpstr, addrstr, 0);
	
	char* p = StrStr(tmpstr, "$$$$");
	if (p == NULL) goto error;
	
	char* pszName = tmpstr;
	p[0] = 0;
	
	p += 4;
	char* pszFirst = p;
	p = StrStr(p, "$$$$");
	if (p == NULL) goto error;
	p[0] = 0;
	
	if (StrLen(pszFirst) > 0) {
		StrCat(pszName, ",");
		StrCat(pszName, pszFirst);
	}
	

	p += 4;
	char* pszPhone = p;
	
	PhoneGroupItemPtr item = new PhoneGroupItem();
	item->SetDisplayName(pszName);
	item->SetPhone(pszPhone);
	
	g_PhoneGroupToEdit->AddPhoneGroupItem(item);
	
	DrawList();
	
error:
	if (tmpstr) MemPtrFree(tmpstr);
}

static void OnOk()
{
	FormPtr form = FrmGetActiveForm();
	if (FormIsNot(form, FormNewGroup)) return;
	
	FieldPtr fieldGroupName = (FieldPtr) GetObjectPtr(form, FieldGroupName);
	
	char* pszGroupName = FldGetTextPtr(fieldGroupName);
	
	if ((pszGroupName == NULL) || (StrLen(pszGroupName) == 0)) {
		ShowMsg("Please input group name.");
		return;
	}
	
	if (g_PhoneGroupToEdit->GetPhoneGroupItemCount() == 0) {
		ShowMsg("No phone number added.");
		return;
	}

	g_PhoneGroupToEdit->SetGroupName(pszGroupName);	
	
	if (g_FormMode == FM_NEW)
		AddPhoneGroup(g_PhoneGroupToEdit);
	else if (g_FormMode == FM_EDIT)
		ModifyPhoneGroup(g_PhoneGroupToEdit);
}

static void InitNewForm(FormPtr frm)
{
	FrmSetTitle(frm , "New Group");
}

static void InitEditForm(FormPtr frm)
{
	FrmSetTitle(frm , "Edit Group");
	SetFieldText((FieldPtr) GetObjectPtr(frm, FieldGroupName), g_PhoneGroupToEdit->GetGroupName());
	FldDrawField((FieldPtr) GetObjectPtr(frm, FieldGroupName));
	DrawList();
}

static void RemoveCurrentSelection()
{
	FormPtr form = FrmGetActiveForm();
	if (FormIsNot(form, FormNewGroup)) return;
	ListPtr list = (ListPtr) GetObjectPtr(form, ListGroupItems);
	Int16 sel = LstGetSelection(list);

	if (sel == noListSelection) {
		ShowMsg("Select a address first.");
		return;
	}

	g_PhoneGroupToEdit->RemovePhoneGroupItem(sel);
	
	DrawList();
}

Boolean FormNewGroupHandleEvent(EventType* eventP)
{
	Boolean handled = false;
	
	switch (eventP->eType) 
	{
		case frmOpenEvent:
			FormPtr frmP = FrmGetActiveForm();
			FrmDrawForm(frmP);

			if (g_FormMode == FM_NEW)
				InitNewForm(frmP);
			else if (g_FormMode == FM_EDIT)
				InitEditForm(frmP);
			handled = true;
			break;
		case frmCloseEvent:
			delete g_PhoneGroupToEdit;
			g_PhoneGroupToEdit = NULL;
			break;
		case ctlSelectEvent:
			switch(eventP->data.ctlSelect.controlID) {
				case ButtonOk:
					OnOk();
					FrmGotoForm(FormGroupManagement);
					break;
				case ButtonAppend:
					AppendAddress();
					break;
				case ButtonRemove:
					RemoveCurrentSelection();				
					break;
			}
			break;
	}
	return handled;
}

void NewGroup()
{
	g_FormMode = FM_NEW;
	g_PhoneGroupToEdit = new PhoneGroup(0);
	FrmGotoForm(FormNewGroup);
}

void EditGroup(PhoneGroupPtr group)
{
	g_FormMode = FM_EDIT;
	g_PhoneGroupToEdit = LoadPhoneGroupByUniqId(group->GetUniqId());
	FrmGotoForm(FormNewGroup);
}
