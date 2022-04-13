#include <tamtypes.h>
#include <libdl/stdio.h>
#include <libdl/pad.h>

int Active = 0;
int SpawnedPack = 0;
int PlayerStruct = 0;
int PlayerUpdateFunc = 0;
void SpawnPack(void)
{
    // Spawn Pack if Health <= zero and if not spawned already.
    if (*(float*)0x00235964 <= 0 && SpawnedPack == 0)
    {
        SpawnedPack = 1;
        int Respawn = (*(u32*)PlayerUpdateFunc) - 0x61B8; // 0x0050FAA8 (Outpost X12)
        int Pack = (*(u32*)PlayerUpdateFunc) - 0x258E8; // 0x004F0378 (Outpost X12)
        // Run normal hook
        ((void (*)())Respawn)();

        // Spawn Pack
        ((void (*)(u32))Pack)(PlayerStruct);
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
        PlayerStruct = (*(u32*)0x00225e70) - 0x430C;
        int PlayerFunctions = (u32)PlayerStruct + 0x14;
        PlayerUpdateFunc = (*(u32*)PlayerFunctions) + 0x34;
        int SpawnPackHook = (*(u32*)PlayerUpdateFunc) + 0x952C; // 0x0051F18C (Outpost X12)
		int WeaponPack = (*(u32*)PlayerUpdateFunc) - 0x26720; // 0x004EF540 (Outpost X12)

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
            // Spawn Pack (Outpost X12)
            //((void (*)(u32))0x004F0378)(0x002F7900);
            // Heal Player
            ((void (*)(u32, u16, u16))0x0041BE98)(PlayerStruct, 1, 57);
        }
        else if ((pad->btns & PAD_R3) == 0 && Active == 0)
	    {
            Active = 1;
            // End Game - Time up
            ((void (*)(u32, u32, u32, u32))0x00539708)(3, 0x2720, 0x0034f420, PlayerStruct);
        }
        if (!(pad->btns & PAD_L3) == 0 && !(pad->btns & PAD_R3) == 0)
        {
            Active = 0;
        }
	}
	return 0;
}
