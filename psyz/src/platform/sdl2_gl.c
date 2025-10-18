#include "psyz.h"

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>

#ifdef _MSC_VER
#include <SDL.h>
#include "glad/glad.h"

#elif defined(__MINGW32__)
#include <SDL3/SDL.h>
#include "glad/glad.h"

#else
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <GLES3/gl3.h>

#endif

// HACK to avoid conflicting with RECT from windef.h
// It renames the libgpu RECT into PS1_RECT, ensuring the windef struct RECT
// does not conflict. The hack is applied to all platform for consistency.
#define RECT PS1_RECT
#include "libgpu.h"
#include "../draw.h"
#undef RECT

static const char gl33_vertex_shader[] = {
    "#version 330 core\n"
    "layout(location = 0) in vec2 pos;\n"
    "layout(location = 1) in vec4 tex;\n"
    "layout(location = 2) in vec4 color;\n"
    "uniform vec2 resolution;\n"
    "out vec4 vertexColor;\n"
    "out vec2 texCoord;\n"
    "flat out uint tpage;\n"
    "flat out uint clut;\n"
    "\n"
    "void main() {\n"
    "    float x = (pos.x / (resolution.x / 2.0)) - 1.0;\n"
    "    float y = 1.0 - (pos.y / (resolution.y / 2.0));\n"
    "    gl_Position = vec4(x, y, 0.0, 1.0);\n"
    // gouraud colors
    "    vertexColor = color * vec4(2, 2, 2, 1);\n"
    // select the right texture coords based on the tpage
    "    clut = uint(tex.z);\n"
    "    tpage = uint(tex.w);\n"
    "    texCoord = vec2(tex.x / 4096.0, tex.y / 512.0);\n"
    "    if ((tpage & 0x100u) != 0u) {\n"
    "        texCoord.x *= 4;"
    "    } else if ((tpage & 0x80u) != 0u) {\n"
    "        texCoord.x *= 2;"
    "    }\n"
    "    vec2 page = vec2(\n"
    "        float((tpage % 32u) % 16u) / 16.0,\n"
    "        float((tpage % 32u) / 16u) / 2.0);\n"
    "    texCoord += page;\n"
    "}\n"};

static const char gl33_fragment_shader[] = {
    "#version 330 core\n"
    "in vec4 vertexColor;\n"
    "in vec2 texCoord;\n"
    "out vec4 FragColor;\n"
    "flat in uint clut;\n"
    "flat in uint tpage;\n"
    "uniform sampler2D tex4;\n"
    "uniform sampler2D tex8;\n"
    "uniform sampler2D tex16;\n"
    "uniform sampler2D textureSampler;\n"
    "\n"
    "vec2 getPsxClutXY(uint clut) {\n"
    "    return vec2(\n"
    "        float((clut % 64u) * 16u) / 1024.0f,\n"
    "        float(clut / 64u) / 512.0f);\n"
    "}\n"
    "\n"
    "void main() {\n"
    "    vec4 texColor;\n"
    "    if (tpage == 0xFFFFu) {\n"
    "        texColor = vec4(1, 1, 1, 2);\n"
    "    } else if ((tpage & 0x180u) >= 0x100u) {\n" // 16-bit
    "        texColor = texture(tex16, texCoord);\n"
    "    } else {\n"
    "        float colIdx;\n"
    "        if (tpage >= 0x80u) {\n" // 8-bit
    "            colIdx = texture(tex8, texCoord).r;\n"
    "        } else {\n" // 4-bit
    "            colIdx = texture(tex4, texCoord).r;\n"
    "        }\n"
    "        float colorIdx = floor(colIdx * 256.0) / 1024.0f;\n"
    "        vec2 colorCoord = getPsxClutXY(clut) + vec2(colorIdx, 0);\n"
    "        texColor = texture(tex16, colorCoord);\n"
    "    }\n"
    // check for full transparency
    "    bool colorDiscard = texColor == vec4(0, 0, 0, 0);"
    "    if (!colorDiscard) {\n"
    // check for setSemiTrans(p, 1)
    "        bool isSemiTrans = vertexColor.a < 0.75;"
    // when a color has the 0x8000 bit left then it has the semitrans flag on
    "        bool colorSemiTrans = texColor.a > 0;"
    "        if (colorSemiTrans && isSemiTrans) {\n"
    "            uint abr = (tpage & 0x60u) >> 5u;\n"
    "            if (abr == 0u) {\n"
    "                texColor.a = 1;\n" // 50% opacity
    "            } else if (abr == 1u) {\n"
    "                texColor.a = 1;\n" // TODO additive blending
    "            } else if (abr == 2u) {\n"
    "                texColor.a = 1;\n"   // TODO subtractive blending
    "            } else {\n"              // abr == 3u
    "                texColor.a = 0.5;\n" // 25% opacity?
    "            }\n"
    "        } else {\n"
    "            texColor.a = 2;\n" // full opacity
    "        }\n"
    "    } else {\n"
    "        texColor.a = 0;\n"
    "    }\n"
    "    FragColor = texColor * vertexColor;\n"
    "}\n"};

