#ifndef LIBGPU_H
#define LIBGPU_H
#include <types.h>

extern int (*GPU_printf)(); /* printf() object */

#define limitRange(x, l, h) ((x) = ((x) < (l) ? (l) : (x) > (h) ? (h) : (x)))

#define setRECT(r, _x, _y, _w, _h)                                             \
    (r)->x = (_x), (r)->y = (_y), (r)->w = (_w), (r)->h = (_h)

/*
 *	Set Primitive Attributes
 */
#define setTPage(p, tp, abr, x, y) ((p)->tpage = getTPage(tp, abr, x, y))

#define setClut(p, x, y) ((p)->clut = getClut(x, y))

/*
 * Set Primitive Colors
 */
#define setRGB0(p, _r0, _g0, _b0) (p)->r0 = _r0, (p)->g0 = _g0, (p)->b0 = _b0

#define setRGB1(p, _r1, _g1, _b1) (p)->r1 = _r1, (p)->g1 = _g1, (p)->b1 = _b1

#define setRGB2(p, _r2, _g2, _b2) (p)->r2 = _r2, (p)->g2 = _g2, (p)->b2 = _b2

#define setRGB3(p, _r3, _g3, _b3) (p)->r3 = _r3, (p)->g3 = _g3, (p)->b3 = _b3

/*
 * Set Primitive Screen Points
 */
#define setXY0(p, _x0, _y0) (p)->x0 = (_x0), (p)->y0 = (_y0)

#define setXY2(p, _x0, _y0, _x1, _y1)                                          \
    (p)->x0 = (_x0), (p)->y0 = (_y0), (p)->x1 = (_x1), (p)->y1 = (_y1)

#define setXY3(p, _x0, _y0, _x1, _y1, _x2, _y2)                                \
    (p)->x0 = (_x0), (p)->y0 = (_y0), (p)->x1 = (_x1), (p)->y1 = (_y1),        \
    (p)->x2 = (_x2), (p)->y2 = (_y2)

#define setXY4(p, _x0, _y0, _x1, _y1, _x2, _y2, _x3, _y3)                      \
    (p)->x0 = (_x0), (p)->y0 = (_y0), (p)->x1 = (_x1), (p)->y1 = (_y1),        \
    (p)->x2 = (_x2), (p)->y2 = (_y2), (p)->x3 = (_x3), (p)->y3 = (_y3)

#define setXYWH(p, _x0, _y0, _w, _h)                                           \
    (p)->x0 = (_x0), (p)->y0 = (_y0), (p)->x1 = (_x0) + (_w), (p)->y1 = (_y0), \
    (p)->x2 = (_x0), (p)->y2 = (_y0) + (_h), (p)->x3 = (_x0) + (_w),           \
    (p)->y3 = (_y0) + (_h)

/*
 * Set Primitive Width/Height
 */
#define setWH(p, _w, _h) (p)->w = _w, (p)->h = _h

/*
 * Set Primitive Texture Points
 */
#define setUV0(p, _u0, _v0) (p)->u0 = (_u0), (p)->v0 = (_v0)

#define setUV3(p, _u0, _v0, _u1, _v1, _u2, _v2)                                \
    (p)->u0 = (_u0), (p)->v0 = (_v0), (p)->u1 = (_u1), (p)->v1 = (_v1),        \
    (p)->u2 = (_u2), (p)->v2 = (_v2)

#define setUV4(p, _u0, _v0, _u1, _v1, _u2, _v2, _u3, _v3)                      \
    (p)->u0 = (_u0), (p)->v0 = (_v0), (p)->u1 = (_u1), (p)->v1 = (_v1),        \
    (p)->u2 = (_u2), (p)->v2 = (_v2), (p)->u3 = (_u3), (p)->v3 = (_v3)

