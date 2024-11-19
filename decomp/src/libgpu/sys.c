#include <common.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>

struct Gpu {
    /* 0x00 */ const char* ver;
    /* 0x04 */ void (*addque)();
    /* 0x08 */ int (*addque2)();
    /* 0x0C */ int (*clr)();
    /* 0x10 */ void (*ctl)(unsigned int);
    /* 0x14 */ int (*cwb)(u32* data, s32 n);
    /* 0x18 */ void (*cwc)();
    /* 0x1C */ int (*drs)();
    /* 0x20 */ int (*dws)();
    /* 0x24 */ int (*exeque)();
    /* 0x28 */ int (*getctl)(int);
    /* 0x2C */ void (*otc)(OT_TYPE* ot, s32 n);
    /* 0x30 */ int (*param)(int);
    /* 0x34 */ int (*reset)(int);
    /* 0x38 */ u_long (*status)(void);
    /* 0x3C */ int (*sync)(int mode);
};
struct Debug {
    // GPU version
    // https://psx-spx.consoledev.net/graphicsprocessingunitgpu/#gpu-versions
    u_char version;

    u_char D_800B8929;
    u_char level;
    u_char reverse;
    short w;
    short h;
    u8 unk8[4];
    void (*drawSyncCb)();
    DRAWENV draw;
    DISPENV disp;
};

#define gpu D_800B8920

static int D_800B88D8[] = {0x15077350, 0x0040899C};

void SetDrawEnv2(DR_ENV* dr_env, DRAWENV* env);
#ifndef __psyz
static void memset(u_char* ptr, int value, int num);
#endif

void _addque();
int _addque2();
int _clr();
void _ctl(unsigned int);
int _cwb(u32* data, s32 n);
void _cwc();
int _drs();
int _dws();
int _exeque();
int _getctl(int);
void _otc(OT_TYPE* ot, s32 n);
int _param(int);
int _reset(int);
u_long _status(void);
int _sync(int mode);
int _version(int mode);
#ifndef __psyz
int printf();
#endif

u_long get_cs(short x, short y);
u_long get_ce(short x, short y);
u_long get_ofs(short x, short y);
u_long get_mode(int dfe, int dtd, u_short tpage);
u_long get_tw(RECT* rect);

static struct Gpu _gpucb = {
    "$Id: sys.c,v 1.129 1996/12/25 03:36:20 noda Exp $",
    _addque,
    _addque2,
    _clr,
    _ctl,
    _cwb,
    _cwc,
    _drs,
    _dws,
    _exeque,
    _getctl,
    _otc,
    _param,
    _reset,
    _status,
    _sync,
};
static struct Gpu* gpu = &_gpucb;
int (*GPU_printf)() = (int (*)())printf;
static struct Debug info = {0};

int D_800B89A8[] = {1024, 1024, 1024, 1024, 1024};
int D_800B89BC[] = {512, 1024, 1024, 512, 1024};
u_long move_image[] = {(4 << 24) | 0xFFFFFF, 0x80000000, 0, 0, 0};
u_long D_800B89E4[] = {(4 << 24) | 0xFFFFFF, 0, 0, 0, 0};
volatile u_long* D_800B89F8 = (u_long*)0x1F801810;
volatile u_long* D_800B89FC = (u_long*)0x1F801814; // stat
volatile u_long* D_800B8A00 = (u_long*)0x1F8010A0; // madr
volatile u_long* D_800B8A04 = (u_long*)0x1F8010A4;
volatile u_long* D_800B8A08 = (u_long*)0x1F8010A8; // chcr
volatile u_long* D_800B8A0C = (u_long*)0x1F8010E0;
volatile u_long* D_800B8A10 = (u_long*)0x1F8010E4;
volatile u_long* D_800B8A14 = (u_long*)0x1F8010E8;
volatile u_long* D_800B8A18 = (u_long*)0x1F8010F0;
u_long* _qlog = NULL;
int D_800B8A20 = 0;
int D_800B8A24 = 0;
volatile int D_800B8A28 = 0;
u_long* _qin = NULL;
u_long* _qout = NULL;
int D_800B8A34 = 0;
int D_800B8A38 = 0;
int D_800B8A3C = 0;
int D_800B8A40 = 0;
int D_800B8A44 = 0;
extern u_char ctlbuf[0x100];
extern u_char _que[0x1800];

