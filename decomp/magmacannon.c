#include <libdl/moby.h>
#include <libdl/math.h>
#include <libdl/math3d.h>
#include <libdl/player.h>
#include <libdl/game.h>
#include <libdl/utils.h>
#include <libdl/spawnpoint.h>
#include <libdl/sound.h>
#include <libdl/random.h>
#include <libdl/weapon.h>

#define JAL2ADDR(jal) ((jal & 0x03FFFFFF) << 2)
#define MobyClassUpdate ((void**)0x00249980)
#define M_CLASS (32)

typedef struct magma_vtable {
    void  (*update)(Moby* moby);
    void  (*GetMobyJointWorld)(Moby* moby, int joint, VECTOR* out);
    int   (*GadgetBox_GetGadgetLevel)(void* gadgetBox, int gadgetId);
    long  (*Hero_PeekGadgetEvent)(Player* player, int a1, int a2, int a3);
    void  (*MB_transAnim)(Moby* moby, int seq, float frm, int steps, int flags);
    void  (*FUN_003eeec0)(Moby* moby, Player* player);
    long  (*GUI_CancelRadarSelect)(int slot);
    void  (*FUN_003eef48)(Moby* moby, Player* player, VECTOR* a2);
    long  (*Hero_GetGadgetEvent)(Player* player, int a1, int a2, GadgetEvent* out);
    int   (*GB_GadgetIdToIndex)(int gadgetId);
    void  (*FUN_003eea78)(Moby* moby, Player* player);
    void  (*FUN_003ed778)(Moby* moby, void* a1);
    void* (*FUN_00449ab8)(Moby* moby, u8 upgradeFlag);
    int   (*GadgetBox_GetActivePostFXMod)(void* gadgetBox, int gadgetId);
    int   (*GadgetBox_GadgetIsModSupported)(int gadgetId, int mod);
    void  (*GadgetBox_AddPoolMod)(void* gadgetBox, int mod, long a2, int gadgetId);
    void  (*sound_MobyPlay)(int soundId, int a1, Moby* moby);
    void  (*FUN_003ef770)(Moby* moby, void* a1);
    void  (*FUN_003ed960)(Moby* moby, void* a1);
    void  (*FUN_003eed80)(Moby* moby);
    void  (*actuator_killWave)(void);
    void  (*FUN_003eece8)(Moby* moby, Player* player, VECTOR* a2, GadgetEvent* a3);
    void  (*WPN_TurnOnHoloShields)(int a0);
    void  (*WPN_TurnOffHoloShields)(void);
    void* (*Guber_GetObject)(void);
} magma_vtable_t;

typedef struct magmaInfo {
    short init;
    short runOldFunction;
    magma_vtable_t vtable;
} magmaInfo_t;
magmaInfo_t magmaInfo;

typedef struct EffectMobyPVar
{
    VECTOR direction;
    float param;
    short lifetime;
    short lifetime2;
    float randScale1;
    float randScale2;

} EffectMobyPVar_t;

typedef struct MagmaCannonPVar {
    EffectMobyPVar_t effect;
    char unk_20[0x20];
    Player* owner;          // 0x40
    u8    unk_44;           // 0x44
    u8    unk_45;           // 0x45
    u8    upgradeFlag;      // 0x46: 0=base, 1=mid, 2=max
    u8    _pad47;
    void* unk_48;           // 0x48: result of FUN_00449ab8
    u32   targetMod;        // 0x4c
    int   actuatorHandle;   // 0x50: -1 = inactive
    u8    _pad54[0xc];
    VECTOR targetDir;       // 0x60
    float unk_70;           // 0x70
    float unk_74;           // 0x74
    float unk_78;           // 0x78
    float unk_7c;           // 0x7c
    u32   unk_80;           // 0x80
    float unk_84;           // 0x84
    u16   timer_88;         // 0x88
    u16   timer_8a;         // 0x8a
    float unk_8c;           // 0x8c
} MagmaCannonPVar_t;