typedef struct {
    short x, y;
    unsigned short u, v, c, t;
    unsigned char r, g, b, a;
} Vertex;
typedef struct {
    GLint x, y, w, h;
} GLrecti;
typedef struct {
    GLint x, y;
} GLposi; // this is custom

#define VRGBA(p) (*(unsigned int*)(&((p).r)))
#define SET_TC(p, tpage, clut) (p)->t = (u16)tpage, (p)->c = (u16)clut;
#define SET_TC_ALL(p, t, c)                                                    \
    SET_TC(p, t, c) SET_TC(&p[1], t, c) SET_TC(&p[2], t, c) SET_TC(&p[3], t, c)

enum TexKind {
    Tex_4bpp,
    Tex_8bpp,
    Tex_16bpp,
    Num_Tex,
};

static u16 g_RawVram[VRAM_W * VRAM_H];
static SDL_Window* window = NULL;
static SDL_GLContext glContext = NULL;
static GLuint fb[2] = {0, 0};
static GLuint fbtex[2] = {0, 0};
static PS1_RECT fbrect[2] = {{0}, {0}};
static unsigned int fb_index = 0;
static GLuint shader_program = 0;
static Uint32 elapsed_from_beginning = 0;
static Uint32 last_vsync = 0;
static u_short cur_tpage = 0;
static GLint uniform_resolution = 0;
static GLint uniform_tex_4bpp = 0;
static GLint uniform_tex_8bpp = 0;
static GLint uniform_tex_16bpp = 0;
static GLuint vram_textures[Num_Tex];
static bool is_vram_texture_invalid = false;
static SDL_AudioStream* audio_stream = NULL;
static SDL_AudioDeviceID audio_device_id = {0};
static GLposi scissor_start = {0};
static GLposi scissor_end = {0x10000, 0x10000};

static GLuint Init_CompileShader(const char* source, GLenum kind) {
    GLuint shader = glCreateShader(kind);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char compilerLog[512];
        glGetShaderInfoLog(shader, 512, NULL, compilerLog);
        switch (kind) {
        case GL_VERTEX_SHADER:
            ERRORF("vertex shader compilation failed:\n%s", compilerLog);
            break;
        case GL_FRAGMENT_SHADER:
            ERRORF("fragment shader compilation failed:\n%s", compilerLog);
            break;
        default:
            ERRORF("shader compilation failed:\n%s", compilerLog);
        }
    }
    return shader;
}

static GLuint Init_SetupShader() {
    GLuint vertShader =
        Init_CompileShader(gl33_vertex_shader, GL_VERTEX_SHADER);
    GLuint fragShader =
        Init_CompileShader(gl33_fragment_shader, GL_FRAGMENT_SHADER);
    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    int success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char compilerLog[512];
        glGetShaderInfoLog(program, 512, NULL, compilerLog);
        ERRORF("shader linking failed:\n%s", compilerLog);
    }
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    return program;
}

static bool disp_on = false;
static int set_wnd_width = 256;
static int set_wnd_height = 240;
static int set_wnd_scale = SCREEN_SCALE;
static int cur_wnd_width = -1;
static int cur_wnd_height = -1;
static int cur_wnd_scale = -1;
static int set_disp_horiz = 256;
static int set_disp_vert = 240;
static int cur_disp_horiz = -1;
static int cur_disp_vert = -1;
static int fb_w = 0, fb_h = 0;

