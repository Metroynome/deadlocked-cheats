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

typedef struct magma_vtable {
    void (*update)(Moby* moby);
    void (*FUN_003eea38)(void);
    void (*GetMobyJointWorld)(Moby* moby, int joint, VECTOR* out);
    void (*FUN_003eea48)(Moby* moby);
    void (*MB_setState)(Moby* moby, int state, int a2);
    int  (*GadgetBox_GetGadgetLevel)(void* gadgetBox, int gadgetId);
    long (*Hero_PeekGadgetEvent)(Player* player, int a1, int a2, int a3);
    int  (*FUN_005f02c0)(Player* player);
    void (*MB_transAnim)(Moby* moby, int seq, float frm, int steps, int flags);
    void (*FUN_003ed308)(Player* player, void* out);
    void (*FUN_003eeec0)(Moby* moby, Player* player);
    long (*GUI_CancelRadarSelect)(int slot);
    void (*FUN_003eef48)(Moby* moby, Player* player, void* a2);
    long (*Hero_GetGadgetEvent)(Player* player, int a1, int a2, GadgetEvent* out);
    int  (*GB_GadgetIdToIndex)(int gadgetId);
    void (*FUN_003eea78)(Moby* moby, Player* player);
    void (*FUN_003ed778)(Moby* moby, void* a1);
    void* (*FUN_00449ab8)(Moby* moby, u8 upgradeFlag);
    int  (*GadgetBox_GetActivePostFXMod)(void* gadgetBox, int gadgetId);
    int  (*GadgetBox_GadgetIsModSupported)(int gadgetId, int mod);
    void (*GadgetBox_AddPoolMod)(void* gadgetBox, int mod, long a2, int gadgetId);
    void (*sound_MobyPlay)(int soundId, int a1, Moby* moby);
    int  (*MB_randRot)(void);
    void (*FUN_003ef770)(Moby* moby, void* a1);
    void (*FUN_003ed960)(Moby* moby, void* a1);
    int  (*MB_randRange)(float min, float max);
    void (*FUN_0043c110)(float a0, float a1, void* a2, long a3, int a4);
    void (*FUN_003eed80)(Moby* moby);
    void (*actuator_killWave)(void);
    void (*FUN_003eece8)(Moby* moby, Player* player, void* a2, void* a3);
    void (*WPN_TurnOnHoloShields)(int a0);
    void (*WPN_TurnOffHoloShields)(void);
    void (*FastDecTimer)(void* timer);
    void (*RegisterDrawFunction)(void** list, void* func, Moby* moby);
    void* (*Guber_GetObject)(void);
} magma_vtable_t;

typedef struct magmaInfo {
    short init;
    short runOldFunction;
    magma_vtable_t vtable;
} magmaInfo_t;
magmaInfo_t magmaInfo;

// Offsets verified against M4231_Update_MagmaCannon assembly (s3 = pvar)
typedef struct MagmaCannonPVar {
    u8      _pad00[0x40];   // 0x00: unknown preceding data
    Player* owner;          // 0x40: lw s1,0x40(pvar)
    u8      unk_44;         // 0x44: sb zero,0x44(s3)
    u8      unk_45;         // 0x45: sb zero,0x45(s3)
    u8      upgradeFlag;    // 0x46: upgrade level tier (0=base, 1=mid, 2=max)
    u8      _pad47;
    void*   unk_48;         // 0x48: result of FUN_00449ab8
    u32     targetMod;      // 0x4c: active post-fx mod
    int     actuatorHandle; // 0x50: rumble handle, -1 = inactive
    u8      _pad54[0xc];
    VECTOR  targetDir;      // 0x60: sq v0,0x60(s3) — 16-byte target direction
    float   unk_70;         // 0x70: swc1 f1,0x70(s3)
    float   unk_74;         // 0x74: swc1 f0,0x74(s3)
    float   unk_78;         // 0x78: swc1 f0,0x78(s3)
    float   unk_7c;         // 0x7c: swc1 f0,0x7c(s3)
    u32     unk_80;         // 0x80: sw v0,0x80(s3) = 4
    float   unk_84;         // 0x84: swc1 f1,0x84(s3)
    u16     timer_88;       // 0x88: sh v1,0x88(s3) = 3 — FastDecTimer target
    u16     timer_8a;       // 0x8a: sh v0,0x8a(s3) = 9 — FastDecTimer target
    float   unk_8c;         // 0x8c: swc1 f0,0x8c(s3)
} MagmaCannonPVar_t;


