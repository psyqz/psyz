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
u_char _que[0x1800];
u32 D_800E8640[0x10];

typedef enum {
    // https://psx-spx.consoledev.net/graphicsprocessingunitgpu/#gpu-versions
    GPU_V0, // 1MB VRAM, retail
    GPU_V1, // 2MB VRAM, arcade
    GPU_V2, // 2MB VRAM, PSone
} GpuVersion;

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

static int queue_len = 0;
static u_long queue_buf[0x4000];
static int GPU_Exeque() {
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
            // https://psx-spx.consoledev.net/graphicsprocessingunitgpu/#gp0e2h-texture-window-setting
            Draw_SetTextureWindow(
                (op & 0x1F) * 8 - 1, ((op >> 5) & 0x1F) * 8 - 1,
                ((op >> 10) & 0x1F) * 8, ((op >> 15) & 0x1F) * 8);
            break;
        case 0xE3:
            Draw_SetAreaStart((int)op & 0x3FF, (int)(op >> 10) & 0x3FF);
            break;
        case 0xE4:
            Draw_SetAreaEnd((int)op & 0x3FF, (int)(op >> 10) & 0x3FF);
            break;
        case 0xE5:
            Draw_SetOffset((int)op & 0x7FF, (int)(op >> 11) & 0x7FF);
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
    Draw_ExequeSync();
    queue_len = 0;
    return queue_len;
}
static int psyz_exeque();
int GPU_Enqueue(u_long p1, u_long p2) {
    int mask = (int)p2;
    if (mask) {
        WARNF("mask not supported (mask:%08X)", mask);
    }
    DR_ENV* env = (DR_ENV*)p1;
    while (true) {
        if (queue_len + env->len > LEN(queue_buf)) {
            INFOF("GPU queue full, calling exeque");
            psyz_exeque();
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
    return 0;
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
static void psyz_ctl(unsigned int cmd) {
    unsigned char op = (cmd >> 24) & 0x3F;
    switch (op) {
    case 0:
        Draw_Reset();
        break;
    case 3:
        Draw_DisplayEnable(!(cmd & 1));
        break;
    case 5:
        Draw_DisplayArea(cmd & 0x3FF, (cmd >> 10) & 0x3FF);
        break;
    case 6:
        Draw_DisplayHorizontalRange(cmd & 0xFFF, (cmd >> 12) & 0xFFF);
        break;
    case 7:
        Draw_DisplayVerticalRange(cmd & 0x3FF, (cmd >> 10) & 0x3FF);
        break;
    case 8:
        cmd &= 0xFFFFFF;
        Draw_SetDisplayMode((DisplayMode*)&cmd);
        break;
    default:
        WARNF("unhandled ctl %02X (%08X)", op, cmd);
        break;
    }
}
static int psyz_cwb() {
    NOT_IMPLEMENTED;
    return 0;
}
static int psyz_exeque() {
    return GPU_Exeque();
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
static int psyz_reset(int) { return GPU_V0; }
static u_long psyz_status(void) {
    NOT_IMPLEMENTED;
    return 0;
}
static int psyz_sync(int mode) {
    // see decomp/src/libgpu/sys.c
    // mode 0 waits until all the queue is drawn on screen
    // mode 1 process the queue and return how many elements have been queued
    // return -1 if GPU has timed out
    // but on PC the implementation is much simpler as it's always synced
    psyz_exeque();
    return 0;
}

int psyz_gpu_version(int mode) { return GPU_V0; }

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
    gpu->sync = psyz_sync;
}

// these are not yet decompiled
int _addque2() { return 0; }
int _exeque() { return 0; }
int get_alarm(void) { return 0; }