void GB_AssignLocalPlayerToWeapon(Moby* weaponMoby, Player** outPlayer)
{
    int i;
    for (i = 0; i < GAME_MAX_PLAYERS; ++i) {
        Player* player = playerGetFromSlot(i);
        if (!player) continue;

        // Primary gadget ownership check
        if (player->gadgets[0].pMoby == weaponMoby) {
            *outPlayer = player;
            weaponMoby->pNetObject = player;
            return;
        }

        // Secondary gadget ownership (dual/special weapons only)
        if (weaponMoby->oClass == MOBY_ID_DUAL_VIPERS && player->gadgets[0].pMoby2 == weaponMoby) {
            *outPlayer = player;
            weaponMoby->pNetObject = player;
            return;
        }
    }

    *outPlayer = NULL;
    weaponMoby->pNetObject = NULL;
}

int FastDecTimer(short* timer)
{
    if (*timer == 0) return 1;
    if (*timer > 0) --(*timer);
    return (*timer > 0) ? 0 : 2;
}

int isUsingGadget(Player *player)
{
    int idx = (player->mpIndex & 0xFF);
    return (idx < 10) && player->pGadgetBox->bButtonDown[idx] && player->stateType != PLAYER_TYPE_DEAD;
}

void magmaCannon_Init(Moby* moby)
{
    MagmaCannonPVar_t* pvar = (MagmaCannonPVar_t*)moby->pVar;
    pvar->actuatorHandle = -1;
    pvar->timer_88 = 0;
    pvar->timer_8a = 0;
    GB_AssignLocalPlayerToWeapon(moby, &pvar->owner);
}

Moby* spawnEffectMoby(float param, float scale, VECTOR position, VECTOR direction, short lifetime)
{
    Moby* moby = mobySpawn(0x2455, 0x20);
    if (!moby) return NULL;

    EffectMobyPVar_t* pvars = (EffectMobyPVar_t*)moby->pVar;
    moby->DrawDist = 0x20;
    moby->UpdateDist = 0xFF;
    vector_copy(moby->pos, position);
    moby->Scale *= scale * 0.4f;
    vector_copy(pvars->direction, direction);
    pvars->param = param;
    pvars->lifetime = lifetime;
    pvars->lifetime2 = lifetime - 15;
    pvars->randScale1 = randRangeFloat(0.105f, 0.21f);
    pvars->randScale2 = randRangeFloat(0.105f, 0.21f);
    mobySetState(moby, 0, -1);
    return moby;
}

void buildWeaponProjectile(Player* player, VECTOR out)
{
    if (!out || !player)
        return;

    // Base tuning vector
    out[0] = 25.0f;
    out[2] = 13.5f;
    out[1] = 30.0f;
    out[3] = 0.0f;

    // Multiplayer/game mode tweak
    if (*(int*)0x0021e694 == 1)
        out[1] = 20.0f;

    GadgetBox* gb = player->pGadgetBox;
    int mod = GB_GetActiveWeaponMod(gb, 3);

    // Tier 3 mod adjustments
    if (mod == 3) {
        out[0] -= 11.0f;
        out[1] += 4.0f;
    }

    // Re-evaluate mod (engine redundancy)
    mod = GB_GetActiveWeaponMod(gb, 3);

    // Tier 2 mod adjustments
    if (mod == 2) {
        out[0] += 16.0f;
        out[2] += 10.0f;
        out[1] -= 3.0f;
        out[3] += 1.0f;
    }

    // Final multiplayer clamp
    if (*(int*)0x0021e694 == 1)
        out[1] = 20.0f;
}

void updateGunPointTarget(Moby* weaponMoby, Player* player)
{
    if (!weaponMoby || !player)
        return;

    if (player->isLocal) {
        if (playerIsFPSModeActive(player)) {
            if (player->fps.fVars.target_blend_fac < 0.8f)
                return;

            player->attack.pGunPointMoby = player->fps.fVars.pTarget;
            return;
        }
    }
    GB_SetGunpointMobyIfTarget(player);
}

bool playerIsFPSModeActive(Player* player)
{
    if (!player)
        return false;

    if (!player->mpFpsEnabled)
        return false;

    if (player->timers.noFpsCamTimer != 0)
        return false;

    int gameMode = *(int*)0x0021ddb4;
    if (gameMode == 2 || gameMode == 6)
        return false;

    if (player->isLocal) {
        UpdateCam* cam = player->camera->pCurrentUpdCam;

        if (cam && cam->type == 5)
            return false;
    }

    switch (player->state) {
        case PLAYER_STATE_CUT_SCENE:
        case PLAYER_STATE_VEHICLE:
        case PLAYER_STATE_VISIBOMB:
        case PLAYER_STATE_GET_FLATTENED:
        case PLAYER_STATE_SKYDIVE:
            return false;
    }
    return player->mpFpsEnabled;
}

