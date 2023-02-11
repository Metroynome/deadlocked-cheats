#include <tamtypes.h>
#include <libdl/stdio.h>
#include <libdl/game.h>
#include <libdl/player.h>

int DualWieldWeapons(void)
{
	// v1 = current weapon
	// v0 = vipers
	register unsigned short WeaponID asm("v1");
	register unsigned short DualWieldIDs asm("v0");
	// If WeaponID = Vipers/Mag/Fusion, continue.
	if (WeaponID == 0x1094 || WeaponID == 0x1087 || WeaponID == 0x1096)
	{
		return DualWieldIDs = WeaponID;
	}
	return DualWieldIDs = -1;
}

int main(void)
{
	if (!isInGame())
		return -1;
	
	// Fix Dual Fusion not shooting dual viper shots out of left hand.
	// int p = 0x00347aa0;
	// int GadgetMoby = *(u32*)(p + 0x22F4);
	// if (GadgetMoby != 0)
	// {
	// 	int AnotherPointer = *(u32*)(GadgetMoby + 0xAC);
	// 	*(u32*)(AnotherPointer + 0x48) = p;
	// }

	// Dual Wield Vipers or Magma Cannon
	// Hook our function
	*(u32*)0x005DDA3C = 0x0c000000 | ((u32)(&DualWieldWeapons) >> 2);
	*(u32*)0x005DDA40 = 0x87A30020; // loads weapon ID into v1.
	// Set Left Hand weapon to needed dual wield weapon.
	*(u32*)0x005DDA54 = 0x0040202D;
	// Constant Write which Weapon PVars to get if Magma Cannon is selected
	if (*(u32*)0x00349f50 == 3)
	{
		// Set to dual viper pvars
		*(u32*)0x00349DB8 = 2;
		// Writes Player Struct to Parent Moby
		*(u32*)0x005CDD38 = 0x24081087;
	}
	// Reset back to Dual Vipers
	else
	{
		*(u32*)0x005CDD38 = 0x24081094;
	}
	// Fusion
	// else if (*(u32*)0x00349f50 == 5)
	// {
	// 	*(u32*)0x00349DB8 = 2;
	// }
	
	return 0;
}
