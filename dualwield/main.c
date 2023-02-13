#include <tamtypes.h>
#include <libdl/stdio.h>
#include <libdl/game.h>
#include <libdl/player.h>


//	/* 0x22d0 */ Gadget gadgets[6];
//	/* 0x24b0 */ int assGadgets[6];

// struct Gadget { // 0x50
//	Gadget starts at: 0x00349D70
	// /* 0x00 */ vec4 jointPos;
	// /* 0x10 */ vec4f jointRot;
	// /* 0x20 */ moby *pMoby;
	// /* 0x24 */ moby *pMoby2;
	// /* 0x28 */ bool padButtonDown;
	// /* 0x2c */ int alignPad;
	// /* 0x30 */ int padButton;
	// /* 0x34 */ int gsSpawnFrame;
	// /* 0x38 */ char noAmmoTime;
	// /* 0x39 */ char unEquipTimer;
	// /* 0x3a */ char detached;
	// /* 0x3b */ char unequipTime;
	// /* 0x3c */ char unEquipStatus;
	// /* 0x3d */ char unEquipDelay;
	// /* 0x40 */ int equippedTime;
	// /* 0x44 */ int state;
	// /* 0x48 */ int id;
	// /* 0x4c */ float lightAng;
// };

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

	// memcpy Dual Vipers pvar data to Harbingers
	if (*(u16*)0x0039a020 == 0x2613)
		memcpy((u32*)0x0039a000, (u32*)0x00399a80, 0xb0);

	*(u16*)0x0039a020 = 0x1087; // Weapon ID
	// *(u32*)0x0039a0a0 = 0x003b6548; // Target Function
	
	// Enable Harbinger
	*(u16*)0x001D4C68 = 0;

	// Dual Wield Vipers or Magma Cannon
	// Hook our function
	*(u32*)0x005DDA3C = 0x0c000000 | ((u32)(&DualWieldWeapons) >> 2);
	*(u32*)0x005DDA40 = 0x87A30020; // loads weapon ID into v1.
	// Set Left Hand weapon to needed dual wield weapon.
	*(u32*)0x005DDA54 = 0x0040202D;
	// Constant Write which Weapon PVars to get if Harbinger is selected
	if (*(u32*)0x00349f50 == 0xa)
	{
		// Set to dual viper pvars
		//	even replacing harbinger with dual vipers
		//	and loading from there doesn't let
		//	left hand moby update.
		// *(u32*)0x00349DB8 = 2;
		// Writes Player Struct to certain Parent Moby
		*(u32*)0x005CDD38 = 0x24081087;
		// Sets branch to true, so any weapon that is
		// dual wield able will spawn second weapon.
		// In this case I can choose via "DualWieldWeapons" function
		//*(u32*)0x005DE33C = 0x1042FF06;
		// Dual Wield Weapon Check 2: Sets s6 to needed weapon to make branch (0x005DE33C) true.
		*(u8*)0x005DDF04 = 0xa;
		// Disabling this does correct jump animation for dual wield weapons
		*(u32*)0x005DDC64 = 0;
		// Reset Idle Arbitor Animation
		*(u8*)0x005D0F90 = 0x40;
		// Change Walking Forward Arbitor Animation
		*(u8*)0x005EF7A0 = 0x56;
		// Change Walking Backward Arbitor Animation
		*(u8*)0x005EF2BC = 0x5D;
	}
	// Reset back to Dual Vipers
	else
	{
		// Reset writing Player Struct to certain Parent Moby
		*(u32*)0x005CDD38 = 0x24081094;
		// Dual Wield Weapon Check 2:  Reset s0 for Dual Wield Branch at 0x005DE33C
		*(u8*)0x005DDF04 = 2;
		// Reset Jumping Animation
		*(u32*)0x005DDC64 = 0x8E442318;
		// Reset Idle Arbitor Animation
		*(u8*)0x005D0F90 = 0x44;
		// Reset Walking Forward Arbitor Animation
		*(u8*)0x005EF7A0 = 0x60;
		// Reset Walking Backward Arbitor Animation
		*(u8*)0x005EF2BC = 0x61;
	}
	
	return 0;
}