#define setUVWH(p, _u0, _v0, _w, _h)                                           \
    (p)->u0 = (_u0), (p)->v0 = (_v0), (p)->u1 = (_u0) + (_w), (p)->v1 = (_v0), \
    (p)->u2 = (_u0), (p)->v2 = (_v0) + (_h), (p)->u3 = (_u0) + (_w),           \
    (p)->v3 = (_v0) + (_h)

/*
 * Primitive Handling Macros
 */
#define setlen(p, _len) (((P_TAG*)(p))->len = (u_char)(_len))
#define setaddr(p, _addr) (((P_TAG*)(p))->addr = (u_long)(_addr))
#define setcode(p, _code) (((P_TAG*)(p))->code = (u_char)(_code))

#define getlen(p) (u_char)(((P_TAG*)(p))->len)
#define getcode(p) (u_char)(((P_TAG*)(p))->code)
#define getaddr(p) (u_long)(((P_TAG*)(p))->addr)

#define nextPrim(p) (void*)((((P_TAG*)(p))->addr) | 0x80000000)
#ifdef __psyz
#undef nextPrim
#define nextPrim(p) (void*)(((P_TAG*)(p))->addr)
#endif

#define isendprim(p) ((((P_TAG*)(p))->addr) == 0xffffff)

#define addPrim(ot, p) setaddr(p, getaddr(ot)), setaddr(ot, p)
#define addPrims(ot, p0, p1) setaddr(p1, getaddr(ot)), setaddr(ot, p0)

#define catPrim(p0, p1) setaddr(p0, p1)
#define termPrim(p) setaddr(p, 0xffffff)

#define _get_mode(dfe, dtd, tpage)                                             \
    ((0xe1000000) | ((dtd) ? 0x0200 : 0) | ((dfe) ? 0x0400 : 0) |              \
     ((tpage) & 0x9ff))
#define setDrawTPage(p, dfe, dtd, tpage)                                       \
    setlen(p, 1), ((u_long*)(p))[1] = _get_mode(dfe, dtd, tpage)

#define setPolyF3(p) setlen(p, 4), setcode(p, 0x20)
#define setPolyFT3(p) setlen(p, 7), setcode(p, 0x24)
#define setPolyF4(p) setlen(p, 5), setcode(p, 0x28)
#define setPolyFT4(p) setlen(p, 9), setcode(p, 0x2c)
#define setPolyG3(p) setlen(p, 6), setcode(p, 0x30)
#define setPolyGT3(p) setlen(p, 9), setcode(p, 0x34)
#define setPolyG4(p) setlen(p, 8), setcode(p, 0x38)
#define setPolyGT4(p) setlen(p, 12), setcode(p, 0x3c)
#define setLineF2(p) setlen(p, 3), setcode(p, 0x40)
#define setLineF3(p) setlen(p, 5), setcode(p, 0x48), (p)->pad = 0x55555555
#define setLineF4(p) setlen(p, 6), setcode(p, 0x4c), (p)->pad = 0x55555555
#define setLineG2(p) setlen(p, 4), setcode(p, 0x50)
#define setLineG3(p)                                                           \
    setlen(p, 7), setcode(p, 0x58), (p)->pad = 0x55555555, (p)->p2 = 0
#define setLineG4(p)                                                           \
    setlen(p, 9), setcode(p, 0x5c),                                            \
        (p)->pad = 0x55555555, (p)->p2 = 0, (p)->p3 = 0
#define setTile(p) setlen(p, 3), setcode(p, 0x60)
#define setSprt(p) setlen(p, 4), setcode(p, 0x64)
#define setTile1(p) setlen(p, 2), setcode(p, 0x68)
#define setTile8(p) setlen(p, 2), setcode(p, 0x70)
#define setSprt8(p) setlen(p, 3), setcode(p, 0x74)
#define setTile16(p) setlen(p, 2), setcode(p, 0x78)
#define setSprt16(p) setlen(p, 3), setcode(p, 0x7c)

#define setSemiTrans(p, abe)                                                   \
    ((abe) ? setcode(p, getcode(p) | 0x02) : setcode(p, getcode(p) & ~0x02))

