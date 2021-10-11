#include <tamtypes.h>
#include <libdl/stdio.h>

int GetLevel(void * GameplayFilePointer)
{
    int WeaponPack_Func = 0;
    switch((u32)GameplayFilePointer)
    {
        // Outpost X12
        case 0x0043A288:
            WeaponPack_Func = 0x004EF540;
            break;
        // Korgon Outpost
        case 0x00437EC8:
            WeaponPack_Func = 0x004ECD58;
            break;
        // Metropolis
        case 0x00437208:
            WeaponPack_Func = 0x004EC0A8;
            break;
        // Blackwater City
        case 0x00434988:
            WeaponPack_Func = 0x004E98C0;
            break;
        // Command Center
        case 0x004357C8:
            WeaponPack_Func = 0x004E9A48;
            break;
        // Blackwater Docks
        case 0x00438008:
            WeaponPack_Func = 0x004EC288;
            break;
        // Aquatos Sewers
        case 0x00437348:
            WeaponPack_Func = 0x004EB5C8;
            break;
        // Marcadia Palace
        case 0x00436C88:
            WeaponPack_Func = 0x004EAF08;
            break;
        // Bakisi Isle
        case 0x00441988:
            WeaponPack_Func = 0x004F7BD0;
            break;
        // Hoven Gorge
        case 0x00443448:
            WeaponPack_Func = 0x004F9C28;
            break;
    }
	return WeaponPack_Func;
}


int main(void)
{
	// Grab GameplayFile Pointer.
	// This is where all the functions for the game are kept.
	void * GameplayFilePointer = (void*)(*(u32*)0x01FFFD00);
	// If pointer doesn't equel  Online Lobby Pointer, proceed.
	if (GameplayFilePointer != 0x00574F88)
	{
		// Check which Level we are on.  Function returns the address of the Weapon Pack function.
		int WeaponPack = GetLevel(GameplayFilePointer);
		// If WeaponPack function isn't Zero, make it zero.
		if (*(u32*)WeaponPack != 0)
			*(u32*)WeaponPack = 0;
	}
	return 1;
}
