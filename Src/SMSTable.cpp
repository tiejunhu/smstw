#include "SMSTable.h"
#include "SMSTools.h"
#include "SMSDb.h"
#include "SMSTW_Rsc.h"
#include "Global.h"

static Int16 g_CurrentPage = 0;
static Int16 g_CurrentSelection = 0;

static Int16 g_CurrentRow = -1;
static MemHandle g_CurrentMemHandle = NULL;
static SMSDbRecordType g_CurrentRecord;

#define TABLE_NUM_COLUMNS 4
#define TABLE_PAGE_SIZE 11

static const int COLUMN_WIDTHS[] = {
	10, 44, 61, 35
};

void PrivDrawCell(void* table, Int16 row, Int16 column, RectanglePtr bounds)
{
	char string[50];
	FontID font = stdFont;

	if (g_CurrentRow != row) {
		if (g_CurrentMemHandle) {
			MemHandleUnlock(g_CurrentMemHandle);
			g_CurrentMemHandle = NULL;
		}
	}
	if (g_CurrentMemHandle == NULL) {
		Err err = GetSMSRecord(g_SmsDb, g_SelectedCategory, row + g_CurrentPage * TABLE_PAGE_SIZE, 
			g_CurrentRecord, g_CurrentMemHandle, true);
		if (err) return;
		g_CurrentRow = row;
	}
		
	switch(column) {
		case 0:
			StrCopy(string, "");
			if (g_SelectedCategory == CAT_INBOX) {
				if (IsRecordRead(&g_CurrentRecord)) {
					StrCopy(string, "");
				} else {
					font = symbolFont;
					StrCopy(string, "\020");
				}
			} else if (g_SelectedCategory == CAT_SENT) {
				if (IsRecordRequestReport(&g_CurrentRecord)) {
					font = symbolFont;
					if (IsRecordDelivered(&g_CurrentRecord)) {
						StrCopy(string, "\026");
					} else {
						StrCopy(string, "\024");
					}
				}
			}
			break;
		case 1:
			StrNCopy(string, g_CurrentRecord.personName, 48);
			if (StrLen(string) == 0) {
				StrNCopy(string, g_CurrentRecord.phoneNumber, 48);
			}
			break;
		case 2:
			StrNCopy(string, g_CurrentRecord.content, 48);
			break;
		case 3:
			{
				DateTimeType smsDatetime;
				TimSecondsToDateTime(g_CurrentRecord.time, &smsDatetime);

				UInt32 nowSeconds = TimGetSeconds();
				DateTimeType nowDatetime;
				TimSecondsToDateTime(nowSeconds, &nowDatetime);
				
				if ((nowDatetime.year == smsDatetime.year) && 
					(nowDatetime.month == smsDatetime.month) &&
					(nowDatetime.day == smsDatetime.day)) {
					TimeToAscii(smsDatetime.hour, smsDatetime.minute, tfColon24h, string);
				} else {
					StrCopy(string, DayOfWeekInChinese[DayOfWeek(smsDatetime.month, smsDatetime.day, smsDatetime.year)]);
					DateTemplateToAscii(" ^3z-^0z", smsDatetime.month, smsDatetime.day, smsDatetime.year, string + 2, 47);
				}
				MemHandleUnlock(g_CurrentMemHandle);
				g_CurrentMemHandle = NULL;
			}
			break;
		default:
			StrCopy(string, "");
	}

	WinPushDrawState();
	
	RGBColorType foreColor, backColor;

	foreColor.index = 0;
	backColor.index = 0;
	if ((row == g_CurrentSelection) && (column != 0)) {
		foreColor.r = 255;
		foreColor.g = 255;
		foreColor.b = 255;
		
		backColor.r = 10;
		backColor.g = 36;
		backColor.b = 106;
	} else if (row % 2 == 0) {
		backColor.r = 255;
		backColor.g = 255;
		backColor.b = 255;
		
		foreColor.r = 0;
		foreColor.g = 0;
		foreColor.b = 0;
	} else {
		backColor.r = 220;
		backColor.g = 220;
		backColor.b = 220;
		
		foreColor.r = 0;
		foreColor.g = 0;
		foreColor.b = 0;
	}
	WinSetForeColorRGB(&foreColor, NULL);
	WinSetTextColorRGB(&foreColor, NULL);
	WinSetBackColorRGB(&backColor, NULL);
	
	WinSetUnderlineMode(noUnderline);
		
	FntSetFont(font);

	Int16 width = bounds->extent.x - 2;
	Int16 len = StrLen(string);
	Boolean noFit = false;
	FntCharsInWidth(string, &width, &len, &noFit);
	
	UInt16 x = bounds->topLeft.x;
	UInt16 y = bounds->topLeft.y;
	
	bounds->topLeft.x += - 1;
	bounds->topLeft.y += 0;
	bounds->extent.x += 2;
	bounds->extent.y += 0;
	
	WinEraseRectangle(bounds, 0);
	WinDrawGrayLine(
		bounds->topLeft.x + bounds->extent.x - 2, 
		bounds->topLeft.y,
		bounds->topLeft.x + bounds->extent.x - 2, 
		bounds->topLeft.y + bounds->extent.y);// - 2);
		
	WinDrawChars(string, len, x, y);
	
	WinPopDrawState();
}

