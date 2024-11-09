#include "psyz.h"
#include <assert.h>
#include <stdlib.h>
#include "libgpu.h"
#ifdef _MSC_VER
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include "../draw.h"
#include <SDL2/SDL_opengl.h>
#include <GLES3/gl3.h>

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
    "    vertexColor = color;\n"
    // select the right texture coords based on the tpage
    "    clut = uint(tex.z);\n"
    "    tpage = uint(tex.w);\n"
    "    vec2 page = vec2(\n"
    "        float(tpage % 16u) / 16.0,\n"
    "        float(tpage / 16u) / 2.0);\n"
    "    texCoord = page + vec2(tex.x / 4096.0, tex.y / 512.0);\n"
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
    "    if (tpage >= 0x40u) {\n"
    "        if (tpage >= 0x80u) {\n" // untextured
    "            FragColor = vertexColor;\n"
    "        }\n"
    "        else {\n"                                  // 16-bit
    "            FragColor = vec4(0.0, 1.0, 0.0, 1.0);" // TODO
    "        }\n"
    "    } else {\n"
    "        if (tpage >= 0x20u) {\n"                   // 8-bit
    "            FragColor = vec4(0.0, 1.0, 1.0, 1.0);" // TODO
    "        }\n"
    "        else {\n" // 4-bit
    "            float colIdx = texture(tex4, texCoord).r;\n"
    "            float palIdx = floor(colIdx * 256.0) / 1024.0f;\n"
    "            vec2 clutIdx = getPsxClutXY(clut);"
    "            vec2 palCoord = clutIdx + vec2(palIdx, 0);"
    "            vec4 palColor = texture(tex16, palCoord);"
    "            palColor.a /= 2;"
    "            FragColor = palColor * vertexColor * 2;\n"
    "        }\n"
    "    }\n"
    "}\n"};

typedef struct {
    short x, y;
    unsigned short u, v, c, t;
    unsigned char r, g, b, a;
} Vertex;

#define VRGBA(p) (*(unsigned int*)(&((p).r)))
#define SET_TC(p, tpage, clut) (p)->t = (u8)tpage, (p)->c = (u16)clut;
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
static SDL_AudioSpec audio_specs = {0};
static SDL_AudioDeviceID audio_device_id = {0};

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

void ResetPlatform(void);
bool InitPlatform() {
    atexit(ResetPlatform);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        ERRORF("SDL_Init: %s", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    window = SDL_CreateWindow(
        "PSY-Z", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        DISP_WIDTH * SCREEN_SCALE, DISP_HEIGHT * SCREEN_SCALE,
        SDL_WINDOW_OPENGL);
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
    SDL_GL_SetSwapInterval(1);
    INFOF("opengl %s initialized", glGetString(GL_VERSION));

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

void ResetPlatform(void) {
    cur_tpage = 0;
    if (shader_program) {
        glDeleteProgram(shader_program);
    }
    if (vram_textures[0]) {
        glDeleteTextures(LEN(vram_textures), vram_textures);
        memset(vram_textures, 0, LEN(vram_textures));
    }
    if (glContext) {
        SDL_GL_DeleteContext(glContext);
        glContext = NULL;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
    SDL_Quit();
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
    return ret;
}

void SDLAudioCallback(void* data, Uint8* buffer, int length) {
    NOT_IMPLEMENTED;
}

void MySsInitHot(void) {
    SDL_AudioSpec specs = {0};
    specs.freq = 44100;
    specs.format = AUDIO_S16;
    specs.channels = 2;
    specs.samples = 2048;
    specs.callback = SDLAudioCallback;

    audio_device_id =
        SDL_OpenAudioDevice(NULL, false, &specs, &audio_specs, false);
    if (audio_device_id == 0) {
        WARNF("SDL_OpenAudioDevice failed: %s", SDL_GetError());
        return;
    }
    INFOF("SDL audio device opened: %d", audio_device_id);
    SDL_PauseAudioDevice(audio_device_id, 0);
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
    const u8* keyb = SDL_GetKeyboardState(NULL);
    u_long pressed = 0;

    if (id == 0) {
        for (int i = 0; i < LEN(keyb_p1); i++) {
            if (keyb[keyb_p1[i]]) {
                pressed |= 1 << i;
            }
        }
    }
    return pressed;
}

int Draw_Sync(int mode) {
    SDL_Event event;
    SDL_GL_SwapWindow(window);
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            exit(0);
        case SDL_KEYDOWN:
            if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                exit(0);
            }
            break;
        }
    }
    return 0;
}

