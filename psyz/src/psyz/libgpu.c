#include <psyz.h>
#include <libgpu.h>
#include <stdarg.h>
#include <stdio.h>
#include <memory.h>
#include <log.h>
#include "../draw.h"

char str[0x400];
SPRT_8 sprt[0x400];
u16 tpage;
u16 clut;
u_char ctlbuf[0x100];

const int RETAIL_CONSOLE_TYPE = 0; // I think?

struct Gpu {
    /* 0x00 */ const char* ver;
    /* 0x04 */ int (*addque)(
        int (*exec)(u_long p1, u_long p2), u_long p1, u_long p2);
    /* 0x08 */ int (*addque2)(
        int (*exec)(u_long p1, u_long p2), u_long p1, int len, u_long p2);
    /* 0x0C */ int (*clr)(u_long p1, u_long p2);
    /* 0x10 */ void (*ctl)(unsigned int);
    /* 0x14 */ int (*cwb)();
    /* 0x18 */ int (*cwc)(u_long p1, u_long p2);
    /* 0x1C */ int (*drs)(u_long p1, u_long p2);
    /* 0x20 */ int (*dws)(u_long p1, u_long p2);
    /* 0x24 */ int (*exeque)();
    /* 0x28 */ int (*getctl)(int);
    /* 0x2C */ void (*otc)(OT_TYPE* ot, s32 n);
    /* 0x30 */ int (*param)(int);
    /* 0x34 */ int (*reset)(int);
    /* 0x38 */ u_long (*status)(void);
    /* 0x3C */ int (*sync)(int mode);
};

DISPENV* PutDispEnv(DISPENV* env) { Draw_PutDispEnv(env); }

static int queue_len = 0;
static u_long queue_buf[0x4000];
int GPU_Enqueue(u_long p1, u_long p2) {
    int mask = (int)p2;
    if (mask) {
        WARNF("mask not supported (mask:%08X)", mask);
    }
    DR_ENV* env = (DR_ENV*)p1;
    while (true) {
        if (queue_len + env->len > LEN(queue_buf)) {
            WARNF("GPU QUEUE FULL");
        } else if (sizeof(u_long) == 4) {
            // this is fine on 32-bit systems
            memcpy(queue_buf + queue_len, env->code, env->len * sizeof(u_long));
        } else if (sizeof(u_long) == 8) {
            // Wow okay, this part is uuuugly...
            // Gpu code is usually written to a u_long array, which will work
            // fine on both 32-bit and 64-bit compiled code.
            // But primitives are mapped from structs, we need to align the data
            int code = getcode(env) & ~3;
            if (code >= 0x20 && code < 0x80) {
                // it is a prim, we need to split
                u32* prim_data = (u32*)env->code;
                for (u_long i = 0; i < env->len; i++) {
                    queue_buf[queue_len + i] = prim_data[i];
                }
            } else {
                // TODO this is a temporary solution:
                // if gpu commands get merged with primitives, this will not
                // work
                memcpy(queue_buf + queue_len, env->code,
                       env->len * sizeof(u_long));
            }
        }
        queue_len += (int)env->len;
        if (isendprim(env)) {
            break;
        }
        env = (DR_ENV*)nextPrim(env);
    }
    return queue_len;
}
static int GPU_Clear(u_long p1, u_long p2) {
    Draw_ClearImage((RECT*)p1, p2 & 0xFF, (p2 >> 8) & 0xFF, (p2 >> 16) & 0xFF);
    return 0;
}
static int GPU_DataWrite(u_long p1, u_long p2) {
    Draw_LoadImage((RECT*)p1, (u_long*)p2);
    return 0;
}
static int GPU_DataRead(u_long p1, u_long p2) {
    Draw_StoreImage((RECT*)p1, (u_long*)p2);
    return 0;
}
static int GPU_Sync(int mode) {
    RECT rect;
    Draw_ResetBuffer();
    for (int i = 0; i < queue_len; i++) {
        u_long op = queue_buf[i];
        int code = (int)(queue_buf[i] >> 24) & 0xFF;
        // https://psx-spx.consoledev.net/graphicsprocessingunitgpu/#gpu-render-polygon-commands
        switch (code) {
        case 0x00:
            // empty?!
            break;
        case 0x02:
            rect.x = (short)(queue_buf[i + 1] & 0xFFFF);
            rect.y = (short)((queue_buf[i + 1] >> 16) & 0xFFFF);
            rect.w = (short)(queue_buf[i + 2] & 0xFFFF);
            rect.h = (short)((queue_buf[i + 2] >> 16) & 0xFFFF);
            Draw_ClearImage(
                &rect, (u_char)(op & 0xFF), (u_char)((op >> 8) & 0xFF),
                (u_char)((op >> 16) & 0xFF));
            i += 2;
            break;
        case 0xE1:
            Draw_SetTexpageMode((ParamDrawTexpageMode*)&op);
            break;
        case 0xE2:
            INFOF("TODO Draw_SetTextureWindow params");
            Draw_SetTextureWindow(0, 0, 0, 0);
            break;
        case 0xE3:
            Draw_SetAreaSXSY((int)op & 0x3FF, (int)(op >> 10) & 0x3FF);
            break;
        case 0xE4:
            Draw_SetAreaEXEY((int)op & 0x3FF, (int)(op >> 10) & 0x3FF);
            break;
        case 0xF7:
            break;
        default:
            if (code >= 0x20 && code < 0x80) {
                i += Draw_PushPrim(&queue_buf[i], queue_len - i) - 1;
                break;
            }
            WARNF("unsupported command %02X", code);
        }
    }
    Draw_FlushBuffer();
    Draw_Sync(mode);
    queue_len = 0;
    return queue_len;
}