static void QuitPlatform(void);
static bool is_window_visible = false;
static bool is_platform_initialized = false;
static bool is_platform_init_successful = false;
bool InitPlatform() {
    if (is_platform_initialized) {
        return is_platform_init_successful;
    }
    // avoid re-initializing it continuously on failures
    is_platform_initialized = true;

    atexit(QuitPlatform);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        ERRORF("SDL_Init: %s", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);
    cur_wnd_height = DISP_HEIGHT;
    window = SDL_CreateWindow(
        "PSY-Z", DISP_WIDTH * SCREEN_SCALE, DISP_HEIGHT * SCREEN_SCALE,
        SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    if (!window) {
        ERRORF("SDL_CreateWindow: %s", SDL_GetError());
        return false;
    }

    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        ERRORF("opengl init failed: %s", SDL_GetError());
        return false;
    }
    SDL_GL_MakeCurrent(window, glContext);

#ifdef _WIN32
    if (!gladLoadGL()) {
        ERRORF("gladLoadGL failed");
        return false;
    }
#endif

    INFOF("opengl %s initialized", glGetString(GL_VERSION));
    glLineWidth(SCREEN_SCALE);
    shader_program = Init_SetupShader();
    if (!shader_program) {
        ERRORF("failed to compile shaders: %s", SDL_GetError());
        return false;
    }
    glUseProgram(shader_program);
    uniform_resolution = glGetUniformLocation(shader_program, "resolution");
    uniform_tex_4bpp = glGetUniformLocation(shader_program, "tex4");
    uniform_tex_8bpp = glGetUniformLocation(shader_program, "tex8");
    uniform_tex_16bpp = glGetUniformLocation(shader_program, "tex16");

    glUniform1i(uniform_tex_4bpp, Tex_4bpp);
    glUniform1i(uniform_tex_8bpp, Tex_8bpp);
    glUniform1i(uniform_tex_16bpp, Tex_16bpp);
    glGenTextures(LEN(vram_textures), vram_textures);

    glActiveTexture(GL_TEXTURE0 + Tex_4bpp);
    glBindTexture(GL_TEXTURE_2D, vram_textures[Tex_4bpp]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_R8, 4096, 512, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

    glActiveTexture(GL_TEXTURE0 + Tex_8bpp);
    glBindTexture(GL_TEXTURE_2D, vram_textures[Tex_8bpp]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_R8, 2048, 512, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

    glActiveTexture(GL_TEXTURE0 + Tex_16bpp);
    glBindTexture(GL_TEXTURE_2D, vram_textures[Tex_16bpp]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 512, 0, GL_RGBA,
                 GL_UNSIGNED_SHORT_1_5_5_5_REV, NULL);

    elapsed_from_beginning = SDL_GetTicks();
    last_vsync = elapsed_from_beginning;
    cur_tpage = 0;
    is_platform_init_successful = true;
    return true;
}

static u8 tex_data_buf[4096 * 512];
static void UploadTextures() {
    u8* srcData = (u8*)g_RawVram;
    for (int i = 0; i < LEN(tex_data_buf) / 2; i++) {
        tex_data_buf[i * 2 + 0] = srcData[i] & 0xF;
        tex_data_buf[i * 2 + 1] = srcData[i] >> 4;
    }

    glActiveTexture(GL_TEXTURE0 + Tex_4bpp);
    glBindTexture(GL_TEXTURE_2D, vram_textures[Tex_4bpp]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4096, 512, GL_RED, GL_UNSIGNED_BYTE,
                    tex_data_buf);

    glActiveTexture(GL_TEXTURE0 + Tex_8bpp);
    glBindTexture(GL_TEXTURE_2D, vram_textures[Tex_8bpp]);
    glTexSubImage2D(
        GL_TEXTURE_2D, 0, 0, 0, 2048, 512, GL_RED, GL_UNSIGNED_BYTE, g_RawVram);

    g_RawVram[1] = 0xFFFF;
    glActiveTexture(GL_TEXTURE0 + Tex_16bpp);
    glBindTexture(GL_TEXTURE_2D, vram_textures[Tex_16bpp]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1024, 512, GL_RGBA,
                    GL_UNSIGNED_SHORT_1_5_5_5_REV, g_RawVram);
}

static void PresentBufferToScreen(unsigned int index) {
    if (!window && !InitPlatform()) {
        return;
    }
    glFlush();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fb[index]);
    glBlitFramebuffer(
        0, 0, fb_w, fb_h, 0, 0, fb_w, fb_h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

static void QuitPlatform() {
    if (glContext) {
        SDL_GL_DestroyContext(glContext);
        glContext = NULL;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = NULL;
        is_window_visible = false;
    }
    SDL_Quit();
    is_platform_initialized = false;
    is_platform_init_successful = false;
}

void ResetPlatform(void) {
    cur_tpage = 0;
    if (shader_program) {
        glDeleteProgram(shader_program);
    }
    if (vram_textures[0]) {
        glDeleteTextures(LEN(vram_textures), vram_textures);
        memset(vram_textures, 0, LEN(vram_textures));
    }
    if (fb[0]) {
        glDeleteFramebuffers(LEN(fb), fb);
        glDeleteTextures(LEN(fbtex), fbtex);
        memset(fb, 0, LEN(fb));
        memset(fbtex, 0, LEN(fbtex));
    }
    QuitPlatform();
}

int PlatformVSync(int mode) {
    Uint32 cur;
    unsigned short ret;
    cur = SDL_GetTicks();
    if (mode >= 0) {
        ret = (unsigned short)(cur - last_vsync);
    } else {
        ret = (unsigned short)(cur - elapsed_from_beginning);
    }
    last_vsync = cur;
    PresentBufferToScreen(fb_index);
    return ret;
}

void SDLAudioCallback(void* userdata, SDL_AudioStream* stream,
                      int additional_amount, int total_amount) {
    NOT_IMPLEMENTED;
}

void MySsInitHot(void) {
    SDL_AudioSpec specs = {0};
    specs.freq = 44100;
    specs.format = SDL_AUDIO_S16;
    specs.channels = 2;
    audio_stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &specs, SDLAudioCallback, NULL);
    if (!audio_stream) {
        WARNF("SDL_OpenAudioDevice failed: %s", SDL_GetError());
        return;
    }
    INFOF("SDL audio device stream opened");
    SDL_PauseAudioStreamDevice(audio_stream);
}

static void PollEvents(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:
            exit(0);
        case SDL_EVENT_KEY_DOWN:
            if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                exit(0);
            }
            break;
        }
    }
}

