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
    void  (*HandleWeaponTargetValidation)(Moby* moby, Player* player, VECTOR* a2);
    long  (*Hero_GetGadgetEvent)(Player* player, int a1, int a2, GadgetEvent* out);
    int   (*GB_GadgetIdToIndex)(int gadgetId);
    void  (*UpdateWeaponAim)(Moby* moby, Player* player, VECTOR out);
    void  (*BuildMagmaCannonShot)(Moby* pMoby, Player* pHero, VECTOR targetPos);
    void* (*SpawnProjectile)(Moby* moby, u8 upgradeFlag);
    int   (*GadgetBox_GetActivePostFXMod)(void* gadgetBox, int gadgetId);
    int   (*GadgetBox_GadgetIsModSupported)(int gadgetId, int mod);
    void  (*GadgetBox_AddPoolMod)(void* gadgetBox, int mod, long a2, int gadgetId);
    void  (*sound_MobyPlay)(int soundId, int a1, Moby* moby);
    void  (*SpawnShotEffects)(Moby* moby, void* a1);
    void  (*ApplyDamage)(Moby* moby, void* a1);
    void  (*PlayMagmaCannonSound)(Moby* moby);
    void  (*actuator_killWave)(void);
    void  (*DoFireEffect)(Moby* moby, Player* player, VECTOR* a2, GadgetEvent* a3);
    void  (*WPN_TurnOnHoloShields)(int a0);
    void  (*WPN_TurnOffHoloShields)(void);
    void* (*Guber_GetObject)(void);
    void* (*buildWeaponProjectile)()
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

