#include <tamtypes.h>
#include <libdl/stdio.h>
#include <libdl/game.h>
#include <libdl/player.h>
#include <libdl/pad.h>
#include <libdl/dl.h>
#include <libdl/weapon.h>
#include <libdl/moby.h>
#include <libdl/utils.h>

#define MAX_WRENCH_MOBIES (16)

typedef struct CustomGear {
	char init;
	char count;
	char head[GAME_MAX_PLAYERS];
	char wrench[GAME_MAX_PLAYERS];
	Moby *wrenchMobies[MAX_WRENCH_MOBIES];
} CustomGear_t;
CustomGear_t gear;

int customMobies[] = {
	NULL,
	MOBY_ID_RATCHET,
	MOBY_ID_BETA_BOX,
};
int customMobyCount = sizeof(customMobies) / sizeof(customMobies[0]);

void gearSpawn(void)
{
	VECTOR z = {0, 0, 0, 0};
	int i;
	for (i = 1; i < customMobyCount; ++i) {
		Moby *m = mobySpawn(customMobies[i], 0x10);
		m->Drawn = 1;
		m->UpdateDist = 1000;
		m->PUpdate = 0x00437f50;
		vector_copy(m->Position, z);
		gear.wrenchMobies[i] = (Moby*)m;
		printf("\nspawned: %08x", gear.wrenchMobies[i]);
	}
	// memset(&gear.wrench, -1, sizeof(char) * 10);
}

Moby *gearWrench_Hook(int oClass, int size)
{
	Moby *a = mobySpawn(MOBY_ID_RATCHET, size);
	a->PUpdate = 0x00437f50;
	a->CollData = 0;
	return a;
}

void gearWrenchUpdate(void)
{
	HOOK_JAL(0x00438010, &gearWrench_Hook);
	POKE_U32(0x00437fc0, 0x50430005);
}

void gearInit(void)
{
	// gearSpawn();
	gearWrenchUpdate();
	gear.init = 1;
	printf("\ninited!");
}

void gearUpdate(int index, char *gear, int which)
{
	gear[index] = which;
}

void gearReplace(Player *p)
{
	int currentWrench = gear.wrench[p->MpIndex];
	if (currentWrench < 1) {
		*(u32*)((u32)p + 0x2fcc) = gear.wrenchMobies[2];
		gearUpdate(p->MpIndex, &gear.wrench, 2);
		printf("\nreplaced gear: %d", currentWrench);
	}
}

int main(void)
{
	dlPreUpdate();

	// Only Run if in game.
	if (!isInGame())
		return -1;

	if (!gear.init)
		gearInit();

	// Grab Player 1's data and run cheat logic.
	int i;
	Player **players = playerGetAll();
	for (i = 0; i < GAME_MAX_PLAYERS; ++i) {
		if (!players[i]) continue;

		// if (playerIsLocal(players[i]))
		// 	gearReplace(players[i]);
	}

	dlPostUpdate();

	return 0;
}
