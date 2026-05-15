#include <libdl/moby.h>
#include <libdl/math.h>
#include <libdl/math3d.h>
#include <libdl/player.h>
#include <libdl/game.h>
#include <libdl/utils.h>
#include <libdl/spawnpoint.h>>
#include <libdl/sound.h>
#include <libdl/player.h>

#define JAL2ADDR(jal) ((jal & 0x03FFFFFF) << 2)

typedef struct gadget_vtable {
    void (*update)(Moby* this);
    void (*GB_AssignLocalPlayerToWeapon)(Moby* moby, Player** outPlayer);
    void (*MB_setState)(Moby* moby, int state, int a2);
    long (*Hero_PeekGadgetEvent)(Player* player, int a1, int a2, int a3);
    int  (*FUN_005f02c0)(Player* player);
    void (*MB_transAnim)(int a0, Moby* moby, int a1, int a2, int a3);
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

typedef struct DualVipersPVar {
    VECTOR transformedPos;
    u8 _pad[0x30];
    u16 shotType;
    u16 shotCount;
    u32 actuatorHandle;
    Player* owner;
    u8  _pad50[6];
    u16 soundReady;
    u8  _pad50b[4];
    u32 idleTimer;
    u8  _pad60[4];
    int* shotStatePtr;
} DualVipersPVar_t;

void M4244_Update_DualVipers(Moby* moby)
{
    return;
    DualVipersPVar_t* pvar = (DualVipersPVar_t*)moby->PVar;

    // First-time initialization
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

    // Skip logic for non-local / unassigned players
    if (!player || !*(u32*)((u32)player + 0x266d)) {
        long event = gadgetInfo.vtable.Hero_PeekGadgetEvent(player, 0, 1, 0);
        if (event == 0 && gadgetInfo.vtable.FUN_005f02c0(player) == 0) {
            // DAT_002204cb = 1;
            return;
        }
    }

    // Update animation if mid-sequence
    if ((moby->AnimFlags & 2) != 0) {
        if (moby->AnimSeqId == 1) {
            moby->AnimSpeed = 0.0f;
        } else {
            gadgetInfo.vtable.MB_transAnim(0, moby, 1, 2, 0);
        }
    }

    // Update transformed world position
    VECTOR transformedPos;
    vector_apply(transformedPos, moby->Position, moby->M0_03);
    vector_add(transformedPos, transformedPos, moby->Position);
    vector_copy(pvar->transformedPos, transformedPos);

    gadgetInfo.vtable.FUN_003b6c28(moby, player);

    if (moby->SubState != 0) {
        // Firing sub-state: get gadget event and shoot if ready
        GadgetEvent gadgetEvent;
        gadgetInfo.vtable.Hero_GetGadgetEvent(player, 0, 1, &gadgetEvent);

        int equippedTime = player->Gadgets[0].equippedTime;
        int canShoot = (gadgetInfo.vtable.FUN_005f02c0(player) != 0) ? (equippedTime >= 0x17) : (equippedTime >= 0x17 || player->timers.noInput != 0);
        if (canShoot) {
            int event = moby->SubState == 1 ? 1 : 0;
            pvar->shotType = 0x14;
            pvar->shotStatePtr = NULL;
            pvar->shotCount++;

            if (event == 8) {
                gadgetInfo.vtable.DualVipers_ShootUpdate(moby, player, &gadgetEvent);
                moby->SubState = '\0';
                *(u8*)(*(int*)((u8*)pvar + 0x64) + 0x67) = 1;
            } else if (event == 4 && pvar->soundReady == 1) {
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

        // End firing animation once idle long enough
        if (firingTimer > 0xf && pvar->idleTimer >= 9)
            gadgetInfo.vtable.Hero_EndFiringAnim(player);

        // Re-arm sound after enough idle time
        if (pvar->idleTimer >= 0x15)
            pvar->soundReady = 1;

        if (pvar->shotCount < 1)
            pvar->shotCount = 0;
    }

    // Update rumble actuator
    int handle = pvar->actuatorHandle;
    long event = moby->SubState == 1 ? 1 : 0;
    if (event != 8 || *(u8*)((u32)player + 0x265a) == 0) {
        if (handle >= 0) {
            gadgetInfo.vtable.actuator_killWave(handle, *(u32*)((u32)player + 0x2f00));
            pvar->actuatorHandle = -1;
            handle = pvar->actuatorHandle;
        }
    }

    if (handle > 0 && gadgetInfo.vtable.FUN_004ad330(handle - 1, *(u32*)((u32)player + 0x2f00)) == 0)
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
            *(u32*)start->PUpdate = &M4244_Update_DualVipers;
        }
        ++start;
    }
}

int gadgetInit(void)
{
    // find all, and update moby update function to ours.
    gadgetFindAndHook();

    // store original functions.  Converts JAL to address.
    u32 start = gadgetInfo.vtable.update;
    printf("\nstart: 0x%08x", start);
    if (!start) return;
    gadgetInfo.vtable.GB_AssignLocalPlayerToWeapon = JAL2ADDR(*(u32*)(start + 0x40));
    gadgetInfo.vtable.MB_setState = JAL2ADDR(*(u32*)(start + 0x58));
    gadgetInfo.vtable.Hero_PeekGadgetEvent = JAL2ADDR(*(u32*)(start + 0xc4));
    gadgetInfo.vtable.FUN_005f02c0 = JAL2ADDR(*(u32*)(start + 0xd8));
    gadgetInfo.vtable.MB_transAnim = JAL2ADDR(*(u32*)(start + 0x11c));
    gadgetInfo.vtable.FUN_003b6c28 = JAL2ADDR(*(u32*)(start + 0x174));
    gadgetInfo.vtable.Hero_GetGadgetEvent = JAL2ADDR(*(u32*)(start + 0x198));
    gadgetInfo.vtable.DualVipers_ShootUpdate = JAL2ADDR(*(u32*)(start + 0x214));
    gadgetInfo.vtable.sound_MobyPlay = JAL2ADDR(*(u32*)(start + 0x254));
    gadgetInfo.vtable.Hero_QueueGadgetEvent = JAL2ADDR(*(u32*)(start + 0x290));
    gadgetInfo.vtable.Hero_EndFiringAnim = JAL2ADDR(*(u32*)(start + 0x2e0));
    gadgetInfo.vtable.actuator_killWave = JAL2ADDR(*(u32*)(start + 0x32c));
    gadgetInfo.vtable.FUN_004ad330 = JAL2ADDR(*(u32*)(start + 0x34C));
    return 1;
}

void dualVipers(void)
{
    if (!isInGame()) {
        // zero ui struct if not in game.
        if (gadgetInfo.init > 0)
            memset(&gadgetInfo, 0, sizeof(gadgetInfo_t));
        
        return;
    }
    
    if (gadgetInfo.init == 0)
        gadgetInfo.init = gadgetInit();

    // Player *p = playerGetFromSlot(0);
    // if (!p) return;
    // if (playerGetState(p) == PLAYER_STATE_MOON_JUMP) {
    //     vector_add(p->playerPosition, p->playerPosition, p->stickInput);
    // }

    // printf("\npState: %d, pType: %d", playerGetState(p), playerDeobfuscate(&p->stateType, DEOBFUSCATE_MODE_STATE));
}
