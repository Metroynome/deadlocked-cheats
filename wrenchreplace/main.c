#include <tamtypes.h>
#include <libdl/game.h>
#include <libdl/player.h>
#include <libdl/dl.h>
#include <libdl/moby.h>
#include <libdl/pad.h>
#include <libdl/utils.h>

#define WRENCH_REPLACEMENT_COUNT    (sizeof(replacementConfigs) / sizeof(replacementConfigs[0]))
#define WRENCH_SPECIAL_EFFECT_TOGGLE (1)
#define WRENCH_THROWING_STATE       (10)
#define WRENCH_RETURNING_STATE      (11)
#define WRENCH_GADGET_ID            (1)
#define PUMA_FLATTEN_DIST_SQR      (4.0f)
#define WRENCH_MOD_CHEAT_ACTIVE    (*(u8*)0x0021DE3C)

typedef enum ReplacementRotationMode
{
	ROTATION_SIMPLE = 0,
	ROTATION_EXACT = 1
} ReplacementRotationMode;

typedef struct ReplacementConfig
{
	int oClass;
	float scale;
	float offsetX;
	float offsetZ;
	float offsetY;
	int rotationMode;
} ReplacementConfig;

ReplacementConfig replacementConfigs[] = {
	{MOBY_ID_BETA_BOX, 0.04, -.5, 0, 0, ROTATION_EXACT},
	{MOBY_ID_SKIN_RATCHET, 0.8, -.5, 0, 0, ROTATION_EXACT},
	{MOBY_ID_PUMA, .09, 0, 0, 0, ROTATION_EXACT},
	{MOBY_ID_CHICKEN, 0.25, 0, 0, 0, ROTATION_SIMPLE},
	{MOBY_ID_UYA_RATCHET, 0.20, 0, 0, 0, ROTATION_EXACT},
};

int replacementConfigIndex = 0;
ReplacementConfig *replacementConfig = &replacementConfigs[0];

int initialized = 0;
int pumaFlattenArmed[GAME_MAX_PLAYERS];

void stripReplacementCollision(Moby *moby)
{
	if (!moby)
		return;

	moby->collData = 0;
	moby->collActive = 0;
	moby->collCnt = 0;
}

void applyReplacementOffset(Moby *moby, Moby *basis)
{
	float x;
	float z;
	float y;

	if (!moby || !basis)
		return;

	x = replacementConfig->offsetX;
	z = replacementConfig->offsetZ;
	y = replacementConfig->offsetY;

	moby->pos[0] += (basis->rMtx.v0[0] * x) + (basis->rMtx.v1[0] * z) + (basis->rMtx.v2[0] * y);
	moby->pos[1] += (basis->rMtx.v0[1] * x) + (basis->rMtx.v1[1] * z) + (basis->rMtx.v2[1] * y);
	moby->pos[2] += (basis->rMtx.v0[2] * x) + (basis->rMtx.v1[2] * z) + (basis->rMtx.v2[2] * y);
}

Moby *getPlayerWrenchMoby(Player *player)
{
	Moby *wrench;

	if (!player)
		return 0;

	wrench = player->gadgets[0].pMoby;
	if (!wrench || mobyIsDestroyed(wrench))
		return 0;

	return wrench;
}
int wrenchReplacementIsThrown(Moby *wrench)
{
	return wrench && (wrench->state == WRENCH_THROWING_STATE || wrench->state == WRENCH_RETURNING_STATE);
}

int playerIsHoldingWrench(Player *player)
{
	if (!player)
		return 0;

	return player->gadgets[0].id == WRENCH_GADGET_ID;
}

int playerIsDyingOrDead(Player *player)
{
	if (!player)
		return 0;

	return player->stateType == PLAYER_TYPE_DEATH || player->hitPoints <= 0;
}

int shouldDrawReplacement(Player *player, Moby *wrench)
{
	if (!player || !wrench || mobyIsDestroyed(wrench))
		return 0;

	if (wrenchReplacementIsThrown(wrench))
		return 1;

	if (player->gadgetsOff || player->hideWeapon || player->hideWrench || player->noWrenchEquip)
		return 0;

	if (!playerIsHoldingWrench(player))
		return 0;

	return player->gadgets[0].state != 3;
}

