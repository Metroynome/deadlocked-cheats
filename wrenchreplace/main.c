#include <tamtypes.h>
#include <libdl/game.h>
#include <libdl/player.h>
#include <libdl/dl.h>
#include <libdl/moby.h>
#include <libdl/utils.h>

#define WRENCH_REPLACEMENT_MOBY_ID  (MOBY_ID_BETA_BOX)
#define WRENCH_REPLACEMENT_SCALE    (0.08f)

int initialized = 0;

void stripReplacementCollision(Moby *moby)
{
	if (!moby)
		return;

	moby->collData = 0;
	moby->collActive = 0;
	moby->collCnt = 0;
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
	moby->scale = WRENCH_REPLACEMENT_SCALE;
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
	moby->scale = WRENCH_REPLACEMENT_SCALE;
}

Moby *wrenchReplacementSpawnHook(int oClass, int pVarSize)
{
	Moby *moby = mobySpawn(WRENCH_REPLACEMENT_MOBY_ID, pVarSize);

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

void updatePlayerReplacement(Player *player)
{
	Moby *replacement;

	if (!player)
		return;

	replacement = player->pWrenchReplacement;
	if (!replacement)
		return;

	if (replacement->oClass != WRENCH_REPLACEMENT_MOBY_ID) {
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



