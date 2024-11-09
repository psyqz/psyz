#include <psyz.h>
#include <libgpu.h>
#include <libgte.h>
#include <libgs.h>
#include <libetc.h>
#include <log.h>

static TILE tile_bg_clear[2];
static int HWD0;
static int VWD0;
static short GsORGOFSX;
static short GsORGOFSY;
static short PSDIDX;
static short PSDGPU;
static short CLIP2;
static short PSDCNT;
static short PSDOFSX[2];
static short PSDOFSY[2];
static short PSDBASEX[2];
static short PSDBASEY[2];
static DRAWENV GsDRAWENV = {0};
static DISPENV GsDISPENV = {0};
static PACKET* GsOUT_PACKET_P;

void gpu_init(unsigned short x, unsigned short y, unsigned short intmode,
              unsigned short dith, unsigned short varmmode) {

    if (((intmode >> 4) & 3) == 3) {
        ResetGraph(3);
    } else {
        ResetGraph(0);
    }
    GsDRAWENV.dtd = dith;
    PutDrawEnv(&GsDRAWENV);
    GsDISPENV.disp.w = x;
    GsDISPENV.disp.h = y;
    if (GetVideoMode() == 1) { // check if PAL
        GsDISPENV.screen.y = 24;
        GsDISPENV.pad0 = 1;
    }
    GsDISPENV.isrgb24 = varmmode;
    GsDISPENV.isinter = intmode & 1;
    PSDGPU = intmode & 4;
    PutDispEnv(&GsDISPENV);
}

void gte_init() {
    InitGeom();
    SetFarColor(0, 0, 0);
    SetGeomOffset(0, 0);
    GsORGOFSY = 0;
    GsORGOFSX = 0;
}

void GsClearVcount(void) { NOT_IMPLEMENTED; }

long GsGetVcount() { NOT_IMPLEMENTED; }

void GsInitVcount() { NOT_IMPLEMENTED; }

void GsClearOt(unsigned short offset, unsigned short point, GsOT* otp) {
    otp->offset = offset;
    otp->point = point;
#if 1
    otp->tag = otp->org;
    ClearOTag((OT_TYPE*)otp->tag, 1 << otp->length);
#else
    // TODO
// otp->tag = &otp->org[4 << otp->length - 1];
//  ClearOTagR(otp->org, 1 << otp->length);
#endif
}

void GsInitGraph(unsigned short x, unsigned short y, unsigned short intmode,
                 unsigned short dith, unsigned short varmmode) {
    gpu_init(x, y, intmode, dith, varmmode);
    gte_init();
    PSDIDX = 0;

    HWD0 = x;
    VWD0 = y;
    // TODO other missing inits
    PSDBASEX[0] = 0;
    PSDBASEX[1] = 0;
    PSDBASEY[0] = 0;
    PSDBASEY[1] = 0;
    // TODO other missing inits
    CLIP2 = 0;
    setlen(&tile_bg_clear[0], 3);
    setTile(&tile_bg_clear[0]);
    // tile_bg_clear[0].code = 2;
    setlen(&tile_bg_clear[1], 3);
    setTile(&tile_bg_clear[1]);
    // tile_bg_clear[1].code = 2;
    PSDCNT = 1;
    // TODO other missing inits

    GsSetDrawBuffClip();
    GsSetDrawBuffOffset();
}
void GsDefDispBuff(unsigned short x0, unsigned short y0, unsigned short x1,
                   unsigned short y1) {
    PSDOFSX[0] = x0;
    PSDOFSX[1] = x1;
    PSDOFSY[0] = y0;
    PSDOFSY[1] = y0;
    if (PSDGPU) {
        PSDBASEX[0] = 0;
        PSDBASEX[1] = 0;
        PSDBASEY[0] = 0;
        PSDBASEY[1] = 0;
    } else {
        PSDBASEX[0] = x0;
        PSDBASEX[1] = x1;
        PSDBASEY[0] = y0;
        PSDBASEY[1] = y1;
    }
    GsSetDrawBuffClip();
    GsSetDrawBuffOffset();
}

int GsGetActiveBuff(void) { return PSDIDX; }

void GsSetWorkBase(PACKET* outpacketp) { GsOUT_PACKET_P = outpacketp; }

void GsSwapDispBuff(void) {
    GsDISPENV.disp.x = PSDOFSX[PSDIDX];
    GsDISPENV.disp.y = PSDOFSY[PSDIDX];
    PutDispEnv(&GsDISPENV);
    SetDispMask(1);
    if (!PSDCNT++)
        PSDCNT = 1;
    PSDIDX = PSDIDX == 0;
    GsSetDrawBuffClip();
    GsSetDrawBuffOffset();
}

void GsSortClear(unsigned char r, unsigned char g, unsigned char b, GsOT* ot) {
    tile_bg_clear[PSDIDX].r0 = r;
    tile_bg_clear[PSDIDX].g0 = g;
    tile_bg_clear[PSDIDX].b0 = b;
    tile_bg_clear[PSDIDX].x0 = PSDOFSX[PSDIDX];
    tile_bg_clear[PSDIDX].y0 = PSDOFSY[PSDIDX];
    if (GsDISPENV.isrgb24) {
        tile_bg_clear[PSDIDX].w = (3 * HWD0) >> 1;
    } else {
        tile_bg_clear[PSDIDX].w = HWD0;
    }
    tile_bg_clear[PSDIDX].h = VWD0;
    AddPrim((OT_TYPE*)ot->tag, &tile_bg_clear[PSDIDX]);
}

void GsDrawOt(GsOT* ot) { DrawOTag((OT_TYPE*)ot->tag); }

void GsSetDrawBuffClip(void) { NOT_IMPLEMENTED; }

void GsSetDrawBuffOffset(void) { NOT_IMPLEMENTED; }
