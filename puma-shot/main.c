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


void correctShooting(Moby *moby, Player *player, u32 stack)
{
	u32 moby1_right = *(u32*)((u32)player + 0x22f0);
	u32 moby2_left = *(u32*)((u32)player + 0x22f4);
	if (moby1_right == 0 || moby2_left == 0)
		return;

	if (padGetButtonDown(0, PAD_R1) > 0)
		((void (*)(Moby *, Player *, u32))DUAL_VIPER_SHOOT_FUNC)(moby1_right, player, stack);

	if (padGetButtonDown(0, PAD_L1) > 0)
		((void (*)(Moby *, Player *, u32))DUAL_VIPER_SHOOT_FUNC)(moby2_left, player, stack);
}

int main(void)
{
	dlPreUpdate();

	if (!isInGame())
		return -1;
	
	Player *player = playerGetFromSlot(0);
	
	static int init = 0;
	if (!init) {
		HOOK_JAL(DUAL_VIPER_SHOOT_HOOK, &correctShooting);
		init = 1;
	}

	dlPostUpdate();

	return 0;
}