static void UpdateUnread(DmOpenRef db)
{
	FormPtr form = FrmGetActiveForm();
	if (FormIsNot(form, FormMain)) return;

	char unreadbuf[20];
	StrPrintF(unreadbuf, "%d unread", (UInt16) CountUnread(db));
	FrmCopyLabel(form, LabelUnread, unreadbuf);
}

static void UpdatePos(DmOpenRef db)
{
	FormPtr form = FrmGetActiveForm();
	if (FormIsNot(form, FormMain)) return;
	char posbuf[20];
	StrPrintF(posbuf, "%d/%d", GetCurrentIndex() + 1,
		(UInt16) GetSMSCount(db, g_SelectedCategory));
	FrmCopyLabel(form, LabelPosition, posbuf);
}

void DrawTable(DmOpenRef db, Int16 category)
{
	FormPtr form = FrmGetActiveForm();
	if (FormIsNot(form, FormMain)) return;
	TablePtr table = (TablePtr) GetObjectPtr(form, SMSTable);
	if (table == NULL) return;
	if (category >= 0) g_SelectedCategory = category;
	
	for (int c = 0; c < TABLE_NUM_COLUMNS; ++c) {
		TblSetCustomDrawProcedure(table, c, PrivDrawCell);
		TblSetColumnUsable(table, c, true);
		TblSetColumnWidth(table, c, COLUMN_WIDTHS[c]);
	}
	
	
	for (int r = 0; r < TABLE_PAGE_SIZE; ++r) {
		UInt32 cursor = g_CurrentPage * TABLE_PAGE_SIZE + r;
		if (cursor >= GetSMSCount(db, g_SelectedCategory)) {
			TblSetRowUsable(table, r, false);
		} else {
			TblSetRowSelectable(table, r, true);
			TblSetRowUsable(table, r, true);
			for(int c = 0; c < TABLE_NUM_COLUMNS; c++ )
				TblSetItemStyle(table, r, c, customTableItem);
		}
	}
	
	TblDrawTable(table);
	UpdateScrollbar(db);
	
	UpdateUnread(db);	
	UpdatePos(db);
}

void FirstPage(DmOpenRef db)
{
	GotoPage(db, 0);
}

void NextPage(DmOpenRef db)
{
	GotoPage(db, g_CurrentPage + 1);
}

void PrevPage(DmOpenRef db)
{
	GotoPage(db, g_CurrentPage - 1);
}

void GotoPage(DmOpenRef db, Int16 page)
{
	UInt16 pageCount = PrivPageCount(db);

	g_CurrentPage = page;
	if (g_CurrentPage < 0) g_CurrentPage = 0;
	if (g_CurrentPage > pageCount) g_CurrentPage = pageCount;
	UInt16 lastPageSize = GetSMSCount(db, g_SelectedCategory) - pageCount * TABLE_PAGE_SIZE;
	if (g_CurrentPage == pageCount) {
		if (g_CurrentSelection >= lastPageSize) {
			g_CurrentSelection = lastPageSize - 1;
		}
	}
	DrawTable(db, g_SelectedCategory);
}

void RefreshPage(DmOpenRef db)
{
	GotoPage(db, g_CurrentPage);
}

void GotoPageOf(DmOpenRef db, Int16 page, Int16 category)
{
	g_SelectedCategory = category;
	GotoPage(db, page);
}

UInt32 PrivPageCount(DmOpenRef db)
{
	UInt32 count = GetSMSCount(db, g_SelectedCategory);
	if (count == 0) return 0;
	return (count - 1) / TABLE_PAGE_SIZE;
}

