/***************************************************
 * FILENAME :		main.c
 * 
 * DESCRIPTION :
 * 		Manages and applies all Deadlocked patches.
 * 
 * NOTES :
 * 		Each offset is determined per app id.
 * 		This is to ensure compatibility between versions of Deadlocked/Gladiator.
 * 		
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */

#include <tamtypes.h>

#include <libdl/dl.h>
#include <libdl/player.h>
#include <libdl/pad.h>
#include <libdl/time.h>
#include "config.h"
#include <libdl/game.h>
#include <libdl/string.h>
#include <libdl/stdio.h>
#include <libdl/patch.h>
#include <libdl/ui.h>
#include <libdl/graphics.h>
#include <libdl/music.h>

void onConfigOnlineMenu(void);
void onConfigGameMenu(void);
void onConfigInitialize(void);
void onConfigUpdate(void);
void configMenuEnable(void);
void configMenuDisable(void);

// 
int hasInitialized = 0;
int lastMenuInvokedTime = 0;
int lastGameState = 0;
int isInStaging = 0;
const char * patchConfigStr = "PATCH CONFIG";

// 
PatchConfig_t config __attribute__((section(".config"))) = {
	0
};

/*
 * NAME :		runEnableSingleplayerMusic
 * 
 * DESCRIPTION :
 * 
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Troy "Agent Moose" Pruitt
 */
void runEnableSingleplayerMusic(void)
{
	static int FinishedConvertingTracks = 0;
	static int AddedTracks = 0;

	if (!config.enableSingleplayerMusic || !musicIsLoaded())
		return;

	u32 NewTracksLocation = 0x001CF940;
	if (!FinishedConvertingTracks || *(u32*)NewTracksLocation == 0)
	{
		AddedTracks = 0;
		int MultiplayerSectorID = *(u32*)0x001CF85C;
		int Stack = 0x0023ad00; // Original Location is 0x23ac00, but moving it somewhat fixes a bug with the SectorID.  But also, unsure if this breaks anything yet.
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

/*
 * NAME :		main
 * 
 * DESCRIPTION :
 * 			Applies all patches and modules.
 * 
 * NOTES :
 * 
 * ARGS : 
 * 
 * RETURN :
 * 
 * AUTHOR :			Daniel "Dnawrkshp" Gerendasy
 */
int main (void)
{
	int i;

	// Call this first
	dlPreUpdate();

	// Run add singleplayer music
	runEnableSingleplayerMusic();

	// config update
	onConfigUpdate();

	// in game stuff
	if (gameIsIn())
	{
		// close config menu on transition to lobby
		if (lastGameState != 1)
			configMenuDisable();

		//
		if (!hasInitialized)
		{
			DPRINTF("patch loaded\n");
			onConfigInitialize();
			hasInitialized = 1;
		}

		// trigger config menu update
		onConfigGameMenu();

		lastGameState = 1;
	}
	else
	{
		// Hook menu loop
		if (*(u32*)0x00594CBC == 0)
			*(u32*)0x00594CB8 = 0x0C000000 | ((u32)(&onOnlineMenu) / 4);

		// send patch game config on create game
		GameSettings * gameSettings = gameGetSettings();
		if (gameSettings && gameSettings->GameLoadStartTime < 0)
		{
			isInStaging = 1;
		}
		else
		{
			isInStaging = 0;
		}
		
		// close config menu on transition to lobby
		if (lastGameState != 0)
			configMenuDisable();

		lastGameState = 0;
	}

	// Call this last
	dlPostUpdate();

	return 0;
}