void MyPadInit(int mode) { INFOF("use keyboard"); }

static u_long keyb_p1[] = {
    SDL_SCANCODE_W,         // PAD_L2
    SDL_SCANCODE_E,         // PAD_R2
    SDL_SCANCODE_Q,         // PAD_L1
    SDL_SCANCODE_R,         // PAD_R1
    SDL_SCANCODE_S,         // PAD_TRIANGLE
    SDL_SCANCODE_D,         // PAD_CIRCLE
    SDL_SCANCODE_X,         // PAD_CROSS
    SDL_SCANCODE_Z,         // PAD_SQUARE
    SDL_SCANCODE_BACKSPACE, // PAD_SELECT
    SDL_SCANCODE_1,         // PAD_L3
    SDL_SCANCODE_2,         // PAD_R3
    SDL_SCANCODE_RETURN,    // PAD_START
    SDL_SCANCODE_UP,        // PAD_UP
    SDL_SCANCODE_RIGHT,     // PAD_RIGHT
    SDL_SCANCODE_DOWN,      // PAD_DOWN
    SDL_SCANCODE_LEFT,      // PAD_LEFT
};
u_long MyPadRead(int id) {
    const bool* keyb;
    int numkeys;
    PollEvents();
    keyb = SDL_GetKeyboardState(&numkeys);
    u_long pressed = 0;

    if (id == 0) {
        const int to_read = LEN(keyb_p1) < numkeys ? LEN(keyb_p1) : numkeys;
        for (int i = 0; i < to_read; i++) {
            if (keyb[keyb_p1[i]]) {
                pressed |= 1 << i;
            }
        }
    }
    return pressed;
}