#ifdef __psyz
#define GPU_PSX_PTR(x) ((u_long*)x)
#define TERM_PRIM(ot, p) termPrim(ot)
#else
#define GPU_PSX_PTR(x) (u_long*)((u_long)x & 0xFFFFFF)
#define TERM_PRIM(ot, p) *ot = (u_long)p & 0xFFFFFF
#endif

void GPU_cw(u_long*);
int ResetGraph(int mode) {
    switch (mode & 7) {
    case 3:
    case 0:
        printf("ResetGraph:jtb=%08x,env=%08x\n", &_gpucb, &info);
    case 5:
        memset(&info, 0, sizeof(info));
        ResetCallback();
        GPU_cw(GPU_PSX_PTR(D_800B8920));
        info.version = _reset(mode);
        info.D_800B8929 = 1;
        info.w = D_800B89A8[info.version];
        info.h = D_800B89BC[info.version];
        memset(&info.draw, -1, sizeof(DRAWENV));
        memset(&info.disp, -1, sizeof(DISPENV));
        return info.version;
    }
    if (info.level >= 2) {
        GPU_printf("ResetGraph(%d)...\n", mode);
    }
    return D_800B8920->reset(1);
}

int SetGraphReverse(int mode) {
    u_char prev = info.reverse;
    if (info.level >= 2) {
        GPU_printf("SetGraphReverse(%d)...\n", mode);
    }
    info.reverse = mode;
    gpu->ctl(gpu->getctl(8) | (info.reverse ? 0x08000080 : 0x08000000));
    if (info.version == 2) {
        gpu->ctl(info.reverse ? 0x20000501 : 0x20000504);
    }
    return prev;
}

int SetGraphDebug(int level) {
    u_char prev = info.level;
    info.level = level;
    if (info.level) {
        GPU_printf("SetGraphDebug:level:%d,type:%d reverse:%d\n", info.level,
                   info.version, info.reverse);
    }
    return prev;
}

int SetGraphQueue(int mode) {
    u_char prev = info.D_800B8929;
    if (info.level >= 2) {
        GPU_printf("SetGrapQue(%d)...\n", mode);
    }
    if (mode != info.D_800B8929) {
        D_800B8920->reset(1);
        info.D_800B8929 = mode;
        DMACallback(2, NULL);
    }
    return prev;
}

int GetGraphType(void) { return info.version; }

int GetGraphDebug(void) { return info.level; }

u_long DrawSyncCallback(void (*func)()) {
    void (*prev)();
    if (info.level >= 2) {
        GPU_printf("DrawSyncCallback(%08x)...\n", func);
    }
    prev = info.drawSyncCb;
    info.drawSyncCb = func;
    return (u_long)prev;
}

void SetDispMask(int mask) {
    if (info.level >= 2) {
        GPU_printf("SetDispMask(%d)...\n", mask);
    }
    if (mask == 0) {
        memset(&info.disp, -1, sizeof(DISPENV));
    }
    D_800B8920->ctl(mask ? 0x03000000 : 0x03000001);
}

int DrawSync(int mode) {
    if (info.level >= 2) {
        GPU_printf("DrawSync(%d)...\n", mode);
    }
    return D_800B8920->sync(mode);
}

void checkRECT(const char* log, RECT* r) {
    switch (info.level) {
    case 1:
        if (r->w > info.w || r->w + r->x > info.w || r->y > info.h ||
            r->y + r->h > info.h || r->w <= 0 || r->x < 0 || r->y < 0 ||
            r->h <= 0) {
            GPU_printf("%s:bad RECT", log);
            GPU_printf("(%d,%d)-(%d,%d)\n", r->x, r->y, r->w, r->h);
        }
        break;
    case 2:
        GPU_printf("%s:", log);
        GPU_printf("(%d,%d)-(%d,%d)\n", r->x, r->y, r->w, r->h);
        break;
    }
}

int ClearImage(RECT* rect, u8 r, u8 g, u8 b) {
    checkRECT("ClearImage", rect);
    return D_800B8920->addque2(
        D_800B8920->clr, rect, sizeof(RECT), (b << 0x10) | (g << 8) | r);
}

int ClearImage2(RECT* rect, u8 r, u8 g, u8 b) {
    checkRECT("ClearImage", rect);
    return D_800B8920->addque2(D_800B8920->clr, rect, sizeof(RECT),
                               0x80000000 | (b << 0x10) | (g << 8) | r);
}

