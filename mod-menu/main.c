#include <tamtypes.h>
#include <libdl/stdio.h>
#include <libdl/graphics.h>
#include <libdl/string.h>
#include <libdl/game.h>
#include <libdl/player.h>
#include <libdl/pad.h>
#include <libdl/math.h>
#include <libdl/math3d.h>
#include <libdl/hud.h>
#include <libdl/ui.h>
#include <libdl/music.h>
#include <libdl/dl.h>
#include "include/menu.h"

// config
PatchConfig_t config __attribute__((section(".config"))) = {
	0, // Infinite Health/Moonjump
    0, // Mask Username
    0 // Hacked Keyboard
};

short Keys[][2] = {
	// Offset, Data
	{0x0918, 0x0008},
	{0x0E58, 0x5009},
	{0x13F8, 0x470A},
	{0x0D38, 0x500B},
	{0x0D98, 0x570C},
	{0x0DF8, 0x420D},
	{0x0EB8, 0x0010},
	{0x0F18, 0x0011},
	{0x0F78, 0x0012},
	{0x0FD8, 0x0013},
	{0x1038, 0x0014},
	{0x1458, 0x0015},
	{0x1098, 0x0016},
	{0x14B8, 0x0017},
	{0x10F8, 0x0018},
	{0x1158, 0x0019},
	{0x1218, 0x001A},
	{0x1278, 0x001B},
	{0x1338, 0x001C},
	{0x12D8, 0x001D},
	{0x11B8, 0x001E},
	{0x1398, 0x001F},
	{0x0CD8, 0x00E9}
};

char _InfiniteHealthMoonjump_Init = 0;
char _MaskUsername_Init = 0;
char _HackedKeyboard_Init = 0;

/*========================================================*\
========                       Offset + 0x00
================      Infinite Health/Moonjump Logic
========
\*========================================================*/
void InfiniteHealthMoonjump()
{

	if (!gameIsIn() || !&config.enableInfiniteHealthMoonjump)
	{
		_InfiniteHealthMoonjump_Init = 0;
		return;
	}

	// Handle On/Off Button Press
	void * PlayerPointer = (void*)(*(u32*)0x001eeb70);
	Player * player = (Player*)((u32)PlayerPointer - 0x2FEC);
	PadButtonStatus * pad = playerGetPad(player);
	if ((pad->btns & (PAD_R3 | PAD_R2)) == 0){
		_InfiniteHealthMoonjump_Init = 1;
	}
	else if ((pad->btns & (PAD_L3)) == 0)
	{
		_InfiniteHealthMoonjump_Init = 0;
	}

	// Handle On/Off
	if(!_InfiniteHealthMoonjump_Init)
		return;

	// Player Health is always max.
	player->Health = PLAYER_MAX_HEALTH;
	// if X is pressed, lower gravity.
	if ((pad->btns & PAD_CROSS) == 0){
		*(float*)(PlayerPointer - 0x2EB4) = 0.125;
	}
}

/*========================================================*\
========
================      Mask Username Logic
========
\*========================================================*/
void MaskUsername()
{
	if (!&config.enableMaskUsername)
		return;

	if (gameIsIn())
	{
		_MaskUsername_Init = 0;
		return;
	}
	PadButtonStatus * pad = (PadButtonStatus*)0x001ee600;
	if((pad->btns & (PAD_L2 | PAD_R2 | PAD_SELECT)) == 0)
	{
		_MaskUsername_Init = 1;
	}
	else
	{
		_MaskUsername_Init = 0;
	}
	if (!_MaskUsername_Init)
		return;

	int Mask = (*(u32*)((u32)0x011C7064 + (UIP_KEYBOARD * 0x4))) + 0x284;
	if (*(u8*)0x0017225e != 0 && *(u8*)Mask != 0){
		memcpy((u8*)0x0017225e, (u8*)Mask, 0xe);
	}
}

/*========================================================*\
========
================      Hacked Keyboard Logic
========
\*========================================================*/
void HackedKeyboard()
{
	if (!config.enableHackedKeyboard)
		return;

	if(gameIsIn())
	{
		_HackedKeyboard_Init = 0;
		return;
	}
	PadButtonStatus * pad = (PadButtonStatus*)0x001ee600;
	if ((pad->btns & (PAD_SELECT | PAD_L2)) == 0)
	{
		_HackedKeyboard_Init = 1;
	}
	else
	{
		_HackedKeyboard_Init = 0;
	}

	if(!_HackedKeyboard_Init)
		return;

	void * Pointer = (void*)(*(u32*)0x011C70B4);
	int KeyboardCheck = ((u32)Pointer + 0x230);
	//if Keyboard is open
	if (*(u32*)KeyboardCheck != -1)
	{
		//printf("Pointer: %p\n", Pointer);
		//printf("Keyboard Check: 0x%x\n", KeyboardCheck);
		int board = (sizeof(Keys)/sizeof(Keys[0]));
		int a;
		for(a = 0; a < board; a++)
		{
			*(u16*)((u32)Pointer + Keys[a][0]) = Keys[a][1];
			//printf("Key: 0x%x: 0x%x\n", ((u32)Pointer + Keys[a][0]), Keys[a][1]);
		}
	}
}

void onOnlineMenu(void)
{
	onConfigOnlineMenu();
}

int main(void)
{
	// Call this first
	dlPreUpdate();

    // R3 + R2/L3
    InfiniteHealthMoonjump();
	// L2 + R2 + Select
	MaskUsername();
	// Select + L2
	HackedKeyboard();


    if (gameIsIn())
    {
        onConfigGameMenu();
    }
    else
    {
        // hook mod menu
        if (*(u32*)0x00594CBC == 0)
		    *(u32*)0x0061E1B4 = 0x08000000 | ((u32)(&onOnlineMenu) / 4);
    }

	// Call this last
	dlPostUpdate();

	return 0;
}
