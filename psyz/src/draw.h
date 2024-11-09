#ifndef DRAW_H
#define DRAW_H
#include <common.h>
#include <libgpu.h>

// ===== GPU backend =====
// as long as the platform implements these API calls, psy-z will work natively
//
// these API are mostly a personal reinterpretation of the following docs:
// https://psx-spx.consoledev.net/graphicsprocessingunitgpu

typedef struct {
    u_long tex_x : 4;
    u_long tex_y : 1;
    u_long semi_transparency : 2;
    u_long bpp : 2;    // this might be wrong? flipped with semi_transparency?
    u_long dither : 1; // 0: disable dither on 16-bit, 1: it is permitted
    u_long dfe : 1;    // 0: drawing to display area is blocked, 1: is permitted
    u_long tex_y_extra_vram : 1;
    u_long tex_flip_x : 1;
    u_long tex_flip_y : 1;
} ParamDrawTexpageMode;
void Draw_SetTexpageMode(ParamDrawTexpageMode* p);
void Draw_SetTextureWindow(int mask_x, int mask_y, int off_x, int off_y);
void Draw_SetAreaStart(int x, int y);
void Draw_SetAreaEnd(int x, int y);
void Draw_SetOffset(int x, int y);
void Draw_ClearImage(RECT* rect, u_char r, u_char g, u_char b);
void Draw_LoadImage(RECT* rect, u_long* p);
void Draw_StoreImage(RECT* rect, u_long* p);
void Draw_PutDispEnv(DISPENV* disp);
void Draw_ResetBuffer(void);
void Draw_FlushBuffer(void);
int Draw_PushPrim(u_long* packets, int max_len);
int Draw_Sync(int mode);

#endif