void M4231_Update_MagmaCannon(Moby* moby)
{
    // Vtable not ready yet — bail and let magmaCannon() retry gadgetInit next frame
    if (!magmaInfo.vtable.FUN_003eea38)
        return;

    MagmaCannonPVar_t* pvar = (MagmaCannonPVar_t*)moby->PVar;
    GadgetEvent gadgetEvent;
    memset(&gadgetEvent, 0, sizeof(GadgetEvent));
    long gadgetEventType = 0;

    magmaInfo.vtable.FUN_003eea38();

    // Save moby position and get joint 0 world position
    VECTOR savedPos;
    vector_copy(savedPos, moby->Position);
    VECTOR jointPos;
    magmaInfo.vtable.GetMobyJointWorld(moby, 0, &jointPos);


    if (!moby->State) {
        magmaInfo.vtable.FUN_003eea48(moby);
        magmaInfo.vtable.MB_setState(moby, 1, -1);
    }

    Player* player = pvar->owner;
    if (!player || (u32)player > 0x02000000)
        return;
    if (!(u32)player->GadgetBox)
        return;

    if (player->PlayerState == PLAYER_STATE_LOOK)
        playerGetVTable(player)->UpdateState(player, PLAYER_STATE_TARGETING, 1, 0, 1);

    // Determine upgrade tier from gadget level
    void* gadgetBox = player->GadgetBox;
    int gadgetLevel = magmaInfo.vtable.GadgetBox_GetGadgetLevel(gadgetBox, 3);
    if (gadgetLevel >= 0x62)
        pvar->upgradeFlag = 2;
    else if (gadgetLevel > 8)
        pvar->upgradeFlag = 1;

    // Skip remote player logic if _heroOpt is set
    if (*(u8*)0x002204c1 && player && !player->IsLocal) {
        long event = magmaInfo.vtable.Hero_PeekGadgetEvent(player, 0, 1, 0);
        if (event == 0 && magmaInfo.vtable.FUN_005f02c0(player) == 0) {
            if (moby->State != 2) {
                *(u8*)0x002204cb = 1;
                return;
            }
        }
    }

    // Update animation if mid-sequence
    if ((moby->AnimFlags & 2) != 0) {
        if (*(u32*)0x0021e694 == 1) {   // g_gameType: singleplayer freezes anim
            moby->AnimSpeed = 0.0f;
        } else {
            magmaInfo.vtable.MB_transAnim(moby, 1, 0.0f, 2, 0);
        }
    }

    void* aimData = (void*)((u32)moby->PVar); // sp (stack data passed as auStack_130)
    magmaInfo.vtable.FUN_003ed308(player, aimData);
    magmaInfo.vtable.FUN_003eeec0(moby, player);


    // ================
    // ==================CRASHES HERE
    // ================
    // Local player additional checks
    // if (player->IsLocal) {
    //     float hitPoints = player->Health;
    //     u32 state = player->PlayerState;
    //     if (hitPoints > 0.0f
    //         && state != PLAYER_STATE_LEDGE_GRAB
    //         && state != PLAYER_STATE_LEDGE_IDLE
    //         && state != PLAYER_STATE_LEDGE_TRAVERSE_LEFT
    //         && state != PLAYER_STATE_LEDGE_TRAVERSE_RIGHT) {
    //         void* vtable = *(void**)((u32)player + 0x14);
    //         int (*getSlot)(Player*) = *(void**)((u32)vtable + 0x4c);
    //         int slot = getSlot(player);
    //         long cancel = magmaInfo.vtable.GUI_CancelRadarSelect(slot);
    //         if (!cancel)
    //             magmaInfo.vtable.FUN_003eef48(moby, player, aimData);
    //     }
    // }
    // ====================================
    // NOTES:
    /*
        1. above commented code crashes.
        2. if above is commented, weapon shots but it has problems.
        3. no aimer is shown
        4. no texture effects show when I shoot.  (puff of smoke does though)
    */

    if (moby->State == 1) {
        gadgetEventType = magmaInfo.vtable.Hero_GetGadgetEvent(player, 0, 1, &gadgetEvent);

        if (gadgetEventType == 8) {
            // Increment shot counter for this gadget slot
            int idx = magmaInfo.vtable.GB_GadgetIdToIndex(3);
            u16* shotCounter = (u16*)((u32)0x0036d810 + idx * 2 + player->MpIndex * 0x12);
            (*shotCounter)++;

            // For remote players, copy target direction from gadget event
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

            pvar->unk_48 = (void*)magmaInfo.vtable.FUN_00449ab8(moby, pvar->upgradeFlag);
            if (pvar->unk_48) {
                void (*callback)(void*) = *(void**)((u32)pvar->unk_48 + 0xa8);
                if (callback)
                    callback(pvar->unk_48);
            }

            // Handle post-fx mod (singleplayer only)
            if (*(u32*)0x0021e694 == 1) {
                if (!player->IsLocal) {
                    u32 modIndex = gadgetEvent.gadgetEventMsg.ExtraData / 10;
                    pvar->targetMod = 0;
                    if (modIndex != 0) {
                        u32 activeMod = magmaInfo.vtable.GadgetBox_GetActivePostFXMod(gadgetBox, 3);
                        if (activeMod != modIndex) {
                            long supported = magmaInfo.vtable.GadgetBox_GadgetIsModSupported(3, modIndex);
                            if (supported)
                                magmaInfo.vtable.GadgetBox_AddPoolMod(gadgetBox, modIndex, -1, 3);
                        }
                    }
                } else {
                    pvar->targetMod = 0;
                }
            } else {
                pvar->targetMod = 0;
            }

            // Resolve target moby from event UID
            u32 targetUID = gadgetEvent.gadgetEventMsg.TargetUID;
            if (targetUID != *(u32*)0x002204b0) {
                void* guberObj = (void*)magmaInfo.vtable.Guber_GetObject();
                if (guberObj) {
                    void* (*getMoby)(void*) = *(void**)( *(u32*)((u32)guberObj + 0x14) + 0x10 );
                    pvar->targetMod = (u32)getMoby(guberObj);
                } else {
                    pvar->targetMod = 0;
                }
            }

            magmaInfo.vtable.sound_MobyPlay(0, 0, moby);

            pvar->timer_88 = 3;
            pvar->timer_8a = 9;
            pvar->unk_70 = 0.3333333f;  // 0x3eaaaaab
            pvar->unk_74 = 0.1111111f;  // 0x3de38e39
            pvar->unk_78 = (float)magmaInfo.vtable.MB_randRot();
            pvar->unk_7c = (float)magmaInfo.vtable.MB_randRot();
            pvar->unk_80 = 4;
            pvar->unk_84 = 0.25f;       // 0x3e800000
            pvar->unk_8c = (float)magmaInfo.vtable.MB_randRot();

            // Muzzle flash: get joint 0 world pos, compute velocity, spawn effect
            VECTOR muzzlePos;
            vector_copy(muzzlePos, moby->Position);
            magmaInfo.vtable.GetMobyJointWorld(moby, 0, &muzzlePos);
            magmaInfo.vtable.FUN_003ef770(moby, &muzzlePos);
            magmaInfo.vtable.FUN_003ed960(moby, aimData);

            VECTOR joint2Pos;
            magmaInfo.vtable.GetMobyJointWorld(moby, 2, &joint2Pos);

            // Scale rMtx.v2 by random speed and add scaled rMtx.v0 for ejection dir
            VECTOR ejVel;
            float speed1 = (float)magmaInfo.vtable.MB_randRange(3.5f, 6.25f) * (1.0f / 60.0f);
            vector_scale(ejVel, moby->M2_03, speed1);

            float speed2 = (float)magmaInfo.vtable.MB_randRange(4.5f, 7.5f) * (1.0f / 60.0f);
            VECTOR rotVel;
            vector_scale(rotVel, moby->M0_03, speed2);
            vector_add(ejVel, ejVel, rotVel);

            magmaInfo.vtable.FUN_0043c110(30.0f, 2.5f, (void*)0, *(long*)ejVel, 100);
            magmaInfo.vtable.MB_setState(moby, 2, -1);

        } else if (gadgetEventType == 4) {
            magmaInfo.vtable.FUN_003eed80(moby);
        }

        // Kill rumble if not firing
        int firing = gadgetEventType == 8 && *(u8*)((u32)player + 0x265a);
        if (!firing) {
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

    magmaInfo.vtable.update = (void*)start;
    MobyClassUpdate[M_CLASS] = &M4231_Update_MagmaCannon;
    magmaInfo.vtable.FUN_003eea38               = JAL2ADDR(*(u32*)(start + 0x028));
    magmaInfo.vtable.GetMobyJointWorld          = JAL2ADDR(*(u32*)(start + 0x040));
    magmaInfo.vtable.FUN_003eea48               = JAL2ADDR(*(u32*)(start + 0x054));
    magmaInfo.vtable.MB_setState                = JAL2ADDR(*(u32*)(start + 0x064));
    magmaInfo.vtable.GadgetBox_GetGadgetLevel   = JAL2ADDR(*(u32*)(start + 0x0b8));
    magmaInfo.vtable.Hero_PeekGadgetEvent       = JAL2ADDR(*(u32*)(start + 0x114));
    magmaInfo.vtable.FUN_005f02c0               = JAL2ADDR(*(u32*)(start + 0x128));
    magmaInfo.vtable.MB_transAnim               = JAL2ADDR(*(u32*)(start + 0x180));
    magmaInfo.vtable.FUN_003ed308               = JAL2ADDR(*(u32*)(start + 0x198));
    magmaInfo.vtable.FUN_003eeec0               = JAL2ADDR(*(u32*)(start + 0x1a4));
    magmaInfo.vtable.GUI_CancelRadarSelect      = JAL2ADDR(*(u32*)(start + 0x20c));
    magmaInfo.vtable.FUN_003eef48               = JAL2ADDR(*(u32*)(start + 0x220));
    magmaInfo.vtable.Hero_GetGadgetEvent        = JAL2ADDR(*(u32*)(start + 0x254));
    magmaInfo.vtable.GB_GadgetIdToIndex         = JAL2ADDR(*(u32*)(start + 0x274));
    magmaInfo.vtable.FUN_003eea78               = JAL2ADDR(*(u32*)(start + 0x2e0));
    magmaInfo.vtable.FUN_003ed778               = JAL2ADDR(*(u32*)(start + 0x2f0));
    magmaInfo.vtable.FUN_00449ab8               = JAL2ADDR(*(u32*)(start + 0x304));
    magmaInfo.vtable.GadgetBox_GetActivePostFXMod   = JAL2ADDR(*(u32*)(start + 0x368));
    magmaInfo.vtable.GadgetBox_GadgetIsModSupported = JAL2ADDR(*(u32*)(start + 0x378));
    magmaInfo.vtable.GadgetBox_AddPoolMod       = JAL2ADDR(*(u32*)(start + 0x390));
    magmaInfo.vtable.sound_MobyPlay             = JAL2ADDR(*(u32*)(start + 0x3f4));
    magmaInfo.vtable.MB_randRot                 = JAL2ADDR(*(u32*)(start + 0x428));
    magmaInfo.vtable.FUN_003ef770               = JAL2ADDR(*(u32*)(start + 0x478));
    magmaInfo.vtable.FUN_003ed960               = JAL2ADDR(*(u32*)(start + 0x484));
    magmaInfo.vtable.MB_randRange               = JAL2ADDR(*(u32*)(start + 0x4b0));
    magmaInfo.vtable.FUN_0043c110               = JAL2ADDR(*(u32*)(start + 0x528));
    magmaInfo.vtable.FUN_003eed80               = JAL2ADDR(*(u32*)(start + 0x550));
    magmaInfo.vtable.actuator_killWave          = JAL2ADDR(*(u32*)(start + 0x58c));
    magmaInfo.vtable.FUN_003eece8               = JAL2ADDR(*(u32*)(start + 0x5d4));
    magmaInfo.vtable.WPN_TurnOnHoloShields      = JAL2ADDR(*(u32*)(start + 0x5c0));
    magmaInfo.vtable.WPN_TurnOffHoloShields     = JAL2ADDR(*(u32*)(start + 0x5dc));
    magmaInfo.vtable.FastDecTimer               = JAL2ADDR(*(u32*)(start + 0x5e4));
    magmaInfo.vtable.RegisterDrawFunction       = JAL2ADDR(*(u32*)(start + 0x628));
    magmaInfo.vtable.Guber_GetObject            = JAL2ADDR(*(u32*)(start + 0x3b0));
    return 1;
}

void magmaCannon(void)
{
    if (!isInGame()) {
        if (magmaInfo.init)
            memset(&magmaInfo, 0, sizeof(magmaInfo_t));
        return;
    }

    // Keep retrying every frame until the moby class is loaded and vtable is populated
    if (!magmaInfo.init)
        magmaInfo.init = gadgetInit();
    // Even after init, if the hook somehow got cleared, re-hook
    else if (MobyClassUpdate[M_CLASS] != &M4231_Update_MagmaCannon)
        MobyClassUpdate[M_CLASS] = &M4231_Update_MagmaCannon;
}