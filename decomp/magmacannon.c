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
#include <libdl/camera.h>

#define JAL2ADDR(jal) ((jal & 0x03FFFFFF) << 2)
#define MobyClassUpdate ((void**)0x00249980)
#define M_CLASS (32)

typedef struct magma_vtable {
    void  (*update)(Moby* moby);
    void  (*GetMobyJointWorld)(Moby* moby, int joint, VECTOR* out);
    int   (*GadgetBox_GetGadgetLevel)(void* gadgetBox, int gadgetId);
    long  (*Hero_PeekGadgetEvent)(Player* player, int a1, int a2, int a3);
    void  (*MB_transAnim)(Moby* moby, int seq, float frm, int steps, int flags);
    void  (*UpdateWeaponGunpoint)(Moby* moby, Player* player);
    long  (*GUI_CancelRadarSelect)(int slot);
    void  (*HandleReticle)(Moby* moby, Player* player, VECTOR* output);
    long  (*Hero_GetGadgetEvent)(Player* player, int a1, int a2, GadgetEvent* out);
    int   (*GB_GadgetIdToIndex)(int gadgetId);
    void  (*UpdateWeaponAim)(Moby* moby, Player* player, VECTOR *out);
    void  (*BuildMagmaCannonShot)(Moby* pMoby, void* out);
    void* (*SpawnProjectile)(Moby* moby, u8 upgradeFlag);
    int   (*GadgetBox_GetActivePostFXMod)(void* gadgetBox, int gadgetId);
    int   (*GadgetBox_GadgetIsModSupported)(int gadgetId, int mod);
    void  (*GadgetBox_AddPoolMod)(void* gadgetBox, int mod, long a2, int gadgetId);
    void  (*sound_MobyPlay)(int soundId, int a1, Moby* moby);
    void  (*SpawnShotEffects)(Moby* moby, VECTOR* a1);
    void  (*ApplyDamage)(Moby* moby, void* shotData);
    void  (*PlayMagmaCannonSound)(Moby* moby);
    void  (*actuator_killWave)(void);
    void  (*DoFireEffect)(Moby* moby, Player* player, void* a2, void* a3);
    void  (*WPN_TurnOnHoloShields)(int a0);
    void  (*WPN_TurnOffHoloShields)(void);
    void* (*Guber_GetObject)(void);
    void* (*BuildWeaponProjectile)(Player* player, VECTOR* output);
} magma_vtable_t;

typedef struct magmaInfo {
    short init;
    short runOldFunction;
    magma_vtable_t vtable;
} magmaInfo_t;
magmaInfo_t magmaInfo;

typedef struct EffectMobyPVar { // 0x20
    VECTOR direction;
    float param;
    short lifetime;
    short lifetime2;
    float randScale1;
    float randScale2;
} EffectMobyPVar_t;

typedef struct MagmaCannonPVar {
/* 0x00 */ VECTOR aimDir;        // normalized direction toward target
/* 0x10 */ VECTOR rightAxis;     // lateral axis (cross product derived, negated)
/* 0x20 */ VECTOR upAxis;        // up axis (completes orthonormal basis)
/* 0x30 */ VECTOR targetPos;  
/* 0x40 */ Player* owner;
/* 0x44 */ bool projectileActive;
/* 0x45 */ bool shotQueued;
/* 0x46 */ u8 upgradeFlag;
/* 0x47 */ char pad_47;
/* 0x48 */ Moby* projectile;
/* 0x4c */ Moby* targetMoby;
/* 0x50 */ int actuatorHandle;
/* 0x54 */ char unk_54[0x0C];
/* 0x60 */ VECTOR targetDir;
/* 0x70 */ float rotSpeed;
/* 0x74 */ float rotDamping;
/* 0x78 */ float randRotX;
/* 0x7c */ float randRotY;
/* 0x80 */ int effectTimer;
/* 0x84 */ float effectScale;
/* 0x88 */ short flashTimer;
/* 0x8a */ short glowTimer;
/* 0x8c */ float randRotZ;
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
            weaponMoby->netObject = player;
            return;
        }

        // Secondary gadget ownership (dual/special weapons only)
        if (weaponMoby->oClass == MOBY_ID_DUAL_VIPERS && player->gadgets[0].pMoby2 == weaponMoby) {
            *outPlayer = player;
            weaponMoby->netObject = player;
            return;
        }
    }

    *outPlayer = NULL;
    weaponMoby->netObject = NULL;
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
    return (idx < 10) && player->pGadgetBox->bButtonDown[idx] && player->stateType != PLAYER_TYPE_DEATH;
}