void hideReplacementMoby(Moby *moby)
{
	if (!moby)
		return;

	moby->modeBits |= MOBY_MODE_BIT_HIDDEN;
	moby->drawn = 0;
	stripReplacementCollision(moby);
}

void wrenchReplacementUpdate(Moby *moby)
{
	Player *owner;
	Moby *basis;
	Moby *parent;
	int useExactRotation;

	if (!moby)
		return;

	owner = 0;
	if (moby->pVar)
		owner = *(Player**)moby->pVar;

	parent = moby->pParent;
	if (!shouldDrawReplacement(owner, parent)) {
		if (playerIsDyingOrDead(owner))
			return;
		hideReplacementMoby(moby);
		return;
	}

	useExactRotation = replacementConfig->rotationMode == ROTATION_EXACT && owner && !wrenchReplacementIsThrown(parent);
	basis = wrenchReplacementIsThrown(parent) ? parent : ((owner && owner->pMoby) ? owner->pMoby : parent);
	if (parent) {
		if (useExactRotation) {
			vector_copy(moby->pos, owner->joints.gadgetMtxs[0].v3);
			matrix_toeuler((float*)&owner->joints.gadgetMtxs[0], moby->rot);
			mobyUpdateTransform(moby);
			applyReplacementOffset(moby, moby);
		} else {
			vector_copy(moby->pos, parent->pos);
			vector_copy(moby->rot, basis ? basis->rot : parent->rot);
			applyReplacementOffset(moby, basis);
		}

		moby->lights[0] = parent->lights[0];
		moby->lights[1] = parent->lights[1];
		moby->drawDist = parent->drawDist;
		moby->updateDist = parent->updateDist;
		parent->modeBits |= MOBY_MODE_BIT_HIDDEN;
		parent->drawn = 0;
	}

	stripReplacementCollision(moby);
	moby->modeBits &= ~MOBY_MODE_BIT_HIDDEN;
	moby->drawn = 1;
	moby->scale = replacementConfig->scale;
	mobyUpdateTransform(moby);
}
void setupReplacementMoby(Moby *moby)
{
	if (!moby)
		return;

	stripReplacementCollision(moby);
	moby->updateDist = 0xff;
	moby->drawDist = 0x7fff;
	moby->pUpdate = &wrenchReplacementUpdate;
	moby->scale = replacementConfig->scale;
}

Moby *wrenchReplacementSpawnHook(int oClass, int pVarSize)
{
	Moby *moby = mobySpawn(replacementConfig->oClass, pVarSize);

	setupReplacementMoby(moby);
	return moby;
}

Moby *spawnPlayerReplacement(Player *player)
{
	Moby *moby;
	Moby *wrench;

	if (!player)
		return 0;

	wrench = getPlayerWrenchMoby(player);
	if (!wrench)
		return 0;

	moby = mobySpawn(replacementConfig->oClass, 0x10);
	if (!moby)
		return 0;

	setupReplacementMoby(moby);
	moby->pParent = wrench;
	if (moby->pVar)
		*(Player**)moby->pVar = player;
	player->pWrenchReplacement = moby;
	wrenchReplacementUpdate(moby);
	return moby;
}
void initWrenchReplacement(void)
{
	HOOK_JAL(0x00438010, &wrenchReplacementSpawnHook);

	/* In multiplayer, vanilla skips the replacement branch before checking pWrenchReplacement.
	 * Keep the branch inside the replacement path so the vanilla MB_SpawnInit still runs.
	 */
	POKE_U32(0x00437fc0, 0x50430005);

	initialized = 1;
}

void setReplacementConfigIndex(int index)
{
	int count = WRENCH_REPLACEMENT_COUNT;

	while (index < 0)
		index += count;

	replacementConfigIndex = index % count;
	replacementConfig = &replacementConfigs[replacementConfigIndex];
}

