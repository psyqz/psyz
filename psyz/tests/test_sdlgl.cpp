
#include <gtest/gtest.h>
#include <GLES2/gl2.h>
#include <SDL3/SDL_opengl.h>
extern "C" {
#include "common.h"
#include "kernel.h"
#include "libetc.h"
#include "libgpu.h"
void Psyz_SetWindowScale(int scale);
void Psyz_GetWindowSize(int* width, int* height);
}

#include "res/4bpp.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_ONLY_PNG
#define STBI_SUPPORT_ZLIB
#define STBI_MAX_DIMENSIONS 1024
#include "stb_image.h"
#include "stb_image_write.h"

class SDLGL_Test : public testing::Test {
    static float img_eq(
        const unsigned char* a, const unsigned char* b, const size_t len) {
        size_t matches = 0;
        for (size_t i = 0; i < len; ++i) {
            // normalize both images to RGB5551
            const int l = static_cast<int>(a[i]) & 0xF8;
            const int r = static_cast<int>(b[i]) & 0xF8;
            if (std::abs(l - r) == 0)
                matches++;
        }
        return static_cast<float>(matches) / static_cast<float>(len);
    }

  protected:
    static const int OT_LENGTH = 1;
    static const int OTSIZE = 1 << OT_LENGTH;
    static const int SCREEN_WIDTH = 256;
    static const int SCREEN_HEIGHT = 240;
    typedef struct DB {
        DRAWENV draw;
        DISPENV disp;
        OT_TYPE ot[OTSIZE];
        POLY_FT4 ft4[4];
        POLY_GT4 gt4[4];
        SPRT sprt[4];
    } DB;
    DB db[2];
    DB* cdb;

