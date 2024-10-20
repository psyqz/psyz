#include <psyz.h>
#include <libgpu.h>
#include <libetc.h>
#include <stdio.h>

#define OT_LENGTH 12
#define OTSIZE (1 << OT_LENGTH)
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

typedef struct {
    POLY_F3 poly_f3;
    POLY_FT3 poly_ft3;
    POLY_G3 poly_g3;
    POLY_GT3 poly_gt3;
    POLY_F4 poly_f4;
    POLY_FT4 poly_ft4;
    POLY_G4 poly_g4;
    POLY_GT4 poly_gt4;
    LINE_F2 line_f2;
    LINE_G2 line_g2;
    LINE_F3 line_f3;
    LINE_G3 line_g3;
    LINE_F4 line_f4;
    LINE_G4 line_g4;
    TILE tile;
    TILE_16 tile_16;
    TILE_8 tile_8;
    TILE_1 tile_1;
    SPRT sprt;
    SPRT_16 sprt_16;
    SPRT_8 sprt_8;
} Shapes;

typedef struct DB {
    DRAWENV draw;
    DISPENV disp;
    OT_TYPE ot[OTSIZE];
    Shapes s;
} DB;

static void initGraphics(DB* db, int is_pal) {
    SetDefDrawEnv(&db[0].draw, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&db[1].draw, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&db[0].disp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&db[1].disp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (is_pal) {
        db[0].disp.isinter = 1;
        db[1].disp.isinter = 1;
        SetVideoMode(MODE_PAL);
    } else {
        SetVideoMode(MODE_NTSC);
    }
    ResetGraph(0);
    PutDrawEnv(&db[0].draw);
    PutDispEnv(&db[0].disp);

    SetDispMask(1);
    SetGraphDebug(0);
}

void AddShapes(DB* db) {
    ClearOTag(db->ot, OTSIZE);

    setPolyF3(&db->s.poly_f3);
    setXY3(&db->s.poly_f3, 8, 40, 72, 40 + 32, 8, 40 + 64);
    setRGB0(&db->s.poly_f3, 255, 255, 0);
    AddPrim(db->ot, &db->s.poly_f3);

    setPolyFT3(&db->s.poly_ft3);
    setXY3(&db->s.poly_ft3, 80, 40, 80 + 64, 40, 80, 40 + 64);
    setRGB0(&db->s.poly_ft3, 0, 0, 255);
    setUV3(&db->s.poly_ft3, 0, 0, 32, 0, 0, 32);
    setClut(&db->s.poly_ft3, 960, 384);
    setTPage(&db->s.poly_ft3, 0, 0, 960, 384);
    AddPrim(db->ot, &db->s.poly_ft3);

    setPolyF4(&db->s.poly_f4);
    setXYWH(&db->s.poly_f4, 152, 40, 64, 64);
    setRGB0(&db->s.poly_f4, 128, 0, 255);
    AddPrim(db->ot, &db->s.poly_f4);

    setPolyFT4(&db->s.poly_ft4);
    setXYWH(&db->s.poly_ft4, 224, 40, 64, 64);
    setRGB0(&db->s.poly_ft4, 0, 255, 0);
    setUVWH(&db->s.poly_ft4, 0, 0, 64, 32);
    db->s.poly_ft4.clut = 0x603C;
    setClut(&db->s.poly_ft4, 960, 384);
    setTPage(&db->s.poly_ft4, 0, 0, 960, 384);
    AddPrim(db->ot, &db->s.poly_ft4);

    setPolyG3(&db->s.poly_g3);
    setXY3(&db->s.poly_g3, 296, 40, 360, 40 + 32, 296, 40 + 64);
    setRGB0(&db->s.poly_g3, 255, 0, 0);
    setRGB1(&db->s.poly_g3, 0, 255, 0);
    setRGB2(&db->s.poly_g3, 0, 0, 255);
    AddPrim(db->ot, &db->s.poly_g3);

    setPolyGT3(&db->s.poly_gt3);
    setXY3(&db->s.poly_gt3, 368, 40, 368 + 64, 40, 368, 40 + 64);
    setRGB0(&db->s.poly_gt3, 255, 0, 0);
    setRGB1(&db->s.poly_gt3, 0, 255, 0);
    setRGB2(&db->s.poly_gt3, 0, 0, 255);
    setUV3(&db->s.poly_gt3, 0, 0, 32, 0, 0, 32);
    setClut(&db->s.poly_gt3, 960, 384);
    setTPage(&db->s.poly_gt3, 0, 0, 960, 384);
    AddPrim(db->ot, &db->s.poly_gt3);

    setPolyG4(&db->s.poly_g4);
    setXYWH(&db->s.poly_g4, 440, 40, 64, 64);
    setRGB0(&db->s.poly_g4, 255, 0, 0);
    setRGB1(&db->s.poly_g4, 0, 255, 0);
    setRGB2(&db->s.poly_g4, 0, 0, 255);
    setRGB3(&db->s.poly_g4, 255, 255, 0);
    AddPrim(db->ot, &db->s.poly_g4);

    setPolyGT4(&db->s.poly_gt4);
    setXYWH(&db->s.poly_gt4, 512, 40, 64, 64);
    setRGB0(&db->s.poly_gt4, 255, 0, 0);
    setRGB1(&db->s.poly_gt4, 0, 255, 0);
    setRGB2(&db->s.poly_gt4, 0, 0, 255);
    setRGB3(&db->s.poly_gt4, 255, 255, 0);
    setUVWH(&db->s.poly_gt4, 64, 32, -64, -32);
    setClut(&db->s.poly_gt4, 960, 384);
    setTPage(&db->s.poly_gt4, 0, 0, 960, 384);
    AddPrim(db->ot, &db->s.poly_gt4);

    setLineF2(&db->s.line_f2);
    setXY2(&db->s.line_f2, 8, 120, 72, 184);
    setRGB0(&db->s.line_f2, 255, 255, 255);
    AddPrim(db->ot, &db->s.line_f2);

    setLineF3(&db->s.line_f3);
    setXY3(&db->s.line_f3, 80, 120, 144, 152, 80, 184);
    setRGB0(&db->s.line_f3, 128, 255, 128);
    AddPrim(db->ot, &db->s.line_f3);

    setLineF4(&db->s.line_f4);
    setXYWH(&db->s.line_f4, 152, 120, 64, 64);
    setRGB0(&db->s.line_f4, 255, 0, 128);
    AddPrim(db->ot, &db->s.line_f4);

    setLineG2(&db->s.line_g2);
    setXY2(&db->s.line_g2, 224, 120, 288, 184);
    setRGB0(&db->s.line_g2, 255, 0, 0);
    setRGB1(&db->s.line_g2, 0, 255, 0);
    AddPrim(db->ot, &db->s.line_g2);

    setLineG3(&db->s.line_g3);
    setXY3(&db->s.line_g3, 296, 120, 360, 152, 296, 184);
    setRGB0(&db->s.line_g3, 255, 0, 0);
    setRGB1(&db->s.line_g3, 0, 255, 0);
    setRGB2(&db->s.line_g3, 0, 0, 255);
    AddPrim(db->ot, &db->s.line_g3);

    setLineG4(&db->s.line_g4);
    setXYWH(&db->s.line_g4, 368, 120, 64, 64);
    setRGB0(&db->s.line_g4, 255, 0, 0);
    setRGB1(&db->s.line_g4, 0, 255, 0);
    setRGB2(&db->s.line_g4, 0, 0, 255);
    setRGB3(&db->s.line_g4, 255, 255, 0);
    AddPrim(db->ot, &db->s.line_g4);

    setTile(&db->s.tile);
    setRGB0(&db->s.tile, 255, 0, 0);
    setXY0(&db->s.tile, 8, 200);
    setWH(&db->s.tile, 64, 64);
    AddPrim(db->ot, &db->s.tile);

    setTile1(&db->s.tile_1);
    setRGB0(&db->s.tile_1, 255, 255, 255);
    setXY0(&db->s.tile_1, 80, 200);
    AddPrim(db->ot, &db->s.tile_1);

    setTile8(&db->s.tile_8);
    setRGB0(&db->s.tile_8, 0, 255, 0);
    setXY0(&db->s.tile_8, 152, 200);
    AddPrim(db->ot, &db->s.tile_8);

    setTile16(&db->s.tile_16);
    setRGB0(&db->s.tile_16, 0, 0, 255);
    setXY0(&db->s.tile_16, 224, 200);
    AddPrim(db->ot, &db->s.tile_16);

    setSprt(&db->s.sprt);
    setRGB0(&db->s.sprt, 0, 0, 255);
    setXY0(&db->s.sprt, 296, 200);
    setWH(&db->s.sprt, 64, 64);
    setUV0(&db->s.sprt, 0, 0);
    setClut(&db->s.sprt, 960, 384);
    AddPrim(db->ot, &db->s.sprt);

    setSprt8(&db->s.sprt_8);
    setRGB0(&db->s.sprt_8, 0, 0, 255);
    setXY0(&db->s.sprt_8, 440, 200);
    setUV0(&db->s.sprt_8, 0, 8);
    setClut(&db->s.sprt_8, 960, 384);
    AddPrim(db->ot, &db->s.sprt_8);

    setSprt16(&db->s.sprt_16);
    setRGB0(&db->s.sprt_16, 0, 0, 255);
    setXY0(&db->s.sprt_16, 512, 200);
    setUV0(&db->s.sprt_16, 8, 16);
    setClut(&db->s.sprt_16, 960, 384);
    AddPrim(db->ot, &db->s.sprt_16);
}

int main(void) {
    DB db[2];
    DB* cdb;

    StopCallback();
    ResetCallback();
    initGraphics(db, 0);

    FntLoad(960, 256);
    SetDumpFnt(FntOpen(8, 30, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 512));

    while (1) {
        cdb = (cdb == &db[0]) ? &db[1] : &db[0];

        AddShapes(cdb);

        FntPrint("POLY_F3  POLY_FT3 POLY_F4  POLY_FT4 ");
        FntPrint("POLY_G3  POLY_GT3 POLY_G4  POLY_GT4\n");
        FntPrint("\n\n\n\n\n\n\n\n\n");
        FntPrint("LINE_F2  LINE_F3  LINE_F4  LINE_G2  ");
        FntPrint("LINE_G3  LINE_G4  ");
        FntPrint("~c\xFF\xFF\xC0yellow text~c\xFF\xFF\xFF\n");
        FntPrint("\n\n\n\n\n\n\n\n\n");
        FntPrint("TILE     TILE_1   TILE_8   TILE_16  ");
        FntPrint("SPRT              SPRT_8   SPRT_16\n");
        FntPrint("\n\n\n\n\n\n\n\n\n");

        DrawSync(0);
        VSync(0);
        SetDispMask(1);

        ClearImage(&cdb->draw.clip, 60, 120, 120);
        DrawOTag(cdb->ot);
        FntFlush(-1);
    }
    return 0;
}
