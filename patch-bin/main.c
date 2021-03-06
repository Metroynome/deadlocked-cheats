/*
Patch.bin subroutine.
 - Starts at 0x01e00000
 - On/Off bytes at 0x01e0efff
    - Offset + 0x00: Infinite Health/Moonjump
	- Offset + 0x01: Mask Username
	- Offset + 0x02: Hacked Keyboard
	- Offset + 0x03: Free Cam
	- Offset + 0x04: sp-music-to-mp
	- Offset + 0x05: Follow Aimer
	- Offset + 0x06: Force G^
	- Offset + 0x07: Host Options
	- Offset + 0x08: Vehicle Select
	- Offset + 0x09: Form Party and Unkick
	- Offset + 0x0a: Max Typing Limit
	- Offset + 0x0b: More Team Colors
	- Offset + 0x0c: Infinite Chargeboot
	- Offset + 0x0d: Render All
	- Offset + 0x0e: Rapid Fire Weapons
	- Offset + 0x0f: Walk Through Walls
	- Offset + 0x10: Rapid Fire Vehicles
	- Offset + 0x11: Lots of Deaths
	- Offset + 0x12: No Respawn Timer
	- Offset + 0x13: Walk Fast
	- Offset + 0x14: AirWalk
	- Offset + 0x15: Flying Vehicles
	- Offset + 0x16: Surfing Vehicles
	- Offset + 0x17: Fast Vehicles
	- Offset + 0x18: Respawn Anywhere
	- Offset + 0x19: vSync
	- Offset + 0x1a: All Alpha/Omega Mods
	- Offset + 0x1b: All Skill Points
	- Offset + 0x1c: Hacked Start Menu
	- Offset + 0x1d: Cheats Menu - Weapons
	- Offset + 0x1e: Lock On Fusion
	- Offset + 0x1f: Cheats Menu - NEW GAME
	- Offset + 0x20: Cheats Menu - Fusion Aimer (Doesn't Work)
*/

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

int CodeArea = 0x01e0efa0; // Where users can check to see if codes are on or off

/*
	Main Settings Address
	0 = All Codes on, No Secondary
	1 = All Codes Off, No Secondary
	2 = All Codes On, Use Secondary
	3 = All Codes Off, Use Secondary
*/
int InitSettings = 0x01e0efff;

// -1 = varify if InitSettings = 1
// 0 = All codes off
// 1 = All codes on
char _InitializeAllCodes = -1;
char _InitializeAllCodes_Secondary = 0;
char _InitializeAllCodes_Secondary_Active = 0;
char _InitializeAllCodes_Toggle = 0;

char _InfiniteHealthMoonjump_Init = 0;
char _MaskUsername_Init = 0;
char _HackedKeyboard_Init = 0;
char _CampaignMusic_Init = 0;
int FinishedConvertingTracks = 0;
int AddedTracks = 0;
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

int Map;

VECTOR CameraPosition,
		PlayerPosition,
		targetPos,
		cameraPos,
		delta;
char RenderAllData[0x280];

int CheckInitCodes(char Active);
int GetActiveUIPointer(u8 UI);
void internal_wadGetSectors(u64, u64, u64);