void Psyz_SetWindowScale(int scale) { set_wnd_scale = scale; }
void Psyz_GetWindowSize(int* width, int* height) {
    SDL_GetWindowSize(window, width, height);
}
static void UpdateScissor() {
    if (!window || !InitPlatform()) {
        return;
    }

    // flush all enqueued geometry with the previous scissor settings
    // TODO don't call Draw_FlushBuffer if equal scissor settings are repeated
    Draw_FlushBuffer();

    int width = scissor_end.x - scissor_start.x + 1;
    int height = scissor_end.y - scissor_start.y + 1;
    if (width <= 0 || height <= 0) {
        WARNF("scissor out of range: {%d, %d, %d, %d}", scissor_start.x,
              scissor_start.y, scissor_end.x, scissor_end.y);
        return;
    }
    int sx = scissor_start.x * cur_wnd_scale;
    int sy = scissor_start.y * cur_wnd_scale;
    int sw = width * cur_wnd_scale;
    int sh = height * cur_wnd_scale;
    int flipped_y = fb_h - (sy + sh); // OpenGL scissor origin to bottom-left
    glEnable(GL_SCISSOR_TEST);
    glScissor(sx, flipped_y, sw, sh);
}
static void ApplyDisplayPendingChanges() {
    if (!disp_on) {
        return;
    }
    if (!window && !InitPlatform()) {
        return;
    }
    if (cur_wnd_width != set_wnd_width || cur_wnd_height != set_wnd_height ||
        cur_wnd_scale != set_wnd_scale) {
        cur_wnd_width = set_wnd_width;
        cur_wnd_height = set_wnd_height;
        cur_wnd_scale = set_wnd_scale;
        fb_w = cur_wnd_width * cur_wnd_scale;
        fb_h = cur_wnd_height * cur_wnd_scale;
        SDL_SetWindowSize(window, fb_w, fb_h);
        glViewport(0, 0, fb_w, fb_h);
        glUniform2f(
            uniform_resolution, (float)cur_wnd_width, (float)cur_wnd_height);
        if (!fb[0]) {
            glGenTextures(LEN(fbtex), fbtex);
            glGenFramebuffers(LEN(fb), fb);
            for (int i = 0; i < LEN(fbtex); i++) {
                glBindFramebuffer(GL_FRAMEBUFFER, fb[i]);
                glBindTexture(GL_TEXTURE_2D, fbtex[i]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fb_w, fb_h, 0, GL_RGBA,
                             GL_UNSIGNED_BYTE, NULL);
                glTexParameteri(
                    GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(
                    GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                       GL_TEXTURE_2D, fbtex[i], 0);
                if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
                    GL_FRAMEBUFFER_COMPLETE) {
                    ERRORF("unable to create framebuffer");
                }
            }
        } else {
            for (int i = 0; i < LEN(fbtex); i++) {
                glBindTexture(GL_TEXTURE_2D, fbtex[i]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fb_w, fb_h, 0, GL_RGBA,
                             GL_UNSIGNED_BYTE, NULL);
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, fb[fb_index]);
    }
    if (!is_window_visible) {
        SDL_ShowWindow(window);
        is_window_visible = true;
    }
    if (cur_disp_horiz != set_disp_horiz || cur_disp_vert != set_disp_vert) {
        cur_disp_horiz = set_disp_horiz;
        cur_disp_vert = set_disp_vert;
        WARNF("setting screen aspect ratio not supported");
    }
}

static bool IsInRect(unsigned int x, unsigned int y, PS1_RECT* rect) {
    return x >= rect->x && x < rect->x + rect->w && y >= rect->y &&
           y < rect->y + rect->h;
}
static int GuessFrameBuffer(unsigned int x, unsigned int y) {
    if (IsInRect(x, y, &fbrect[0])) {
        return 0;
    }
    if (IsInRect(x, y, &fbrect[1])) {
        return 1;
    }
    return -1;
}

void Draw_Reset() { NOT_IMPLEMENTED; }

void Draw_DisplayEnable(unsigned int on) {
    disp_on = on;
    if (!on) {
        // TODO unbind the frame buffer and display a black screen
        glBindFramebuffer(GL_FRAMEBUFFER, fb[fb_index]);
        glClearColor(0, 0, 0, 1);
        glDisable(GL_SCISSOR_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_SCISSOR_TEST);
    } else {
        ApplyDisplayPendingChanges();
    }
}

void Draw_DisplayArea(unsigned int x, unsigned int y) {
    // TODO dirty hack where the frame buffer gets always flipped regardless
    // of x and y being different. Maybe a future idea would be to have a
    // fbidx_prev, so if the actual fbidx doesn't change between frames, we
    // still force a SDL_GL_SwapWindow
    int fbidx = y >= 240 || x >= 256;
    fbrect[fbidx].x = (short)x;
    fbrect[fbidx].y = (short)y;
    fbrect[fbidx].w = (short)cur_wnd_width;
    fbrect[fbidx].h = (short)cur_wnd_height;
    if (fb_index != fbidx) {
        PresentBufferToScreen(fb_index);
        SDL_GL_SwapWindow(window);
        fb_index = fbidx;
    }
    glFlush();
    glBindFramebuffer(GL_FRAMEBUFFER, fb[fb_index]);
    UpdateScissor();
}

void Draw_DisplayHorizontalRange(unsigned int start, unsigned int end) {
    set_disp_horiz = (int)(end - start) / 10;
    ApplyDisplayPendingChanges();
}

void Draw_DisplayVerticalRange(unsigned int start, int unsigned end) {
    set_disp_vert = (int)(end - start);
    ApplyDisplayPendingChanges();
}

void Draw_SetDisplayMode(DisplayMode* mode) {
    // TODO the interlace flag is ignored
    // TODO rgb24 is ignored, the color output will always max the color space
    if (mode->reversed) {
        WARNF("reverse mode not supported");
    }
    if (mode->horizontal_resolution_368) {
        set_wnd_width = 368;
    } else {
        switch (mode->horizontal_resolution) {
        case 0:
            set_wnd_width = 256;
            break;
        case 1:
            set_wnd_width = 320;
            break;
        case 2:
            set_wnd_width = 512;
            break;
        case 3:
            set_wnd_width = 640;
            break;
        }
    }
    set_wnd_height = mode->vertical_resolution ? 480 : 240;
    ApplyDisplayPendingChanges();
}

int Draw_ExequeSync() {
    PollEvents();
    glFlush();
    return 0;
}

#define MAX_VERTEX_COUNT 4096
#define MAX_INDEX_COUNT (MAX_VERTEX_COUNT / 4 * 6)

static unsigned int VAO = -1, VBO = -1, EBO = -1;
static Vertex vertex_buf[MAX_VERTEX_COUNT];
static unsigned short index_buf[MAX_INDEX_COUNT];
static Vertex* vertex_cur;
static unsigned short* index_cur;
static unsigned short n_vertices;
static int n_indices;
static GLenum flush_mode = GL_TRIANGLES;

static void Draw_InitBuffer() {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(vertex_buf), vertex_buf, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buf), index_buf, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(
        0, 2, GL_SHORT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, u));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex),
                          (void*)offsetof(Vertex, r));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}