#define setShadeTex(p, tge)                                                    \
    ((tge) ? setcode(p, getcode(p) | 0x01) : setcode(p, getcode(p) & ~0x01))

#define getTPage(tp, abr, x, y)                                                \
    ((((tp) & 0x3) << 7) | (((abr) & 0x3) << 5) | (((y) & 0x100) >> 4) |       \
     (((x) & 0x3ff) >> 6) | (((y) & 0x200) << 2))

#define getClut(x, y) ((y << 6) | ((x >> 4) & 0x3f))

typedef struct {
    /* 0x0 */ short x;
    /* 0x2 */ short y; /* offset point on VRAM */
    /* 0x4 */ short w; /* width */
    /* 0x6 */ short h; /* height */
} RECT;                /* size = 0x8 */

#ifdef __psyz
#define O_TAG                                                                  \
    u_long tag;                                                                \
    u_long len
typedef struct {
    O_TAG;
} OT_TYPE;
#else
#define O_TAG u_long tag
#define OT_TYPE u_long
#endif

typedef struct {
#ifndef __psyz
    unsigned addr : 24;
    unsigned len : 8;
#else
    u_long addr;
    u_long len;
#endif
    u_char r0, g0, b0, code;
} P_TAG;

typedef struct {
    u_char r0, g0, b0, code;
} P_CODE;

typedef struct {
    /* 0x00 */ O_TAG;
    /* 0x4 */ u_long code[15];
} DR_ENV; /* Packed Drawing Environment, size = 0x40 */

typedef struct {
    O_TAG;
    u_char r0, g0, b0, code;
    short x0, y0;
    short x1, y1;
    short x2, y2;
} POLY_F3; // 0x20

typedef struct {
    O_TAG;
    u_char r0, g0, b0, code;
    short x0, y0;
    u_char u0, v0;
    u_short clut;
    short x1, y1;
    u_char u1, v1;
    u_short tpage;
    short x2, y2;
    u_char u2, v2;
    u_short pad1;
} POLY_FT3; // 0x24

typedef struct {
    O_TAG;
    u_char r0, g0, b0, code;
    short x0, y0;
    short x1, y1;
    short x2, y2;
    short x3, y3;
} POLY_F4; // 0x28

typedef struct {
    O_TAG;
    u_char r0, g0, b0, code;
    short x0, y0;
    u_char u0, v0;
    u_short clut;
    short x1, y1;
    u_char u1, v1;
    u_short tpage;
    short x2, y2;
    u_char u2, v2;
    u_short pad1;
    short x3, y3;
    u_char u3, v3;
    u_short pad2;
} POLY_FT4; // 0x2C

typedef struct {
    O_TAG;
    u_char r0, g0, b0, code;
    short x0, y0;
    u_char r1, g1, b1, pad1;
    short x1, y1;
    u_char r2, g2, b2, pad2;
    short x2, y2;
} POLY_G3; // 0x30

typedef struct {
    O_TAG;
    u_char r0;
    u_char g0;
    u_char b0;
    u_char code;
    short x0;
    short y0;
    u_char u0;
    u_char v0;
    u_short clut;
    u_char r1;
    u_char g1;
    u_char b1;
    u_char p1;
    short x1;
    short y1;
    u_char u1;
    u_char v1;
    u_short tpage;
    u_char r2;
    u_char g2;
    u_char b2;
    u_char p2;
    short x2;
    short y2;
    u_char u2;
    u_char v2;
    u_short pad2;
} POLY_GT3; // 0x34

typedef struct {
    O_TAG;
    u_char r0;
    u_char g0;
    u_char b0;
    u_char code;
    short x0;
    short y0;
    u_char r1;
    u_char g1;
    u_char b1;
    u_char pad1;
    short x1;
    short y1;
    u_char r2;
    u_char g2;
    u_char b2;
    u_char pad2;
    short x2;
    short y2;
    u_char r3;
    u_char g3;
    u_char b3;
    u_char pad3;
    short x3;
    short y3;
} POLY_G4; // 0x38

