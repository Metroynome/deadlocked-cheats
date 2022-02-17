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
int SpawnedPack = 0;
int PlayerUpdateFunc = 0;
void SpawnPack(void)
{
    // Spawn Pack if Health <= zero and if not spawned already.
    if (*(float*)0x00235964 <= 0 && SpawnedPack == 0)
    {
        SpawnedPack = 1;
        void * GameplayFilePointer = (void*)(*(u32*)0x01FFFD00);
        int WP =  GetLevel(GameplayFilePointer);
        int Respawn = (*(u32*)PlayerUpdateFunc) - 0x61B8; // 0x0050FAA8
        int Pack = WP + 0xE38; // 0x004F0378
        // Run Normal Respawn functions
        ((void (*)())Respawn)();

        // Spawn Pack
        ((void (*)(u32))Pack)(0x002F7900);
    }
}

int main(void)
{
	// Grab GameplayFile Pointer.
	// This is where all the functions for the game are kept.
	void * GameplayFilePointer = (void*)(*(u32*)0x01FFFD00);
    PadButtonStatus * pad = (PadButtonStatus*)0x00225980;
	// If pointer doesn't equel Online Lobby Pointer, proceed.
	if (GameplayFilePointer != 0x00574F88)
	{
        int PlayerStruct = (*(u32*)0x00225e70) - 0x430C;
        int PlayerFunctions = (u32)PlayerStruct + 0x14;
        PlayerUpdateFunc = (*(u32*)PlayerFunctions) + 0x34;
        int SpawnPackHook = (*(u32*)PlayerUpdateFunc) + 0x952C; // This is 0x0051F18C
		// Check which Level we are on.  Function returns the address of the Weapon Pack function.
		int WeaponPack = GetLevel(GameplayFilePointer);

		// If WeaponPack function isn't Zero, make it zero.
		if (*(u32*)WeaponPack != 0)
			*(u32*)WeaponPack = 0;

        // if Health is greater than zero and pack has spawned
        if (*(float*)0x00235964 > 0 && SpawnedPack == 1)
            SpawnedPack = 0;

        // Hook SpawnPack (On Outpost x12 address: 0x0051F18C)
		*(u32*)SpawnPackHook = 0x0c000000 | ((u32)(&SpawnPack) / 4);

        // DEBUG OPTIONS: L3 = Spawn Pack, R3 = Hurt Player
        if ((pad->btns & PAD_L3) == 0 && Active == 0)
	    {
            Active = 1;
            // Outpost X12 only currently
            ((void (*)(u32))0x004F0378)(0x002F7900);
        }
        else if ((pad->btns & PAD_R3) == 0 && Active == 0)
	    {
            Active = 1;
            // Outpost X12 only currently
            // ((void (*)(u32))0x004F0378)(0x002F7900);
            // Hurt Player
            ((void (*)(u32, u16, u16))0x00502658)(0x002F7900, 5, 0);
        }
        if (!(pad->btns & PAD_L3) == 0 && !(pad->btns & PAD_R3) == 0)
        {
            Active = 0;
        }
	}
	return 0;
}
