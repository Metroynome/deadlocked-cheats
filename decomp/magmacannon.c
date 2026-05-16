#include <libdl/moby.h>
#include <libdl/math.h>
#include <libdl/math3d.h>
#include <libdl/player.h>
#include <libdl/game.h>
#include <libdl/utils.h>
#include <libdl/spawnpoint.h>
#include <libdl/sound.h>

#define JAL2ADDR(jal) ((jal & 0x03FFFFFF) << 2)
#define MobyClassUpdate ((void**)0x00249980)
#define M_CLASS (32)

typedef struct magma_vtable {
    void  (*update)(Moby* moby);
    void  (*GetMobyJointWorld)(Moby* moby, int joint, VECTOR* out);
    void  (*FUN_003eea48)(Moby* moby);
    int   (*GadgetBox_GetGadgetLevel)(void* gadgetBox, int gadgetId);
    long  (*Hero_PeekGadgetEvent)(Player* player, int a1, int a2, int a3);
    void  (*MB_transAnim)(Moby* moby, int seq, float frm, int steps, int flags);
    void  (*FUN_003ed308)(Player* player, VECTOR* out);
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
    int   (*MB_randRot)(void);
    void  (*FUN_003ef770)(Moby* moby, void* a1);
    void  (*FUN_003ed960)(Moby* moby, void* a1);
    int   (*MB_randRange)(float min, float max);
    void  (*FUN_003eed80)(Moby* moby);
    void  (*actuator_killWave)(void);
    void  (*FUN_003eece8)(Moby* moby, Player* player, VECTOR* a2, GadgetEvent* a3);
    void  (*WPN_TurnOnHoloShields)(int a0);
    void  (*WPN_TurnOffHoloShields)(void);
    void  (*FastDecTimer)(void* timer);
    void  (*RegisterDrawFunction)(void** list, void* func, Moby* moby);
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

int isUsingGadget(Player *player)
{
    int idx = (player->MpIndex & 0xFF);
    return (idx < 10) && player->GadgetBox->bButtonDown[idx] && player->PlayerStateType != 20;
}

Moby* spawnEffectMoby(float param, float scale, VECTOR position, VECTOR direction, short lifetime)
{
    Moby* moby = mobySpawn(0x2455, 0x20);
    if (!moby) return NULL;

    EffectMobyPVar_t* pvars = (EffectMobyPVar_t*)moby->PVar;
    moby->DrawDist = 0x20;
    moby->UpdateDist = 0xFF;
    vector_copy(moby->Position, position);
    moby->Scale *= scale * 0.4f;
    vector_copy(pvars->direction, direction);
    pvars->param = param;
    pvars->lifetime = lifetime;
    pvars->lifetime2 = lifetime - 15;
    pvars->randScale1 = randRange(0.105f, 0.21f);
    pvars->randScale2 = randRange(0.105f, 0.21f);
    mobySetState(moby, 0, -1);
    return moby;
}

void M4231_Update_MagmaCannon(Moby* moby)
{
    // Vtable not ready — let magmaCannon() retry gadgetInit next frame
    if (!magmaInfo.vtable.GetMobyJointWorld)
        return;

    MagmaCannonPVar_t* pvar = (MagmaCannonPVar_t*)moby->PVar;
    GadgetEvent gadgetEvent = {0};
    long gadgetEventType = 0;

    // set bangles
    *(u16*)(moby + 0x74) |= 3;

    // Save moby position and get joint 0 world position
    VECTOR savedPos;
    vector_copy(savedPos, moby->Position);
    VECTOR jointPos;
    magmaInfo.vtable.GetMobyJointWorld(moby, 0, &jointPos);

    if (!moby->State) {
        magmaInfo.vtable.FUN_003eea48(moby);
        mobySetState(moby, 1, -1);
    }

    Player* player = pvar->owner;
    if (!player || !player->GadgetBox)
        return;

    if (player->PlayerState == PLAYER_STATE_LOOK)
        playerGetVTable(player)->UpdateState(player, PLAYER_STATE_TARGETING, 1, 0, 1);

    // Determine upgrade tier from gadget level
    int gadgetLevel = magmaInfo.vtable.GadgetBox_GetGadgetLevel(player->GadgetBox, 3);
    if (gadgetLevel >= 0x62)
        pvar->upgradeFlag = 2;
    else if (gadgetLevel > 8)
        pvar->upgradeFlag = 1;

    // Remote player early-out
    if (*(u8*)0x002204c1 && player && !player->IsLocal) {
        long event = magmaInfo.vtable.Hero_PeekGadgetEvent(player, 0, 1, 0);
        if (event == 0 && !isUsingGadget(player)) {
            if (moby->State != 2) {
                *(u8*)0x002204cb = 1;
                return;
            }
        }
    }

    // Update animation if mid-sequence
    if ((moby->AnimFlags & 2) != 0) {
        if (*(u32*)0x0021e694 == 1) {   // g_gameType
            moby->AnimSpeed = 0.0f;
        } else {
            magmaInfo.vtable.MB_transAnim(moby, 1, 0.0f, 2, 0);
        }
    }

    // Stack buffer for aim data (auStack_130 in Ghidra = sp+0x00, frame size 0x130)
    VECTOR aimData[2];
    magmaInfo.vtable.FUN_003ed308(player, aimData);
    magmaInfo.vtable.FUN_003eeec0(moby, player);

    // Local player checks
    // if (player->IsLocal) {
    //     if (player->Health > 0.0f
    //         && player->PlayerState != PLAYER_STATE_LEDGE_GRAB
    //         && player->PlayerState != PLAYER_STATE_LEDGE_IDLE
    //         && player->PlayerState != PLAYER_STATE_LEDGE_TRAVERSE_LEFT
    //         && player->PlayerState != PLAYER_STATE_LEDGE_TRAVERSE_RIGHT) {
    //         void* vtable = *(void**)((u32)player + 0x14);
    //         int (*getSlot)(Player*) = *(void**)((u32)vtable + 0x4c);
    //         int slot = getSlot(player);
    //         long cancel = magmaInfo.vtable.GUI_CancelRadarSelect(slot);
    //         if (!cancel)
    //             magmaInfo.vtable.FUN_003eef48(moby, player, aimData);
    //     }
    // }

    if (moby->State == 1) {
        gadgetEventType = magmaInfo.vtable.Hero_GetGadgetEvent(player, 0, 1, &gadgetEvent);

        if (gadgetEventType == 8) {
            int idx = magmaInfo.vtable.GB_GadgetIdToIndex(3);
            u16* shotCounter = (u16*)((u32)0x0036d810 + idx * 2 + player->MpIndex * 0x12);
            (*shotCounter)++;

            // Remote players: copy target direction from gadget event
            if (!player->IsLocal) {
                pvar->targetDir[0] = gadgetEvent.gadgetEventMsg.TargetDir[0];
                pvar->targetDir[1] = gadgetEvent.gadgetEventMsg.TargetDir[1];
                pvar->targetDir[2] = gadgetEvent.gadgetEventMsg.TargetDir[2];
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
            if (*(u32*)0x0021e694 == 1 && !player->IsLocal) {
                u32 modIndex = gadgetEvent.gadgetEventMsg.ExtraData / 10;
                if (modIndex != 0) {
                    u32 activeMod = magmaInfo.vtable.GadgetBox_GetActivePostFXMod(player->GadgetBox, 3);
                    if (activeMod != modIndex) {
                        long supported = magmaInfo.vtable.GadgetBox_GadgetIsModSupported(3, modIndex);
                        if (supported)
                            magmaInfo.vtable.GadgetBox_AddPoolMod(player->GadgetBox, modIndex, -1, 3);
                    }
                }
            }

            // Resolve target moby from event UID
            if (gadgetEvent.gadgetEventMsg.TargetUID != *(u32*)0x002204b0) {
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
            pvar->unk_78   = (float)magmaInfo.vtable.MB_randRot();
            pvar->unk_7c   = (float)magmaInfo.vtable.MB_randRot();
            pvar->unk_80   = 4;
            pvar->unk_84   = 0.25f;
            pvar->unk_8c   = (float)magmaInfo.vtable.MB_randRot();

            // Muzzle flash and ejection velocity
            VECTOR muzzlePos;
            vector_copy(muzzlePos, moby->Position);
            magmaInfo.vtable.GetMobyJointWorld(moby, 0, &muzzlePos);
            magmaInfo.vtable.FUN_003ef770(moby, &muzzlePos);
            magmaInfo.vtable.FUN_003ed960(moby, aimData);

            VECTOR joint2Pos;
            magmaInfo.vtable.GetMobyJointWorld(moby, 2, &joint2Pos);

            // Shell/ejection effect velocity
            VECTOR ejVel, rotVel;
            float speed1 = magmaInfo.vtable.MB_randRange(3.5f, 6.25f) * (1.0f / 60.0f);
            vector_scale(ejVel, moby->M2_03, speed1);
            float speed2 = randRange(4.5f, 7.5f) * (1.0f / 60.0f);
            vector_scale(rotVel, moby->M0_03, speed2);
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

    } else if (moby->State == 2) {
        magmaInfo.vtable.FUN_003eea78(moby, player);
        magmaInfo.vtable.FUN_003ed778(moby, &gadgetEvent);
        magmaInfo.vtable.WPN_TurnOnHoloShields(player->Team);
        magmaInfo.vtable.FUN_003eece8(moby, player, aimData, &gadgetEvent);
        magmaInfo.vtable.WPN_TurnOffHoloShields();
    }

    magmaInfo.vtable.FastDecTimer(&pvar->timer_8a);
    magmaInfo.vtable.FastDecTimer(&pvar->timer_88);
    magmaInfo.vtable.FastDecTimer(&pvar->unk_80);

    if (player && player->IsLocal && pvar->timer_88 != 0)
        magmaInfo.vtable.RegisterDrawFunction((void**)0x0022251c, (void*)0x003efd78, moby);
}

int gadgetInit(void)
{
    u32 start = (u32)MobyClassUpdate[M_CLASS];
    if (!start || start == (u32)&M4231_Update_MagmaCannon) return 0;

    magmaInfo.vtable.update                         = (void*)start;
    MobyClassUpdate[M_CLASS]                        = &M4231_Update_MagmaCannon;
    magmaInfo.vtable.GetMobyJointWorld              = JAL2ADDR(*(u32*)(start + 0x040));
    magmaInfo.vtable.FUN_003eea48                   = JAL2ADDR(*(u32*)(start + 0x054));
    magmaInfo.vtable.GadgetBox_GetGadgetLevel       = JAL2ADDR(*(u32*)(start + 0x0b8));
    magmaInfo.vtable.Hero_PeekGadgetEvent           = JAL2ADDR(*(u32*)(start + 0x114));
    magmaInfo.vtable.MB_transAnim                   = JAL2ADDR(*(u32*)(start + 0x180));
    magmaInfo.vtable.FUN_003ed308                   = JAL2ADDR(*(u32*)(start + 0x198));
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
    magmaInfo.vtable.MB_randRot                     = JAL2ADDR(*(u32*)(start + 0x428));
    magmaInfo.vtable.FUN_003ef770                   = JAL2ADDR(*(u32*)(start + 0x478));
    magmaInfo.vtable.FUN_003ed960                   = JAL2ADDR(*(u32*)(start + 0x484));
    magmaInfo.vtable.MB_randRange                   = JAL2ADDR(*(u32*)(start + 0x4b0));
    magmaInfo.vtable.FUN_003eed80                   = JAL2ADDR(*(u32*)(start + 0x550));
    magmaInfo.vtable.WPN_TurnOnHoloShields          = JAL2ADDR(*(u32*)(start + 0x5c0));
    magmaInfo.vtable.actuator_killWave              = JAL2ADDR(*(u32*)(start + 0x58c));
    magmaInfo.vtable.FUN_003eece8                   = JAL2ADDR(*(u32*)(start + 0x5d4));
    magmaInfo.vtable.WPN_TurnOffHoloShields         = JAL2ADDR(*(u32*)(start + 0x5dc));
    magmaInfo.vtable.FastDecTimer                   = JAL2ADDR(*(u32*)(start + 0x5e4));
    magmaInfo.vtable.RegisterDrawFunction           = JAL2ADDR(*(u32*)(start + 0x628));
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