void M4231_Update_MagmaCannon(Moby* moby)
{
    // Vtable not ready — let magmaCannon() retry gadgetInit next frame
    if (!magmaInfo.vtable.GetMobyJointWorld)
        return;

    MagmaCannonPVar_t* pvar = (MagmaCannonPVar_t*)moby->pVar;
    GadgetEvent gadgetEvent = {0};
    long gadgetEventType = 0;

    // set bangles
    moby->bangles |= 3;

    // Save moby position and get joint 0 world position
    VECTOR savedPos;
    vector_copy(savedPos, moby->pos);
    VECTOR jointPos;
    magmaInfo.vtable.GetMobyJointWorld(moby, 0, &jointPos);

    if (!moby->state) {
        magmaCannon_Init(moby);
        mobySetState(moby, 1, -1);
    }

    Player* player = pvar->owner;
    if (!player || !player->pGadgetBox)
        return;

    if (player->state == PLAYER_STATE_LOOK)
        playerGetVTable(player)->InitBodyState(player, PLAYER_STATE_TARGETING, 1, 0, 1);

    // Determine upgrade tier from gadget level
    int gadgetLevel = magmaInfo.vtable.GadgetBox_GetGadgetLevel(player->pGadgetBox, 3);
    if (gadgetLevel >= 0x62)
        pvar->upgradeFlag = 2;
    else if (gadgetLevel > 8)
        pvar->upgradeFlag = 1;

    // Remote player early-out
    if (*(u8*)0x002204c1 && player && !player->isLocal) {
        long event = magmaInfo.vtable.Hero_PeekGadgetEvent(player, 0, 1, 0);
        if (event == 0 && !isUsingGadget(player)) {
            if (moby->state != 2) {
                *(u8*)0x002204cb = 1;
                return;
            }
        }
    }

    // Update animation if mid-sequence
    if ((moby->animFlags & 2) != 0) {
        if (*(u32*)0x0021e694 == 1) {   // g_gameType
            moby->animSpeed = 0.0f;
        } else {
            magmaInfo.vtable.MB_transAnim(moby, 1, 0.0f, 2, 0);
        }
    }

    // Stack buffer for aim data (auStack_130 in Ghidra = sp+0x00, frame size 0x130)
    VECTOR aimData[2];
    buildWeaponProjectile(player, aimData);
    magmaInfo.vtable.FUN_003eeec0(moby, player);

    // Local player checks
    // if (player->isLocal) {
    //     if (player->Health > 0.0f && player->state != PLAYER_STATE_LEDGE_GRAB && player->state != PLAYER_STATE_LEDGE_IDLE && player->state != PLAYER_STATE_LEDGE_TRAVERSE_LEFT && player->state != PLAYER_STATE_LEDGE_TRAVERSE_RIGHT) {
    //         int slot = *(u8*)((u32)p[layer + 0x2fea]);
    //         long cancel = magmaInfo.vtable.GUI_CancelRadarSelect(slot);
    //         if (!cancel)
    //             magmaInfo.vtable.FUN_003eef48(moby, player, aimData);
    //     }
    // }

    if (moby->state == 1) {
        gadgetEventType = magmaInfo.vtable.Hero_GetGadgetEvent(player, 0, 1, &gadgetEvent);

        if (gadgetEventType == 8) {
            int idx = magmaInfo.vtable.GB_GadgetIdToIndex(3);
            u16* shotCounter = (u16*)((u32)0x0036d810 + idx * 2 + player->mpIndex * 0x12);
            (*shotCounter)++;

            // Remote players: copy target direction from gadget event
            if (!player->isLocal) {
                pvar->targetDir[0] = gadgetEvent.gadgetEventMsg.targetDir[0];
                pvar->targetDir[1] = gadgetEvent.gadgetEventMsg.targetDir[1];
                pvar->targetDir[2] = gadgetEvent.gadgetEventMsg.targetDir[2];
                pvar->targetDir[3] = *(float*)0x0022100c;
            }

            magmaInfo.vtable.FUN_003eea78(moby, player);
            magmaInfo.vtable.FUN_003ed778(moby, &savedPos);

            pvar->unk_45 = 0;
            pvar->unk_44 = 0;

            pvar->unk_48 = magmaInfo.vtable.FUN_00449ab8(moby, pvar->upgradeFlag);
            if (pvar->unk_48) {
                void (*callback)(void*) = *(void**)((u32)pvar->unk_48 + 0xa8);
                if (callback)
                    callback(pvar->unk_48);
            }

            // Post-fx mod (singleplayer only)
            pvar->targetMod = 0;
            if (*(u32*)0x0021e694 == 1 && !player->isLocal) {
                u32 modIndex = gadgetEvent.gadgetEventMsg.extraData / 10;
                if (modIndex != 0) {
                    u32 activeMod = magmaInfo.vtable.GadgetBox_GetActivePostFXMod(player->pGadgetBox, 3);
                    if (activeMod != modIndex) {
                        long supported = magmaInfo.vtable.GadgetBox_GadgetIsModSupported(3, modIndex);
                        if (supported)
                            magmaInfo.vtable.GadgetBox_AddPoolMod(player->pGadgetBox, modIndex, -1, 3);
                    }
                }
            }

            // Resolve target moby from event UID
            if (gadgetEvent.gadgetEventMsg.targetUID != *(u32*)0x002204b0) {
                void* guberObj = magmaInfo.vtable.Guber_GetObject();
                if (guberObj) {
                    void* (*getMoby)(void*) = *(void**)(*(u32*)((u32)guberObj + 0x14) + 0x10);
                    pvar->targetMod = (u32)getMoby(guberObj);
                }
            }

            magmaInfo.vtable.sound_MobyPlay(0, 0, moby);

            pvar->timer_88 = 3;
            pvar->timer_8a = 9;
            pvar->unk_70   = 0.3333333f;
            pvar->unk_74   = 0.1111111f;
            pvar->unk_78   = randRot();
            pvar->unk_7c   = randRot();
            pvar->unk_80   = 4;
            pvar->unk_84   = 0.25f;
            pvar->unk_8c   = randRot();

            // Muzzle flash and ejection velocity
            VECTOR muzzlePos;
            vector_copy(muzzlePos, moby->pos);
            magmaInfo.vtable.GetMobyJointWorld(moby, 0, &muzzlePos);
            magmaInfo.vtable.FUN_003ef770(moby, &muzzlePos);
            magmaInfo.vtable.FUN_003ed960(moby, aimData);

            VECTOR joint2Pos;
            magmaInfo.vtable.GetMobyJointWorld(moby, 2, &joint2Pos);

            // Shell/ejection effect velocity
            VECTOR ejVel, rotVel;
            float speed1 = randRangeFloat(3.5f, 6.25f) * (1.0f / 60.0f);
            vector_scale(ejVel, moby->rMtx.v2, speed1);
            float speed2 = randRangeFloat(4.5f, 7.5f) * (1.0f / 60.0f);
            vector_scale(rotVel, moby->rMtx.v0, speed2);
            vector_add(ejVel, ejVel, rotVel);

            // Spawn shell/effect moby
            spawnEffectMoby(30.0f, 2.5f, joint2Pos, ejVel, 100);
            mobySetState(moby, 2, -1);

        } else if (gadgetEventType == 4) {
            magmaInfo.vtable.FUN_003eed80(moby);
        }

        // Kill rumble if not firing
        if (gadgetEventType != 8 || !*(u8*)((u32)player + 0x265a)) {
            if (pvar->actuatorHandle >= 0) {
                magmaInfo.vtable.actuator_killWave();
                pvar->actuatorHandle = -1;
            }
        }

    } else if (moby->state == 2) {
        magmaInfo.vtable.FUN_003eea78(moby, player);
        magmaInfo.vtable.FUN_003ed778(moby, &gadgetEvent);
        magmaInfo.vtable.WPN_TurnOnHoloShields(player->mpTeam);
        magmaInfo.vtable.FUN_003eece8(moby, player, aimData, &gadgetEvent);
        magmaInfo.vtable.WPN_TurnOffHoloShields();
    }

    FastDecTimer(&pvar->timer_8a);
    FastDecTimer(&pvar->timer_88);
    FastDecTimer(&pvar->unk_80);

    if (player && player->isLocal && pvar->timer_88 != 0)
        gfxRegisterDrawFunction((void**)0x0022251c, 0x003efd78, moby);
}

