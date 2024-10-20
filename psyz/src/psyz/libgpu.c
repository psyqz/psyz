#include <psyz.h>
#include <libgpu.h>
#include <stdarg.h>
#include <stdio.h>
#include <memory.h>
#include <log.h>

char str[0x400];
SPRT_8 sprt[0x400];
u16 tpage;
u16 clut;

int (*GPU_printf)() = (int (*)())printf;

void SetDispMask(int mask) { NOT_IMPLEMENTED; }

int MyClearImage(RECT* rect, u_char r, u_char g, u_char b);
int ClearImage(RECT* rect, u_char r, u_char g, u_char b) {
    return MyClearImage(rect, r, g, b);
}

int MyLoadImage(RECT* rect, u_long* p);
int LoadImage(RECT* rect, u_long* p) { return MyLoadImage(rect, p); }

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

OT_TYPE* MyClearOTag(OT_TYPE* ot, int n);
OT_TYPE* ClearOTag(OT_TYPE* ot, int n) { return MyClearOTag(ot, n); }

OT_TYPE* ClearOTagR(OT_TYPE* ot, int n) {
    NOT_IMPLEMENTED;
    return NULL;
}

void SetDrawEnv(DR_ENV* dr_env, DRAWENV* env) { NOT_IMPLEMENTED; }

DISPENV* SetDefDispEnv(DISPENV* env, int x, int y, int w, int h) {
    env->disp.x = x;
    env->disp.y = y;
    env->disp.w = w;
    env->disp.h = h;
    env->screen.x = 0;
    env->screen.y = 0;
    env->screen.w = 0;
    env->screen.h = 0;
    env->isrgb24 = 0;
    env->isinter = 0;
    env->pad0 = 0;
    env->pad1 = 0;
    return env;
}

void DrawPrim(OT_TYPE* p) {
    NOT_IMPLEMENTED;
}