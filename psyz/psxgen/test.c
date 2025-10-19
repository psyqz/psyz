#include <kernel.h>
#include <libetc.h>
#include <libgpu.h>

#define OT_LENGTH 1
#define OTSIZE 1 << OT_LENGTH
#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240

typedef struct DB {
    DRAWENV draw;
    DISPENV disp;
    OT_TYPE ot[OTSIZE];
    POLY_FT4 ft4[4];
} DB;
DB db[2];
DB* cdb;

#include "../tests/res/4bpp.h"

static int LoadTim(void* data, u_short* outTpage, u_short* outClut) {
    if (OpenTIM((u_long*)data)) {
        return 1;
    }
    TIM_IMAGE tim;
    if (!ReadTIM(&tim)) {
        return 1;
    }
    LoadImage(tim.prect, tim.paddr);
    if (outTpage) {
        *outTpage = GetTPage((int)tim.mode, 0, tim.prect->x, tim.prect->y);
    }
    if (tim.caddr) {
        LoadImage(tim.crect, tim.caddr);
        if (outClut) {
            *outClut = GetClut(tim.crect->x, tim.crect->y);
        }
    }
    return 0;
}
void RunTest() {
    u_short tpage, clut;
    if (LoadTim(img_4bpp, &tpage, &clut)) {
        return;
    }

    SetPolyFT4(&cdb->ft4[0]);
    setXYWH(&cdb->ft4[0], 16, 16, 64, 64);
    setRGB0(&cdb->ft4[0], 255, 128, 128);
    setUVWH(&cdb->ft4[0], 0, 0, 64, 64);
    setSemiTrans(&cdb->ft4[0], 0);
    cdb->ft4[0].tpage = tpage;
    cdb->ft4[0].clut = clut;

    ClearImage(&cdb->draw.clip, 60, 120, 120);
    AddPrim(cdb->ot, &db[0].ft4[0]);
    DrawOTag(cdb->ot);
    DrawSync(0);
    VSync(0);
    PutDispEnv(&cdb->disp);
}

void SetUp() {
    SetDefDrawEnv(&db[0].draw, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&db[1].draw, SCREEN_WIDTH, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&db[0].disp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&db[1].disp, SCREEN_WIDTH, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetVideoMode(MODE_NTSC);
    ResetGraph(0);
    PutDrawEnv(&db[0].draw);
    PutDispEnv(&db[0].disp);
    ClearOTag(db[0].ot, OTSIZE);
    ClearOTag(db[1].ot, OTSIZE);
    SetDispMask(1);
    cdb = &db[0];
}

void TearDown() { ResetGraph(0); }

int main() {
    SetUp();
    RunTest();
    TearDown();
    return 0;
}
