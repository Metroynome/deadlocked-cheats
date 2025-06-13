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

int patchCrouchJumpTransitions(Player *this)
{
	// call original CrouchJumpTransitions function
	return ((int (*)(Player* ))0x005efd20)(this);
}

void lagjump(void)
{
	int hook = 0x006095b8;
	if (*(u32*)hook == 0x0c17bf48) {
		// hook our custom crouchjumptransitions logic
		HOOK_JAL(hook, &patchCrouchJumpTransitions);
		// disable "LookmodeTransition" in "InitBodyState"
		POKE_U32(0x00606aa0, 0);
	}

	// force player to crouch if R2 is pressed
	Player *player = playerGetFromSlot(0);
	PlayerVTable *vtable = playerGetVTable(player);
	if (playerPadGetButtonDown(player, PAD_R2 | PAD_CROSS) > 0) {
		float stickStrength = *(float*)((u32)player + 0x2e08);
		int jumpDir = ((int (*)(Player *))0x005ee3b8)(player);
		short int onGood = *(short*)((u32)player + 0x2f0);
		if (0.9 < stickStrength && onGood && jumpDir != 2) {
			vtable->UpdateState(player, 0xb, 1, 0, 1);
		}
	}
}

int main(void)
{
	dlPreUpdate();

	if (isInGame()) {
		lagjump();
	}

	dlPostUpdate();

	return 0;
}
