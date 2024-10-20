#include <psyz.h>
#include <libgpu.h>
#include <stdio.h>

void MyDrawSyncCallback(int mode) { NOT_IMPLEMENTED; }

void (*g_VsyncCallback)() = NULL;
int MyVSyncCallback(void (*f)()) { g_VsyncCallback = f; }

int PlatformVSync(int mode);
int MyVSync(int mode) {
    // TODO the implementation is most likely incorrect
    int elapsed = (unsigned short)PlatformVSync(mode);
    if (g_VsyncCallback) {
        //g_VsyncCallback();
    }
    return elapsed;
}
