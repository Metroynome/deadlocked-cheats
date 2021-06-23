/*
Patch.bin subroutine.
 - Starts at 01E00000
 - On/Off bytes at 000fffe0
    - 000fffe0: Infinite Health/Moonjump
	- 000fffe1: Mask Username
	- 000fffe2: Hacked Keyboard
	- 000fffe3: Free Cam
	- 000fffe4: sp-music-to-mp
	- 000fffe5: Follow Aimer v4
	- 000fffe6: Force G^
	- 000fffe7: Host Options
	- 000fffe8: Vehicle Select
*/

#include <tamtypes.h>
#include <libdl/stdio.h>
#include <libdl/game.h>
#include <libdl/player.h>
#include <libdl/pad.h>
#include <libdl/math.h>
#include <libdl/math3d.h>
#include <libdl/hud.h>
#include <libdl/ui.h>

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

int Tracks[][2] = {
	/*
		Commented tracks are duplicates, or already loaded in multiplayer by default.
	*/
	// DreadZone Station
	{0xa59e, 0xab5a}, // DreadZone Station
	// {0xb1be, 0xb77a}, // DreadZone Station
	{0xbe86, 0xc3c4}, // DreadZone Training Course
	// {0xc99a, 0xcec1}, // Marauder Tournament - Advanced Qualifier, Avenger - The Tower of Power, Perfect Chrome Finish, Higher Ground, Liberator - Swarmer Surprise, Dynamite Baseball
	{0xd470, 0xd937}, // Grist for the Mill, the Corkscrew, Liberator - Accelerator
	{0xde88, 0xe3a5}, // The Big Sleep, Avenger - Climb the Tower of Power, Close and Personal, Crusader - Reactor Battle, Vindicator - Eviscerator Battle
	{0xe95c, 0xee25}, // Avenger - Manic Speed Demon, Vindicator - Murphy's Law
	{0xf3fa, 0xf92d}, // Zombie Attack, Less is More
	{0xfee8, 0x103af}, // Crusader - Static Death Trap, Liberator - Ace Hardlight Battle
	{0x10910, 0x10e26}, // Crusader - Marathon
	{0x113d4, 0x118df}, // Heavy Metal
	{0x11efa, 0x123c1}, // Endzone
	{0x1299c, 0x12e74}, // Vindicator - Air Drop
	// Catacrom IV
	{0x13411, 0x1393b}, // Valley of Heroes
	// {0x13ee9, 0x1439f}, // On the Prowl, Hoverbike Madness
	{0x14985, 0x14df8}, // All Aboard the Landstalker, Destruction Derby
	{0x15377, 0x158aa}, // Rise Up, Moments of Death
	// Sarathos
	{0x15eae, 0x16381}, // Alien Soil
	// {0x168ec, 0x16da2}, // Sarathos Spirit, Check Me Out
	{0x172e8, 0x177ee}, // Where the Laviathans Roam
	{0x17e04, 0x182cb}, // King of the Laviathans, Swamp Fever
	// Kronos
	{0x18881, 0x18d37}, // Infiltrate the Cathedral, Infiltrate the Cathedral Part 2, Inverted Action
	{0x19287, 0x19750}, // Introducing Shellshock, Shellshock Returns!
	{0x19ca5, 0x1a18f}, // Enemies are Such a Grind, Fight at the Cathedral, Night Flight
	{0x1a703, 0x1ac20}, // Showdown with Shellshock, Dark City Arenas
	// Shaar
	// {0x1b22c, 0x1b6e2}, // Ghost Station - Running the Gauntlet (????)
	{0x1bcc6, 0x1c213}, // Return to the Deathbowl, Whack-A-Swarmer, Robots in a Barrel
	{0x1c81e, 0x1ccec}, // Landstalker Stalkin', Shoot to Kill
	{0x1d244, 0x1d7e1}, // As the Wrench Turns
	{0x1de16, 0x1e2cc}, // Assault on the Frozen Tundra
	// Orxon
	{0x21f24, 0x2246b}, // Labyrinth of Death
	// {0x22a4a, 0x22f55}, // Node Overload, Swarming the Gauntlet
	{0x234fc, 0x239a8}, // Refinery Ambush
	{0x23f02, 0x243b8}, // Chains of Villainy, Scoring with the Blarg
	// The Valix Belt
	{0x1e907, 0x1ee45}, // DreadZone Training Course
	{0x1f4a3, 0x1f9b3}, // Lost and Found, Time Extension!
	{0x1ffe9, 0x204d6}, // Space Ace
	{0x20a4b, 0x20f12}, // Power Up!
	{0x2145f, 0x21946}, // Let the Light Shine Through, Your Land is My Land
	// Planet Torval
	// {0x24961, 0x24e2f}, // Shaar - Landstalker Stalkin'
	{0x253b1, 0x25894}, // Hoverbike or Bust
	{0x25e0b, 0x262c1}, // Seek and Destroy, Turret Trouble
	{0x2680f, 0x26cc5}, // The Turn On
	{0x27215, 0x2772b}, // Blast of Fresh Air
	// Stygia
	// {0x27d0e, 0x28235}, // Marauder Tournament - Advanced Qualifier
	{0x287ec, 0x28d3a}, // Junking the Jammers
	// {0x29314, 0x297fe}, // Energy Collector
	{0x29d90, 0x2a27a}, // Shields Up!
	{0x2a7e6, 0x2ac6c}, // Shield Survival
	// Maraxus
	// {0x2b1b1, 0x2b74e}, // Shaar - As the Wrench Turns
	{0x2bde1, 0x2c282}, // Jail Break
	// {0x2c7b9, 0x2cc5c}, // Total Control, Speed Trap
	{0x2d175, 0x2d604}, // Leave No Man Behind
	{0x2db1b, 0x2dfe2}, // Shutdown the Delta Block, Spider on a Wire
	// Ghost Station
	{0x2e598, 0x2ea4e}, // Running the Gauntlet
	// {0x2efb8, 0x2f46e}, // The Grinding Chase
	{0x2f9c8, 0x2fe69}, // Prison Break
	{0x303ba, 0x30886}, // Escape
	// DreadZone Station Interior
	{0x30e0f, 0x312d6}, // Credits
	// {0x3182b, 0x31d04}, // Destroy the East Generator, Destroy the Central Core
	// {0x3227f, 0x32735}, // Destroy the West Generator
	{0x32cfb, 0x332d2} // Defeat Gleemon Vox
};

