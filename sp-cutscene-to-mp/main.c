/*
 sp-cutscene-to-mp Code Created by Troy "Agent Moose" Pruitt
 Allows all single cutscene audio to be played in Multiplayer.

 Although this code is similar to the sp=music-to-mp, it does have some major differences.

 if SelectLevel is 1 (aka Dreadzone Station):
	0x15e: Courtney Gears and Al.
*/

#include <tamtypes.h>
#include <libdl/stdio.h>
#include <libdl/game.h>
#include <libdl/player.h>
#include <libdl/pad.h>
#include <libdl/music.h>

int Active = 0;
int FinishedConvertingTracks = 0;
int AddedTracks = 0;

int main(void)
{
	// check to see if multiplayer tracks are loaded
	if (!musicIsLoaded())
	{
		Active = 0;
		return -1;
	}

	Active = 1;

	int NewTracksLocation = 0x001CF940;
	if (!FinishedConvertingTracks || *(u32*)NewTracksLocation == 0)
	{
		AddedTracks = 0;
		int MultiplayerSectorID = *(u32*)0x001CF85C;
		int Stack = 0x0023ac00; // Original Location is 0x23ac00, but moving it somewhat fixes a bug with the SectorID.  But also, unsure if this breaks anything yet.
		int Sector = 0x001ce480; // Original Start Sector: 0x001ce470
		int a;
		int Offset = 0;
		
		// Zero out stack by the appropriate heap size (0x2a0 in this case)
		// This makes sure we get the correct values we need later on.
		memset((u32*)Stack, 0, 0x26F0);

		// Loop through each Sector
		for(a = 0; a < 1; a++)
		{
			int SelectedLevel = 1; // 1 - 12 : This is which planet to load.  It does not take into acount the cut places.
			Offset += 0x18 * SelectedLevel;
			int MapSector = *(u32*)(Sector + Offset);
			// Check if Map Sector is not zero
			if (MapSector != 0)
			{
				internal_wadGetSectors(MapSector, 1, Stack);
				int SectorID = *(u32*)(Stack + 0x4);
				if (SectorID != 0)
				{
					printf("Sector: 0x%X\n", MapSector);
					printf("Sector ID: 0x%X\n", SectorID);

					// do music stuffs~
					// Get SP 2 MP Offset for current SectorID.
					int SP2MP = SectorID - MultiplayerSectorID;
					// Remember we skip the first track because it is the start of the sp track, not the body of it.
					int b = 0;
					int Songs = Stack + 0x08;
					// while current song doesn't equal zero, then convert.
					// if it does equal zero, that means we reached the end of the list and we move onto the next batch of tracks.
					do
					{
						if (*(u32*)(Songs + b) != 0)
						{
							// Left Audio
							int StartingSong = *(u32*)(Songs + b);
							// Right Audio
							int EndingSong = *(u32*)((u32)(Songs + b) + 0x4);
							// Convert Left/Right Audio
							int ConvertedSong_Start = SP2MP + StartingSong;
							int ConvertedSong_End = SP2MP + EndingSong;
							// Apply newly Converted tracks
							*(u32*)(NewTracksLocation) = ConvertedSong_Start;
							*(u32*)(NewTracksLocation + 0x8) = ConvertedSong_Start; // Made the left and right audio just be the start audio.
							// *(u32*)(NewTracksLocation + 0x10) = ConvertedSong_End;
							// *(u32*)(NewTracksLocation + 0x18) = ConvertedSong_End;
							NewTracksLocation += 0x10;
							AddedTracks++;	
						}
						b += 0x4;
						//printf("Is less than: 0x%x\n", (u32)(Songs + b));
					}
					while ((Songs + b) <= 0x0023C500);
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
		memset((u32*)Stack, 0, 0x26F0);

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
	return 1;
}
