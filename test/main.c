#include <tamtypes.h>

#include <libdl/stdio.h>
#include <libdl/string.h>
#include <libdl/player.h>
#include <libdl/utils.h>
#include <libdl/game.h>
#include <libdl/pad.h>
#include <libdl/dl.h>
#include <libdl/weapon.h>
#include <libdl/moby.h>
#include <libdl/graphics.h>
#include <libdl/gamesettings.h>
#include <libdl/spawnpoint.h>
#include <libdl/team.h>
#include <libdl/ui.h>
#include <libdl/time.h>
#include <libdl/camera.h>
#include <libdl/gameplay.h>
#include <libdl/map.h>
#include <libdl/collision.h>
#include <libdl/guber.h>
#include <libdl/sound.h>

#define TestMoby	(*(Moby**)0x00091004)

void DebugInGame(Player* player)
{
    if (playerPadGetButtonDown(player, PAD_LEFT) > 0) {
		// Nothing Yet!
	} else if (playerPadGetButtonDown(player, PAD_RIGHT) > 0) {
		// Nothing Yet!
	} else if (playerPadGetButtonDown(player, PAD_UP) > 0) {
		// Nothing Yet!
	} else if(playerPadGetButtonDown(player, PAD_DOWN) > 0) {
		// Nothing Yet!
	} else if (playerPadGetButtonDown(player, PAD_L3) > 0) {
		// Nothing Yet!
	} else if (playerPadGetButtonDown(player, PAD_R3) > 0) {
		// Nothing Yet!
	}
}

void DebugInMenus(void)
{
    if (padGetButtonDown(0, PAD_LEFT) > 0) {
		// Nothing Yet!
	} else if (padGetButtonDown(0, PAD_RIGHT) > 0) {
		// Nothing Yet!
	} else if (padGetButtonDown(0, PAD_UP) > 0) {
		// Nothing Yet!
	} else if(padGetButtonDown(0, PAD_DOWN) > 0) {
		// Nothing Yet!
	} else if (padGetButtonDown(0, PAD_L3) > 0) {
		// Nothing Yet!
	} else if (padGetButtonDown(0, PAD_R3) > 0) {
		// Nothing Yet!
	}
}

void InfiniteChargeboot(void)
{
	int i;
	Player** players = playerGetAll();
	for (i = 0; i < GAME_MAX_PLAYERS; ++i){
		Player * p = players[i];
		if (!p)
			continue;

		if (p->PlayerState == PLAYER_STATE_CHARGE && playerPadGetButton(p, PAD_L2) > 0 && p->timers.state > 55)
			p->timers.state = 55;
	}
}

void InfiniteHealthMoonjump(void)
{
	int _InfiniteHealthMoonjump_Init = 0;
	// Handle On/Off Button Press
	void * PlayerPointer = (void*)(*(u32*)0x001eeb70);
	Player * player = (Player*)((u32)PlayerPointer - 0x2FEC);
	PadButtonStatus * pad = playerGetPad(player);
	if ((pad->btns & (PAD_R3 | PAD_R2)) == 0)
		_InfiniteHealthMoonjump_Init = 1;
	else if ((pad->btns & (PAD_L3)) == 0)
		_InfiniteHealthMoonjump_Init = 0;

	// Handle On/Off
	if(!_InfiniteHealthMoonjump_Init)
		return;

	// Player Health is always max.
	player->Health = player->MaxHealth;
	// if X is pressed, lower gravity.
	if ((pad->btns & PAD_CROSS) == 0)
		*(float*)(PlayerPointer - 0x2EB4) = 0.125;
}

int main(void)
{
	dlPreUpdate();

	// GameSettings * gameSettings = gameGetSettings();
	// GameOptions * gameOptions = gameGetOptions();

    if (isInGame()) {
		Player * p = playerGetFromSlot(0);
		if (!p)
			return 0;

		printf("\nanimId: %d", p->PlayerMoby->AnimSeqId);

		InfiniteChargeboot();
		InfiniteHealthMoonjump();
    	// DebugInGame(p);
    } else {
		DebugInMenus();
	}

	dlPostUpdate();

	return 0;
}
