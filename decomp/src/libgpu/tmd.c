#include <common.h>
#include <og/libgte.h>
#include <og/libgpu.h>

#pragma pack(push, 4)
typedef struct {
    int vert;
    int nvert;
    int norm;
    int nnorm;
    int prim;
    int nprim;
    int scaling;
} TmdObj;
typedef struct {
    int id;
    int flags;
    int nobj;
    TmdObj obj[1];
} TMD;
#pragma pack(pop)

extern u_long* tim;
extern int n_prim;
extern u_long* t_prim;
extern u_long* v_ofs;
extern u_long* n_ofs;
u_long get_tim_addr(unsigned int* addr, TIM_IMAGE*);
u_long get_tmd_addr(
    TMD* tmd, int obj_no, u_long** t_prim, u_long** v_ofs, u_long** n_ofs);

long OpenTIM(u_long* addr) {
    tim = addr;
    return 0;
}

TIM_IMAGE* ReadTIM(TIM_IMAGE* timimg) {
    u_long len = get_tim_addr(tim, timimg);
    if (len == -1) {
        return NULL;
    }
    tim += len;
    return timimg;
}

int OpenTMD(u_long* tmd, int obj_no) {
    u_long len = get_tmd_addr(tmd, obj_no, &t_prim, &v_ofs, &n_ofs);
    n_prim = len;
    return len;
}

INCLUDE_ASM("asm/nonmatchings/libgpu/tmd", ReadTMD);

// https://www.psxdev.net/forum/viewtopic.php?t=109
u_long get_tim_addr(unsigned int* timaddr, TIM_IMAGE* img) {
    unsigned int clut_len;
    unsigned int img_len;
    if (*(int*)timaddr++ != 0x10) {
        return -1;
    }
    img->mode = *timaddr++;
    if (GetGraphDebug() == 2) {
        printf("id  =%08x\n", 0x10);
    }
    if (GetGraphDebug() == 2) {
        printf("mode=%08x\n", img->mode);
    }
    if (GetGraphDebug() == 2) {
        printf("timaddr=%08x\n", timaddr);
    }
    if (img->mode & 8) {
        clut_len = *timaddr >> 2;
        img->crect = (RECT*)(timaddr + 1);
        img->caddr = timaddr + 3;
        timaddr = &timaddr[clut_len];
    } else {
        img->crect = NULL;
        img->caddr = NULL;
        clut_len = 0;
    }
    img_len = *timaddr >> 2;
    img->prect = (RECT*)(timaddr + 1);
    img->paddr = timaddr + 3;
    return 2 + clut_len + img_len;
}

u_long get_tmd_addr(
    TMD* tmd, int objid, u_long** t_prim, u_long** v_ofs, u_long** n_ofs) {
    TmdObj* obj = tmd->obj;
    if (GetGraphDebug() == 2) {
        printf("analizing TMD...\n");
    }
    if (GetGraphDebug() == 2) {
        printf("\tid=%08X, flags=%d, nobj=%d, objid=%d\n", tmd->id, tmd->flags,
               tmd->nobj, objid);
    }
    if (GetGraphDebug() == 2) {
        printf("\tvert=%08X, nvert=%d\n", obj[objid].vert, obj[objid].nvert);
    }
    if (GetGraphDebug() == 2) {
        printf("\tnorm=%08X, nnorm=%d\n", obj[objid].norm, obj[objid].nnorm);
    }
    if (GetGraphDebug() == 2) {
        printf("\tprim=%08X, nprim=%d\n", obj[objid].prim, obj[objid].nprim);
    }
    *v_ofs = (u_long*)((unsigned char*)obj + obj[objid].vert);
    *n_ofs = (u_long*)((unsigned char*)obj + obj[objid].norm);
    *t_prim = (u_long*)((unsigned char*)obj + obj[objid].prim);
    return obj[objid].nprim;
}

INCLUDE_ASM("asm/nonmatchings/libgpu/tmd", unpack_packet);