typedef struct MagmaCannonPVar
{
/* 0x00 */ EffectMobyPVar_t effect;
/* 0x20 */ VECTOR aim;
/* 0x30 */ VECTOR unk_30;
/* 0x40 */ Player* owner;
/* 0x44 */ u8 projectileActive;
/* 0x45 */ u8 shotQueued;
/* 0x46 */ u8 upgradeFlag;
/* 0x47 */ u8 pad_47;
/* 0x48 */ Moby* projectile;
/* 0x4c */ Moby* targetMoby;
/* 0x50 */ int actuatorHandle;
/* 0x54 */ u8 unk_54[0x0C];
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

void M4231_Update_MagmaCannon(Moby* this)
{
    VECTOR muzzlePos;
    VECTOR shotDir;
    VECTOR shellPos;
    VECTOR shellVel;
    GadgetEvent event;
    Hero* hero;
    GadgetBox* gadgetBox;
    MagmaCannonPVar_t* pvars;
    int gadgetLevel;
    int modId;
    int target;
    float scale;
    long gadgetEventType = 0;

    this->bangles |= 3;

    pvars = (MagmaCannonPVar_t*)this->pVar;
    vector_copy(muzzlePos, this->pos);
    magmaInfo.vtable.GetMobyJointWorld(this, 0, &muzzlePos);

    if (this->state == 0) {
        magmaCannon_Init(this);
        mobySetState(this, 1, -1);
    }

    hero = pvars->owner;
    if (!hero)
        return;

    gadgetBox = hero->pGadgetBox;
    if (!gadgetBox)
        return;

    if (hero->state == PLAYER_STATE_LOOK)
        playerGetVTable(hero)->InitBodyState(hero, 0x1D, 1, 0, 1);

    gadgetLevel = magmaInfo.vtable.GadgetBox_GetGadgetLevel(gadgetBox, 3);
    if (gadgetLevel >= 0x62)
        pvars->upgradeFlag = 2;
    else if (gadgetLevel > 8)
        pvars->upgradeFlag = 1;
    else
        pvars->upgradeFlag = 0;

    if (*(u8*)0x002204c1 && !hero->isLocal) {
        gadgetEventType = magmaInfo.vtable.Hero_PeekGadgetEvent(hero, 0, 1, 0);
        if (gadgetEventType == 0 && isUsingGadget(hero) == 0) {
            if (this->state != 2) {
                *(u8*)0x002204cb = 1;
                return;
            }
        }
    }
    if (this->animFlags & 2) {
        if (gameType == 1)
            this->animSpeed = 0;
        else
            magmaInfo.vtable.MB_transAnim(0, this, 1, 2, 0);
    }

    magmaInfo.vtable.buildWeaponProjectile(hero, pvars->aim);
    magmaInfo.vtable.UpdateWeaponGunpoint(this, hero);
    // if (hero->isLocal) {
    //     if (hero->hitPoints > 0 &&
    //         hero->state != PLAYER_STATE_LEDGE_GRAB &&
    //         hero->state != PLAYER_STATE_LEDGE_IDLE &&
    //         hero->state != PLAYER_STATE_LEDGE_TRAVERSE_LEFT &&
    //         hero->state != PLAYER_STATE_LEDGE_TRAVERSE_RIGHT)
    //     {
    //         target = playerGetVTable(hero)->GetSlot(hero);
    //         if (!magmaInfo.vtable.GUI_CancelRadarSelect(target))
    //             magmaInfo.vtable.HandleWeaponTargetValidation(this, hero, &event);
    //     }
    // }

    if (this->state == 1) {
        magmaInfo.vtable.Hero_GetGadgetEvent(hero, 0, 1, &event);
        if (gadgetEventType == 8) {
            // short* stat = (short*)((u32)&DAT_0036d810 + (GB_GadgetIdToIndex(3) * 2) + (hero->mpIndex * 0x12));
            // ++(*stat);

            if (!hero->isLocal)
                vector_copy(pvars->targetDir, event.gadgetEventMsg.targetDir);

            magmaInfo.vtable.UpdateWeaponAim(this, hero, muzzlePos);
            magmaInfo.vtable.BuildMagmaCannonShot(this, hero, shotDir);

            pvars->shotQueued = 0;
            pvars->projectileActive = 0;

            // pvars->projectile = magmaInfo.vtable.SpawnProjectile(this, pvars->upgradeFlag);
            // if (pvars->projectile && pvars->projectile->pUpdate)
            //     pvars->projectile->pUpdate(pvars->projectile);

            pvars->projectile = magmaInfo.vtable.SpawnProjectile(this, pvars->upgradeFlag);
            if (pvars->projectile) {
                void (*callback)(void*) = *(void**)(pvars->projectile->pUpdate);
                if (callback)
                    callback(pvars->projectile);
            }

            pvars->targetMoby = NULL;
            if (gameType == 1 && !hero->isLocal) {
                modId = event.gadgetEventMsg.extraData / 10;
                if (modId) {
                    if (magmaInfo.vtable.GadgetBox_GetActivePostFXMod(hero->pGadgetBox, 3) != modId && magmaInfo.vtable.GadgetBox_GadgetIsModSupported(3, modId)) {
                        magmaInfo.vtable.GadgetBox_AddPoolMod(hero->pGadgetBox, modId, -1, 3);
                    }
                }
            }
            if (event.gadgetEventMsg.targetUID != *(u32*)0x002204b0) {
                Guber* guber = magmaInfo.vtable.Guber_GetObject();
                if (guber)
                    pvars->targetMoby = guber->vtable->GetMoby(guber);
            }

            magmaInfo.vtable.sound_MobyPlay(0, 0, this);

            pvars->flashTimer = 3;
            pvars->glowTimer = 9;

            pvars->rotSpeed = 0.333333f;
            pvars->rotDamping = 0.111111f;
            pvars->randRotX = randRot();
            pvars->effectTimer = 4;
            pvars->effectScale = 0.25f;
            pvars->randRotY = randRot();
            pvars->randRotZ = randRot();

            memcpy(&shellPos, &this->pos, sizeof(VECTOR));
            magmaInfo.vtable.GetMobyJointWorld(this, 0, &shellPos);

            magmaInfo.vtable.SpawnShotEffects(this, &shellPos);
            magmaInfo.vtable.ApplyDamage(this, &shotDir);

            VECTOR upVel;
            magmaInfo.vtable.GetMobyJointWorld(this, 2, &shellPos);
            vector_copy(shellVel, this->rMtx.v2);
            scale = randRangeFloat(3.75f, 6.25f) * (1.f / 60.f);
            vector_scale(shellVel, shellVel, scale);
            vector_copy(upVel, this->rMtx.v0);
            scale = randRangeFloat(4.5f, 7.5f) * (1.f / 60.f);
            vector_scale(upVel, upVel, scale);
            vector_add(shellVel, shellVel, upVel);
            spawnMagmaCannonShell(30.f, 2.5f, shellPos, shellVel, 100);
            mobySetState(this, 2, -1);
        }
        else if (gadgetEventType == 4) {
            magmaInfo.vtable.PlayMagmaCannonSound(this);
        }
        if ((gadgetEventType != 8 || !hero->firing) && pvars->actuatorHandle >= 0) {
            magmaInfo.vtable.actuator_killWave();
            pvars->actuatorHandle = -1;
        }
    } else if (this->state == 2) {
        magmaInfo.vtable.UpdateWeaponAim(this, hero, muzzlePos);
        magmaInfo.vtable.BuildMagmaCannonShot(this, hero, shotDir);
        magmaInfo.vtable.WPN_TurnOnHoloShields(*(u32*)((u32)hero + 0x2F14));
        magmaInfo.vtable.DoFireEffect(this, hero, &event, &shotDir);
        magmaInfo.vtable.WPN_TurnOffHoloShields();
    }

    FastDecTimer(&pvars->glowTimer);
    FastDecTimer(&pvars->flashTimer);
    FastDecTimer(&pvars->effectTimer);

    if (hero && hero->isLocal && pvars->flashTimer)
        gfxRegisterDrawFunction((void**)0x0022251c, 0x003efd78, this);
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
    magmaInfo.vtable.buildWeaponProjectile          = JAL2ADDR(*(u32*)(start + 0x198));
    magmaInfo.vtable.UpdateWeaponGunpoint           = JAL2ADDR(*(u32*)(start + 0x1a4));
    magmaInfo.vtable.GUI_CancelRadarSelect          = JAL2ADDR(*(u32*)(start + 0x20c));
    magmaInfo.vtable.HandleWeaponTargetValidation   = JAL2ADDR(*(u32*)(start + 0x220));
    magmaInfo.vtable.Hero_GetGadgetEvent            = JAL2ADDR(*(u32*)(start + 0x254));
    magmaInfo.vtable.GB_GadgetIdToIndex             = JAL2ADDR(*(u32*)(start + 0x274));
    magmaInfo.vtable.UpdateWeaponAim                = JAL2ADDR(*(u32*)(start + 0x2e0));
    magmaInfo.vtable.BuildMagmaCannonShot           = JAL2ADDR(*(u32*)(start + 0x2f0));
    magmaInfo.vtable.SpawnProjectile                   = JAL2ADDR(*(u32*)(start + 0x304));
    magmaInfo.vtable.Guber_GetObject                = JAL2ADDR(*(u32*)(start + 0x3b0));
    magmaInfo.vtable.GadgetBox_GetActivePostFXMod   = JAL2ADDR(*(u32*)(start + 0x368));
    magmaInfo.vtable.GadgetBox_GadgetIsModSupported = JAL2ADDR(*(u32*)(start + 0x378));
    magmaInfo.vtable.GadgetBox_AddPoolMod           = JAL2ADDR(*(u32*)(start + 0x390));
    magmaInfo.vtable.sound_MobyPlay                 = JAL2ADDR(*(u32*)(start + 0x3f4));
    magmaInfo.vtable.SpawnShotEffects                   = JAL2ADDR(*(u32*)(start + 0x478));
    magmaInfo.vtable.ApplyDamage                   = JAL2ADDR(*(u32*)(start + 0x484));
    magmaInfo.vtable.PlayMagmaCannonSound                   = JAL2ADDR(*(u32*)(start + 0x550));
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