static inline void Draw_EnsureBufferWillNotOverflow(int vertices, int indices) {
    bool bufferFull = n_vertices + vertices > MAX_VERTEX_COUNT ||
                      n_indices + indices > MAX_INDEX_COUNT;
    if (bufferFull) {
        Draw_FlushBuffer();
    }
}
static inline void Draw_EnqueueBuffer(int vertices, int indices) {
    bool bufferFull = n_vertices + vertices > MAX_VERTEX_COUNT ||
                      n_indices + indices > MAX_INDEX_COUNT;
    if (n_vertices > 0 && bufferFull) {
        // to flush, we need to save the new coming buffer and flush the rest
        Vertex* lastV = vertex_cur;
        unsigned short* lastI = index_cur;
        unsigned short lastVertexCount = n_vertices;
        Draw_FlushBuffer();
        memmove(vertex_cur, lastV, vertices * sizeof(*vertex_cur));
        memmove(index_cur, lastI, indices * sizeof(*index_cur));
        for (int i = 0; i < indices; i++) {
            index_buf[i] -= lastVertexCount;
        }
    }
    vertex_cur += vertices;
    index_cur += indices;
    n_vertices += vertices;
    n_indices += indices;
}

#define SEMITRANSP 0x02
#define TEXTURED 0x04
#define EXTRA_VERTEX 0x08
#define GOURAUD 0x10
#define TRIANGLE 0x20

static int writePacket(Vertex* v, int code, int n, u_long* packet, u16* pOut) {
    int w;
    if (!n) {
        return 0;
    }
    v->x = ((s16*)packet)[0];
    v->y = ((s16*)packet)[1];
    packet++;
    n--;
    if (!n) {
        return 1;
    }
    w = 1;
    if (code & TEXTURED) {
        v->u = ((u8*)packet)[0];
        v->v = ((u8*)packet)[1];
        *pOut = ((u16*)packet)[1];
        w++;
        packet++;
        n--;
        if (!n) {
            return w;
        }
    } else {
        *pOut = 0;
    }
    if (code & GOURAUD) {
        v++;
        v->r = ((u8*)packet)[0];
        v->g = ((u8*)packet)[1];
        v->b = ((u8*)packet)[2];
        v->a = code & SEMITRANSP ? 0x80 : 0xFF;
        w++;
    }
    return w;
}

