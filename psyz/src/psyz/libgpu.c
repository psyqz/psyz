#include <psyz.h>
#include <libgpu.h>
#include <stdarg.h>
#include <stdio.h>
#include <memory.h>
#include <log.h>

void SetDispMask(int mask) { NOT_IMPLEMENTED; }

int MyClearImage(RECT* rect, u_char r, u_char g, u_char b);
int ClearImage(RECT* rect, u_char r, u_char g, u_char b) {
    return MyClearImage(rect, r, g, b);
}

int MyLoadImage(RECT* rect, u_long* p);
int LoadImage(RECT* rect, u_long* p) { return MyLoadImage(rect, p); }

int MargePrim(void* p0, void* p1) {
    int newLen = getlen(p0) + getlen(p1) + 1;
    if (newLen > 18)
        return -1;
    setlen(p0, newLen);
    setaddr(p1, 0);
    setcode(p1, 0);
    return 0;
}

int MyStoreImage(RECT* rect, u_long* p);
int StoreImage(RECT* rect, u_long* p) { return MyStoreImage(rect, p); }

int MoveImage(RECT* rect, int x, int y) { NOT_IMPLEMENTED; }

int MyDrawSync(int mode);
int DrawSync(int mode) { return MyDrawSync(mode); }

DRAWENV* MyPutDrawEnv(DRAWENV* env);
DRAWENV* PutDrawEnv(DRAWENV* env) { return MyPutDrawEnv(env); }

DISPENV* MyPutDispEnv(DISPENV* env);
DISPENV* PutDispEnv(DISPENV* env) { MyPutDispEnv(env); }

void MyDrawOTag(u_long* p);
void DrawOTag(OT_TYPE* p) { return MyDrawOTag((u_long*)p); }

void SetDrawMode(DR_MODE* p, int dfe, int dtd, int tpage, RECT* tw) {
    u_long v5;
    u_long v6;

    v5 = 0xE1000000u;
    setlen(p, 2);
    if ( dtd )
        v5 = 0xE1000200u;
    v6 = tpage & 0x9FF;
    if ( dfe )
        v6 |= 0x400u;
    p->code[0] = v5 | v6;
    if ( tw )
        p->code[1] = ((u8)tw->y >> 3 << 15) | ((u8)tw->x >> 3 << 10) | 0xE2000000 | (-4 * tw->h) & 0x3E0 | ((u8)-tw->w >> 3);
    else
        p->code[1] = 0;
}

int MyResetGraph(int mode);
int ResetGraph(int mode) { return MyResetGraph(mode); }

int SetGraphDebug(int level) { NOT_IMPLEMENTED; }

int GetGraphType(void) {
    NOT_IMPLEMENTED;
    return 0;
}

int GetGraphDebug(void) {
    NOT_IMPLEMENTED;
    return 0;
}

u_long* MyClearOTag(OT_TYPE* ot, int n);
u_long* ClearOTag(OT_TYPE* ot, int n) { return MyClearOTag(ot, n); }

u_long* ClearOTagR(OT_TYPE* ot, int n) {
    NOT_IMPLEMENTED;
    return NULL;
}

u_short GetTPage(int tp, int abr, int x, int y) {
    return getTPage(tp, abr, x, y);
}

void SetDrawEnv(DR_ENV* dr_env, DRAWENV* env) { NOT_IMPLEMENTED; }

u_short GetClut(int x, int y) { return getClut(x, y); }
void AddPrim(void* ot, void* p) { addPrim(ot, p); }
void AddPrims(void* ot, void* p0, void* p1) { addPrims(ot, p0, p1); }
void CatPrim(void* p0, void* p1) { catPrim(p0, p1); }
void SetShadeTex(void* p, int tge) { setShadeTex(p, tge); }
void SetPolyF3(POLY_F3* p) { setPolyF3(p); }
void SetPolyF4(POLY_F4* p) { setPolyF4(p); }
void SetPolyFT3(POLY_FT3* p) { setPolyFT3(p); }
void SetPolyFT4(POLY_FT4* p) { setPolyFT4(p); }
void SetLineF2(LINE_F2* p) { setLineF2(p); };
void SetLineF3(LINE_F3* p) { setLineF3(p); };
void SetLineF4(LINE_F4* p) { setLineF4(p); };
void SetLineG2(LINE_G2* p) { setLineG2(p); }
void SetPolyGT3(POLY_GT3* p) { setPolyGT3(p); }
void SetPolyG4(POLY_G4* p) { setPolyG4(p); }
void SetPolyGT4(POLY_GT4* p) { setPolyGT4(p); }
void SetSemiTrans(void* p, int abe) { setSemiTrans(p, abe); }
void SetSprt(SPRT* p) { setSprt(p); }
void SetSprt16(SPRT_16* p) { setSprt16(p); }
void SetSprt8(SPRT_8* p) { setSprt8(p); }
void SetTile(TILE* p) { setTile(p); }