int LoadImage(RECT* rect, u_long* p) {
    checkRECT("LoadImage", rect);
    return D_800B8920->addque2(D_800B8920->dws, rect, sizeof(RECT), p);
}

int StoreImage(RECT* rect, u_long* p) {
    checkRECT("StoreImage", rect);
    return D_800B8920->addque2(D_800B8920->drs, rect, sizeof(RECT), p);
}

int MoveImage(RECT* rect, int x, int y) {
    checkRECT("MoveImage", rect);
    if (!rect->w || !rect->h) {
        return -1;
    }
    move_image[2] = *(int*)&rect->x;
    move_image[3] = y << 0x10 | (u16)x;
    move_image[4] = *(int*)&rect->w;
    return D_800B8920->addque2(
        D_800B8920->cwc, move_image, sizeof(move_image), 0);
}

OT_TYPE* ClearOTag(OT_TYPE* ot, int n) {
    if (info.level >= 2) {
        GPU_printf("ClearOTag(%08x,%d)...\n", ot, n);
    }

    while (--n) {
        setlen(ot, 0);
        setaddr(ot, ot + 1);
        ot++;
    }
    TERM_PRIM(ot, D_800B89E4);
    return ot;
}

OT_TYPE* ClearOTagR(OT_TYPE* ot, int n) {
    if (info.level >= 2) {
        GPU_printf("ClearOTagR(%08x,%d)...\n", ot, n);
    }

    D_800B8920->otc(ot, n);
    TERM_PRIM(ot, D_800B89E4);
    return ot;
}

void DrawPrim(void* p) {
    int len = getlen(p);
    D_800B8920->sync(0);
    D_800B8920->cwb((u32*)&((P_TAG*)p)->r0, len);
}

void DrawOTag(OT_TYPE* p) {
    if (info.level >= 2) {
        GPU_printf("DrawOTag(%08x)...\n", p);
    }
    D_800B8920->addque2(D_800B8920->cwc, p, 0, 0);
}

DRAWENV* PutDrawEnv(DRAWENV* env) {
    if (info.level >= 2) {
        GPU_printf("PutDrawEnv(%08x)...\n", env);
    }

    SetDrawEnv2(&env->dr_env, env);
    termPrim(&env->dr_env);
    D_800B8920->addque2(D_800B8920->cwc, &env->dr_env, sizeof(DR_ENV), 0);
    __builtin_memcpy(&info.draw, env, sizeof(DRAWENV));
    return env;
}

void DrawOTagEnv(OT_TYPE* p, DRAWENV* env) {
    if (info.level >= 2) {
        GPU_printf("DrawOTagEnv(%08x,&08x)...\n", p, env);
    }
    SetDrawEnv2(&env->dr_env, env);
    setaddr(&env->dr_env, p);
    D_800B8920->addque2(D_800B8920->cwc, &env->dr_env, sizeof(DR_ENV), 0);
    __builtin_memcpy(&info.draw, env, sizeof(DRAWENV));
}

DRAWENV* GetDrawEnv(DRAWENV* env) {
    memcpy(env, &info.draw, sizeof(DRAWENV));
    return env;
}

#define RECT_EQ(r1, r2)                                                        \
    (((volatile RECT*)r1)->x == r2.x && ((volatile RECT*)r1)->y == r2.y &&     \
     ((volatile RECT*)r1)->w == r2.w && ((volatile RECT*)r1)->h == r2.h)