char _InfiniteHealthMoonjump_Init = 0;
char _MaskUsername_Init = 0;
char _HackedKeyboard_Init = 0;
char _CampaignMusic_Init = 0;
char _FollowAimer_Init = 0;
char _ForceGUp_Init = 0;
char _HostOptions_Init = 0;
char _HostOptions_ReadyPlayer = 0;
char _HostOptions_TeamColor = 0;
char _VehicleSelect_Init = 0;
char SavedMap = -1;
char CurrentMap = 0;
char VehiclesStatus = 0;
int Secondary_Save = 0;
char _FreeCam_Init = 0;
char ToggleScoreboard = 0;
char ToggleRenderAll = 0;

int OriginalMusicVolume = 0;
int Map;

VECTOR CameraPosition,
		PlayerPosition,
		targetPos,
		cameraPos,
		delta;
char RenderAllData[0x280];


/*========================================================*\
========                       000fffe0
================      Infinite Health/Moonjump Logic
========
\*========================================================*/
void InfiniteHealthMoonjump()
{

	if (!gameIsIn())
	{
		_InfiniteHealthMoonjump_Init = 0;
		return -1;
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
		return 0;

	// Player Health is always max.
	player->Health = PLAYER_MAX_HEALTH;
	// if X is pressed, lower gravity.
	if ((pad->btns & PAD_CROSS) == 0){
		*(float*)(PlayerPointer - 0x2EB4) = 0.125;
	}
	return 1;
}

/*========================================================*\
========                   000fffe1
================      Mask Username Logic
========
\*========================================================*/
void MaskUsername()
{
	if (gameIsIn())
	{
		_MaskUsername_Init = 0;
		return -1;
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
		return 0;

	if (*(u8*)0x0017225e != 0 && *(u8*)0x0133D804 != 0){
		memcpy((u8*)0x0017225e, (u8*)0x0133D804, 0xe);
	}
	return 1;
}

/*========================================================*\
========                     000fffe2
================      Hacked Keyboard Logic
========
\*========================================================*/
void HackedKeyboard()
{
	if(gameIsIn())
	{
		_HackedKeyboard_Init = 0;
		return -1;
	}
	PadButtonStatus * pad = (PadButtonStatus*)0x001ee600;
	if ((pad->btns & (PAD_START | PAD_SELECT)) == 0)
	{
		_HackedKeyboard_Init = 1;
	}
	else
	{
		_HackedKeyboard_Init = 0;
	}

	if(!_HackedKeyboard_Init)
		return 0;

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
	return 1;
}

/*========================================================*\
========                 000fffe3
================      Free Cam Logic
========
\*========================================================*/
void MovementInputs(Player * player, PadButtonStatus * pad)
{
	VECTOR v;

	// get rotation from yaw and pitch
	float ySin = sinf(player->CameraYaw.Value);
	float yCos = cosf(player->CameraYaw.Value);
	float pSin = sinf(player->CameraPitch.Value);
	float pCos = cosf(player->CameraPitch.Value);

	// Handle Speed
	// Default Speed
	float MOVE_SPEED = 0.5;
	// L1: Fast Speed
	if ((pad->btns & PAD_L1) == 0 && (pad->btns & PAD_R1) != 0)
	{
		MOVE_SPEED = 2.0;
	}
	// R1: Slow Speed
	if ((pad->btns & PAD_R1) == 0 && (pad->btns & PAD_L1) != 0)
	{
		MOVE_SPEED = 0.12;
	}

	// Left Analog
	// Swapped library pad->ljoy with another line.
	// It adds more error for drifting analog sticks.
	float LeftAnalogH = (*(float*)0x001ee708);
	float LeftAnalogV = (*(float*)0x001ee70c);
	if ((LeftAnalogV || LeftAnalogH) != 0 || (pad->btns != 0xFFFF))
	{
		float hSpeed = LeftAnalogH * MOVE_SPEED;
		float vSpeed = -LeftAnalogV * MOVE_SPEED;

		// Using ternary statements fixes a bug with movement.
		v[0] = ((pad->btns & PAD_UP) == 0) ? (yCos * MOVE_SPEED)
			: ((pad->btns & PAD_DOWN) == 0) ? (-yCos * MOVE_SPEED)
			: ((pad->btns & PAD_LEFT) == 0) ? (-ySin * MOVE_SPEED)
			: ((pad->btns & PAD_RIGHT) == 0) ? (ySin * MOVE_SPEED)
			: (yCos * vSpeed) + (ySin * hSpeed);
		v[1] = ((pad->btns & PAD_UP) == 0) ? (ySin * MOVE_SPEED)
			: ((pad->btns & PAD_DOWN) == 0) ? (-ySin * MOVE_SPEED)
			: ((pad->btns & PAD_LEFT) == 0) ? (yCos * MOVE_SPEED)
			: ((pad->btns & PAD_RIGHT) == 0) ? (-yCos * MOVE_SPEED)
			: (ySin * vSpeed) + (-yCos * hSpeed);
		v[2] = ((pad->btns & PAD_L2) == 0 && (pad->btns & PAD_R2) != 0) ? (pCos * -MOVE_SPEED)
			: ((pad->btns & PAD_R2) == 0 && (pad->btns & PAD_L2) != 0) ? (pCos * MOVE_SPEED)
			: (pSin * -vSpeed);
		v[3] = 0;
		vector_add(CameraPosition, CameraPosition, v);
	}
	// R3: Select target for lock rotation
	if ((pad->btns & PAD_R3) == 0)
	{
		vector_copy(targetPos, CameraPosition);
	}
	// Hold Circle: lock camera
	if ((pad->btns & PAD_CIRCLE) == 0)
	{
		vector_copy(cameraPos, CameraPosition);
		vector_subtract(delta, targetPos, cameraPos);
		vector_normalize(delta, delta);
		float pitch = asinf(-delta[2]);
		float yaw = atan2f(delta[1], delta[0]);

		player->CameraPitch.Value = pitch;
		player->CameraYaw.Value = yaw;
	}
}

void activate(Player * player, PlayerHUDFlags * hud)
{
	// Update stored camera position
	vector_copy(CameraPosition, player->CameraPos);

	// Copy Current Player Position and store it.
	vector_copy(PlayerPosition, player->PlayerPosition);

	// Let Camera go past the death barrier
	*(u32*)0x005F40DC = 0x10000006;

	// Set Respawn Timer to Zero, then negative so player can't respawn
	player->RespawnTimer = -1;

	// deactivate hud
	hud->Flags.Healthbar = 0;
	hud->Flags.Minimap = 0;
	hud->Flags.Weapons = 0;
	hud->Flags.Popup = 0;
	hud->Flags.NormalScoreboard = 0;
}

void deactivate(Player * player, PlayerHUDFlags * hud)
{
	// Reset Player Position
	float * PlayerCoordinates = (float*) player->UNK24;
	PlayerCoordinates[0] = PlayerPosition[0];
	PlayerCoordinates[1] = PlayerPosition[1];
	PlayerCoordinates[2] = PlayerPosition[2];

	// Set Camera Distance to Default
	player->CameraDistance = -6;

	// Don't let Camera go past death barrier
	*(u32*)0x005F40DC = 0x10400006;

	// Reset Respawn timer
	player->RespawnTimer = 0;

	// Reset Render Data/Function
	if (*(u32*)0x004D7168 == 0x03e00008)
	{
		*(u32*)0x004C0760 = 0x0C135C40;
		*(u32*)0x004C0878 = 0x0C135BD8;
		*(u32*)0x004C09E0 = 0x0C135C40;
		*(u32*)0x004C0A50 = 0x0C135C40;
		*(u32*)0x004D7168 = 0x78A20000;
		*(u32*)0x004D716C = 0x20A50010;
		// If off, grab RenderAllData and set it to render data.
		memcpy((u8*)0x00240A40, RenderAllData, 0x280);
	}
	RenderAllData[0x280] = 0;

	// reactivate hud
	hud->Flags.Healthbar = 1;
	hud->Flags.Minimap = 1;
	hud->Flags.Weapons = 1;
	hud->Flags.Popup = 1;
	hud->Flags.NormalScoreboard = 1;
}

void FreeCam()
{
	// ensure we're in game
	if (!gameIsIn())
	{
		_FreeCam_Init = 0;
		return -1;
	}

	// Get Local Player
	Player * player = (Player*)0x00347aa0;
	PadButtonStatus * pad = playerGetPad(player);
	PlayerHUDFlags * hud = hudGetPlayerFlags(0);

	if (!_FreeCam_Init)
	{
		// Don't activate if player is in Vehicle
		// Activate with L1 + R1 + L3
		if (!player->Vehicle && (pad->btns & (PAD_L1 | PAD_R1 | PAD_L3)) == 0)
		{
			_FreeCam_Init = 1;
			activate(player, hud);
		}
	}
	else if (_FreeCam_Init)
	{
		// Deactivate with L1 + R1 + R3
		if ((pad->btns & (PAD_L1 | PAD_R1 | PAD_R3)) == 0 || (pad->btns & PAD_TRIANGLE) == 0)
		{
			_FreeCam_Init = 0;
			// Triange: Set Player Position to current Camera Position
			if((pad->btns & PAD_TRIANGLE) == 0)
			{
				// Update PlayerPosition to CameraPosition.
				// Could just do vector_copy, but this is actually shorter in assembly.
				PlayerPosition[0] = CameraPosition[0];
				PlayerPosition[1] = CameraPosition[1];
				PlayerPosition[2] = CameraPosition[2];
			}
			deactivate(player, hud);
		}
	}
	
	if (!_FreeCam_Init)
		return 0;

	// If start isn't open, let inputs go through.
	if ((*(u32*)0x00347E58) == 0)
	{
		// Select: Toggle Score
		if ((pad->btns & PAD_SELECT) == 0 && ToggleScoreboard == 0)
		{
			ToggleScoreboard = 1;
			hud->Flags.NormalScoreboard = !hud->Flags.NormalScoreboard;
		}
		else if (!(pad->btns & PAD_SELECT) == 0)
		{
			ToggleScoreboard = 0;
		}
		// Square: Toggle Render All
		if ((pad->btns & PAD_SQUARE) == 0 && ToggleRenderAll == 0)
		{
			ToggleRenderAll = 1;
			
			// if render function has not been modified, then do so.
			if (*(u32*)0x004D7168 != 0x03e00008)
			{
				// Copy Render Data and save it.
				memcpy(RenderAllData, (u8*)0x00240A40, 0x280);

				// Turn off render functions
				*(u32*)0x004C0760 = 0x00000000;
				*(u32*)0x004C0878 = 0x00000000;
				*(u32*)0x004C09E0 = 0x00000000;
				*(u32*)0x004C0A50 = 0x00000000;
				*(u32*)0x004D7168 = 0x03e00008;
				*(u32*)0x004D716C = 0x00000000;

				// Set render data to -1.
				memset((u8*)0x00240A40, 0xff, 0x280);
			}
			else
			{
				// If Off, turn functions back to normal.
				*(u32*)0x004C0760 = 0x0C135C40;
				*(u32*)0x004C0878 = 0x0C135BD8;
				*(u32*)0x004C09E0 = 0x0C135C40;
				*(u32*)0x004C0A50 = 0x0C135C40;
				*(u32*)0x004D7168 = 0x78A20000;
				*(u32*)0x004D716C = 0x20A50010;

				// If off, grab RenderAllData and set it to render data.
				memcpy((u8*)0x00240A40, RenderAllData, 0x280);
			}
		}
		else if (!(pad->btns & PAD_SQUARE) == 0)
		{
			ToggleRenderAll = 0;
		}
		// Handle All Movement Inputs
		MovementInputs(player, pad);
	}
	// Apply Camera Position
	vector_copy(player->CameraPos, CameraPosition);

	// If player isn't dead, move player to X: Zero
	if ((player->PlayerState) != 0x99)
	{
		float * PlayerCoordinates = (float*) player->UNK24;
		PlayerCoordinates[0] = 0;
		PlayerCoordinates[1] = PlayerPosition[1];

		// Add 0x00100000 to Y so it doesn't hit death barriers.
		PlayerCoordinates[2] = PlayerPosition[2] + 0x00100000;
	}

	// Force Hold Wrench
	player->ChangeWeaponHeldId = 1;

	// Fix Void fall bug.  This only needs to load if fallen in the void.
	// Running any other time will cause the player to keep getting deaths if not in void.
	if ((*(u8*)0x0034A078) == 0x76)
		player->UNK19[4] = 0;

	// Constanty Set Camera Distance to Zero
	player->CameraDistance = 0;

	// fix death camera lock
	player->CameraPitchMin = 1.48353;
	player->CameraPitchMax = -1.22173;
	
	return 1;
}

/*========================================================*\
========                    000fffe4
================      sp-music-to-mp Logic
========
\*========================================================*/
void CampaignMusic()
{

	// check to see if multiplayer tracks are loaded
	if (*(u32*)0x001CF85C != 0x000F8D29)
	{
		_CampaignMusic_Init = 0;
		return -1;
	}

	_CampaignMusic_Init = 1;

	int DefaultMultiplayerTracks = 0x0d; // This number will never change
	int AddedTracks = (sizeof(Tracks)/sizeof(Tracks[0]));
	int StartingTrack = *(u8*)0x0021EC08;
	int AllTracks = DefaultMultiplayerTracks + AddedTracks;
	// Fun fact: The math for TotalTracks is exactly how the assembly is.  Didn't mean to do it that way.  (Minus the AddedTracks part)
	int TotalTracks = (DefaultMultiplayerTracks - StartingTrack + 1) + AddedTracks;
	int MusicDataPointer = *(u32*)0x0021DA24; // This is more than just music data pointer, but it's what Im' using it for.
	int CurrentTrack = *(u16*)0x00206990;
	int NextAddress = 0;
	for(Map = 0; Map < AddedTracks; Map++)
	{
		*(u32*)(0x001CF940 + NextAddress) = Tracks[Map][0];
		*(u32*)(0x001CF948 + NextAddress) = Tracks[Map][1];
		NextAddress += 0x10;
	}
	/*
		MusicFunctionData was hard to find.
		The value at 0x0021DA24 is where all the music logic first gets written too.
		Once written, it is then copied to the needed location to run the funcion.
		Then right when it's written, it is loaded and ran.
		In order for the game to randomize each track (including new added tracks),
		I had to write to the area before the finalized music function got written,
		so it would copy my data instead of the written data.
		If I wrote it to late, the game would only randomize the original song set for the first played song.
		Luckly the offset for the area before it's finilized area is the same for each map.
	*/
	// If not in main lobby, game lobby, ect.
	if(MusicDataPointer != 0x01430700){
		// if Last Track doesn't equal TotalTracks
		if(*(u32*)0x0021EC0C != TotalTracks){
			int MusicFunctionData = MusicDataPointer + 0x28A0D4;
			*(u16*)MusicFunctionData = AllTracks;
		}
	}

	// If in game
	if(gameIsIn())
	{
		// if volume doesn't equal zero, save it.
		if(OriginalMusicVolume == 0 && *(u32*)0x00171D44 != 0)
		{
			OriginalMusicVolume = *(u32*)0x00171D44;
		}
		int MusicVolume = OriginalMusicVolume;
		int TrackDuration = *(u32*)0x002069A4;
		// if TrackDuration less thn or equal to MusicVolume times 10,
		// and if MusicVolume is greater than zero
		// and if current track isn't original multiplayer track
		if(TrackDuration <= (MusicVolume * 10) && MusicVolume > 0 && CurrentTrack > DefaultMultiplayerTracks * 2)
		{
			// Set music volume by dividing track duration by 10.
			*(u32*)0x00171D44 = (TrackDuration / 10);
		}
		// if track is switching, or volume is less than or equal to zero
		if(TrackDuration == 0xBB80 || *(u32*)0x00171D44 <= 0)
		{
			// set volume back to original volume.
			*(u32*)0x00171D44 = OriginalMusicVolume;
		}
	}
	else if(!gameIsIn())
	{
		// Reset music volume if not in game.
		if(OriginalMusicVolume != 0){
			*(u32*)0x00171D44 = OriginalMusicVolume;
			OriginalMusicVolume = 0;
		}
	}
	return 1;
}

/*========================================================*\
========                   000fffe5
================      Follow Aimer Logic
========
\*========================================================*/
void FollowAimer()
{
	if (!gameIsIn())
	{
		_FollowAimer_Init = 0;
		return -1;
	}

	Player * player = (Player*)0x00347aa0;
	PadButtonStatus * pad = playerGetPad(player);
	if ((pad->btns & (PAD_L3 | PAD_R1)) == 0 || (pad->btns & (PAD_L3 | PAD_L2)) == 0)
	{
		_FollowAimer_Init = 1;
	}
	else
	{
		_FollowAimer_Init = 0;
	}
	if (!_FollowAimer_Init)
		return 0;

	memcpy((u8*)0x0034A9A4, (u8*)0x00349520, 0xc);
	return 1;
}

/*========================================================*\
========                 000fffe6
================      Force G^ Logic
========
\*========================================================*/
void ForceGUp()
{
	if (gameIsIn())
	{
		_ForceGUp_Init = 0;
		return -1;
	}
	PadButtonStatus * pad = (PadButtonStatus*)0x001ee600;
	if ((pad->btns & (PAD_L3 | PAD_R3 | PAD_L1)) == 0 || (pad->btns & (PAD_L3 | PAD_R3 | PAD_L2)) == 0)
	{
		_ForceGUp_Init = 1;
	}
	else
	{
		_ForceGUp_Init = 0;
	}

	if (!_ForceGUp_Init)
		return 0;

	void * GameSettings = (void*)(*(u32*)0x0021dfe8);
	if (*(u32*)GameSettings != 0)
	{
		int GreenUp = ((u32)GameSettings + 0x10F);
		if ((pad->btns & (PAD_L3 | PAD_R3 | PAD_L1)) == 0) memset((u8*)GreenUp, 0x06, 0x9);
		if ((pad->btns & (PAD_L3 | PAD_R3 | PAD_L2)) == 0) memset((u8*)GreenUp, 0x00, 0x9);
	}
	return 1;
}

/*========================================================*\
========                  000fffe7
================      Host Options Logic
========
\*========================================================*/

void HostOptions()
{
	// exit if in game or not in game lobby and not host
	if (gameIsIn() || *(u32*)0x00173aec == -1 || *(u32*)0x001723b0 == 0)
	{
		_HostOptions_Init = 0;
		_HostOptions_ReadyPlayer = 0;
		_HostOptions_TeamColor = 0;
		return -1;
	}

	_HostOptions_Init = 1;
	PadButtonStatus * pad = (PadButtonStatus*)0x001ee600;	
	int Selection = *(u32*)0x013C9310 - 0xa;
	void * Pointer = (void*)(*(u32*)0x0021dfe8);
	int ReadyStatus = ((u32)Pointer + 0x10e);
	if ((pad->btns & PAD_START) == 0 && Selection <= 0x9 && !_HostOptions_ReadyPlayer && (*(u32*)0x003434B8 != 0x259 || *(u32*)0x003434B8 != 0x261))
	{
		_HostOptions_ReadyPlayer = 1;
		*(u8*)((u32)ReadyStatus + Selection) = (*(u8*)((u32)ReadyStatus + Selection) == 0x06) ? 0x00 : 0x06;
		//printf("host value: 0x%x\n", ((u32)ReadyStatus + Selection));
	}
	else if (!(pad->btns & PAD_START) == 0 && _HostOptions_ReadyPlayer)
	{
		_HostOptions_ReadyPlayer = 0;
	}

	// Change Team/Color Logic
	if ((pad->btns & (PAD_L2)) == 0 && Selection <= 0x9 && !_HostOptions_TeamColor)
	{
		_HostOptions_TeamColor = 1;
		*(u32*)0x00172170 = *(u32*)0x013C9310 - 0xa;
		//printf("host value: 0x%x\n", Selection);
	}
	// if Change Team/Color Menu is open
	else if ((*(u32*)0x003434B8 == 0x259 || *(u32*)0x003434B8 == 0x261) && _HostOptions_TeamColor)
	{
		_HostOptions_TeamColor = 1;
		// ReadyPlayer is 1 so that when you exit, it doesn't ready the selected player.
		_HostOptions_ReadyPlayer = 1;
		// Set User ID back to zero.  Once menu is open, it doesn't matter what the ID is.
		*(u32*)0x00172170 = 0;
	}
	else if (!(pad->btns & (PAD_L2)) == 0 && _HostOptions_TeamColor && (*(u32*)0x003434B8 != 0x259 || *(u32*)0x003434B8 != 0x261))
	{
		_HostOptions_TeamColor = 0;
		*(u32*)0x00172170 = 0;
	}

	return 1;
}

/*========================================================*\
========                 000fffe8
================      Vehicle Select
========
\*========================================================*/

void LoadVehicleSub()
{
	asm __volatile__ (
		// Original Assembly
		//"lui    $t0, 0x012A \n"
		//"ori    $t0, $t0, 0xA254 \n"
		//"lui    $t1, 0x000F \n"
		//"ori    $t1, $t1, 0x6000 \n"

		/*
		inline asm automatically uses v0 and v1 for some stupid reason, even if in
		the clobbered register list. Due to this, I move them to temp registers,
		then move them back to where they need to go.
		*/
		"move   $t5, $v0;" 
		"move   $t6, $v1;" 
		"lb     $t3, 0x0(%0);"
		"lb     $t4, 0x0(%1);"
		"beq    $t3, $t4, _exit;"
		"move   $v0, $t5;"
		"move   $v1, $t6;"
		"sb     $t3, 0x0(%1);"
		"addiu  $a0, $a0, 0x0260;"
		"addiu  $v1, $zero, 0x0003;"
		"_Loop:"
		"lw     $v0, 0x0($a1);"
		"addiu  $v1, $v1, -0x1;"
		"addiu  $a1, $a1, 0x4;"
		"blez   $v0, _ZeroIt;"
		"sw     $v0, 0x0($a0);"
		"beq    $zero, $zero, _GoToLoop;"
		"_ZeroIt:"
		"sw     $zero, 0x0($a0);"
		"_GoToLoop:"
		"addiu  $a0, $a0, 0x4;"
		"bgez   $v1, _Loop;"
		"lui    $t0, 0x0072;"
		"ori    $t0, $t0, 0xD854;"
		"sw     $zero, 0x0($t0);"
		"_exit:"
		"jr     $ra;"
		"nop"
		: // Output
		: "r" (&CurrentMap), "r" (&SavedMap) // Input
		: "v0", "v1", "a0", "a1" // Clobbered Registers (Don't use these registers)
	);
}

void VehicleSelect()
{
	// If not on Create Game menu or in game or in local play
	if (*(u32*)0x00172194 == -1 || gameIsIn() || *(u32*)0x003434B8 != 0x136)
	{
		_VehicleSelect_Init = 0;
		return -1;
	}

	_VehicleSelect_Init = 1;
	// Load Vehicle Subroutine address
	u32 * Vehicles = (u32*)0x00719E24;
	// Check to see if vehicles are on or off
	VehiclesStatus = *(u32*)0x012C23B4;
	// Save current selected map
	CurrentMap = *(u8*)0x012AA254;
	// Secondary Save Function for Online
	Secondary_Save = 0x0072D854;
	// if vehicles subroutine address = original
	if (/* *Vehicles == 0x0C1D8A78 */*(u32*)Secondary_Save != 0)
	{
		// set our new subroutine address
		*Vehicles = 0x0c000000 | ((u32)(&LoadVehicleSub) >> 2);
	}
	// if Vehicles are turned off.
	if (VehiclesStatus == 0x3 && *(u32*)Secondary_Save != 0xA0400000)
	{
		// Reset secondary save function to normal
		*(u32*)Secondary_Save = 0xA0400000;
		// Set Saved map to -1
		SavedMap = -1;
	}
	return 1;
}

int main(void)
{
	// R3 + R2/L3
	if (*(u8*)0x000fffe0 == 1) InfiniteHealthMoonjump();
	// L1 + R1 + L3/L1 + R1 + R3
	if (*(u8*)0x000fffe3 == 1) FreeCam();
	// R3 + R2/L3 + R2
	if (*(u8*)0x000fffe5 == 1) FollowAimer();
	// L2 + R2 + Select
	if (*(u8*)0x000fffe1 == 1) MaskUsername();
	// Start + Select
	if (*(u8*)0x000fffe2 == 1) HackedKeyboard();
	// L3 + R3 + L1/L2
	if (*(u8*)0x000fffe6 == 1) ForceGUp();
	// L2 + X: Change Team, Start: Ready Player
	if (*(u8*)0x000fffe7 == 1) HostOptions();
	// Always Run
	if (*(u8*)0x000fffe4 == 1) CampaignMusic();
	if (*(u8*)0x000fffe8 == 1) VehicleSelect();

	return 1;
}