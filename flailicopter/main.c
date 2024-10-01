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

// In Seconds.  If set to -1, this is ignored and will
// only lift off if FLAIL_SCROLL_SPEED_MAX is met.
#define LIFT_OFF_WAIT_SECONDS (-1)
#define LIFT_OFF_VELOCITY (.75f)

#define FLAIL_HAND_ANIMATION_SPEED_MAX (3.0f)
#define FLAIL_HAND_ANIMATION_SPEED_ADDITIVE (0.2f)

#define FLAIL_SCROLL_SPEED_ADDR (*(float*)0x00221838)
#define FLAIL_SCROLL_SPEED_MAX (1.09f)
#define FLAIL_SCROLL_SPEED_ADDITIVE (0.00035)

void flailicopter(void)
{
	static int flailicopterActivated = 0;
	int i;
	Player ** players = playerGetAll();
	for (i = 0; i < GAME_MAX_PLAYERS; ++i) {
		Player * player = players[i];
		if (!player)
			continue;

		u32 animation_speed = ((u32)player + 0x640);

		// If state isn't flail attack and animation isn't spinning flail
		if (player->PlayerState != PLAYER_STATE_FLAIL_ATTACK && player->PlayerMoby->AnimSeqId != 0x32) {
			if (*(float*)animation_speed > 1)
				*(float*)animation_speed = 1;

			if (FLAIL_SCROLL_SPEED_ADDR > 1)
				FLAIL_SCROLL_SPEED_ADDR = 1;

			return;
		}

		// Make hand animation speed faster. (player->anim.speed)
		if (*(float*)animation_speed < FLAIL_HAND_ANIMATION_SPEED_MAX) {
			*(float*)animation_speed += FLAIL_HAND_ANIMATION_SPEED_ADDITIVE;
		}

		// Make flail scroll speed faster.
		if (FLAIL_SCROLL_SPEED_ADDR < FLAIL_SCROLL_SPEED_MAX) {
			FLAIL_SCROLL_SPEED_ADDR += FLAIL_SCROLL_SPEED_ADDITIVE;
			return;
		}

		// If animation timer is less than needed lift of time.
		if (LIFT_OFF_WAIT_SECONDS > 0 && player->timers.animState < LIFT_OFF_WAIT_SECONDS * 60)
			return;
	
		// Modify player Y coordinate.
		VECTOR t = {0, 0, LIFT_OFF_VELOCITY, 0};
		vector_add(player->PlayerPosition, player->PlayerPosition, t);
	}
}

int main(void)
{
	dlPreUpdate();

	if (!isInGame())
		return -1;
	
	flailicopter();

	dlPostUpdate();

	return 0;
}
