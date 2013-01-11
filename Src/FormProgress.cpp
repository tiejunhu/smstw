#include "FormProgress.h"

Boolean FormProgressHandleEvent(EventType* eventP)
{
	Boolean handled = false;
	FormType * frmP;

	switch (eventP->eType) 
	{
		case frmOpenEvent:
			frmP = FrmGetActiveForm();
			FrmDrawForm(frmP);
			handled = true;
			break;
		case ctlSelectEvent:
			switch(eventP->data.ctlSelect.controlID) {
			}
			break;
	}
	return handled;
}