int wnd_width = -1;
int wnd_height = -1;
RECT g_SetDisp = {0};
RECT g_PrevDisp = {0};
void Draw_PutDispEnv(DISPENV* disp) {
    if (!window) {
        InitPlatform();
    }
    g_PrevDisp = g_SetDisp;
    g_SetDisp = disp->disp;
    int w = disp->disp.w * SCREEN_SCALE;
    int h = disp->disp.h * SCREEN_SCALE;
    if (wnd_width == w && wnd_height == h) {
        return;
    }
    glViewport(0, 0, w, h);
    glUniform2f(uniform_resolution, (float)disp->disp.w, (float)disp->disp.h);

    wnd_width = w;
    wnd_height = h;
    SDL_SetWindowSize(window, w, h);
}

#define MAX_VERTEX_COUNT 600
#define MAX_INDEX_COUNT (MAX_VERTEX_COUNT / 4 * 6)

static unsigned int VAO = -1, VBO = -1, EBO = -1;
static Vertex vertex_buf[MAX_VERTEX_COUNT];
static unsigned short index_buf[MAX_INDEX_COUNT];
static Vertex* vertex_cur;
static unsigned short* index_cur;
static unsigned short n_vertices;
static int n_indices;

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

void Draw_FlushBuffer(void);
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
    bool isShadeTex = !((code & 1) && isTextured);
    ushort tpage = -1, clut = -1, pad2, pad3;
    Vertex* v = vertex_cur;

    // to ensure we always have space, we pretend we want to allocate a quad
    Draw_EnsureBufferWillNotOverflow(4, 6);
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
        SDL_Point points[4];
        int nPoints = ((code >> 2) & 3) + 1;
        for (int i = 0; len > 0 && i < nPoints; i++) {
            points[i].x = ((s16*)packets)[0];
            points[i].y = ((s16*)packets)[1];
            packets++;
            len--;
            if (len > 0 && isGouraud) {
                // TODO not supported on SDL2+OpenGL
                packets++;
                len--;
            }
            // HACK last rgb are not read by writePacket, so we patch the amount
            packets--;
            len++;
        }
        // TODO not sure how to do this with OpenGL
        // SDL_SetRenderDrawColor(
        //     renderer, v->r, v->g, v->b, v->a);
        // SDL_RenderDrawLines(renderer, points, nPoints);
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
    cur_tpage = *(u_short*)p & 0x1F; // TODO
    NOT_IMPLEMENTED;
}
void Draw_SetTextureWindow(int mask_x, int mask_y, int off_x, int off_y) {
    // implements SetTexWindow
    // it seems it is some kind of texture clamp/repeat
    NOT_IMPLEMENTED;
}
void Draw_SetAreaStart(int x, int y) {
    // implements SetDrawArea, SetDrawEnv
    NOT_IMPLEMENTED;
}
void Draw_SetAreaEnd(int x, int y) {
    // implements ??
    NOT_IMPLEMENTED;
}
void Draw_SetOffset(int x, int y) {
    // implements SetDrawEnv, SetDrawOffset
    NOT_IMPLEMENTED;
}
void Draw_ClearImage(RECT* rect, u_char r, u_char g, u_char b) {
    // TODO HACK comparing both g_SetDisp and g_PrevDisp because in one of the
    // samples PutDrawEnv is called after PutDispEnv, clearing the buffer that
    // is currently in the background instead of the front one
    if ((rect->x == g_SetDisp.x && rect->y == g_SetDisp.y &&
         rect->w == g_SetDisp.w && rect->h == g_SetDisp.h) ||
        rect->x == g_PrevDisp.x && rect->y == g_PrevDisp.y &&
            rect->w == g_PrevDisp.w && rect->h == g_PrevDisp.h) {
        glClearColor(
            (float)r / 255.f, (float)g / 255.f, (float)b / 255.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        return;
    } else {
        WARNF("TODO implement partial clear screen");
    }

    u16* vram = g_RawVram;
    vram += rect->x + rect->y * VRAM_W;

    for (int i = 0; i < rect->h; i++) {
        for (int j = 0; j < rect->w; j++) {
            vram[j] = (r >> 3 << 5) | (g >> 3 << 10) | (b >> 3 << 15) | 0x8000;
        }
        vram += VRAM_W;
    }
}
void Draw_LoadImage(RECT* rect, u_long* p) {
    ushort* mem = (ushort*)p;
    ushort* vram = g_RawVram;
    vram += rect->x + rect->y * VRAM_W;

    for (int i = 0; i < rect->h; i++) {
        memcpy(vram, mem, rect->w * sizeof(ushort));
        mem += rect->w;
        vram += VRAM_W;
    }
    is_vram_texture_invalid = true;
}
void Draw_StoreImage(RECT* rect, u_long* p) {
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
    glDrawElements(GL_TRIANGLES, n_indices, GL_UNSIGNED_SHORT, 0);
    Draw_ResetBuffer();
}
