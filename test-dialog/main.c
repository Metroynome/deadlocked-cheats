#include <tamtypes.h>
#include <libdl/stdio.h>
#include <libdl/game.h>
#include <libdl/player.h>
#include <libdl/dialog.h>

/*
CQ Help Message
EQUIP_HACKER_RAY = 0x2e9c
AIM_YOUR_HACKER_RAY = 0x2e9e

CLANK:
TRY_USING_FUSION_RIFLE = 0x2f2C
TWO_PRESSURE_PADS = 0x2f2D
CHOOSE_WHERE_TO_COMPETE = 0x2f2E
ADVANCE_TO_KRONOS = 0x2f30
TRAVEL_TO_PLANET_MARAXUS = 0x2F31
TRAVEL_TO_PLANET_ORXON = 0x2f32
CAN_ACCESS_PLANET_SARATHOS = 0x2f33
ADVANCE_TO_PLANET_SHAAR = 0x2f34
TRAVEL_TO_PLANET_TORVAL = 0x2f36
UNLOCKED_VALIX_BELT_CAMPAIGN = 0x2f37
GO_FIND_AL = 0x2f38
THIS_IS_CATACROM = 0x2f39
ENTERING_KRONOS = 0x2f3a
ARRIVING_AT_MARAXUS = 0x2f3b
WELCOME_TO_ORXON = 0x2f3c
THIS_IS_PLANET_SARATHOS = 0x2f3d
PREPARE_FOR_SHAAR = 0x2f3e
THIS_IS_PLANET_TORVAL = 0x2f40
HEADED_TO_GHOST_STATION = 0x2f41
WELCOME_TO_VALIX = 0x2f42
COMPETE_IN_AVENGER_TOURNAMENT = 0x2f43
SURVIVED_QUALIFICATION_COURSE = 0x2f44
COMMAND_BOT_EMP = 0x2f45
SECRET_MESSAGES = 0x2f46
PURCHASE_THE_MOD = 0x2f47
TRY_USING_RIGHT_ANALOG = 0x2f48
BOLT_CRANKS = 0x2f49
QUALIFIED_FOR_CRUSADER_TOURNAMENT = 0x2f4a
DEADLY_STALKER_TURRET = 0x2f4b
DREAD_CHALLENGES_ARE_OPTIONAL = 0x2f4c
ADJUST_GAMES_DIFFICULTY = 0x2f4d
MUST_EARN_DREADPOINTS = 0x2f4e
EARN_MEDALS_TO_ADVANCED = 0x2f4f
USE_EMP_ON_SHIELD_TURRET = 0x2f50
ENTER_PLANATARY_TRANSPORT = 0x2f51
DONT_STRAY_TOO_FAR_FROM_PARTNER = 0x2f53

MERC:
DOUBLE_TAP_R2_TO_USE_CHARGEBOOTS = 0x2f54

*/

int PlaySound = 0;
int StartSound = 0x2C96; // Starting Dialog Sound.

int main(void)
{
	// If not in game, don't run.
	if (!gameIsIn())
	{
		PlaySound = 0;
		return -1;
	}
	// Grab player pad
	Player * player = (Player*)0x00347aa0;
	PadButtonStatus * pad = playerGetPad(player);

	// L3: Previous Sound
	if ((pad->btns & PAD_L3) == 0 && PlaySound == 0)
	{
		// Setting PlaySound to 1 will make it so the current playing sound will play once.
		PlaySound = 1;
		StartSound -= 0x1; // Subtract 1 from StartSound
		dialogPlaySound(StartSound, 0); // Play Sound
		printf("Sound Byte: 0x%x\n", StartSound); // print ID of sound played.
	}
	// R3: Next Sound
	if ((pad->btns & PAD_R3) == 0 && PlaySound == 0)
	{
		PlaySound = 1;
		StartSound += 0x1;
		dialogPlaySound(StartSound, 0);
		printf("Sound Byte: 0x%x\n", StartSound);
	}
	// Circle: Replay Sound
	if ((pad->btns & PAD_CIRCLE) == 0 && PlaySound == 0)
	{
		PlaySound = 1;
		dialogPlaySound(StartSound, 0);
		printf("Sound Byte: 0x%x\n", StartSound);
	}
	// If neither of the above are pressed, PlaySound = 0.
	if (!(pad->btns & PAD_L3) == 0 && !(pad->btns & PAD_R3) == 0 && !(pad->btns & PAD_CIRCLE) == 0)
	{
		PlaySound = 0;
	}
	return 1;
}