int gadgetInit(void)
{
    u32 start = (u32)MobyClassUpdate[M_CLASS];
    if (!start || start == (u32)&M4231_Update_MagmaCannon) return 0;

    magmaInfo.vtable.update                         = (void*)start;
    MobyClassUpdate[M_CLASS]                        = &M4231_Update_MagmaCannon;
    magmaInfo.vtable.GetMobyJointWorld              = JAL2ADDR(*(u32*)(start + 0x040));
    magmaInfo.vtable.GadgetBox_GetGadgetLevel       = JAL2ADDR(*(u32*)(start + 0x0b8));
    magmaInfo.vtable.Hero_PeekGadgetEvent           = JAL2ADDR(*(u32*)(start + 0x114));
    magmaInfo.vtable.MB_transAnim                   = JAL2ADDR(*(u32*)(start + 0x180));
    magmaInfo.vtable.FUN_003eeec0                   = JAL2ADDR(*(u32*)(start + 0x1a4));
    magmaInfo.vtable.GUI_CancelRadarSelect          = JAL2ADDR(*(u32*)(start + 0x20c));
    magmaInfo.vtable.FUN_003eef48                   = JAL2ADDR(*(u32*)(start + 0x220));
    magmaInfo.vtable.Hero_GetGadgetEvent            = JAL2ADDR(*(u32*)(start + 0x254));
    magmaInfo.vtable.GB_GadgetIdToIndex             = JAL2ADDR(*(u32*)(start + 0x274));
    magmaInfo.vtable.FUN_003eea78                   = JAL2ADDR(*(u32*)(start + 0x2e0));
    magmaInfo.vtable.FUN_003ed778                   = JAL2ADDR(*(u32*)(start + 0x2f0));
    magmaInfo.vtable.FUN_00449ab8                   = JAL2ADDR(*(u32*)(start + 0x304));
    magmaInfo.vtable.Guber_GetObject                = JAL2ADDR(*(u32*)(start + 0x3b0));
    magmaInfo.vtable.GadgetBox_GetActivePostFXMod   = JAL2ADDR(*(u32*)(start + 0x368));
    magmaInfo.vtable.GadgetBox_GadgetIsModSupported = JAL2ADDR(*(u32*)(start + 0x378));
    magmaInfo.vtable.GadgetBox_AddPoolMod           = JAL2ADDR(*(u32*)(start + 0x390));
    magmaInfo.vtable.sound_MobyPlay                 = JAL2ADDR(*(u32*)(start + 0x3f4));
    magmaInfo.vtable.FUN_003ef770                   = JAL2ADDR(*(u32*)(start + 0x478));
    magmaInfo.vtable.FUN_003ed960                   = JAL2ADDR(*(u32*)(start + 0x484));
    magmaInfo.vtable.FUN_003eed80                   = JAL2ADDR(*(u32*)(start + 0x550));
    magmaInfo.vtable.WPN_TurnOnHoloShields          = JAL2ADDR(*(u32*)(start + 0x5c0));
    magmaInfo.vtable.actuator_killWave              = JAL2ADDR(*(u32*)(start + 0x58c));
    magmaInfo.vtable.FUN_003eece8                   = JAL2ADDR(*(u32*)(start + 0x5d4));
    magmaInfo.vtable.WPN_TurnOffHoloShields         = JAL2ADDR(*(u32*)(start + 0x5dc));
    return 1;
}

void magmaCannon(void)
{
    if (!isInGame()) {
        if (magmaInfo.init)
            memset(&magmaInfo, 0, sizeof(magmaInfo_t));
        return;
    }

    if (!magmaInfo.init)
        magmaInfo.init = gadgetInit();
    else if (MobyClassUpdate[M_CLASS] != &M4231_Update_MagmaCannon)
        MobyClassUpdate[M_CLASS] = &M4231_Update_MagmaCannon;
}