// PutDispEnv PSY-Q 4.0
// heavily inspired by joshlory PutDispEnv PSY-Q 3.5:
// https://decomp.me/scratch/7H6Nk
u_long get_dx(DISPENV* env);
DISPENV* PutDispEnv(DISPENV* env) {
    s32 h_start, h_end;
    s32 v_start, v_end;
    s32 mode;

    mode = 0x08000000;
    if (info.level >= 2) {
        GPU_printf("PutDispEnv(%08x)...\n", env);
    }
    D_800B8920->ctl(
        info.version == 1 || info.version == 2
            ? ((env->disp.y & 0xFFF) << 12) | (get_dx(env) & 0xFFF) | 0x05000000
            : ((env->disp.y & 0x3FF) << 10) | (env->disp.x & 0x3FF) |
                  0x05000000);
    if (!RECT_EQ(&info.disp.screen, env->screen)) {
        env->pad0 = GetVideoMode();
        h_start = env->screen.x * 10 + 0x260;
        v_start = env->screen.y + (env->pad0 ? 0x13 : 0x10);
        h_end = h_start + (env->screen.w ? env->screen.w * 10 : 2560);
        v_end = v_start + (env->screen.h ? env->screen.h : 240);
        h_start = CLAMP(h_start, 500, 3290);
        h_end = CLAMP(h_end, h_start + 0x50, 3290);
        v_start = CLAMP(v_start, 0x10, (env->pad0 ? 310 : 256));
        v_end = CLAMP(v_end, v_start + 2, (env->pad0 ? 312 : 258));
        D_800B8920->ctl( // set horizontal display range
            ((h_end & 0xFFF) << 12) | 0x06000000 | (h_start & 0xFFF));
        D_800B8920->ctl( // set vertical display range
            ((v_end & 0x3FF) << 10) | 0x07000000 | (v_start & 0x3FF));
    }
    if (info.disp.isinter != env->isinter ||
        info.disp.isrgb24 != env->isrgb24 || info.disp.pad0 != env->pad0 ||
        info.disp.pad1 != env->pad1 || !RECT_EQ(&info.disp.disp, env->disp)) {
        env->pad0 = GetVideoMode();
        if (env->pad0 == MODE_PAL) {
            mode |= 0x8;
        }
        if (env->isrgb24) {
            mode |= 0x10;
        }
        if (env->isinter) {
            mode |= 0x20;
        }
        if (info.reverse) {
            mode |= 0x80;
        }
        if (env->disp.w <= 280) {
        } else if (env->disp.w <= 352) {
            mode |= 1;
        } else if (env->disp.w <= 400) {
            mode |= 0x40;
        } else if (env->disp.w <= 560) {
            mode |= 2;
        } else {
            mode |= 3;
        }
        if (env->disp.h <= (!env->pad0 ? 256 : 288)) {
        } else {
            mode |= 0x24;
        }
        D_800B8920->ctl(mode);
    }
    memcpy((u8*)&info.disp, (u8*)env, sizeof(DISPENV));
    return env;
}

DISPENV* GetDispEnv(DISPENV* env) {
    memcpy(env, &info.disp, sizeof(DISPENV));
    return env;
}

int GetODE(void) { return D_800B8920->status() >> 0x1F; }

void SetTexWindow(DR_TWIN* p, RECT* tw) {
    setlen(p, 2);
    p->code[0] = get_tw(tw);
    p->code[1] = 0;
}

void SetDrawArea(DR_AREA* p, RECT* r) {
    setlen(p, 2);
    p->code[0] = get_cs(r->x, r->y);
    p->code[1] = get_ce(r->x + r->w - 1, r->y + r->h - 1);
}

void SetDrawOffset(DR_OFFSET* p, u_short* ofs) {
    setlen(p, 2);
    p->code[0] = get_ofs(ofs[0], ofs[1]);
    p->code[1] = 0;
}

typedef struct {
    u_long tag;
    u_long code[2];
} DR_PRIO;
void SetPriority(DR_PRIO* p, int pbc, int pbw) {
    int data;
    setlen(p, 2);
    data = 0xE6000000;
    if (pbc) {
        data = 0xE6000002;
    }
    p->code[0] = data | (pbw != 0);
    p->code[1] = 0;
}

void SetDrawMode(DR_MODE* p, int dfe, int dtd, int tpage, RECT* tw) {
    setlen(p, 2);
    p->code[0] = get_mode(dfe, dtd, tpage);
    p->code[1] = get_tw(tw);
}

void SetDrawEnv(DR_ENV* dr_env, DRAWENV* env) {
#define PUSH_CODE ((u_long*)(&dr->code[-1]))[len++]
    DR_ENV* dr;
    int len;
    RECT rect;
    u16 w, h;
    len = 1;
    dr = dr_env;
    PUSH_CODE = get_cs(env->clip.x, env->clip.y);
    PUSH_CODE =
        get_ce(env->clip.w + env->clip.x - 1, env->clip.y + env->clip.h - 1);
    PUSH_CODE = get_ofs(env->ofs[0], env->ofs[1]);
    PUSH_CODE = get_mode(env->dfe, env->dtd, env->tpage);
    PUSH_CODE = get_tw(&env->tw);
    PUSH_CODE = 0xE6000000;
    if (env->isbg) {
        rect.x = env->clip.x;
        rect.y = env->clip.y;
        rect.w = env->clip.w;
        rect.h = env->clip.h;
        if (rect.w >= 0) {
            if (info.w - 1 < rect.w) {
                w = info.w - 1;
            } else {
                w = rect.w;
            }
        } else {
            w = 0;
        }
        rect.w = w;
        if (rect.h >= 0) {
            if (info.h - 1 < rect.h) {
                h = info.h - 1;
            } else {
                h = rect.h;
            }
        } else {
            h = 0;
        }
        rect.h = h;
        rect.x -= env->ofs[0];
        rect.y -= env->ofs[1];
        PUSH_CODE = 0x60000000 | (env->b0 << 16) | (env->g0 << 8) | env->r0;
        PUSH_CODE = *(u_long*)&rect.x;
        PUSH_CODE = *(u_long*)&rect.w;
        rect.x += env->ofs[0];
        rect.y += env->ofs[1];
    }
    setlen(dr, len - 1);
#undef PUSH_CODE
}

