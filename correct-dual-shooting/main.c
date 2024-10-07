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

#define DUAL_VIPER_HOOK (0x003B7A08)
// function i'm overwriting
#define DUAL_VIPER_SHOOT_FUNC (0x005F03D0)

int correctShooting(u32 a0, u32 a1, u32 a2)
{
	int ret = ((int (*)(u32, u32, u32))DUAL_VIPER_SHOOT_FUNC)(a0, a1, a2);
	Player * player = playerGetFromSlot(0);
	// register unsigned int CurrentViper asm("s2");
	static u32 right_viper = 0;
	static u32 left_viper = 0;
	u32 moby1 = *(u32*)((u32)player + 0x22f0);
	u32 moby2 = *(u32*)((u32)player + 0x22f4);
	if (moby1 == 0 || moby2 == 0)
		return 0;

	if (right_viper != moby1)
		right_viper = moby1;
	if (left_viper != moby2)
		left_viper = moby2;

	int jointCnt = 0;
	if ( padGetButtonDown(0, PAD_R1) > 0)
		jointCnt = 8;
	// Shoot Right Hand
	if (padGetButtonDown(0, PAD_L1) > 0)
		jointCnt = 8;

	// printf("\nRight: %08x", right_viper);
	// printf("\nLeft : %08x", left_viper);
	static int old_ret = -1;
	if (old_ret != ret) {
		printf("\nret: %d, jointCnt: %d", ret, jointCnt);
		old_ret = ret;
	}
	if (ret == jointCnt)
		return ret;
	
	return 0;
}

int main(void)
{
	dlPreUpdate();

	if (!isInGame())
		return -1;
	
	static int init = 0;
	if (!init) {
		HOOK_JAL(DUAL_VIPER_HOOK, &correctShooting);
		init = 1;
	}

	dlPostUpdate();

	return 0;
}
