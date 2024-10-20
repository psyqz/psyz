#include <psyz.h>
#include <libgpu.h>
#include <stdio.h>

bool g_IsQuitRequested;
u16 g_RawVram[VRAM_W * VRAM_H];

int g_Frame = 0;
void MyDrawSyncCallback(int mode) { g_Frame++; }

void (*g_VsyncCallback)() = NULL;
int MyVSyncCallback(void (*f)()) { g_VsyncCallback = f; }

int PlatformVSync(int mode);
int MyVSync(int mode) {
    int elapsed = (unsigned short)PlatformVSync(mode);
    if (g_VsyncCallback) {
        //g_VsyncCallback();
    }
    return elapsed;
}
