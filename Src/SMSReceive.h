#ifndef _SMTW_RECEIVE
#define _SMTW_RECEIVE

Err ReceiveSMS(DmOpenRef db, ExgSocketPtr exgSocketP, MemHandle& numberH, MemHandle& textH, UInt32 *lengthP, Int8* coding);

#endif