typedef struct {
    O_TAG;
    u_char r0;
    u_char g0;
    u_char b0;
    u_char code;
    short x0;
    short y0;
    u_char u0;
    u_char v0;
    u_short clut;
    u_char r1;
    u_char g1;
    u_char b1;
    u_char p1;
    short x1;
    short y1;
    u_char u1;
    u_char v1;
    u_short tpage;
    u_char r2;
    u_char g2;
    u_char b2;
    u_char p2;
    short x2;
    short y2;
    u_char u2;
    u_char v2;
    u_short pad2;
    u_char r3;
    u_char g3;
    u_char b3;
    u_char p3;
    short x3;
    short y3;
    u_char u3;
    u_char v3;
    u_short pad3;
} POLY_GT4; // 0x3C

typedef struct {
    O_TAG;
    u_char r0, g0, b0, code;
    short x0, y0;
    short x1, y1;
} LINE_F2; // 0x40

typedef struct {
    O_TAG;
    u_char r0, g0, b0, code;
    short x0, y0;
    short x1, y1;
    short x2, y2;
    u_long pad;
} LINE_F3; // 0x48

typedef struct {
    O_TAG;
    u_char r0, g0, b0, code;
    short x0, y0;
    short x1, y1;
    short x2, y2;
    short x3, y3;
    u_long pad;
} LINE_F4; // 0x4C

typedef struct {
    O_TAG;
    u_char r0, g0, b0, code;
    short x0, y0;
    u_char r1, g1, b1, p1;
    short x1, y1;
} LINE_G2; // 0x50

typedef struct {
    O_TAG;
    u_char r0, g0, b0, code;
    short x0, y0;
    u_char r1, g1, b1, p1;
    short x1, y1;
    u_char r2, g2, b2, p2;
    short x2, y2;
    u_long pad;
} LINE_G3; // 0x58

typedef struct {
    O_TAG;
    u_char r0, g0, b0, code;
    short x0, y0;
    u_char r1, g1, b1, p1;
    short x1, y1;
    u_char r2, g2, b2, p2;
    short x2, y2;
    u_char r3, g3, b3, p3;
    short x3, y3;
    u_long pad;
} LINE_G4; // 0x5C

typedef struct {
    O_TAG;
    u_char r0, g0, b0, code;
    short x0, y0;
    short w, h;
} TILE; // 0x60

typedef struct {
    O_TAG;
    u_char r0;
    u_char g0;
    u_char b0;
    u_char code;
    short x0;
    short y0;
    u_char u0;
    u_char v0;
    u_short clut;
    short w;
    short h;
} SPRT; // 0x64

typedef struct {
    O_TAG;
    u_char r0, g0, b0, code;
    short x0, y0;
} TILE_1; // 0x68

typedef struct {
    O_TAG;
    u_char r0, g0, b0, code;
    short x0, y0;
} TILE_8; // 0x70

typedef struct {
    O_TAG;
    u_char r0, g0, b0, code;
    short x0, y0;
    u_char u0, v0;
    u_short clut;
} SPRT_8; // 0x74

typedef struct {
    O_TAG;
    u_char r0, g0, b0, code;
    short x0, y0;
} TILE_16; // 0x78

typedef struct {
    O_TAG;
    u_char r0;
    u_char g0;
    u_char b0;
    u_char code;
    short x0;
    short y0;
    u_char u0;
    u_char v0;
    u_short clut;
} SPRT_16; // 0x7C

/*
 *  Special Primitive Definitions
 */
typedef struct {
    O_TAG;
    u_long code[2];
} DR_MODE; /* Drawing Mode */

typedef struct {
    O_TAG;
    u_long code[2];
} DR_TWIN; /* Texture Window */

typedef struct {
    O_TAG;
    u_long code[2];
} DR_AREA; /* Drawing Area */