void magmaCannon_Init(Moby* moby)
{
    MagmaCannonPVar_t* pvar = (MagmaCannonPVar_t*)moby->pVar;
    pvar->actuatorHandle = -1;
    pvar->flashTimer = 0;
    pvar->glowTimer = 0;
    GB_AssignLocalPlayerToWeapon(moby, &pvar->owner);
}

Moby* spawnMagmaCannonShell(float param, float scale, VECTOR position, VECTOR direction, short lifetime)
{
    Moby* moby = mobySpawn(0x2455, 0x20);
    if (!moby) return NULL;

    EffectMobyPVar_t* pvars = (EffectMobyPVar_t*)moby->pVar;
    moby->drawDist = 0x20;
    moby->updateDist = 0xFF;
    vector_copy(moby->pos, position);
    moby->scale *= scale * 0.4f;
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

int playerIsFPSModeActive(Player *this)
{
    if (!Settings.FirstPersonModeOn[this->mpIndex])
        return false;

    if (this->timers.noFpsCamTimer != 0)
        return false;

    if (gameMode == GAME_MODE_SCENE || gameMode == GAME_MODE_SPACE)
        return false;

    // Local players cannot use FPS mode during certain camera updates
    if (this->isLocal) {
        struct UpdateCam *cam = this->camera->pCurrentUpdCam;
        if (cam && cam->type == 5)
            return false;
    }

    // Hero states that disable FPS mode
    switch (this->state) {
        case PLAYER_STATE_CUT_SCENE:
        case PLAYER_STATE_VEHICLE:
        case PLAYER_STATE_VISIBOMB:
        case PLAYER_STATE_GET_FLATTENED:
        case PLAYER_STATE_SKYDIVE:
            return false;
    }

    return true;
}

int playerGetSlot(Player *player)
{
    return player->LocalHero.slot;
}



void M4231_Update_MagmaCannon(Moby* moby)
{
    if (!magmaInfo.vtable.GetMobyJointWorld)
        return;

    printf("\nnew mag: start");

    VECTOR reticleData;
    VECTOR damageData;
    VECTOR aimData;
    VECTOR jointPos;
    struct COLL_DAM_IN shotData;
    MagmaCannonPVar_t* pvar = (MagmaCannonPVar_t*)moby->pVar;
    GadgetEvent gadgetEvent = {0};
    long gadgetEventType = 0;

    *(u16*)(moby + 0x74) |= 3;

    vector_copy(jointPos, moby->pos);
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

    int gadgetLevel = magmaInfo.vtable.GadgetBox_GetGadgetLevel(player->pGadgetBox, 3);
    if (gadgetLevel >= 0x62)
        pvar->upgradeFlag = 2;
    else if (gadgetLevel > 8)
        pvar->upgradeFlag = 1;

    printf("\nnew mag: after gadgetLevel");

    if (*(u8*)0x002204c1 && player && !player->isLocal) {
        long event = magmaInfo.vtable.Hero_PeekGadgetEvent(player, 0, 1, 0);
        if (event == 0 && !isUsingGadget(player)) {
            if (moby->state != 2) {
                *(u8*)0x002204cb = 1;
                return;
            }
        }
    }

    if ((moby->animFlags & 2) != 0) {
        if (*(u32*)0x0021e694 == 1) {
            moby->animSpeed = 0.0f;
            *(u32*)((u32)moby + 0x48) = 0;
        } else {
            magmaInfo.vtable.MB_transAnim(moby, 1, 0.0f, 2, 0);
        }
    }

    magmaInfo.vtable.BuildWeaponProjectile(player, aimData);
    magmaInfo.vtable.UpdateWeaponGunpoint(moby, player);
    

    if (player->isLocal) {
        if (player->hitPoints > 0.0f
            && player->state != PLAYER_STATE_LEDGE_GRAB
            && player->state != PLAYER_STATE_LEDGE_IDLE
            && player->state != PLAYER_STATE_LEDGE_TRAVERSE_LEFT
            && player->state != PLAYER_STATE_LEDGE_TRAVERSE_RIGHT) {
            // u32 slot = playerGetVTable(player)->getSlot(player);
            // long cancel = magmaInfo.vtable.GUI_CancelRadarSelect(slot);
            // if (!cancel)
                magmaInfo.vtable.HandleReticle(moby, player, aimData);
        }
    }
    printf("\nnew mag: after handleReticle");
    if (moby->state == 1) {
        gadgetEventType = magmaInfo.vtable.Hero_GetGadgetEvent(player, 0, 1, &gadgetEvent);

        if (gadgetEventType == 8) {
            int idx = magmaInfo.vtable.GB_GadgetIdToIndex(3);
            u16* shotCounter = (u16*)((u32)0x0036d810 + idx * 2 + player->mpIndex * 0x12);
            (*shotCounter)++;

            if (!player->isLocal) {
                pvar->targetDir[0] = gadgetEvent.gadgetEventMsg.targetDir[0];
                pvar->targetDir[1] = gadgetEvent.gadgetEventMsg.targetDir[1];
                pvar->targetDir[2] = gadgetEvent.gadgetEventMsg.targetDir[2];
                pvar->targetDir[3] = *(float*)0x0022100c;
            }
        
            printf("\nnew mag: after pvar->targetDir");

            magmaInfo.vtable.UpdateWeaponAim(moby, player, aimData);
            magmaInfo.vtable.BuildMagmaCannonShot(moby, &shotData);

            pvar->shotQueued = 0;
            pvar->projectileActive = 0;

            printf("\nnew mag: after pvar->shotQueueued");

            pvar->projectile = magmaInfo.vtable.SpawnProjectile(moby, pvar->upgradeFlag);
            if (pvar->projectile) {
                void (*callback)(void*) = *(void**)((u32)pvar->projectile + 0xa8);
                if (callback)
                    callback(pvar->projectile);
            }

            printf("\nnew mag: after pvar->projectile");

            pvar->targetMoby = 0;
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

            if (gadgetEvent.gadgetEventMsg.targetUID != *(u32*)0x002204b0) {
                void* guberObj = magmaInfo.vtable.Guber_GetObject();
                if (guberObj) {
                    void* (*getMoby)(void*) = *(void**)(*(u32*)((u32)guberObj + 0x14) + 0x10);
                    pvar->targetMoby = (u32)getMoby(guberObj);
                }
            }

            magmaInfo.vtable.sound_MobyPlay(0, 0, moby);

            pvar->flashTimer  = 3;
            pvar->glowTimer   = 9;
            pvar->rotSpeed    = 0.333333f;          // 0x70
            pvar->rotDamping  = 0.111111f;          // 0x74
            pvar->effectTimer = 4;                  // 0x80
            pvar->effectScale = 0.25f;             // 0x84
            pvar->randRotX = randRot();
            pvar->randRotY = randRot();
            pvar->randRotZ = randRot();
            
            VECTOR joint0Pos;
            // vector_copy(joint0Pos, moby->pos);
            magmaInfo.vtable.GetMobyJointWorld(moby, 0, &joint0Pos);
            magmaInfo.vtable.SpawnShotEffects(moby, &joint0Pos);
            magmaInfo.vtable.ApplyDamage(moby, &shotData);

            VECTOR joint2Pos;
            magmaInfo.vtable.GetMobyJointWorld(moby, 2, &joint2Pos);

            VECTOR ejVel, rotVel;
            float speed1 = randRangeFloat(3.5f, 6.25f) * (1.0f / 60.0f);
            vector_scale(ejVel, moby->rMtx.v2, speed1);
            float speed2 = randRangeFloat(4.5f, 7.5f) * (1.0f / 60.0f);
            vector_scale(rotVel, moby->rMtx.v0, speed2);
            vector_add(ejVel, ejVel, rotVel);

            printf("\nnew mag: after shell math");


            spawnMagmaCannonShell(30.0f, 2.5f, joint2Pos, ejVel, 100);
            mobySetState(moby, 2, -1);

        } else if (gadgetEventType == 4) {
            magmaInfo.vtable.PlayMagmaCannonSound(moby);
        }

        if (gadgetEventType != 8 || !*(u8*)((u32)player + 0x265a)) {
            if (pvar->actuatorHandle >= 0) {
                magmaInfo.vtable.actuator_killWave();
                pvar->actuatorHandle = -1;
            }
        }

    } else if (moby->state == 2) {
        magmaInfo.vtable.UpdateWeaponAim(moby, player, jointPos);
        magmaInfo.vtable.BuildMagmaCannonShot(moby, (struct COLL_DAM_IN*)&gadgetEvent);
        magmaInfo.vtable.WPN_TurnOnHoloShields(player->mpTeam);
        magmaInfo.vtable.DoFireEffect(moby, player, aimData, (struct COLL_DAM_IN*)&gadgetEvent);
        magmaInfo.vtable.WPN_TurnOffHoloShields();
    }

    FastDecTimer(&pvar->glowTimer);
    FastDecTimer(&pvar->flashTimer);
    FastDecTimer(&pvar->effectTimer);

    // lwu loads both flashTimer(0x88) and glowTimer(0x8a) as one u32
    if (player && player->isLocal && *(u32*)((u32)pvar + 0x88) != 0)
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
    magmaInfo.vtable.BuildWeaponProjectile          = JAL2ADDR(*(u32*)(start + 0x198));
    magmaInfo.vtable.UpdateWeaponGunpoint           = JAL2ADDR(*(u32*)(start + 0x1a4));
    magmaInfo.vtable.GUI_CancelRadarSelect          = JAL2ADDR(*(u32*)(start + 0x20c));
    magmaInfo.vtable.HandleReticle                  = JAL2ADDR(*(u32*)(start + 0x220));
    magmaInfo.vtable.Hero_GetGadgetEvent            = JAL2ADDR(*(u32*)(start + 0x254));
    magmaInfo.vtable.GB_GadgetIdToIndex             = JAL2ADDR(*(u32*)(start + 0x274));
    magmaInfo.vtable.UpdateWeaponAim                = JAL2ADDR(*(u32*)(start + 0x2e0));
    magmaInfo.vtable.BuildMagmaCannonShot           = JAL2ADDR(*(u32*)(start + 0x2f0));
    magmaInfo.vtable.SpawnProjectile                = JAL2ADDR(*(u32*)(start + 0x304));
    magmaInfo.vtable.Guber_GetObject                = JAL2ADDR(*(u32*)(start + 0x3b0));
    magmaInfo.vtable.GadgetBox_GetActivePostFXMod   = JAL2ADDR(*(u32*)(start + 0x368));
    magmaInfo.vtable.GadgetBox_GadgetIsModSupported = JAL2ADDR(*(u32*)(start + 0x378));
    magmaInfo.vtable.GadgetBox_AddPoolMod           = JAL2ADDR(*(u32*)(start + 0x390));
    magmaInfo.vtable.sound_MobyPlay                 = JAL2ADDR(*(u32*)(start + 0x3f4));
    magmaInfo.vtable.SpawnShotEffects               = JAL2ADDR(*(u32*)(start + 0x478));
    magmaInfo.vtable.ApplyDamage                    = JAL2ADDR(*(u32*)(start + 0x484));
    magmaInfo.vtable.PlayMagmaCannonSound           = JAL2ADDR(*(u32*)(start + 0x550));
    magmaInfo.vtable.WPN_TurnOnHoloShields          = JAL2ADDR(*(u32*)(start + 0x5c0));
    magmaInfo.vtable.actuator_killWave              = JAL2ADDR(*(u32*)(start + 0x58c));
    magmaInfo.vtable.DoFireEffect                   = JAL2ADDR(*(u32*)(start + 0x5d4));
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