int Draw_PushPrim(u_long* packets, int max_len) {
    int len = max_len;
    int code = (int)(*packets >> 24) & 0xFF;
    bool isPoly = !(code & 0x40);
    bool isLine = (code & 0x40) && !(code & 0x20);
    bool isTile = (code & 0x40) && (code & 0x20);
    bool isTextured = (code & TEXTURED) != 0;
    bool isGouraud = (code & GOURAUD) != 0;
    bool isShadeTex = !((code & 1) && isTextured && !isLine);
    u16 tpage = -1, clut = -1, pad2, pad3;
    Vertex* v;

    // to ensure we always have space, we pretend we want to allocate a quad
    Draw_EnsureBufferWillNotOverflow(4, 6);
    v = vertex_cur;
    if (isShadeTex) {
        v->r = *packets >> 0;
        v->g = *packets >> 8;
        v->b = *packets >> 16;
    } else {
        v->r = v->g = v->b = 0x80;
    }
    v->a = code & SEMITRANSP ? 0x80 : 0xFF;
    packets++;
    len--;
    if (isPoly) {
        if (code & TRIANGLE) {
            int wr, nVertices, nIndices;
            wr = writePacket(v++, code, len, packets, &clut);
            packets += wr;
            len -= wr;
            wr = writePacket(v++, code, len, packets, &tpage);
            packets += wr;
            len -= wr;
            wr = writePacket(v++, code, len, packets, &pad2);
            packets += wr;
            len -= wr;

            index_cur[0] = n_vertices + 0;
            index_cur[1] = n_vertices + 1;
            index_cur[2] = n_vertices + 2;
            if (code & EXTRA_VERTEX) {
                index_cur[3] = n_vertices + 1;
                index_cur[4] = n_vertices + 3;
                index_cur[5] = n_vertices + 2;
                nVertices = 4;
                nIndices = 6;
                wr = writePacket(v, code, len, packets, &pad3);
                packets += wr;
                len -= wr;
            } else {
                nVertices = 3;
                nIndices = 3;
            }
            // HACK last rgb are not read by writePacket, so we patch the amount
            packets--;
            len++;

            if (!isTextured) {
                clut = -1;
                tpage = -1;
            }
            if (!isGouraud || !isShadeTex) {
                VRGBA(vertex_cur[1]) = VRGBA(vertex_cur[2]) =
                    VRGBA(vertex_cur[3]) = VRGBA(vertex_cur[0]);
            }

            SET_TC_ALL(vertex_cur, tpage, clut);
            Draw_EnqueueBuffer(nVertices, nIndices);
        } else {
            // shouldn't never happen on a normal PSX application
            WARNF("code %02X not supported", code);
        }
    } else if (isLine) {
        bool padding = true;
        int nPoints = ((code >> 2) & 3) + 1;
        if (nPoints == 1) {
            padding = false;
            nPoints++; // don't ask, have faith
        }
        Draw_FlushBuffer();
        for (int i = 0; len > 0 && i < nPoints; i++) {
            vertex_cur[i].x = ((s16*)packets)[0];
            vertex_cur[i].y = ((s16*)packets)[1];
            vertex_cur[i].c = -1;
            vertex_cur[i].t = -1;
            packets++;
            len--;
            if (len > 0 && i + 1 < nPoints) {
                if (isGouraud) {
                    vertex_cur[i + 1].r = ((u8*)packets)[0];
                    vertex_cur[i + 1].g = ((u8*)packets)[1];
                    vertex_cur[i + 1].b = ((u8*)packets)[2];
                    vertex_cur[i + 1].a = code & SEMITRANSP ? 0x80 : 0xFF;
                    packets++;
                    len--;
                }
            }
        }
        if (!isGouraud) {
            VRGBA(vertex_cur[1]) = VRGBA(vertex_cur[2]) = VRGBA(vertex_cur[3]) =
                VRGBA(vertex_cur[0]);
        }
        if (padding) {
            len--;
        }
        for (int i = 0; i < nPoints - 1; i++) {
            index_cur[i * 2] = n_vertices + i;
            index_cur[i * 2 + 1] = n_vertices + i + 1;
        }
        flush_mode = GL_LINES;
        Draw_EnqueueBuffer(nPoints, (nPoints - 1) * 2);
        Draw_FlushBuffer();
        flush_mode = GL_TRIANGLES;
    } else if (isTile) {
        int x, y, w, h, tu, tv;
        x = ((s16*)packets)[0];
        y = ((s16*)packets)[1];
        packets++;
        len--;
        if (code & TEXTURED) {
            tu = ((u8*)packets)[0];
            tv = ((u8*)packets)[1];
            clut = ((s16*)packets)[1];
            tpage = cur_tpage;
            packets++;
            len--;
        } else {
            clut = -1;
            tpage = -1;
        }
        if ((code & 0x24) == 0x20) {
            // halve the brightness only for TILE
            vertex_cur[0].r >>= 1;
            vertex_cur[0].g >>= 1;
            vertex_cur[0].b >>= 1;
        }
        switch (code & ~3) {
        case 0x60: // TILE
        case 0x64: // SPRT
            w = ((s16*)packets)[0];
            h = ((s16*)packets)[1];
            packets++;
            len--;
            break;
        case 0x68: // TILE_1
        case 0x6C:
            w = 1;
            h = 1;
            break;
        case 0x70: // TILE_8
        case 0x74: // SPRT_8
            w = 8;
            h = 8;
            break;
        case 0x78: // TILE_16
        case 0x7C: // SPRT_16
            w = 16;
            h = 16;
            break;
        }
        vertex_cur[0].x = (short)(x);
        vertex_cur[0].y = (short)(y);
        vertex_cur[1].x = (short)(x + w);
        vertex_cur[1].y = (short)(y);
        vertex_cur[2].x = (short)(x);
        vertex_cur[2].y = (short)(y + h);
        vertex_cur[3].x = (short)(x + w);
        vertex_cur[3].y = (short)(y + h);
        if (code & TEXTURED) {
            vertex_cur[0].u = tu;
            vertex_cur[0].v = tv;
            vertex_cur[1].u = tu + w;
            vertex_cur[1].v = tv;
            vertex_cur[2].u = tu;
            vertex_cur[2].v = tv + h;
            vertex_cur[3].u = tu + w;
            vertex_cur[3].v = tv + h;
        }
        VRGBA(vertex_cur[1]) = VRGBA(vertex_cur[2]) = VRGBA(vertex_cur[3]) =
            VRGBA(vertex_cur[0]);
        index_cur[0] = n_vertices + 0;
        index_cur[1] = n_vertices + 1;
        index_cur[2] = n_vertices + 2;
        index_cur[3] = n_vertices + 1;
        index_cur[4] = n_vertices + 3;
        index_cur[5] = n_vertices + 2;
        SET_TC_ALL(vertex_cur, tpage, clut);
        Draw_EnqueueBuffer(4, 6);
    }
    return max_len - len;
}

