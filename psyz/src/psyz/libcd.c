#include <psyz.h>
#include <libcd.h>
#include <log.h>

#include <inttypes.h>
#include <stdio.h>

#define SECTOR_SIZE 2352

int CdInit(void) {
    NOT_IMPLEMENTED;
    return 0;
}

int CdReading() {
    NOT_IMPLEMENTED;
    return 0;
}

CdlCB CdReadyCallback(CdlCB func) {
    NOT_IMPLEMENTED;
    return func;
}

char* CdSyncModeToStr(int mode) {
    switch (mode) {
    case CdlNop:
        return "CdlNop";
    case CdlSetloc:
        return "CdlSetloc";
    case CdlPlay:
        return "CdlPlay";
    case CdlForward:
        return "CdlForward";
    case CdlBackward:
        return "CdlBackward";
    case CdlReadN:
        return "CdlReadN";
    case CdlSetfilter:
        return "CdlSetfilter";
    case CdlSetmode:
        return "CdlSetmode";
    case CdlGetTD:
        return "CdlGetTD";
    default:
        break;
    }

    return "";
}

#define DECODE_BCD(x) (((x) >> 4) * 10 + ((x)&0xF))

int CdPosToInt(CdlLOC* p) {
    return (75 * (60 * DECODE_BCD(p->minute) + DECODE_BCD(p->second))) +
           DECODE_BCD(p->sector) - 150;
}

int CdControl(u_char com, u_char* param, u_char* result) { NOT_IMPLEMENTED; }

int CdSync(int mode, u_char* result) {
    DEBUGF("mode %0d %s", mode, CdSyncModeToStr(mode));
    NOT_IMPLEMENTED;
    return CdlComplete;
}

int CdMix(CdlATV* vol) {
    NOT_IMPLEMENTED;
    return 0;
}

void ExecCd() { NOT_IMPLEMENTED; }
