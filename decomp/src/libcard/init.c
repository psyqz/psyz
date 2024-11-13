#include <common.h>
#include <kernel.h>
#include <libapi.h>

void InitCARD2(long val);
long ReadInitPadFlag(void);
void InitCARD(long val) {
    ChangeClearPAD(0);
    EnterCriticalSection();
    if (!ReadInitPadFlag()) {
        val = 0;
    }
    InitCARD2(val);
    ExitCriticalSection();
}

void StartCARD2(void);
long StartCARD(void) {
    EnterCriticalSection();
    StartCARD2();
    ChangeClearPAD(0);
    ExitCriticalSection();
}

void StopCARD2(void);
void _ExitCard(void);
long StopCARD(void) {
    StopCARD2();
    _ExitCard();
}
