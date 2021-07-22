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

/*
Cheats Struction for Gansta GunsL
Starts: 0x00393650
	int ID = 0x0;
	int Unlocked = 0x4; // 0 = Unlocked, 1 = Locked
	int HowManyOptions = 0x8;
	int Name = 0xC;
	int Description = 0x10;
	int FunctionPointer = 0x18;
	int Option_1 = 0x1C;
	Int Option_2 = 0x20;
	Int Option_3 = 0x24;

String ID pointer: 0x002226A4
	int StringPointer: 0x00
	short StringID; // 0x04
	short unk = -1; // 0x06
*/

char Strings[3][10] = {
	"RAWR",
	"TEST",
	"ANOTHER TEST?!?!?!"
};

void NewStrings()
{
	int NewStringPointer = 0x0009F000;
	char NewStrings = NewStringPointer + 0x500;
	*(char*)NewStrings = Strings;
	printf("Pointer: %p", &Strings);
	// *(char*)NewStrings = Strings;
	// *(u32*)(NewStringPointer) = &NewStrings;
	// *(u16*)(NewStringPointer + 0x04) = 0x00;
	// *(u16*)(NewStringPointer + 0x06) = -1;
	// int StringsPointer = 0x002226A4;
	// if (*(u32*)StringsPointer != NewStringPointer)
	// {
	// 	*(u32*)StringPointer = NewStringPointer;
	// }
}

// void CheatOption(int ID, int HowManyOptions, const char * Name, const char * Description, const char * Option_1, const char * Option_2)
// {
// 	int NewStringPointer = 0x0009F000;
// 	int NewStrings = NewStringPointer + 0x500;
// 	*(u32*)NewStrings = Name;
// 	int StringsPointer = 0x002226A4;
// 	if (*(u32*)StringsPointer !=NewStringPointer)
// 	{
// 		*(u32*)StringPointer = NewStringPointer;
// 	}
// }

void OmegaAlphaMods()
{
	void * Pointer = (void*)(*(u32*)0x0034a184);
	if(*(u32*)(Pointer + 0x20) != 0x63636300)
	{
		*(u32*)(Pointer + 0x20) = 0x63636300;
		*(u32*)(Pointer + 0x24) = 0x63636363;
		*(u32*)(Pointer + 0x28) = 0x00ff0063;
	}
}

void SkillPoints()
{
	int num;
	for(num = 0; num < 0xb; num++)
	{
		*(u32*)(0x00171BA8 + (num * 4)) = 0x00107FFF;
	}
}

void HackedStartMenu()
{
	if (gameIsIn())
	{
		*(u16*)0x00560338 = 0x1190;
		*(u32*)0x003104c4 = 0x4954504f;
		*(u32*)0x003104c8 = 0x00534e4f;
		*(u16*)0x00560350 = 0x1678;
		*(u32*)0x00310504 = 0x41454843;
		*(u32*)0x00310508 = 0x00005354;
		*(u16*)0x00560368 = 0x0fe0;
		*(u32*)0x00310544 = 0x50414557;
		*(u32*)0x00310548 = 0x00534e4f;
		// Load this if Hacked Start Menu is On
		OmegaAlphaMods();
		SkillPoints();
		//NewStrings();
	}
	// If not in game, set Remove Helmet cheat back off.
	else if (*(u8*)0x0021de40 != 0 || *(u8*)0x0021de50 != 0)
	{
		*(u8*)0x0021de40 = 0;
		*(u8*)0x0021de50 = 0;
	}
}

int main(void)
{
	if (*(u32*)0x001CF85C != 0x000F8D29)
		return -1;

	HackedStartMenu();

	return 1;
}
