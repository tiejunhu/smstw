#include "SMSNew.h"
#include "Global.h"
#include "SMSTW_Rsc.h"

void NewSMS()
{
	g_ComposeMode = COMPOSE_NEW;
	FrmGotoForm(FormReply);
}