void destroyPlayerReplacement(Player *player)
{
	if (!player || !player->pWrenchReplacement)
		return;

	mobyDestroy(player->pWrenchReplacement);
	player->pWrenchReplacement = 0;
}

void handleReplacementDebugInput(Player *player)
{
	if (!player)
		return;

	if (playerPadGetButton(player, PAD_L1) <= 0)
		return;

	if (playerPadGetButtonDown(player, PAD_RIGHT) > 0) {
		setReplacementConfigIndex(replacementConfigIndex + 1);
		destroyPlayerReplacement(player);
	} else if (playerPadGetButtonDown(player, PAD_LEFT) > 0) {
		setReplacementConfigIndex(replacementConfigIndex - 1);
		destroyPlayerReplacement(player);
	}
}
int isPumaReturningToPlayer(Player *player, Moby *replacement)
{
	Moby *wrench;
	float dx;
	float dz;
	float dy;

	if (!player || !player->pMoby || !replacement)
		return 0;

	if (replacementConfig->oClass != MOBY_ID_PUMA)
		return 0;

	wrench = replacement->pParent;
	if (!wrench || wrench->state != WRENCH_RETURNING_STATE)
		return 0;

	dx = wrench->pos[0] - player->pMoby->pos[0];
	dz = wrench->pos[1] - player->pMoby->pos[1];
	dy = wrench->pos[2] - player->pMoby->pos[2];

	return ((dx * dx) + (dz * dz) + (dy * dy)) <= PUMA_FLATTEN_DIST_SQR;
}

void updatePumaFlatten(Player *player, Moby *replacement)
{
	int playerIdx;

	if (!WRENCH_SPECIAL_EFFECT_TOGGLE)
		return;

	if (!player)
		return;

	playerIdx = player->mpIndex;
	if (playerIdx < 0 || playerIdx >= GAME_MAX_PLAYERS)
		playerIdx = 0;

	if (replacementConfig->oClass != MOBY_ID_PUMA || !replacement || !replacement->pParent || replacement->pParent->state != WRENCH_RETURNING_STATE) {
		pumaFlattenArmed[playerIdx] = 1;
		return;
	}

	if (!pumaFlattenArmed[playerIdx])
		return;

	if (!isPumaReturningToPlayer(player, replacement))
		return;

	pumaFlattenArmed[playerIdx] = 0;
	playerGetVTable(player)->InitBodyState(player, PLAYER_STATE_GET_FLATTENED, 1, 0, 1);
}
void updatePlayerReplacement(Player *player)
{
	Moby *replacement;
	Moby *wrench;

	if (!player)
		return;

	WRENCH_MOD_CHEAT_ACTIVE = 1;
	handleReplacementDebugInput(player);

	wrench = getPlayerWrenchMoby(player);
	replacement = player->pWrenchReplacement;

	if (!shouldDrawReplacement(player, wrench)) {
		if (playerIsDyingOrDead(player))
			return;
		if (replacement && !mobyIsDestroyed(replacement))
			mobyDestroy(replacement);
		player->pWrenchReplacement = 0;
		return;
	}

	if (!replacement || mobyIsDestroyed(replacement) || replacement->oClass != replacementConfig->oClass) {
		player->pWrenchReplacement = 0;
		replacement = spawnPlayerReplacement(player);
		if (!replacement)
			return;
	}

	if (wrench)
		replacement->pParent = wrench;
	if (replacement->pVar)
		*(Player**)replacement->pVar = player;

	setupReplacementMoby(replacement);
	wrenchReplacementUpdate(replacement);
	updatePumaFlatten(player, replacement);
}

void wrenchReplace(void)
{
	int i;
	Player **players = playerGetAll();

	if (!initialized)
		initWrenchReplacement();

	for (i = 0; i < GAME_MAX_PLAYERS; ++i) {
		updatePlayerReplacement(players[i]);
	}
}

int main(void)
{
	dlPreUpdate();

	if (!isInGame())
		return -1;

	wrenchReplace();

	dlPostUpdate();

	return 0;
}



