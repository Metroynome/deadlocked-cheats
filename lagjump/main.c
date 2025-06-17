#include <tamtypes.h>
#include <libdl/dl.h>
#include <libdl/player.h>
#include <libdl/pad.h>
#include <libdl/stdio.h>
#include <libdl/game.h>
#include <libdl/moby.h>
#include <libdl/utils.h>
#include <libdl/math.h>
#include <libdl/math3d.h>
#include <libdl/vehicle.h>
#include <libdl/time.h>

void lagjump(void)
{
	if (!isInGame()) return;

	static volatile int state[2] = {0,0};

	//Player* lp = playerGetFromSlot(0);
	//char buf[64];
	//snprintf(buf, sizeof(buf), "%d", lp->PlayerState);
	//gfxHelperDrawText(15, SCREEN_HEIGHT - 15, 0, 0, 1, 0x80FFFFFF, buf, -1, TEXT_ALIGN_BOTTOMLEFT, COMMON_DZO_DRAW_NORMAL);

	int i;
	for (i = 0; i < 2; ++i) {
		Player *player = playerGetFromSlot(i);
		if (player && player->PlayerMoby) {
			int hasState = player->PlayerState == PLAYER_STATE_SKID
			|| player->PlayerState == PLAYER_STATE_WALK
			|| player->PlayerState == PLAYER_STATE_IDLE
			|| player->PlayerState == PLAYER_STATE_FALL
			//|| player->PlayerState == PLAYER_STATE_FLIP_JUMP
			;
			PlayerVTable *vtable = playerGetVTable(player);
			float stickStrength = *(float*)((u32)player + 0x2e08);
			int jumpDir = ((int (*)(Player *))0x005ee3b8)(player);
			if (playerPadGetButton(player, PAD_R2 | PAD_CROSS) > 0) {
				if (hasState && state[i] < 4) {
					if (0.9 < stickStrength && player->Ground.onGood && jumpDir != 2) {
						vtable->UpdateState(player, 0xb, 1, 0, 1);
					}
				} else {
					state[i]++;
				}
			//printf("%08X state:%d jumpDir:%d stick:%.1f ongood:%d pad:%d stategood:%d\n", gameGetTime(), player->PlayerState, jumpDir, stickStrength, player->Ground.onGood, state[i], hasState);
			} else {
				state[i] = 0;
			}
		}
	}
}

int main(void)
{
	dlPreUpdate();

	lagjump();

	dlPostUpdate();

	return 0;
}