typedef struct {
    O_TAG;
    u_long code[2];
} DR_OFFSET; /* Drawing Offset */

typedef struct { /* MoveImage */
    O_TAG;
    u_long code[5];
} DR_MOVE;

typedef struct { /* LoadImage */
    O_TAG;
    u_long code[3];
    u_long p[13];
} DR_LOAD;

typedef struct {
    O_TAG;
    u_long code[1];
} DR_TPAGE; /* Drawing TPage */

/*
 *	Font Stream Parameters
 */
#define FNT_MAX_ID 8      /* max number of stream ID */
#define FNT_MAX_SPRT 1024 /* max number of sprites in all streams */

typedef struct {
    /* 0x00 */ RECT clip;     /* clip area */
    /* 0x08 */ short ofs[2];  /* drawing offset */
    /* 0x0C */ RECT tw;       /* texture window */
    /* 0x14 */ u_short tpage; /* texture page */
    /* 0x16 */ u_char dtd;    /* dither flag (0:off, 1:on) */
    /* 0x17 */ u_char dfe;    /* flag to draw on display area (0:off 1:on) */
    /* 0x18 */ u_char isbg;   /* enable to auto-clear */
    /* 0x19 */ u_char r0, g0, b0; /* initital background color */
    /* 0x1C */ DR_ENV dr_env;     /* reserved */
} DRAWENV;                        /* size = 0x58 */

typedef struct {
    /* 0x00 */ RECT disp;      /* display area */
    /* 0x08 */ RECT screen;    /* display start point */
    /* 0x10 */ u_char isinter; /* interlace 0: off 1: on */
    /* 0x11 */ u_char isrgb24; /* RGB24 bit mode */
    /* 0x12 */ u_char pad0;    /* reserved */
    /* 0x13 */ u_char pad1;    /* reserved */
} DISPENV;                     /* size = 0x14 */

typedef struct PixPattern {
    u8 w;
    u8 h;
    u8 x;
    u8 y;
} PixPattern;

/*
 *	Multi-purpose TIM image
 */
typedef struct {
    u_long mode;   /* pixel mode */
    RECT* crect;   /* CLUT rectangle on frame buffer */
    u_long* caddr; /* CLUT address on main memory */
    RECT* prect;   /* texture image rectangle on frame buffer */
    u_long* paddr; /* texture image address on main memory */
} TIM_IMAGE;

/*
 * Loads a texture pattern from the memory area starting at the address pix into
 * the frame buffer area starting at the address (x, y), and calculates the
 * texture page ID for the loaded texture pattern. The texture pattern size w
 * represents the number of pixels, not the actual size of the transfer area in
 * the frame buffer.
 */
extern u_short LoadTPage(
    u_long* pix,  // Pointer to texture pattern start address
    int tp,       // Bit depth (0 = 4-bit; 1 = 8-bit; 2 = 16-bit)
    int abr,      // Semitransparency rate
    int x, int y, // Destination frame buffer address
    int w, int h  // Texture pattern size
);

extern u_short LoadClut(u_long* clut, int x, int y);
extern u_short LoadClut2(u_long* clut, int x, int y);

/**
 * Calculates and returns the texture CLUT ID.
 * The CLUT address is limited to multiples of 16 in the x direction
 */
u_short GetClut(int x, // Horizontal frame buffer address of CLUT
                int y  // Vertical frame buffer address of CLUT
);

