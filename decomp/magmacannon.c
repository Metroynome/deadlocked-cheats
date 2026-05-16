#include <libdl/moby.h>
#include <libdl/math.h>
#include <libdl/math3d.h>
#include <libdl/player.h>
#include <libdl/game.h>
#include <libdl/utils.h>
#include <libdl/spawnpoint.h>
#include <libdl/sound.h>
#include <libdl/player.h>

#define JAL2ADDR(jal) ((jal & 0x03FFFFFF) << 2)
#define MobyClassUpdate ((void**)0x00249980)
#define M_CLASS (32)

typedef struct gadget_vtable {
    void (*update)(Moby* this);
    void (*GB_AssignLocalPlayerToWeapon)(Moby* moby, Player** outPlayer);
    void (*MB_setState)(Moby* moby, int state, int a2);
    long (*Hero_PeekGadgetEvent)(Player* player, int a1, int a2, int a3);
    int  (*FUN_005f02c0)(Player* player);
    void (*MB_transAnim)(Moby* moby, int seq, float frm, int steps, int flags);
    void (*FUN_003b6c28)(Moby* moby, Player* player);
    long (*Hero_GetGadgetEvent)(Player* player, int a1, int a2, GadgetEvent* out);
    void (*DualVipers_ShootUpdate)(Moby* moby, Player* player, GadgetEvent* event);
    void (*sound_MobyPlay)(int soundId, int a1, Moby* moby);
    void (*Hero_QueueGadgetEvent)(Player* player, int a1, int a2, int a3, int a4, int a5);
    void (*Hero_EndFiringAnim)(Player* player);
    void (*actuator_killWave)(int handle, void* pad);
    int  (*FUN_004ad330)(int handle, void* pad);
} gadget_vtable_t;

typedef struct gadgetInfo {
    short init;
    short runOldFunction;
    int numGadgets;
    gadget_vtable_t vtable;
} gadgetInfo_t;
gadgetInfo_t gadgetInfo;

typedef struct gadgetPVar { // 0x100
/* 0x00 */ VECTOR weaponPos;
/* 0x10 */ VECTOR fireDir;
/* 0x20 */ VECTOR targetPos;
/* 0x30 */ VECTOR targetAimPos;
/* 0x40 */ short shotType;
/* 0x42 */ short shotCount;
/* 0x44 */ int actuatorHandle;
/* 0x48 */ Player* owner;
/* 0x4c */ u32 targetUID;
/* 0x50 */ char pad_50[6];
/* 0x56 */ short soundReady;
/* 0x58 */ int pad_58;
/* 0x5c */ u32 idleTimer;
/* 0x60 */ Moby* targetMoby;
/* 0x64 */ u32 shotStatePtr;
/* 0x68 */ char unk_68[0x98];
} gadgetPVar_t;

void M4231_Update_MagmaCannon(Moby* moby)
{
    return;
}

int gadgetInit(void)
{
    // backup original function pointer, and set our own.
    gadgetInfo.vtable.update = *(u32*)MobyClassUpdate[M_CLASS];
    MobyClassUpdate[M_CLASS] = &M4231_Update_MagmaCannon;

    // u32 start = gadgetInfo.vtable.update;
    // if (!start) return;
    // gadgetInfo.vtable.GB_AssignLocalPlayerToWeapon = JAL2ADDR(*(u32*)(start + 0x40));
    // gadgetInfo.vtable.MB_setState = JAL2ADDR(*(u32*)(start + 0x58));
    // gadgetInfo.vtable.Hero_PeekGadgetEvent = JAL2ADDR(*(u32*)(start + 0xc4));
    // gadgetInfo.vtable.FUN_005f02c0 = JAL2ADDR(*(u32*)(start + 0xd8));
    // gadgetInfo.vtable.MB_transAnim = JAL2ADDR(*(u32*)(start + 0x11c));
    // gadgetInfo.vtable.FUN_003b6c28 = JAL2ADDR(*(u32*)(start + 0x174));
    // gadgetInfo.vtable.Hero_GetGadgetEvent = JAL2ADDR(*(u32*)(start + 0x198));
    // gadgetInfo.vtable.DualVipers_ShootUpdate = JAL2ADDR(*(u32*)(start + 0x214));
    // gadgetInfo.vtable.sound_MobyPlay = JAL2ADDR(*(u32*)(start + 0x254));
    // gadgetInfo.vtable.Hero_QueueGadgetEvent = JAL2ADDR(*(u32*)(start + 0x290));
    // gadgetInfo.vtable.Hero_EndFiringAnim = JAL2ADDR(*(u32*)(start + 0x2e0));
    // gadgetInfo.vtable.actuator_killWave = JAL2ADDR(*(u32*)(start + 0x32c));
    // gadgetInfo.vtable.FUN_004ad330 = JAL2ADDR(*(u32*)(start + 0x34c));
    return 1;
}

void magmaCannon(void)
{
    if (!isInGame()) {
        if (gadgetInfo.init)
            memset(&gadgetInfo, 0, sizeof(gadgetInfo_t));
        return;
    }

    if (!gadgetInfo.init)
        gadgetInfo.init = gadgetInit();

}