void Draw_SetTexpageMode(ParamDrawTexpageMode* p) {
    // implements SetDrawMode, SetDrawEnv
    cur_tpage = *(u_short*)p & 0xFF; // TODO
    NOT_IMPLEMENTED;
}
void Draw_SetTextureWindow(unsigned int mask_x, unsigned int mask_y,
                           unsigned int off_x, unsigned int off_y) {
    // implements SetTexWindow
    // it seems it is some kind of texture clamp/repeat
    NOT_IMPLEMENTED;
}
void Draw_SetAreaStart(int x, int y) {
    // implements SetDrawArea, SetDrawEnv
    scissor_start.x = x;
    scissor_start.y = y;
    UpdateScissor();
}
void Draw_SetAreaEnd(int x, int y) {
    // implements SetDrawArea, SetDrawEnv
    scissor_end.x = x;
    scissor_end.y = y;
    UpdateScissor();
}
void Draw_SetOffset(int x, int y) {
    // implements SetDrawEnv, SetDrawOffset
    NOT_IMPLEMENTED;
}
static bool DoRectTouch(PS1_RECT* r1, PS1_RECT* r2) {
    return !(r1->x + r1->w <= r2->x || r2->x + r2->w <= r1->x ||
             r1->y + r1->h <= r2->y || r2->y + r2->h <= r1->y);
}
void Draw_ClearImage(PS1_RECT* rect, u_char r, u_char g, u_char b) {
    int fbidx = GuessFrameBuffer(rect->x, rect->y);
    if (fbidx >= 0) {
        GLposi prev_start = scissor_start;
        GLposi prev_end = scissor_end;
        scissor_start.x = rect->x;
        scissor_start.y = rect->y;
        scissor_end.x = rect->x + rect->w - 1;
        scissor_end.y = rect->y + rect->h - 1;
        UpdateScissor();
        glClearColor(
            (float)r / 255.f, (float)g / 255.f, (float)b / 255.f, 1.0f);
        if (fbidx == fb_index) {
            glClear(GL_COLOR_BUFFER_BIT);
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, fb[fbidx]);
            glClear(GL_COLOR_BUFFER_BIT);
            glBindFramebuffer(GL_FRAMEBUFFER, fb[fb_index]);
        }
        scissor_start = prev_start;
        scissor_end = prev_end;
        UpdateScissor();
    }

    PS1_RECT fixedRect;
    fixedRect.x = CLAMP(rect->x, 0, 1024);
    fixedRect.y = CLAMP(rect->y, 0, 1024);
    fixedRect.w = CLAMP(rect->w, 0, 1024);
    fixedRect.h = CLAMP(rect->h, 0, 1024);

    u16* vram = g_RawVram;
    vram += fixedRect.x + fixedRect.y * VRAM_W;
    for (int i = 0; i < fixedRect.h; i++) {
        for (int j = 0; j < fixedRect.w; j++) {
            vram[j] = (r >> 3 << 5) | (g >> 3 << 10) | (b >> 3 << 15);
        }
        vram += VRAM_W;
    }
}
void Draw_LoadImage(PS1_RECT* rect, u_long* p) {
    u16* mem = (u16*)p;
    u16* vram = g_RawVram;
    vram += rect->x + rect->y * VRAM_W;
    for (int i = 0; i < rect->h; i++) {
        memcpy(vram, mem, rect->w * sizeof(u16));
        mem += rect->w;
        vram += VRAM_W;
    }
    is_vram_texture_invalid = true;
}
void Draw_StoreImage(PS1_RECT* rect, u_long* p) {
    u16* mem = (u16*)p;
    u16* vram = g_RawVram;
    vram += rect->x + rect->y * VRAM_W;

    for (int i = 0; i < rect->h; i++) {
        for (int j = 0; j < rect->w; j++) {
            *mem++ = vram[j];
        }
        vram += VRAM_W;
    }
}
void Draw_ResetBuffer(void) {
    n_vertices = 0;
    n_indices = 0;
    vertex_cur = vertex_buf;
    index_cur = index_buf;
}
void Draw_FlushBuffer(void) {
    if (n_vertices == 0) {
        return;
    }
    if (VBO == -1) {
        Draw_InitBuffer();
    }
    if (is_vram_texture_invalid) {
        UploadTextures();
        is_vram_texture_invalid = false;
    }
    glUseProgram(shader_program);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(
        GL_ARRAY_BUFFER, 0, sizeof(Vertex) * n_vertices, vertex_buf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferSubData(
        GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(*index_buf) * n_indices, index_buf);
    glBindVertexArray(VAO);
    glDrawElements(flush_mode, n_indices, GL_UNSIGNED_SHORT, 0);
    Draw_ResetBuffer();
}