static int psyz_addque2(
    int (*exec)(u_long p1, u_long p2), u_long p1, int len, u_long p2) {
    return exec(p1, p2);
}
static int psyz_addque(
    int (*exec)(u_long p1, u_long p2), u_long p1, u_long p2) {
    return psyz_addque2(exec, p1, 0, p2);
}
static int psyz_clr() {
    NOT_IMPLEMENTED;
    return 0;
}
static void psyz_ctl(unsigned int cmd) { NOT_IMPLEMENTED; }
static int psyz_cwb() {
    NOT_IMPLEMENTED;
    return 0;
}
static int psyz_exeque() {
    NOT_IMPLEMENTED;
    return 0;
}
static int psyz_getctl(int) {
    NOT_IMPLEMENTED;
    return 0;
}
static void psyz_otc(OT_TYPE* ot, s32 n) { NOT_IMPLEMENTED; }
static int psyz_param(int) {
    NOT_IMPLEMENTED;
    return 0;
}
static int psyz_reset(int) {
    NOT_IMPLEMENTED;
    return 0;
}
static u_long psyz_status(void) {
    NOT_IMPLEMENTED;
    return 0;
}

#define CLAMP(value, low, high)                                                \
    value < low ? low : (value > high ? high : value)
u_long get_cs(short x, short y) {
    int var_v0;
    int var_v1;

    x = CLAMP(x, 0, 1023);
    y = CLAMP(y, 0, 511);
    var_v1 = (y & 0x3FF) << 10;
    var_v0 = x & 0x3FF;
    return 0xE3000000 | var_v1 | var_v0;
}

u_long get_ce(int w, int h) {
    NOT_IMPLEMENTED;
    return 0;
}

u_long get_ofs(int w, int h) {
    NOT_IMPLEMENTED;
    return 0;
}

void GPU_cw(u_long* param) {
    struct Gpu* gpu = (struct Gpu*)param;
    gpu->ver = "psyz";
    gpu->addque = psyz_addque;
    gpu->addque2 = psyz_addque2;
    gpu->clr = GPU_Clear;
    gpu->ctl = psyz_ctl;
    gpu->cwb = psyz_cwb;
    gpu->cwc = GPU_Enqueue;
    gpu->drs = GPU_DataRead;
    gpu->dws = GPU_DataWrite;
    gpu->exeque = psyz_exeque;
    gpu->getctl = psyz_getctl;
    gpu->otc = psyz_otc;
    gpu->param = psyz_param;
    gpu->reset = psyz_reset;
    gpu->status = psyz_status;
    gpu->sync = GPU_Sync;
}

// these are not yet decompiled
int _addque2() { return 0; }
int _clr() { return 0; }
s32 _cwb() { return 0; }
int _cwc(OT_TYPE* p, int mask) { return 0; }
int _drs() { return 0; }
int _dws() { return 0; }
int _exeque() { return 0; }
void _otc(OT_TYPE* ot, s32 n) {}
int _reset(int mode) { return RETAIL_CONSOLE_TYPE; }
void _sync(int) {}