    void SetUp() override {
        Psyz_SetWindowScale(1);
        SetDefDrawEnv(&db[0].draw, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        SetDefDrawEnv(
            &db[1].draw, SCREEN_WIDTH, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        SetDefDispEnv(&db[0].disp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        SetDefDispEnv(
            &db[1].disp, SCREEN_WIDTH, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        SetVideoMode(MODE_NTSC);
        ResetGraph(0);
        PutDrawEnv(&db[0].draw);
        PutDispEnv(&db[0].disp);
        ClearOTagR(db[0].ot, OTSIZE);
        ClearOTagR(db[1].ot, OTSIZE);
        SetDispMask(1);
        RECT clearRect = {0, 0, 0x7FFF, 0x7FFF};
        ClearImage(&clearRect, 0, 0, 0);
        cdb = &db[0];
    }
    void TearDown() override { ResetGraph(0); }

    static unsigned char* AllocAndCaptureFrame(int* w, int* h) {
        const int channels = 3;
        Psyz_GetWindowSize(w, h);
        auto* pixels = static_cast<unsigned char*>(malloc(*w * *h * channels));
        glReadPixels(0, 0, *w, *h, GL_RGB, GL_UNSIGNED_BYTE, pixels);
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            free(pixels);
            return nullptr;
        }
        int stride = *w * channels;
        std::vector<unsigned char> row(stride);
        for (int y = 0; y < *h / 2; ++y) {
            unsigned char* top = pixels + y * stride;
            unsigned char* bottom = pixels + (*h - 1 - y) * stride;
            memcpy(row.data(), top, stride);
            memcpy(top, bottom, stride);
            memcpy(bottom, row.data(), stride);
        }
        return pixels;
    }
    static void WriteToFile(const char* filename, void* data, size_t len) {
        FILE* f = fopen(filename, "wb");
        ASSERT_TRUE(f != nullptr);
        fwrite(data, 1, len, f);
        fclose(f);
    }
    static void AssertFrame(const char* png_path, float precision = 0.98f) {
        char filename[FILENAME_MAX];
        int exp_w, exp_h, act_w, act_h, ch;
        snprintf(filename, sizeof(filename), "../expected/%s.png", png_path);
        unsigned char* exp_d = stbi_load(filename, &exp_w, &exp_h, &ch, 3);
        ch = 3;
        ASSERT_NE(exp_d, nullptr);
        unsigned char* act_d = AllocAndCaptureFrame(&act_w, &act_h);
        ASSERT_NE(act_d, nullptr);
        ASSERT_EQ(exp_w, act_w);
        ASSERT_EQ(exp_h, act_h);
        auto eq = img_eq(exp_d, act_d, exp_w * exp_h * ch);
        if (eq < precision) {
            snprintf(filename, sizeof(filename), "../expected/%s.actual.png",
                     png_path);
            stbi_write_png(filename, act_w, act_h, ch, act_d, act_w * ch);
            snprintf(filename, sizeof(filename), "../expected/%s.expected.bin",
                     png_path);
            WriteToFile(filename, exp_d, act_w * act_h * ch);
            snprintf(filename, sizeof(filename), "../expected/%s.actual.bin",
                     png_path);
            WriteToFile(filename, act_d, act_w * act_h * ch);
        }
        ASSERT_GE(eq, precision);
        stbi_image_free(exp_d);
        free(act_d);
    }

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
};

TEST_F(SDLGL_Test, fnt_print) {
    FntLoad(960, 256);
    SetDumpFnt(FntOpen(4, 4, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 512));
    ClearImage(&cdb->draw.clip, 60, 120, 120);
    FntPrint("hello psyz!");
    FntFlush(-1);
    DrawSync(0);
    VSync(0);
    PutDrawEnv(&cdb->draw);
    PutDispEnv(&cdb->disp);
    AssertFrame("fnt_print");
}

TEST_F(SDLGL_Test, draw_ft4) {
    u_short tpage, clut;
    if (LoadTim(img_4bpp, &tpage, &clut)) {
        return;
    }
    SetPolyFT4(&cdb->ft4[0]);
    setXYWH(&cdb->ft4[0], 16, 16, 64, 64);
    setRGB0(&cdb->ft4[0], 128, 128, 128);
    setUVWH(&cdb->ft4[0], 0, 0, 64, 64);
    setSemiTrans(&cdb->ft4[0], 0);
    cdb->ft4[0].tpage = tpage;
    cdb->ft4[0].clut = clut;

    ClearOTag(cdb->ot, OTSIZE);
    AddPrim(cdb->ot, &db[0].ft4[0]);

    ClearImage(&cdb->draw.clip, 60, 120, 120);
    DrawOTag(cdb->ot);
    DrawSync(0);
    VSync(0);
    PutDispEnv(&cdb->disp);
    AssertFrame("draw_ft4");
}

TEST_F(SDLGL_Test, draw_ft4_colored) {
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

    ClearOTag(cdb->ot, OTSIZE);
    AddPrim(cdb->ot, &db[0].ft4[0]);

    ClearImage(&cdb->draw.clip, 60, 120, 120);
    DrawOTag(cdb->ot);
    DrawSync(0);
    VSync(0);
    PutDispEnv(&cdb->disp);
    AssertFrame("draw_ft4_colored");
}

TEST_F(SDLGL_Test, draw_gt4) {
    u_short tpage, clut;
    if (LoadTim(img_4bpp, &tpage, &clut)) {
        return;
    }
    SetPolyGT4(&cdb->gt4[0]);
    setXYWH(&cdb->gt4[0], 16, 16, 64, 64);
    setRGB0(&cdb->gt4[0], 128, 0, 0);
    setRGB1(&cdb->gt4[0], 0, 128, 0);
    setRGB2(&cdb->gt4[0], 0, 0, 128);
    setRGB3(&cdb->gt4[0], 128, 128, 0);
    setUVWH(&cdb->gt4[0], 0, 0, 64, 64);
    setSemiTrans(&cdb->gt4[0], 0);
    cdb->gt4[0].tpage = tpage;
    cdb->gt4[0].clut = clut;

    ClearOTag(cdb->ot, OTSIZE);
    AddPrim(cdb->ot, &db[0].gt4[0]);

    ClearImage(&cdb->draw.clip, 60, 120, 120);
    DrawOTag(cdb->ot);
    DrawSync(0);
    VSync(0);
    PutDispEnv(&cdb->disp);
    AssertFrame("draw_gt4");
}

TEST_F(SDLGL_Test, set_draw_area) {
    u_short tpage, clut;
    if (LoadTim(img_4bpp, &tpage, &clut)) {
        return;
    }

    SetPolyFT4(&cdb->ft4[0]);
    AddPrim(cdb->ot, &cdb->ft4[0]);
    setXYWH(&cdb->ft4[0], 0, 0, 64, 64);
    setRGB0(&cdb->ft4[0], 128, 128, 128);
    setUVWH(&cdb->ft4[0], 0, 0, 64, 64);
    setSemiTrans(&cdb->ft4[0], 0);
    cdb->ft4[0].tpage = tpage;
    cdb->ft4[0].clut = clut;

    DR_AREA drArea;
    AddPrim(cdb->ot, &drArea);
    RECT area = {4, 8, 56, 48};
    SetDrawArea(&drArea, &area);

    setRECT(&cdb->draw.clip, 32, 24, 160, 128);
    ClearImage(&cdb->draw.clip, 60, 120, 120);
    setRECT(&cdb->draw.clip, 128, 128, 64, 64);
    ClearImage(&cdb->draw.clip, 120, 120, 60);

    DrawOTag(cdb->ot);
    DrawSync(0);
    VSync(0);
    PutDispEnv(&cdb->disp);
    AssertFrame("set_draw_area");
}

TEST_F(SDLGL_Test, swap_buffer) {
    u_short tpage, clut;
    if (LoadTim(img_4bpp, &tpage, &clut)) {
        return;
    }

    for (int fbidx = 0; fbidx < 2; fbidx++) {
        cdb = &db[fbidx&1];
        cdb->draw.isbg = 1;
        cdb->draw.tpage = tpage;
        setRGB0(&cdb->draw, 60, 120, 120);
        PutDrawEnv(&cdb->draw);
        PutDispEnv(&cdb->disp);

        SetSprt(cdb->sprt);
        SetSemiTrans(cdb->sprt, 0);
        SetShadeTex(cdb->sprt, 1);
        setXY0(cdb->sprt, 0, fbidx * 16);
        setWH(cdb->sprt, 64, 64);
        setUV0(cdb->sprt, 0, 0);
        cdb->sprt[0].clut = clut;
        ClearOTag(cdb->ot, OTSIZE);
        AddPrim(cdb->ot, cdb->sprt);
        DrawOTag(cdb->ot);

        DrawSync(0);
        VSync(0);
        AssertFrame((fbidx & 1) ? "swap_buffer_fb2" : "swap_buffer_fb1");
    }
}
