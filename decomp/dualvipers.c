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
#define BARREL_OFFSET ((VECTOR*)0x220b20)

typedef struct gadget_vtable {
    void (*update)(Moby* this);
    void (*GB_AssignLocalPlayerToWeapon)(Moby* moby, Player** outPlayer);
    void (*MB_setState)(Moby* moby, int state, int a2);
    long (*Hero_PeekGadgetEvent)(Player* player, int a1, int a2, int a3);
    int  (*FUN_005f02c0)(Player* player);
    void (*MB_transAnim)(Moby* moby, int seq, float frm, int steps, int flags);
    void (*DualVipers_Aiming)(Moby* moby, Player* player);
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

typedef struct DualVipersPVar { // 0x100
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
} DualVipersPVar_t;

void M4244_Update_DualVipers(Moby* moby)
{
    DualVipersPVar_t* pvar = (DualVipersPVar_t*)moby->PVar;

    if (!moby->State) {
        moby->UpdateDist = 0xFF;
        pvar->shotCount = 0;
        pvar->actuatorHandle = -1;
        gadgetInfo.vtable.GB_AssignLocalPlayerToWeapon(moby, &pvar->owner);
        pvar->soundReady = 1;
        mobySetState(moby, 1, -1);
    }

    Player* player = pvar->owner;
    PlayerVTable* vtable = playerGetVTable(player);

    if (*(u32*)((u32)player + 0x25cc) == PLAYER_STATE_LOOK)
        vtable->UpdateState(player, PLAYER_STATE_TARGETING, 1, 0, 1);

    if (!player || !*(u32*)((u32)player + 0x266d)) {
        long event = gadgetInfo.vtable.Hero_PeekGadgetEvent(player, 0, 1, 0);
        if (event == 0 && gadgetInfo.vtable.FUN_005f02c0(player) == 0) {
            return;
        }
    }

    if ((moby->AnimFlags & 2) != 0) {
        if (moby->AnimSeqId == 1) {
            if (*(u32*)0x0021e694 == 1) // g_gameType: only freeze anim speed in singleplayer
                moby->AnimSpeed = 0.0f;
        } else {
            gadgetInfo.vtable.MB_transAnim(moby, 1, 0.0f, 2, 0);
        }
    }

    VECTOR barrelWorldPos;
    vector_apply(barrelWorldPos, BARREL_OFFSET, &moby->M0_03);
    vector_add(barrelWorldPos, barrelWorldPos, moby->Position);
    vector_copy(pvar->weaponPos, barrelWorldPos);
    gadgetInfo.vtable.DualVipers_Aiming(moby, player);

    long gadgetEventType = 0;
    if (moby->SubState != 0) {
        GadgetEvent gadgetEvent;
        gadgetEventType = gadgetInfo.vtable.Hero_GetGadgetEvent(player, 0, 1, &gadgetEvent);

        int equippedTime = player->Gadgets[0].equippedTime;
        int canShoot = (gadgetInfo.vtable.FUN_005f02c0(player) != 0) ? (equippedTime >= 0x17) : (equippedTime >= 0x17 || player->timers.noInput != 0);
        if (canShoot) {
            pvar->shotType = 0x14;
            pvar->idleTimer = 0;    // sw zero,0x5c(s2): reset idle timer on fire
            pvar->shotCount++;

            if (gadgetEventType == 8) {
                gadgetInfo.vtable.DualVipers_ShootUpdate(moby, player, &gadgetEvent);
                moby->SubState = 0;
                *(u8*)(pvar->shotStatePtr + 0x67) = 1;
            } else if (gadgetEventType == 4 && pvar->soundReady == 1) {
                mobyPlaySound(4, 0, moby);
                pvar->soundReady = 0;
            }
        }
    } else {
        if (player->timers.noInput != 0 && player->Gadgets[0].padButtonDown == 1) {
            player->Gadgets[0].padButtonDown = 0;
            gadgetInfo.vtable.Hero_QueueGadgetEvent(player, 2, -1, -1, 0, 0);
        }

        int firingTimer = player->timers.firing;
        if (gadgetInfo.vtable.FUN_005f02c0(player) == 0 || firingTimer == 0)
            pvar->idleTimer++;

        if (firingTimer > 0xf && pvar->idleTimer >= 9)
            gadgetInfo.vtable.Hero_EndFiringAnim(player);

        if (pvar->idleTimer >= 0x15)
            pvar->soundReady = 1;

        if (pvar->shotCount < 1)
            pvar->shotCount = 0;
    }

    int handle = pvar->actuatorHandle;
    if (gadgetEventType != 8 || *(u8*)((u32)player + 0x265a) == 0) {
        if (handle >= 0) {
            gadgetInfo.vtable.actuator_killWave(handle, player->Paddata);
            pvar->actuatorHandle = -1;
            handle = pvar->actuatorHandle;
        }
    }

    if (handle > 0 && gadgetInfo.vtable.FUN_004ad330(handle - 1, player->Paddata) == 0)
        pvar->actuatorHandle = -1;
}

void gadgetFindAndHook(void)
{
    Moby *start = mobyListGetStart();
    Moby *end = mobyListGetEnd();
    while (start < end) {
        if (start->OClass == 0x1094 && start->PParent->OClass == 0x251c) {
            if (!gadgetInfo.vtable.update) {
                gadgetInfo.vtable.update = start->PUpdate;
            }
            MobyClassUpdate[start->MClass] = &M4244_Update_DualVipers;
            start->PUpdate = &M4244_Update_DualVipers;
        }
        ++start;
    }
}

int gadgetInit(void)
{
    gadgetFindAndHook();

    u32 start = gadgetInfo.vtable.update;
    if (!start) return;
    gadgetInfo.vtable.GB_AssignLocalPlayerToWeapon = JAL2ADDR(*(u32*)(start + 0x40));
    gadgetInfo.vtable.MB_setState = JAL2ADDR(*(u32*)(start + 0x58));
    gadgetInfo.vtable.Hero_PeekGadgetEvent = JAL2ADDR(*(u32*)(start + 0xc4));
    gadgetInfo.vtable.FUN_005f02c0 = JAL2ADDR(*(u32*)(start + 0xd8));
    gadgetInfo.vtable.MB_transAnim = JAL2ADDR(*(u32*)(start + 0x11c));
    gadgetInfo.vtable.DualVipers_Aiming = JAL2ADDR(*(u32*)(start + 0x174));
    gadgetInfo.vtable.Hero_GetGadgetEvent = JAL2ADDR(*(u32*)(start + 0x198));
    gadgetInfo.vtable.DualVipers_ShootUpdate = JAL2ADDR(*(u32*)(start + 0x214));
    gadgetInfo.vtable.sound_MobyPlay = JAL2ADDR(*(u32*)(start + 0x254));
    gadgetInfo.vtable.Hero_QueueGadgetEvent = JAL2ADDR(*(u32*)(start + 0x290));
    gadgetInfo.vtable.Hero_EndFiringAnim = JAL2ADDR(*(u32*)(start + 0x2e0));
    gadgetInfo.vtable.actuator_killWave = JAL2ADDR(*(u32*)(start + 0x32c));
    gadgetInfo.vtable.FUN_004ad330 = JAL2ADDR(*(u32*)(start + 0x34c));
    return 1;
}

void dualVipers(void)
{
    if (!isInGame()) {
        if (gadgetInfo.init)
            memset(&gadgetInfo, 0, sizeof(gadgetInfo_t));
        return;
    }

    if (!gadgetInfo.init)
        gadgetInfo.init = gadgetInit();
}