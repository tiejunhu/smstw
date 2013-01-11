#ifndef _SMTW_TABLE
#define _SMTW_TABLE

UInt32 PrivPageCount(DmOpenRef db);
void PrivDrawCell(void* table, Int16 row, Int16 column, RectanglePtr bounds);
void DrawTable(DmOpenRef db, Int16 category);
void FirstPage(DmOpenRef db);
void GotoPage(DmOpenRef db, Int16 page);
void NextPage(DmOpenRef db);
void PrevPage(DmOpenRef db);
void GotoPageOf(DmOpenRef db, Int16 page, Int16 category);
void RefreshPage(DmOpenRef db);
void SelectRow(DmOpenRef db, Int16 row);
void SelectNextRow(DmOpenRef db, Int16 step);

Err DisplayCurrentRecord(DmOpenRef db);
void DisplayNextRecord(DmOpenRef db, Int16 step);
void DisplayNextUnreadRecord(DmOpenRef db);

Err DisplayRecord(DmOpenRef db, Int16 category, UInt16 index);
void UpdateScrollbar(DmOpenRef db);

void SetCurrentRecordCategory(DmOpenRef db, Int16 category);
void DeleteCurrentRecord(DmOpenRef db);

UInt16 GetCurrentIndex();

#endif