u_short GetTPage(int tp, int abr, int x, int y);
extern void* NextPrim(void* p);
extern void AddPrim(void* ot, void* p);
extern void AddPrims(void* ot, void* p0, void* p1);
extern void CatPrim(void* p0, void* p1);
extern void DrawOTag(OT_TYPE* p);
extern void DrawOTagIO(OT_TYPE* p);
extern void DrawOTagEnv(OT_TYPE* p, DRAWENV* env);
extern void DrawPrim(void* p);
extern void DumpClut(u_short clut);
extern void DumpDispEnv(DISPENV* env);
extern void DumpDrawEnv(DRAWENV* env);
extern void DumpOTag(OT_TYPE* p);
extern void DumpTPage(u_short tpage);
extern void FntLoad(int tx, int ty);
extern void SetDispMask(int mask);
extern void SetDrawArea(DR_AREA* p, RECT* r);
extern void SetDrawEnv(DR_ENV* dr_env, DRAWENV* env);
extern void SetDrawLoad(DR_LOAD* p, RECT* rect);
extern void SetDrawMode(DR_MODE* p, int dfe, int dtd, int tpage, RECT* tw);
extern void SetDrawTPage(DR_TPAGE* p, int dfe, int dtd, int tpage);
extern void SetDrawMove(DR_MOVE* p, RECT* rect, int x, int y);
extern void SetDrawOffset(DR_OFFSET* p, u_short* ofs);
extern void SetDumpFnt(int id);
extern void SetLineF2(LINE_F2* p);
extern void SetLineF3(LINE_F3* p);
extern void SetLineF4(LINE_F4* p);
extern void SetLineG2(LINE_G2* p);
extern void SetLineG3(LINE_G3* p);
extern void SetLineG4(LINE_G4* p);
extern void SetPolyF3(POLY_F3* p);
extern void SetPolyF4(POLY_F4* p);
extern void SetPolyFT3(POLY_FT3* p);
extern void SetPolyFT4(POLY_FT4* p);
extern void SetPolyG3(POLY_G3* p);
extern void SetPolyG4(POLY_G4* p);
extern void SetPolyGT3(POLY_GT3* p);
extern void SetPolyGT4(POLY_GT4* p);
extern void SetSemiTrans(void* p, int abe);
extern void SetShadeTex(void* p, int tge);
extern void SetSprt(SPRT* p);
extern void SetSprt16(SPRT_16* p);
extern void SetSprt8(SPRT_8* p);
extern void SetTexWindow(DR_TWIN* p, RECT* tw);
extern void SetTile(TILE* p);
extern void SetTile1(TILE_1* p);
extern void SetTile16(TILE_16* p);
extern void SetTile8(TILE_8* p);
extern void TermPrim(void* p);
extern int ResetGraph(int mode);
int SetGraphDebug(int level);
extern int SetGraphReverse(int mode);
extern int SetGraphQueue(int mode);
extern u_long DrawSyncCallback(void (*func)());
extern void FntLoad(int tx, int ty);
int FntPrint(const char* fmt, ...);
extern void SetDispMask(int mask);
extern void SetDrawMode(DR_MODE* p, int dfe, int dtd, int tpage, RECT* tw);
extern void SetDumpFnt(int id);
extern int CheckPrim(char* s, OT_TYPE* p);
extern int ClearImage(RECT* rect, u_char r, u_char g, u_char b);
extern int DrawSync(int mode);
extern int FntOpen(int x, int y, int w, int h, int isbg, int n);
extern int GetGraphDebug(void);
extern u_long* FntFlush(int id);
extern int KanjiFntOpen(int x, int y, int w, int h, int dx, int dy, int cx,
                        int cy, int isbg, int n);
extern int LoadImage(RECT* rect, u_long* p);
extern int MargePrim(void* p0, void* p1);
extern int StoreImage(RECT* rect, u_long* p);
extern int MoveImage(RECT* rect, int x, int y);
extern int OpenTIM(u_long* addr);
extern OT_TYPE* ClearOTag(OT_TYPE* ot, int n);
extern OT_TYPE* ClearOTagR(OT_TYPE* ot, int n);
extern DRAWENV* PutDrawEnv(DRAWENV* env);
extern DISPENV* PutDispEnv(DISPENV* env);
extern DISPENV* SetDefDispEnv(DISPENV* env, int x, int y, int w, int h);
extern DRAWENV* SetDefDrawEnv(DRAWENV* env, int x, int y, int w, int h);
extern TIM_IMAGE* ReadTIM(TIM_IMAGE* timimg);

#endif