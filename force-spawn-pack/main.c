#include <tamtypes.h>
#include <libdl/stdio.h>
#include <libdl/pad.h>

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

int Active = 0;
int main(void)
{
	// Grab GameplayFile Pointer.
	// This is where all the functions for the game are kept.
	void * GameplayFilePointer = (void*)(*(u32*)0x01FFFD00);
    PadButtonStatus * pad = (PadButtonStatus*)0x00225980;
	// If pointer doesn't equel Online Lobby Pointer, proceed.
	if (GameplayFilePointer != 0x00574F88)
	{
        if (*(u8*)0x00248B90 == 0x1)
        {
            ((void (*)(u32))0x004F0378)(0x002F7900);
        }
        if ((pad->btns & PAD_L3) == 0 && Active == 0)
	    {
            Active = 1;
            // Outpost X12 only currently
            ((void (*)(u32))0x004F0378)(0x002F7900);
        }
        if (!(pad->btns & PAD_L3) == 0)
        {
            Active = 0;
        }
	}
	return 1;
}