void SelectRow(DmOpenRef db, Int16 row)
{
	FormPtr form = FrmGetActiveForm();
	if (FormIsNot(form, FormMain)) return;

	TablePtr table = (TablePtr) GetObjectPtr(form, SMSTable);
	g_CurrentSelection = row;
	TblMarkRowInvalid(table, row);
	TblRedrawTable(table);
	UpdatePos(db);
}



Err DisplayCurrentRecord(DmOpenRef db)
{
	return DisplayRecord(db, g_SelectedCategory, GetCurrentIndex());
}

void SetCurrentRecordCategory(DmOpenRef db, Int16 category)
{
	MoveRecordCategory(db, GetCurrentIndex(), g_SelectedCategory, category);
}

void UpdateScrollbar(DmOpenRef db)
{
	FormPtr form = FrmGetActiveForm();
	if (FormIsNot(form, FormMain)) return;
	ScrollBarPtr scrollbar = (ScrollBarPtr) GetObjectPtr(form, SMSScroll);
	SclSetScrollBar(scrollbar, g_CurrentPage, 0, PrivPageCount(db), 1);
}

void DeleteCurrentRecord(DmOpenRef db)
{
	DeleteRecordInCategory(db, GetCurrentIndex(), g_SelectedCategory);
}

static void CalculateNextRow(DmOpenRef db, Int16 step)
{
	if (step > TABLE_PAGE_SIZE) step = TABLE_PAGE_SIZE;
	if (step < -TABLE_PAGE_SIZE) step = -TABLE_PAGE_SIZE;
	g_CurrentSelection += step;
	UInt32 pageCount = PrivPageCount(db);
	
	if (g_CurrentSelection < 0) {
		if (g_CurrentPage == 0) {
			g_CurrentSelection = 0;
		} else {
			g_CurrentSelection += TABLE_PAGE_SIZE;
			g_CurrentPage --;
		}
	} else if (g_CurrentPage == pageCount) {
		UInt16 lastPageSize = GetSMSCount(db, g_SelectedCategory) - pageCount * TABLE_PAGE_SIZE;
		if (g_CurrentSelection >= lastPageSize) g_CurrentSelection = lastPageSize - 1;
	} else if (g_CurrentSelection >= TABLE_PAGE_SIZE) {
		g_CurrentPage ++;
		g_CurrentSelection -= TABLE_PAGE_SIZE;
	}
}

void SelectNextRow(DmOpenRef db, Int16 step)
{
	UInt16 oldPage = g_CurrentPage;
	UInt16 oldSel = g_CurrentSelection;
	
	CalculateNextRow(db, step);
	
	if (g_CurrentPage == oldPage) {
		if (oldSel != g_CurrentSelection) {
			FormPtr form = FrmGetActiveForm();
			if (!FormIsNot(form, FormMain)) {
				TablePtr table = (TablePtr) GetObjectPtr(form, SMSTable);
				TblMarkRowInvalid(table, oldSel);
				TblMarkRowInvalid(table, g_CurrentSelection);
				TblRedrawTable(table);
			}
		}
	} else {
		GotoPage(db, g_CurrentPage);
	}
	
	UpdatePos(db);
}

void DisplayNextRecord(DmOpenRef db, Int16 step)
{
	FormPtr form = FrmGetActiveForm();
	if (FormIsNot(form, FormDetail)) return;

	CalculateNextRow(db, step);
	DisplayCurrentRecord(db);
	
	FieldPtr fieldContent = (FieldPtr) GetObjectPtr(form, FieldContent);
	FldDrawField(fieldContent);
	UpdateScrollbar(form, FieldContent, ScrollbarContent);
}

void DisplayNextUnreadRecord(DmOpenRef db)
{
	Int16 index = GetNextUnreadIndex(db, g_SelectedCategory);
	if (index == -1)
		DisplayNextRecord(db, 1);
	else {
		FormPtr form = FrmGetActiveForm();
		if (FormIsNot(form, FormDetail)) return;
		FieldPtr fieldContent = (FieldPtr) GetObjectPtr(form, FieldContent);
		DisplayRecord(db, -1, index);
		FldDrawField(fieldContent);
	}
}

UInt16 GetCurrentIndex()
{
	return g_CurrentSelection + g_CurrentPage * TABLE_PAGE_SIZE;
}
