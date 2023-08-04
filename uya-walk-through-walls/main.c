#include <tamtypes.h>
#include <libdl/stdio.h>
#include <libdl/pad.h>

int _WalkThroughWallsToggle = 0;
int WalkThroughWallsJAL = 0;

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
        int PlayerUpdateFunc = (*(u32*)PlayerFunctions) + 0x34; // 0x00515C60 (Outpost X12)
		int WalkThroughWalls = (*(u32*)PlayerUpdateFunc) - 0x24178; // 0x004F1AE8 (Outpost X12)
        
        if (WalkThroughWallsJAL == 0)
            WalkThroughWallsJAL = *(u32*)WalkThroughWalls;

        // if Health is greater than zero and pack has spawned
        // if (*(float*)0x00235964 > 0 && WalkThoughWalls != 0)
        //     WalkThoughWalls = 0;

        // L3: On
        if ((pad->btns & PAD_L3) == 0 && _WalkThroughWallsToggle == 0)
	    {
            _WalkThroughWallsToggle = 1;
            *(u32*)WalkThroughWalls = 0;
        }
        // R3: Off
        else if ((pad->btns & PAD_R3) == 0 && _WalkThroughWallsToggle == 0)
	    {
            _WalkThroughWallsToggle = 0;
            *(u32*)WalkThroughWalls = WalkThroughWallsJAL;
        }
        if (!(pad->btns & PAD_L3) == 0 && !(pad->btns & PAD_R3) == 0)
        {
            _WalkThroughWallsToggle = 0;
        }
	}
	return 0;
}