void SetDrawEnv2(DR_ENV* dr_env, DRAWENV* env) {
#define PUSH_CODE ((u_long*)(&dr->code[-1]))[len++]
    DR_ENV* dr;
    int len;
    RECT rect;
    u16 w, h;
    len = 1;
    dr = dr_env;
    PUSH_CODE = get_cs(env->clip.x, env->clip.y);
    PUSH_CODE =
        get_ce(env->clip.w + env->clip.x - 1, env->clip.y + env->clip.h - 1);
    PUSH_CODE = get_ofs(env->ofs[0], env->ofs[1]);
    PUSH_CODE = get_mode(env->dfe, env->dtd, env->tpage);
    PUSH_CODE = get_tw(&env->tw);
    PUSH_CODE = 0xE6000000;
    if (env->isbg) {
        rect.x = env->clip.x;
        rect.y = env->clip.y;
        rect.w = env->clip.w;
        rect.h = env->clip.h;
        if (rect.w >= 0) {
            if (info.w - 1 < rect.w) {
                w = info.w - 1;
            } else {
                w = rect.w;
            }
        } else {
            w = 0;
        }
        rect.w = w;
        if (rect.h >= 0) {
            if (info.h - 1 < rect.h) {
                h = info.h - 1;
            } else {
                h = rect.h;
            }
        } else {
            h = 0;
        }
        rect.h = h;
        if (rect.x & 0x3F || rect.w & 0x3F) {
            rect.x -= env->ofs[0];
            rect.y -= env->ofs[1];
            PUSH_CODE = 0x60000000 | (env->b0 << 16) | (env->g0 << 8) | env->r0;
            PUSH_CODE = *(u_long*)&rect.x;
            PUSH_CODE = *(u_long*)&rect.w;
            rect.x += env->ofs[0];
            rect.y += env->ofs[1];
        } else {
            PUSH_CODE = 0x02000000 | (env->b0 << 16) | (env->g0 << 8) | env->r0;
            PUSH_CODE = *(u_long*)&rect.x;
            PUSH_CODE = *(u_long*)&rect.w;
        }
    }
    setlen(dr, len - 1);
#undef PUSH_CODE
}

u_long get_mode(int dfe, int dtd, u_short tpage) {
    if (info.version == 1 || info.version == 2) {
        return (dtd ? 0xE1000800 : 0xE1000000) | (dfe ? 0x1000 : 0) |
               (tpage & 0x27FF);
    } else {
        return (dtd ? 0xE1000200 : 0xE1000000) | (dfe ? 0x400 : 0) |
               (tpage & 0x9FF);
    }
}

u_long get_cs(short x, short y) {
    x = CLAMP(x, 0, info.w - 1);
    y = CLAMP(y, 0, info.h - 1);
    if (info.version == 1 || info.version == 2) {
        return 0xE3000000 | ((y & 0xFFF) << 12) | (x & 0xFFF);
    } else {
        return 0xE3000000 | ((y & 0x3FF) << 10) | (x & 0x3FF);
    }
}

u_long get_ce(short x, short y) {
    x = CLAMP(x, 0, info.w - 1);
    y = CLAMP(y, 0, info.h - 1);
    if (info.version == 1 || info.version == 2) {
        return 0xE4000000 | ((y & 0xFFF) << 12) | (x & 0xFFF);
    } else {
        return 0xE4000000 | ((y & 0x3FF) << 10) | (x & 0x3FF);
    }
}

u_long get_ofs(short x, short y) {
    if (info.version == 1 || info.version == 2) {
        return 0xE5000000 | ((y & 0xFFF) << 12) | (x & 0xFFF);
    } else {
        return 0xE5000000 | ((y & 0x7FF) << 11) | (x & 0x7FF);
    }
}

