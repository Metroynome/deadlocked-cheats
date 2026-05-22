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
#define M_CLASS (13)

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

typedef struct DualViperspVar { // 0x70
/* 0x00 */ VECTOR weaponPos;
/* 0x10 */ VECTOR fireDir;
/* 0x20 */ VECTOR aimPos;
/* 0x30 */ VECTOR targetAimPos;
/* 0x40 */ short shotType;
/* 0x42 */ short shotCount;
/* 0x44 */ int actuatorHandle;
/* 0x48 */ Player* owner;
/* 0x4c */ u32 targetUID;
/* 0x50 */ int pointLight;
/* 0x54 */ short int fireFromTopBarrel;
/* 0x56 */ short int outAmmoFirstTime;
/* 0x58 */ float scopeAng1;
/* 0x5c */ u32 idleTimer;
/* 0x60 */ Moby* pTarget;
/* 0x64 */ Moby* pOtherGun;
/* 0x68 */ unsigned char cIsMainGun;
/* 0x69 */ char pad_69[0x3];
/* 0x6c */ int pad_6c;
} DualViperspVar_t;

void M4244_Update_DualVipers(Moby* moby)
{
    DualViperspVar_t* pvar = (DualViperspVar_t*)moby->pVar;

    if (!moby->state) {
        moby->updateDist = 0xFF;
        pvar->shotCount = 0;
        pvar->actuatorHandle = -1;
        gadgetInfo.vtable.GB_AssignLocalPlayerToWeapon(moby, &pvar->owner);
        pvar->outAmmoFirstTime = 1;
        mobySetState(moby, 1, -1);
    }

    Player* player = pvar->owner;
    PlayerVTable* vtable = playerGetVTable(player);

    if (player->state == PLAYER_STATE_LOOK)
        vtable->InitBodyState(player, PLAYER_STATE_TARGETING, 1, 0, 1);

    if (!player || !player->isLocal) {
        long event = gadgetInfo.vtable.Hero_PeekGadgetEvent(player, 0, 1, 0);
        if (event == 0 && gadgetInfo.vtable.FUN_005f02c0(player) == 0) {
            return;
        }
    }

    if ((moby->animFlags & 2) != 0) {
        if (moby->animSeqId == 1) {
            if (gameType == 1)
                moby->animSpeed = 0.0f;
        } else {
            gadgetInfo.vtable.MB_transAnim(moby, 1, 0.0f, 2, 0);
        }
    }

    VECTOR barrelWorldPos;
    vector_apply(barrelWorldPos, BARREL_OFFSET, &moby->rMtx.v0);
    vector_add(barrelWorldPos, barrelWorldPos, moby->pos);
    vector_copy(pvar->weaponPos, barrelWorldPos);
    gadgetInfo.vtable.DualVipers_Aiming(moby, player);

    long gadgetEventType = 0;
    if (moby->subState != 0) {
        GadgetEvent gadgetEvent;
        gadgetEventType = gadgetInfo.vtable.Hero_GetGadgetEvent(player, 0, 1, &gadgetEvent);

        int equippedTime = player->gadgets[0].equippedTime;
        int canShoot = (gadgetInfo.vtable.FUN_005f02c0(player) != 0) ? (equippedTime >= 0x17) : (equippedTime >= 0x17 || player->timers.noInput != 0);
        if (canShoot) {
            pvar->shotType = 0x14;
            pvar->idleTimer = 0;    // sw zero,0x5c(s2): reset idle timer on fire
            pvar->shotCount++;

            if (gadgetEventType == 8) {
                gadgetInfo.vtable.DualVipers_ShootUpdate(moby, player, &gadgetEvent);
                moby->subState = 0;
                
                // get new shot targets.
                DualViperspVar_t* otherPvar = (DualViperspVar_t*)pvar->pOtherGun->pVar;
                vector_copy(otherPvar->weaponPos, pvar->weaponPos);
                vector_copy(otherPvar->fireDir, pvar->fireDir);
                vector_copy(otherPvar->aimPos, pvar->aimPos);
                vector_copy(otherPvar->targetAimPos, pvar->targetAimPos);

                otherPvar->shotType = pvar->shotType;
                otherPvar->pTarget = pvar->pTarget;
                otherPvar->targetUID = pvar->targetUID;

                pvar->pOtherGun->subState = 1;
            } else if (gadgetEventType == 4 && pvar->outAmmoFirstTime == 1) {
                mobyPlaySound(4, 0, moby);
                pvar->outAmmoFirstTime = 0;
            }
        }
    } else {
        if (player->timers.noInput != 0 && player->gadgets[0].padButtonDown == 1) {
            player->gadgets[0].padButtonDown = 0;
            gadgetInfo.vtable.Hero_QueueGadgetEvent(player, 2, -1, -1, 0, 0);
        }

        int firingTimer = player->timers.firing;
        if (gadgetInfo.vtable.FUN_005f02c0(player) == 0 || firingTimer == 0)
            pvar->idleTimer++;

        if (firingTimer > 0xf && pvar->idleTimer >= 9)
            gadgetInfo.vtable.Hero_EndFiringAnim(player);

        if (pvar->idleTimer >= 0x15)
            pvar->outAmmoFirstTime = 1;

        if (pvar->shotCount < 1)
            pvar->shotCount = 0;
    }

    int handle = pvar->actuatorHandle;
    if (gadgetEventType != 8 || player->firing == 0) {
        if (handle >= 0) {
            gadgetInfo.vtable.actuator_killWave(handle, player->pPad);
            pvar->actuatorHandle = -1;
            handle = pvar->actuatorHandle;
        }
    }

    if (handle > 0 && gadgetInfo.vtable.FUN_004ad330(handle - 1, player->pPad) == 0)
        pvar->actuatorHandle = -1;
}

void gadgetFindAndHook(void)
{
    Moby *start = mobyListGetStart();
    Moby *end = mobyListGetEnd();
    while (start < end) {
        if (start->oClass == 0x1094 && start->pParent->oClass == 0x251c) {
            if (!gadgetInfo.vtable.update) {
                gadgetInfo.vtable.update = start->pUpdate;
            }
            MobyClassUpdate[start->mClass] = &M4244_Update_DualVipers;
            start->pUpdate = &M4244_Update_DualVipers;
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
    else if (MobyClassUpdate[M_CLASS] != &M4244_Update_DualVipers)
        MobyClassUpdate[M_CLASS] = &M4244_Update_DualVipers;
}