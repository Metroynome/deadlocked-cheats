#include <tamtypes.h>
#include <libdl/game.h>
#include <libdl/player.h>
#include <libdl/dl.h>
#include <libdl/moby.h>
#include <libdl/pad.h>
#include <libdl/utils.h>

#define WRENCH_REPLACEMENT_COUNT    (sizeof(replacementConfigs) / sizeof(replacementConfigs[0]))

typedef struct ReplacementConfig
{
	int oClass;
	float scale;
	float offsetX;
	float offsetZ;
	float offsetY;
} ReplacementConfig;

ReplacementConfig replacementConfigs[] = {
	{MOBY_ID_BETA_BOX, 0.04, -.5, 0, 0},
	{MOBY_ID_SKIN_RATCHET, 0.8, -.5, 0, 0},
	{MOBY_ID_PUMA, .09, 0, 0, 0},
	{MOBY_ID_CHICKEN, 0.25, 0, 0, 0},
	{MOBY_ID_UYA_RATCHET, 0.20, 0, 0, 0},
};

int replacementConfigIndex = 0;
ReplacementConfig *replacementConfig = &replacementConfigs[0];

int initialized = 0;

void stripReplacementCollision(Moby *moby)
{
	if (!moby)
		return;

	moby->collData = 0;
	moby->collActive = 0;
	moby->collCnt = 0;
}

void applyReplacementOffset(Moby *moby, Moby *parent)
{
	float x;
	float z;
	float y;

	if (!moby || !parent)
		return;

	x = replacementConfig->offsetX;
	z = replacementConfig->offsetZ;
	y = replacementConfig->offsetY;

	moby->pos[0] += (parent->rMtx.v0[0] * x) + (parent->rMtx.v1[0] * z) + (parent->rMtx.v2[0] * y);
	moby->pos[1] += (parent->rMtx.v0[1] * x) + (parent->rMtx.v1[1] * z) + (parent->rMtx.v2[1] * y);
	moby->pos[2] += (parent->rMtx.v0[2] * x) + (parent->rMtx.v1[2] * z) + (parent->rMtx.v2[2] * y);
}

void wrenchReplacementUpdate(Moby *moby)
{
	Moby *parent;

	if (!moby)
		return;

	parent = moby->pParent;
	if (parent) {
		vector_copy(moby->pos, parent->pos);
		vector_copy(moby->rot, parent->rot);
		applyReplacementOffset(moby, parent);
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
	moby->drawn = 1;
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
void updatePlayerReplacement(Player *player)
{
	Moby *replacement;

	if (!player)
		return;

	handleReplacementDebugInput(player);

	replacement = player->pWrenchReplacement;
	if (!replacement)
		return;

	if (replacement->oClass != replacementConfig->oClass) {
		player->pWrenchReplacement = 0;
		return;
	}

	setupReplacementMoby(replacement);
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