u_long get_tw(RECT* rect) {
    u_long code[4];
    if (rect) {
        code[0] = (rect->x & 0xFF) >> 3;
        code[2] = (-rect->w & 0xFF) >> 3;
        code[1] = (rect->y & 0xFF) >> 3;
        code[3] = (-rect->h & 0xFF) >> 3;
        return 0xE2000000 | (code[1] << 15) | (code[0] << 10) | (code[3] << 5) |
               code[2];
    }
    return 0;
}

u_long get_dx(DISPENV* env);
INCLUDE_ASM("asm/nonmatchings/libgpu/sys", get_dx);

u_long _status(void) { return *D_800B89FC; }

INCLUDE_ASM("asm/nonmatchings/libgpu/sys", _otc);

INCLUDE_ASM("asm/nonmatchings/libgpu/sys", _clr);

INCLUDE_ASM("asm/nonmatchings/libgpu/sys", _dws);

INCLUDE_ASM("asm/nonmatchings/libgpu/sys", _drs);

void _ctl(unsigned int status) {
    *D_800B89FC = status;
    ctlbuf[status >> 0x18] = status & 0xFFFFFF;
}

int _getctl(int n) { return ctlbuf[n]; }

int _cwb(u32* data, s32 n) {
    int i;

    *D_800B89FC = 0x04000000;
    for (i = n - 1; i != -1; i--) {
        *D_800B89F8 = *data++;
    }
    return 0;
}

void _cwc(s32 arg0) {
    *D_800B89FC = 0x04000002;
    *D_800B8A00 = arg0;
    *D_800B8A04 = 0;
    *D_800B8A08 = 0x01000401;
}

int _param(int x) {
    *D_800B89FC = x | 0x10000000;
    return *D_800B89F8 & 0xFFFFFF;
}

void _addque(int arg0, int arg1, int arg2) { _addque2(arg0, arg1, 0, arg2); }

INCLUDE_ASM("asm/nonmatchings/libgpu/sys", _addque2);

INCLUDE_ASM("asm/nonmatchings/libgpu/sys", _exeque);

int SetIntrMask(int);
int _reset(int mode) {
#ifdef __psyz
    int psyz_gpu_version(int);
    return psyz_gpu_version(mode);
#endif
    u_long** queue = &_qout;
    D_800B8A3C = SetIntrMask(0);
    *queue = NULL;
    _qin = _qout;
    switch (mode & 7) {
    case 0:
    case 5:
        // complete reset, re-initialize draw and disp environments
        *D_800B8A08 = 0x401;
        *D_800B8A18 |= 0x800;
        *D_800B89FC = 0;
        memset(ctlbuf, 0, sizeof(ctlbuf));
        memset(_que, 0, sizeof(_que));
        break;
    case 1:
    case 3:
        // cancels the current drawing and flushes the command buffer
        // preserves the current draw and disp environments
        *D_800B8A08 = 0x401;
        *D_800B8A18 |= 0x800;
        *D_800B89FC = 0x02000000;
        *D_800B89FC = 0x01000000;
        break;
    }
    SetIntrMask(D_800B8A3C);
    return !(mode & 7) ? _version(mode) : 0;
}

INCLUDE_ASM("asm/nonmatchings/libgpu/sys", _sync);

void set_alarm(void) {
    D_800B8A40 = VSync(-1) + 0xF0;
    D_800B8A44 = 0;
}

INCLUDE_ASM("asm/nonmatchings/libgpu/sys", get_alarm);

int _version(int mode) {
    *D_800B89FC = 0x10000007;
    if ((*D_800B89F8 & 0xFFFFFF) != 2) { // check for GPUv2
        *D_800B89F8 = 0xE1001000 | (*D_800B89FC & 0x3FFF);
        *D_800B89F8;
        if (!(*D_800B89FC & 0x1000)) {
            return 0;
        }
        if (!(mode & 8)) {
            return 1;
        }
        *D_800B89FC = 0x20000504;
        return 2;
    } else if (!(mode & 8)) {
        return 3;
    } else {
        *D_800B89FC = 0x09000001;
        return 4;
    }
}

#ifndef __psyz
static void memset(u_char* ptr, int value, int num) {
    int i;
    for (i = num - 1; i != -1; i--) {
        *ptr++ = value;
    }
}
#endif

NOP;
