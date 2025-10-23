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

#define DUAL_VIPER_SHOOT_HOOK (0x003B7A84)
#define DUAL_VIPER_SHOOT_FUNC (0x003B66A0)

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
	
	static int holdingDualVipers = 0;
	if (*(u32*)((u32)player + 0x22f4) != 0 && !holdingDualVipers) {
		// update p->gadgets[0].padButton;
		*(u32*)((u32)player + 0x22d0 + 0x30) = 12;
		// Make L1 not jump
		*(u16*)0x005ef0e0 = 0x70;
		*(u16*)0x00607084 = 0x70;
		*(u16*)0x00608674 = 0x70;
		holdingDualVipers = 1;
	} else if (*(u32*)((u32)player + 0x22f4) == 0 && holdingDualVipers){
		*(u16*)0x005ef0e0 = 0x40;
		*(u16*)0x00607084 = 0x40;
		*(u16*)0x00608674 = 0x40;
		holdingDualVipers = 0;
	}

	static int init = 0;
	if (!init) {
		HOOK_JAL(DUAL_VIPER_SHOOT_HOOK, &correctShooting);
		// let both vipers shoot at same time
		POKE_U32(0x003B7A8C, 0);
		POKE_U32(0x003B7A9C, 0);
		// change rate of fire to something really high.
		POKE_F32(0x00399acc, 100.00);
		init = 1;
	}

	dlPostUpdate();

	return 0;
}
