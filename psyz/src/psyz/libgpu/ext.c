#include <psyz.h>
#include <libgpu.h>

u_short LoadTPage(u_long* pix, int tp, int abr, int x, int y, int w, int h) {
    RECT rect = {(short)x, (short)y, (short)w, (short)h};

    switch (tp) {
    case 0:
        rect.w /= 4;
        break;
    case 1:
        rect.w /= 2;
        break;
    case 2:
        break;
    }
    LoadImage(&rect, pix);
    return GetTPage(tp, abr, x, y);
}

u_short LoadClut(u_long* clut, int x, int y) {
    return LoadClut2(clut, x, y);
}

u_short LoadClut2(u_long* clut, int x, int y) {
    RECT rect = {x, y, 16, 1};
    LoadImage(&rect, clut);
    return GetClut(x, y);
}

DRAWENV* SetDefDrawEnv(DRAWENV* env, int x, int y, int w, int h) {
    env->clip.x = (short)x;
    env->clip.y = (short)y;
    env->clip.w = (short)w;
    env->clip.h = (short)h;
    env->ofs[0] = (short)x;
    env->ofs[1] = (short)y;
    env->tw.x = 0;
    env->tw.y = 0;
    env->tw.w = 0;
    env->tw.h = 0;
    env->r0 = 0;
    env->g0 = 0;
    env->b0 = 0;
    env->dtd = 1;
    env->dfe = h != 480;
    env->tpage = GetTPage(0, 0, 640, 0);
    env->isbg = 0;
    return env;
}

DISPENV* SetDefDispEnv(DISPENV* env, int x, int y, int w, int h) {
    env->disp.x = (short)x;
    env->disp.y = (short)y;
    env->disp.w = (short)w;
    env->screen.x = 0;
    env->screen.y = 0;
    env->screen.w = 0;
    env->screen.h = 0;
    env->isrgb24 = 0;
    env->isinter = 0;
    env->pad1 = 0;
    env->pad0 = 0;
    env->disp.h = h;
    return env;
}
