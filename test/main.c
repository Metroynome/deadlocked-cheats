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

VECTOR hits[250];
u32 hitsColor[250];
int hitsCount = 0;
u32 hitsColorCurrent = 0x80FFFFFF;

void testPlayerCollider(int pid)
{
	VECTOR pos,t,o = {0,0,0.7,0};
	int i,j = 0;
	int x,y;
	char buf[12];
	Player * p = playerGetAll()[pid];

	const int steps = 5 * 2;
	const float radius = 2;

	for (i = 0; i < steps; ++i) {
		for (j = 0; j < steps; ++j) {
			if (hitsCount >= 250) break;

			float theta = (i / (float)steps) * MATH_TAU;
			float omega = (j / (float)steps) * MATH_TAU;  

			vector_copy(pos, p->PlayerPosition);
			pos[0] += radius * sinf(theta) * cosf(omega);
			pos[1] += radius * sinf(theta) * sinf(omega);
			pos[2] += radius * cosf(theta);

			vector_add(t, p->PlayerPosition, o);

			if (CollLine_Fix(pos, t, COLLISION_FLAG_IGNORE_STATIC, NULL, 0)) {
				vector_copy(t, CollLine_Fix_GetHitPosition());
				hitsColor[hitsCount] = hitsColorCurrent;
				vector_copy(hits[hitsCount++], CollLine_Fix_GetHitPosition());
				if (gfxWorldSpaceToScreenSpace(t, &x, &y)) {
					gfxScreenSpaceText(x, y, 1, 1, 0x80FFFFFF, "+", -1, 4);
				}
			}
		}
	}
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

		// printf("\nanimId: %d", p->PlayerMoby->AnimSeqId);

		InfiniteChargeboot();
		InfiniteHealthMoonjump();
    	// DebugInGame(p);

		// testPlayerCollider(0);
    } else {
		DebugInMenus();
	}

	dlPostUpdate();

	return 0;
}