/*========================================================*\
========                       Offset + 0x00
================      Infinite Health/Moonjump Logic
========
\*========================================================*/
void InfiniteHealthMoonjump(char Active)
{

	if (!gameIsIn() || !CheckInitCodes(Active))
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
========                   Offset + 0x01
================      Mask Username Logic
========
\*========================================================*/
void MaskUsername(char Active)
{
	if (!CheckInitCodes(Active))
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
========                     Offset + 0x02
================      Hacked Keyboard Logic
========
\*========================================================*/
void HackedKeyboard(char Active)
{
	if (!CheckInitCodes(Active))
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

/*========================================================*\
========                 Offset + 0x03
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

void FreeCam(char Active)
{
	// ensure we're in game
	if (!gameIsIn())
	{
		_FreeCam_Init = 0;
		return;
	}

	// Get Local Player
	Player * player = (Player*)0x00347aa0;
	PadButtonStatus * pad = playerGetPad(player);
	PlayerHUDFlags * hud = hudGetPlayerFlags(0);

	if (!_FreeCam_Init && CheckInitCodes(Active))
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
		// Deactivate with L1 + R1 + R3 or _InitiateAllCodes == 0
		if ((pad->btns & (PAD_L1 | PAD_R1 | PAD_R3)) == 0 || (pad->btns & PAD_TRIANGLE) == 0 || !CheckInitCodes(Active))
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
		return;

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

	return;
}

/*========================================================*\
========                    Offset + 0x04
================      sp-music-to-mp Logic
========
\*========================================================*/
void CampaignMusic(char Active)
{
	// check to see if multiplayer tracks are loaded
	if ((*(u32*)0x001CF85C != 0x000F8D29))
	{
		_CampaignMusic_Init = 0;
		return;
	}

	_CampaignMusic_Init = 1;

	int NewTracksLocation = 0x001CF940;
	if (!FinishedConvertingTracks || *(u32*)NewTracksLocation == 0)
	{
		AddedTracks = 0;
		int MultiplayerSectorID = *(u32*)0x001CF85C;
		int Stack = 0x0023ac00; // Original Location is 0x23ac00, but moving it somewhat fixes a bug with the SectorID.  But also, unsure if this breaks anything yet.
		int Sector = 0x001CE470;
		int a;
		int Offset = 0;
		
		// Zero out stack by the appropriate heap size (0x2a0 in this case)
		// This makes sure we get the correct values we need later on.
		memset((u32*)Stack, 0, 0x2A0);

		// Loop through each Sector
		for(a = 0; a < 12; a++)
		{
			Offset += 0x18;
			int MapSector = *(u32*)(Sector + Offset);
			// Check if Map Sector is not zero
			if (MapSector != 0)
			{
				internal_wadGetSectors(MapSector, 1, Stack);
				int SectorID = *(u32*)(Stack + 0x4);

				// BUG FIX AREA: If Stack is set to 0x23ac00, you need to add SectorID != 0x1DC1BE to if statement.
				// The bug is: On first load, the SectorID isn't what I need it to be,
				// the internal_wadGetSectors function doesn't update it quick enough for some reason.
				// the following if statement fixes it

				// make sure SectorID doesn't match 0x1dc1be, if so:
				// - Subtract 0x18 from offset and -1 from loop.
				if (SectorID != 0x0)
				{
					DPRINTF("Sector: 0x%X\n", MapSector);
					DPRINTF("Sector ID: 0x%X\n", SectorID);

					// do music stuffs~
					// Get SP 2 MP Offset for current SectorID.
					int SP2MP = SectorID - MultiplayerSectorID;
					// Remember we skip the first track because it is the start of the sp track, not the body of it.
					int b = 0;
					int Songs = Stack + 0x18;
					// while current song doesn't equal zero, then convert.
					// if it does equal zero, that means we reached the end of the list and we move onto the next batch of tracks.
					do
					{
						// Left Audio
						int StartingSong = *(u32*)(Songs + b);
						// Right Audio
						int EndingSong = *(u32*)((u32)(Songs + b) + 0x8);
						// Convert Left/Right Audio
						int ConvertedSong_Start = SP2MP + StartingSong;
						int ConvertedSong_End = SP2MP + EndingSong;
						// Apply newly Converted tracks
						*(u32*)(NewTracksLocation) = ConvertedSong_Start;
						*(u32*)(NewTracksLocation + 0x08) = ConvertedSong_End;
						NewTracksLocation += 0x10;
						b += 0x20;
						AddedTracks++;
					}
					while (*(u32*)(Songs + b) != 0);
				}
				else
				{
					Offset -= 0x18;
					a--;
				}
			}
			else
			{
				a--;
			}
		}
		// Zero out stack to finish the job.
		memset((u32*)Stack, 0, 0x2A0);

		FinishedConvertingTracks = 1;
		DPRINTF("AddedTracks: %d\n", AddedTracks);
	};
	

	int DefaultMultiplayerTracks = 0x0d; // This number will never change
	int StartingTrack = *(u8*)0x0021EC08;
	int AllTracks = DefaultMultiplayerTracks + AddedTracks;
	// Fun fact: The math for TotalTracks is exactly how the assembly is.  Didn't mean to do it that way.  (Minus the AddedTracks part)
	int TotalTracks = (DefaultMultiplayerTracks - StartingTrack + 1) + AddedTracks;
	int MusicDataPointer = *(u32*)0x0021DA24; // This is more than just music data pointer, but it's what Im' using it for.
	int CurrentTrack = *(u16*)0x00206990;
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
		int MusicFunctionData = MusicDataPointer + 0x28A0D4;
		if(*(u32*)0x0021EC0C != TotalTracks && *(u16*)MusicFunctionData != AllTracks){
			*(u16*)MusicFunctionData = AllTracks;
		}
	}

	// If in game
	if(gameIsIn())
	{
		// if Cheats are Active
		if (CheckInitCodes(Active))
		{
			// if TRACK_MAX_RANGE doesn't equal TotalTracks
			// This will only happen in game if all codes are turned off and back on.
			if (*(u32*)0x0021EC0C != TotalTracks)
			{
				*(u32*)0x0021EC0C = TotalTracks;
			}
		}
		else if (!CheckInitCodes(Active))
		{
			// Reset number of tracks to play to original 10.
			if (*(u32*)0x0021EC0C != 0x0a)
			{
				*(u32*)0x0021EC0C = 0x0a;
			}
		}
		int TrackDuration = *(u32*)0x002069A4;
		if (*(u32*)0x002069A0 <= 0)
		{
			/*
				This part: (CurrentTrack != -1 && *(u32*)0x020698C != 0)
				fixes a bug when switching tracks, and running the command
				to set your own track or stop a track.
			*/
			if ((CurrentTrack > DefaultMultiplayerTracks * 2) && (CurrentTrack != -1 && *(u32*)0x020698C != 0) && (TrackDuration <= 0x3000))
			{
				// This techinally cues track 1 (the shortest track) with no sound to play.
				// Doing this lets the current playing track to fade out.
				musicTransitionTrack(0,0,0,0);
			}
		}
	}
}

/*========================================================*\
========                   Offset + 0x05
================      Follow Aimer Logic
========
\*========================================================*/
void FollowAimer(char Active)
{
	if (!gameIsIn() || !CheckInitCodes(Active))
	{
		_FollowAimer_Init = 0;
		return;
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
		return;

	memcpy((u8*)0x0034A9A4, (u8*)0x00349520, 0xc);
}

/*========================================================*\
========                 Offset + 0x06
================      Force G^ Logic
========
\*========================================================*/
void ForceGUp(char Active)
{
	if (gameIsIn() || !CheckInitCodes(Active))
	{
		_ForceGUp_Init = 0;
		return;
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
		return;

	void * GameSettings = (void*)(*(u32*)0x0021dfe8);
	if (*(u32*)GameSettings != 0)
	{
		int GreenUp = ((u32)GameSettings + 0x10F);
		if ((pad->btns & (PAD_L3 | PAD_R3 | PAD_L1)) == 0) memset((u8*)GreenUp, 0x06, 0x9);
		if ((pad->btns & (PAD_L3 | PAD_R3 | PAD_L2)) == 0) memset((u8*)GreenUp, 0x00, 0x9);
	}
}

/*========================================================*\
========                  Offset + 0x07
================      Host Options Logic
========
\*========================================================*/
void HostOptions(char Active)
{
	// exit if in game or not in game lobby and not host
	if (gameIsIn() || (*(u32*)0x00173aec == -1) || (*(u32*)0x001723b0 == 0) || !CheckInitCodes(Active))
	{
		_HostOptions_Init = 0;
		_HostOptions_ReadyPlayer = 0;
		_HostOptions_TeamColor = 0;
		return;
	}

	_HostOptions_Init = 1;
	PadButtonStatus * pad = (PadButtonStatus*)0x001ee600;	
	//int Selection = *(u32*)0x013C9310 - 0xa;
	// Updated Selection freezes when loading screen shows.
	// Needs fixing.
	if ((pad->btns & PAD_SELECT) == 0 && !gameIsIn())
	{
		int Selection = *(u32*)((*(u32*)((u32)0x011C7064 + (UIP_STAGING * 0x4))) + 0x230) - 0xa;
		void * Pointer = (void*)(*(u32*)0x0021dfe8);
		int ReadyStatus = ((u32)Pointer + 0x10e);
		if (Selection <= 0x9 && !_HostOptions_ReadyPlayer && (*(u32*)0x003434B8 != 0x259 || *(u32*)0x003434B8 != 0x261))
		{
			_HostOptions_ReadyPlayer = 1;
			*(u8*)((u32)ReadyStatus + Selection) = (*(u8*)((u32)ReadyStatus + Selection) == 0x06) ? 0x00 : 0x06;
			//printf("host value: 0x%x\n", ((u32)ReadyStatus + Selection));
		}
	}
	else if (!(pad->btns & PAD_SELECT) == 0 && _HostOptions_ReadyPlayer)
	{
		_HostOptions_ReadyPlayer = 0;
	}

	// Change Team/Color Logic
	if ((pad->btns & (PAD_L2)) == 0 && !gameIsIn())
	{
		int Selection = *(u32*)((*(u32*)((u32)0x011C7064 + (UIP_STAGING * 0x4))) + 0x230) - 0xa;
		if (Selection <= 0x9 && !_HostOptions_TeamColor)
		{
			_HostOptions_TeamColor = 1;
			*(u32*)0x00172170 = *(u32*)((*(u32*)((u32)0x011C7064 + (UIP_STAGING * 0x4))) + 0x230) - 0xa;
			//*(u32*)0x00172170 = *(u32*)0x013C9310 - 0xa;
			//printf("host value: 0x%x\n", Selection);
		}
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
}

/*========================================================*\
========                 Offset + 0x08
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

		// Nop secondary save function
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

void VehicleSelect(char Active)
{
	// If not on Create Game menu or in game or in local play
	if ((*(u32*)0x00172194 == -1) || gameIsIn() || (*(u32*)0x003434B8 != 0x136) || !CheckInitCodes(Active))
	{
		_VehicleSelect_Init = 0;
		if (gameIsIn())
		{
			SavedMap = -1;
		}
		return;
	}

	_VehicleSelect_Init = 1;
	// Load Vehicle Subroutine address
	u32 * Vehicles = (u32*)0x00719E24;
	// Get Create Game UI Pointer
	int CreateGameUIP = (*(u32*)((u32)0x011C7064 + (UIP_CREATE_GAME * 0x4)));
	// Check to see if vehicles are on or off
	VehiclesStatus = *(u32*)((u32)CreateGameUIP + 0x1B860);
	// Save current selected map
	CurrentMap = *(u8*)((u32)CreateGameUIP + 0x3700);
	// Secondary Save Function for Online
	Secondary_Save = 0x0072D854;
	// if vehicles subroutine address = original
	if (/* *Vehicles == 0x0C1D8A78 */*(u32*)Secondary_Save != 0)
	{
		// set our new subroutine address
		*Vehicles = 0x0c000000 | ((u32)(&LoadVehicleSub) >> 2);

		// Noping of the Secondary_save function is done in the assembly function
		// I do it because I beleive it caused problems when noping it this if statement.
	}
	// if Vehicles are turned off.
	if (VehiclesStatus == 0x3 && *(u32*)Secondary_Save != 0xA0400000)
	{
		// Reset secondary save function to normal
		*(u32*)Secondary_Save = 0xA0400000;

		// Set Saved map to -1
		SavedMap = -1;
	}
}

/*========================================================*\
========                    Offset + 0x09
================      Form Party and Unkick
========
\*========================================================*/
void FormPartyUnkick(char Active)
{
	if(!gameIsIn() && (*(u32*)0x00173aec == -1))
	{
		if (CheckInitCodes(Active))
		{
			// Online Lobby, show Form Party option
			int ActiveUI = GetActiveUIPointer(UIP_ONLINE_LOBBY);
			if (ActiveUI != 0)
			{
				//if ((*(u8*)0x01365724 != 0x4))
				if (*(u8*)((u32)ActiveUI + 0x2EE4) != 0x4)
				{
					// Enable Form Party Options
					*(u8*)((u32)ActiveUI + 0x2EE4) = 0x4;

					// Enable all Form Party Options if not Host
					// - Different ActiveUI
					// *(u8*)0x013AB7AC = 4;
					// *(u8*)0x013AB80C = 4;
					// *(u8*)0x013AB86C = 4;
					// *(u8*)0x013AB8CC = 4;

					// Enable Unkick
					// - Different ActiveUI
					// *(u32*)0x00759448 = 0;
					// *(u32*)0x0075945c = 0;
					// *(u32*)0x0075948c = 0;
				}
			}
			if (*(u32*)0x0075948c != 0)
			{
				// Enable Unkick
				*(u32*)0x00759448 = 0;
				*(u32*)0x0075945c = 0;
				*(u32*)0x0075948c = 0;
			}
		}
		else if(!CheckInitCodes(Active))
		{
			int ActiveUI = GetActiveUIPointer(UIP_ONLINE_LOBBY);
			if (ActiveUI != 0)
			{
				if (*(u8*)((u32)ActiveUI + 0x2EE4) == 0x4)
				{
					// Disable Form Party Options if All codes are off
					*(u8*)((u32)ActiveUI + 0x2EE4) = 1;
				}
			}
			if (*(u32*)0x0075948c == 0)
			{
				// Disable Unkick
				*(u32*)0x00759448 = 0x0C1C668A;
				*(u32*)0x0075945c = 0x10A0006A;
				*(u32*)0x0075948c = 0x10690237;
			}
		}
	}
}

/*========================================================*\
========                 Offset + 0x0a
================      Max Typing Limit
========
\*========================================================*/
void MaxTypingLimit(char Active)
{
	if(!gameIsIn() && (*(u32*)0x00173aec == -1) && CheckInitCodes(Active))
	{
		PadButtonStatus * pad = (PadButtonStatus*)0x001ee600;
		if ((pad->btns & (PAD_L1 | PAD_R1)) == 0)
		{
			int TypingLimit = (*(u32*)((u32)0x011C7064 + (UIP_KEYBOARD * 0x4))) + 0x350;
			//*(u8*)0x0133d8d0 = 0x4F;
			*(u8*)TypingLimit = 0x4F;
			return;
		}
	}
}

/*========================================================*\
========                  Offset + 0x0b
================      More Team Colors
========
\*========================================================*/
void MoreTeamColors(char Active)
{
	if(!gameIsIn() && CheckInitCodes(Active))
	{
		int ChangeSkinTeamArea = GetActiveUIPointer(UIP_CHANGE_SKIN_TEAM);
		if (ChangeSkinTeamArea != 0)
		{
			PadButtonStatus * pad = (PadButtonStatus*)0x001ee600;
			if ((pad->btns & (PAD_L2 | PAD_R2)) == 0)
			{
				//int ChangeSkinTeamArea = (*(u32*)((u32)0x011C7064 + (UIP_CHANGE_SKIN_TEAM * 0x4)));
				int AllowTeams = ChangeSkinTeamArea + 0x370;
				int Colors = ChangeSkinTeamArea + 0x2E0;
				*(u32*)AllowTeams = 0x01010101;
				*(u8*)Colors = 0xff;
			}
		}
	}
}

/*========================================================*\
========                   Offset + 0x0c
================      Infinite Chargeboot
========
\*========================================================*/
void InfiniteChargeboot(char Active)
{
	if (gameIsIn() && CheckInitCodes(Active))
	{
		Player * player = (Player*)0x00347aa0;
		PadButtonStatus * pad = playerGetPad(player);
		// I joker it this way because it will always load whenever I press L2
		// if done with PAD_L2
		// This is used to see if a certain byte is set. If so joker with L1 + L2, if not, just use L2.
		if (pad->btns == ((*(u8*)(CodeArea - 0x01) == 1) ? 0xFAFF : 0xFEFF))
		{
			player->TicksSinceStateChanged = 0x27;
		}
	}
}

/*========================================================*\
========               Offset + 0x0d
================      Render All
========
\*========================================================*/
void RenderAll(char Active)
{
	if (gameIsIn())
	{
		Player * player = (Player*)0x00347aa0;
		PadButtonStatus * pad = playerGetPad(player);
		// if render function has not been modified, then do so.
		if ((pad->btns & (PAD_SELECT | PAD_LEFT)) == 0 && (*(u32*)0x00240b40 == 0) && CheckInitCodes(Active))
		{
			// Copy Render Data and save it.
			memcpy((u8*)0x00240b40, (u8*)0x00240A40, 0x280);

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
		else if (((pad->btns & (PAD_SELECT | PAD_RIGHT)) == 0 && *(u32*)0x00240A40 == -1) || (!CheckInitCodes(Active) && *(u32*)0x00240A40 == -1))
		{
			// If Off, turn functions back to normal.
			*(u32*)0x004C0760 = 0x0C135C40;
			*(u32*)0x004C0878 = 0x0C135BD8;
			*(u32*)0x004C09E0 = 0x0C135C40;
			*(u32*)0x004C0A50 = 0x0C135C40;
			*(u32*)0x004D7168 = 0x78A20000;
			*(u32*)0x004D716C = 0x20A50010;

			// If off, restore render data.
			memcpy((u8*)0x00240A40, (u8*)0x00240b40, 0x280);
			// Set saved render data to 0.
			memset((u8*)0x00240B40, 0x0, 0x280);
		}
	}
}

/*========================================================*\
========                   Offset + 0x0e
================      Rapid Fire Weapons
========
\*========================================================*/
void RapidFireWeapons(char Active)
{
	if (gameIsIn() && CheckInitCodes(Active))
	{
		Player * player = (Player*)0x00347aa0;
		PadButtonStatus * pad = playerGetPad(player);
		if ((pad->btns & (PAD_R3 | PAD_R1)) == 0 || (pad->btns & (PAD_R3 | PAD_R1 | PAD_L2)) == 0)
		{
			player->WeaponCooldownTimer = 0;
		}
	}
}

/*========================================================*\
========                  Offset + 0x0f
================      Walk Through Walls
========
\*========================================================*/
void WalkThroughWalls(char Active)
{
	if (gameIsIn())
	{
		Player * player = (Player*)0x00347aa0;
		PadButtonStatus * pad = playerGetPad(player);
		// PAD_L1 | PAD_LEFT
		if (pad->btns == 0xFB7F && CheckInitCodes(Active))
		{
			*(u32*)0x00347e40 = 0x22a3;
		}
		// PAD_L1 | PAD_RIGHT
		else if (pad->btns == 0xFBDF || !CheckInitCodes(Active))
		{
			*(u32*)0x00347e40 = 0;
		}
	}
}

/*========================================================*\
========                   Offset + 0x10
================      Rapid Fire Vehicles
========
\*========================================================*/
void RapidFireVehicles(char Active)
{
	if (gameIsIn())
	{
		Player * player = (Player*)0x00347aa0;
		PadButtonStatus * pad = playerGetPad(player);
		if (player->Vehicle != 0 && (pad->btns & (PAD_R1 | PAD_R3)) == 0 && CheckInitCodes(Active))
		{
			*(u32*)0x00453C7C = 0x24020000;
			*(u32*)0x0045A440 = 0x24030000;
			*(u32*)0x00465AA4 = 0x24020000;
			*(u32*)0x00471D1C = 0x24030000;
			*(u32*)0x00473630 = 0xA21401FA;
			*(u32*)0x004737C8 = 0x24060000;
			*(u32*)0x0047EAD4 = 0xAE00030C;
			*(u32*)0x0047CB50 = 0xA24202F1;
			*(u32*)0x003BF0F8 = 0x24020000;
		}
		else if ((*(u32*)0x00453C7C == 0x24020000))
		{
			*(u32*)0x00453C7C = 0x24020004;
			*(u32*)0x0045A440 = 0x2403003C;
			*(u32*)0x00465AA4 = 0x24020004;
			*(u32*)0x00471D1C = 0x2403000F;
			*(u32*)0x00473630 = 0xA20001FA;
			*(u32*)0x004737C8 = 0x2406000F;
			*(u32*)0x0047EAD4 = 0xAE03030C;
			*(u32*)0x0047CB50 = 0xA24302F1;
			*(u32*)0x003BF0F8 = 0x24020008;
		}
	}
}

/*========================================================*\
========                 Offset + 0x11
================      Lots of Deaths
========
\*========================================================*/
void LotsOfDeaths(char Active)
{
	if (gameIsIn() && CheckInitCodes(Active) && _FreeCam_Init == 0)
	{
		Player * player = (Player*)0x00347aa0;
		PadButtonStatus * pad = playerGetPad(player);
		if ((pad->btns & (PAD_L1 | PAD_UP)) == 0)
		{
			*(u32*)0x0034a074 = 0;
		}
	}
}

/*========================================================*\
========                  Offset + 0x12
================      No Respawn Timer
========
\*========================================================*/
void NoRespawnTimer(char Active)
{
	Player * player = (Player*)0x00347aa0;
	PadButtonStatus * pad = playerGetPad(player);
	if (gameIsIn() && pad->btns == 0xBFFF && _FreeCam_Init == 0 && CheckInitCodes(Active))
	{
		*(u16*)0x00347e52 = 0;
	}
}

/*========================================================*\
========              Offset + 0x13
================      Walk Fast
========
\*========================================================*/
void WalkFast(char Active)
{
	if (gameIsIn())
	{
		Player * player = (Player*)0x00347aa0;
		PadButtonStatus * pad = playerGetPad(player);
		if ((pad->btns & (PAD_R3 | PAD_LEFT)) == 0 && CheckInitCodes(Active))
		{
			*(u32*)0x0034aa60 = 0x60f00000;
		}
		else if ((pad->btns & (PAD_R3 | PAD_RIGHT)) == 0 || (!CheckInitCodes(Active) && *(u32*)0x0034aa60 != 0x3f800000))
		{
			*(u32*)0x0034aa60 = 0x3f800000;
		}
	}
}

/*========================================================*\
========              Offset + 0x14
================      AirWalk
========
\*========================================================*/
void AirWalk(char Active)
{
	if (gameIsIn() && CheckInitCodes(Active))
	{
		Player * player = (Player*)0x00347aa0;
		PadButtonStatus * pad = playerGetPad(player);
		//(0xFFFD || 0xF7FD || 0xBFFD || 0x7FFD || 0x6FFD)
		if (pad->btns == 0xFFFD || pad->btns == 0xBFFD || pad->btns == 0x6FFD)
		{
			player->Airwalk = 0;
		}
		else if (pad->btns == 0xf7f9)
		{
			player->Airwalk = 0;
			player->WeaponCooldownTimer = 0;
		}
		else if (pad->btns == 0xfefd)
		{
			player->Airwalk = 0;
			player->TicksSinceStateChanged = 0x27;
		}
	}
}

/*========================================================*\
========                 Offset + 0x15
================      Flying Vehicles
========
\*========================================================*/
void FlyingVehicles(char Active)
{
	if (gameIsIn() && CheckInitCodes(Active))
	{
		Player * player = (Player*)0x00347aa0;
		PadButtonStatus * pad = playerGetPad(player);
		if (player->Vehicle != 0)
		{
			if (pad->btns == 0xFFFD)
			{
				void * Pointer = (void*)(*(u32*)0x0034a994);
				*(float*)(Pointer + 0x258) = 2.0;
			}
			else if (pad->btns == 0xFFFB)
			{
				void * Pointer = (void*)(*(u32*)0x0034a994);
				*(float*)(Pointer + 0x258) = 0.00557899475098;
			}
		}
	}
}

/*========================================================*\
========                  Offset + 0x16
================      Surfing Vehicles
========
\*========================================================*/
void SurfingVehicles(char Active)
{
	if (gameIsIn() && CheckInitCodes(Active))
	{
		Player * player = (Player*)0x00347aa0;
		PadButtonStatus * pad = playerGetPad(player);
		if (player->Vehicle != 0)
		{
			void * Pointer = (void*)(*(u32*)0x0034a994);
			// L2 + Up: On
			if (pad->btns == 0xfeef)
			{
				*(u8*)(Pointer + 0x304) = 1;
			}
			// L2 + Down: Off
			else if (pad->btns == 0xFFFB)
			{
				*(u8*)(Pointer + 0x304) = 0;
			}
		}
	}
}

/*========================================================*\
========                Offset + 0x17
================      Fast Vehicles
========
\*========================================================*/
void FastVehicles(char Active)
{
	if (gameIsIn() && CheckInitCodes(Active))
	{
		Player * player = (Player*)0x00347aa0;
		PadButtonStatus * pad = playerGetPad(player);
		if (player->Vehicle != 0)
		{
			// L2 + R2: Fast
			if ((pad->btns & (PAD_L2 | PAD_R2)) == 0)
			{
				void * Pointer = (void*)(*(u32*)0x0034a994);
				*(float*)(Pointer + 0x2f0) = 0.5039062;
			}
			// L2 + R1: Faster
			else if ((pad->btns & (PAD_L2 | PAD_R1)) == 0)
			{
				void * Pointer = (void*)(*(u32*)0x0034a994);
				*(float*)(Pointer + 0x2f0) = 2.0;
			}
		}
	}
}

/*========================================================*\
========                 Offset + 0x18
================      Respawn Anywhere
========
\*========================================================*/
void RespawnAnywhere(char Active)
{
	if (gameIsIn() && CheckInitCodes(Active))
	{
		Player * player = (Player*)0x00347aa0;
		PadButtonStatus * pad = playerGetPad(player);
		if ((pad->btns & (PAD_CIRCLE | PAD_SQUARE)) == 0)
		{
			player->PlayerState = 0x99;
		}
	}
}

/*========================================================*\
========              Offset + 0x19
================      vSync
========
\*========================================================*/
void vSync(char Active)
{
	PadButtonStatus * pad = (PadButtonStatus*)0x001ee600;
	if ((pad->btns & (PAD_R3 | PAD_UP)) == 0 && (*(u32*)0x00138dd0 != 0) && CheckInitCodes(Active))
	{
		*(u32*)0x00138dd0 = 0;
	}
	else if (((pad->btns & (PAD_R3 | PAD_DOWN)) == 0 || !CheckInitCodes(Active)) && *(u32*)0x00138dd0 == 0)
	{
		*(u32*)0x00138dd0 = 0x0c049c30;
	}
}

/*========================================================*\
========                   Offset + 0x1a
================      All Omega and Alpha Mods
========
\*========================================================*/
void OmegaAlphaMods(char Active)
{
	if (gameIsIn() && CheckInitCodes(Active))
	{
		void * Pointer = (void*)(*(u32*)0x0034a184);
		if(*(u32*)(Pointer + 0x20) != 0x63636300)
		{
			*(u32*)(Pointer + 0x20) = 0x63636300;
			*(u32*)(Pointer + 0x24) = 0x63636363;
			*(u32*)(Pointer + 0x28) = 0x00ff0063;
		}
	}
}

/*========================================================*\
========               Offset + 0x1b
================      All Skill Points
========
\*========================================================*/
void SkillPoints(char Active)
{
	if (gameIsIn() && (*(u32*)0x00171BA8 == 0) && CheckInitCodes(Active))
	{
		int num;
		for(num = 0; num < 0xb; num++)
		{
			*(u32*)(0x00171BA8 + (num * 4)) = 0x00107FFF;
		}
	}
}

/*========================================================*\
========                  Offset + 0x1d
================      Cheats Menu - Weapons
========
\*========================================================*/
void CheatsMenuWeapons(char Active)
{
	if (!CheckInitCodes(Active))
		return;

	// No need to check if in game because it does that in HackedStartMenu()
	*(u8*)0x00393740 = 0x00000010;
	*(u8*)0x00393748 = 0x0000000b;
	*(u16*)0x0039374c = 0x00000102;
	*(u16*)0x00393750 = 0x000023b6;
	*(u16*)0x00393760 = 0x00002626;
	*(u16*)0x00393764 = 0x00002627;
	*(u16*)0x00393768 = 0x00002622;
	*(u16*)0x0039376c = 0x00002628;
	*(u16*)0x00393770 = 0x00002624;
	*(u16*)0x00393774 = 0x00002623;
	*(u16*)0x00393778 = 0x0000262a;
	*(u16*)0x0039377c = 0x00002625;
	*(u16*)0x00393780 = 0x0000262e;
	*(u16*)0x00393784 = 0x0000262d;
	Player * player = (Player*)0x00347aa0;
	switch(*(u8*)0x0021de40)
	{
		case 1:
			player->ChangeWeaponHeldId = 0x2;
			break;
		case 2:
			player->ChangeWeaponHeldId = 0x3;
			break;
		case 3:
			player->ChangeWeaponHeldId = 0x4;
			break;
		case 4:
			player->ChangeWeaponHeldId = 0x5;
			break;
		case 5:
			player->ChangeWeaponHeldId = 0x6;
			break;
		case 6:
			player->ChangeWeaponHeldId = 0x7;
			break;
		case 7:
			player->ChangeWeaponHeldId = 0x8;
			break;
		case 8:
			player->ChangeWeaponHeldId = 0xf;
			break;
		case 9:
			player->ChangeWeaponHeldId = 0xd;
			break;
		case 10:
			player->ChangeWeaponHeldId = 0xc;
			break;
	}
}

/*========================================================*\
========                  Offset + 0x1f
================      Cheats Menu - END GAME
========
\*========================================================*/
 void CheatsMenuEndGame(char Active)
 {
	if (CheckInitCodes(Active))
	{
		*(u8*)0x00393a88 = 0x20;
		*(u16*)0x00393a94 = 0x014e;
		switch(*(u8*)0x0021de50)
		{
			case 1:
				gameEnd(4);
				*(u8*)0x0021de50 = 0;
				break;
		}
	}
 }

 /*========================================================*\
========                  Offset + 0x20
================      Cheats Menu - Fusion Aimer
========
\*=========================================================*/
void CheatsMenuFusionAimer(char Active)
{
	if (!CheckInitCodes(Active))
		return;
	
	// Use this is you want the pointer to be where the function is at
	//*(u32*)0x00393668 = (u32)(&testing);

	int CheatsAddress = 0x0021DE30;
	int MyCheatAddress = 0x01BF0000;
	int CheatID = MyCheatAddress + 0x0A;
	// All cheats are loaded from here: 0x0021DE30
	// the index of the cheat is then added to the above address.
	*(u32*)0x00393650 = (u32)CheatID;
	if (*(u32*)(CheatID + CheatsAddress) == 0x01 && *(u8*)0x0034A12C == 5)
	{
		*(u32*)0x004B3840 = 0x00000000;
		*(u32*)0x004B3848 = 0x00000000;
		*(u32*)0x0022DFD0 = 0x3F4CCCCD;
		*(u32*)0x0022DFD4 = 0x40FFFFFF;
		*(u32*)0x0022DFD8 = 0x00000003;
		*(u32*)0x0022DFDC = 0x00000000;
		*(u32*)0x0022DFF8 = 0x42200000;
		*(u32*)0x0022DFFC = 0x42200000;
	}
	else
	{
		*(u32*)0x004B3840 = 0x10600059;
		*(u32*)0x004B3848 = 0x18600050;
	}
}

/*========================================================*\
========                Offset + 0x1c
================      Hacked Start Menu
========
\*========================================================*/
void HackedStartMenu(char Active)
{
	if (gameIsIn())
	{
		if (CheckInitCodes(Active))
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
			OmegaAlphaMods(*(u8*)(CodeArea + 0x1a));
			SkillPoints(*(u8*)(CodeArea + 0x1b));
			CheatsMenuWeapons(*(u8*)(CodeArea + 0x1d));
			CheatsMenuEndGame(*(u8*)(CodeArea + 0x1f));
			//CheatsMenuFusionAimer(*(u8*)(CodeArea + 0x20));
		}
		else if (!CheckInitCodes(Active) && *(u16*)0x00560338 != 0x19A0)
		{
			*(u16*)0x00560338 = 0x19A0;
			*(u16*)0x00560350 = 0x1A30;
			*(u16*)0x00560368 = 0x1AD0;
		}
	}
	// If not in game, set Remove Helmet cheat back off.
	else if (*(u8*)0x0021de40 != 0)
	{
		*(u8*)0x0021de40 = 0;
	}
}

/*========================================================*\
========              Offset + 0x1e
================      Lock On Fusion
========
\*========================================================*/
void LockOnFusion(char Active)
{
	if (gameIsIn() && CheckInitCodes(Active) && _FreeCam_Init == 0)
	{
		Player * player = (Player*)0x00347aa0;
		PadButtonStatus * pad = playerGetPad(player);
		// On: R2 + Up
		if (pad->btns == 0xfdef)
		{
			*(u32*)0x0399cb0 = 0xffff1864;
			*(u32*)0x0399d30 = 0x00405748;
		}
		// Off: R2 + Down
		else if (pad->btns == 0xfdbf)
		{
			*(u32*)0x0399cb0 = 0xffff1096;
			*(u32*)0x0399d30 = 0x003f8ee0;
		}
	}
}

/*========================================================*\
========              Checks to see if code is supposed to
================      be on or off.
========
\*========================================================*/
int CheckInitCodes(char Active)
{
	// 0 = all codes on, no secondary
	// 1 = all codes off, no secondary
	// 2 = all codes on, use secondary
	// 3 = all codes off, use secondary
	char IsOn = 0;
	if (*(u8*)InitSettings == 0 || *(u8*)InitSettings == 1)
	{
		IsOn = 1;
		if (Active == 2)
			IsOn = -1;
	}
	else if (*(u8*)InitSettings == 2 || *(u8*)InitSettings == 3)
	{
		if (Active == 0)
		{
			IsOn = 1;
		}
		else if (Active == 1)
		{
			if (_InitializeAllCodes == 1)
			{
				IsOn = _InitializeAllCodes_Secondary_Active;
			}
			else if (_InitializeAllCodes == 0)
			{
				IsOn = 1;
			}
		}
		else if (Active == 2)
		{
			IsOn = -1;
		}
	}
	return _InitializeAllCodes == IsOn;
}

/*========================================================*\
========              Grabs the Active Pointer
================      if true: returns Pointer
========              if false: returns zero
\*========================================================*/
int GetActiveUIPointer(u8 UI)
{
	int UI_POINTERS = 0x011C7064;
	int Pointer = (*(u32*)((u32)UI_POINTERS + (UI * 0x4)));
	int ActiveUIPointer = (*(u32*)0x011C7108);
	if (ActiveUIPointer == Pointer)
	{
		return Pointer;
		// return printf("Pointer: %p\n", Pointer);
	}
	else
	{
		return 0;
	}
}

/*========================================================*\
========              
================      main function
========
\*========================================================*/
int main(void)
{
	// if Music isn't loaded, don't load other codes.
	// If going into Network Coniguration, you will need to go back to Start Menu to load patch.
	if (*(u32*)0x001CF85C != 0x000F8D29)
		return -1;

	if (_InitializeAllCodes == -1)
	{
		//InitSettings = *(u8*)0x01e0efff;
		switch(*(u8*)InitSettings)
		{
			/*	
				InitSetting
					0 = All Codes on, No Secondary
					1 = All Codes Off, No Secondary
					2 = All Codes On, Use Secondary
					3 = All Codes Off, Use Secondary

				_InitializeAllCodes
					- These are the normal codes to run.  Either on or off.
					0 = All Codes Off
					1 = All Codes On
				_InitializeAllCodes_Secondary
					0 = No secondary Codes, no need to use secondary joker.
					1 = Use Secondary Codes, use secondary joker for listed codes.
				_InitializeAllCodes_Secondary_Active
					0 = Secondary Codes Off
					1 = Secondary Codes On
			*/
			case 0:
			// all codes on, no secondary
				_InitializeAllCodes = 1;
				_InitializeAllCodes_Secondary = 0;
				break;
			case 1:
			// all codes off, no secondary
				_InitializeAllCodes = 0;
				_InitializeAllCodes_Secondary = 0;
				break;
			case 2:
			// all codes on, use secondary
				_InitializeAllCodes = 1;
				_InitializeAllCodes_Secondary = 1;
				break;
			case 3:
			// all codes off, use secondary
				_InitializeAllCodes = 0;
				_InitializeAllCodes_Secondary = 1;
				break;
		}
	}

	PadButtonStatus * pad = (PadButtonStatus*)0x001ee600;
	// L1 + L2 + R2 + Right + Down: Turn on/off all cheats
	if ((pad->btns & (PAD_L1 | PAD_L2 | PAD_R2 | PAD_RIGHT | PAD_DOWN)) == 0 && _InitializeAllCodes_Toggle == 0)
	{
		_InitializeAllCodes_Toggle = 1;
		_InitializeAllCodes = !_InitializeAllCodes;
	}
	// Secondary Codes: L1 + L2 + R2 + Left + Up
	else if ((pad->btns & (PAD_L1 | PAD_L2 | PAD_R2 | PAD_LEFT | PAD_UP)) == 0 && _InitializeAllCodes == 1 && _InitializeAllCodes_Toggle == 0 && _InitializeAllCodes_Secondary == 1)
	{
		_InitializeAllCodes_Toggle = 1;
		_InitializeAllCodes_Secondary_Active = !_InitializeAllCodes_Secondary_Active;
	}
	else if ((pad->btns & (PAD_L1 | PAD_L2 | PAD_R2 | PAD_RIGHT | PAD_DOWN)) != 0 && (pad->btns & (PAD_L1 | PAD_L2 | PAD_R2 | PAD_LEFT | PAD_UP)) != 0)
	{
		_InitializeAllCodes_Toggle = 0;
	}

	// R3 + R2/L3
	InfiniteHealthMoonjump(*(u8*)(CodeArea + 0x00));
	// L2 + R2 + Select
	MaskUsername(*(u8*)(CodeArea + 0x01));
	// Select + L2
	HackedKeyboard(*(u8*)(CodeArea + 0x02));
	// L1 + R1 + L3/L1 + R1 + R3
	FreeCam(*(u8*)(CodeArea + 0x03));
	// R3 + R2/L3 + R2
	FollowAimer(*(u8*)(CodeArea + 0x05));
	// L3 + R3 + L1/L2
	ForceGUp(*(u8*)(CodeArea + 0x06));
	// L2 + X: Change Team, Start: Ready Player
	HostOptions(*(u8*)(CodeArea + 0x07));
	// L1 + R1
	MaxTypingLimit(*(u8*)(CodeArea + 0x0a));
	// L2 + R2
	MoreTeamColors(*(u8*)(CodeArea + 0x0b));
	// Hold L2
	InfiniteChargeboot(*(u8*)(CodeArea + 0x0c));
	// Select + Left/Right
	RenderAll(*(u8*)(CodeArea + 0x0d));
	// R3 + R1 or R3 + R1 + L2
	RapidFireWeapons(*(u8*)(CodeArea + 0x0e));
	// L1 + Left/Right
	WalkThroughWalls(*(u8*)(CodeArea + 0x0f));
	// Hold R3 + R1
	RapidFireVehicles(*(u8*)(CodeArea + 0x10));
	// L1 + Up
	LotsOfDeaths(*(u8*)(CodeArea + 0x11));
	// Press X
	NoRespawnTimer(*(u8*)(CodeArea + 0x12));
	// R3 + Left/Right
	WalkFast(*(u8*)(CodeArea + 0x13));
	// Hold L3
	AirWalk(*(u8*)(CodeArea + 0x14));
	// Hold L3: High; or Hold R3: Float
	FlyingVehicles(*(u8*)(CodeArea + 0x15));
	// Hold L3 or R3
	SurfingVehicles(*(u8*)(CodeArea + 0x16));
	// L2 + R2: Fast; L2 + R1: Faster
	FastVehicles(*(u8*)(CodeArea + 0x17));
	// Circle + Square
	RespawnAnywhere(*(u8*)(CodeArea + 0x18));
	// R3 + Up/Down
	vSync(*(u8*)(CodeArea + 0x19));
	// R2 + Up/Down
	LockOnFusion(*(u8*)(CodeArea + 0x1e));

	// Always Run
	// CampaignMusic(*(u8*)(CodeArea + 0x04)); // Added to Server
	VehicleSelect(*(u8*)(CodeArea + 0x08));
	FormPartyUnkick(*(u8*)(CodeArea + 0x09));
	// Hacked Start Menu loads following codes inside:
	// Hacked Cheats Menu, All Skill Points, All Omega/Alpha Mods
	// No use to have those codes on if cheat menu isn't.
	HackedStartMenu(*(u8*)(CodeArea + 0x1c));

	return 1;
}
