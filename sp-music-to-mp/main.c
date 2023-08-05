/*
 sp-music-to-mp Code Created by Troy "Agent Moose" Pruitt
 Allows all single player songs to be played in Multiplayer.
*/

#include <tamtypes.h>
#include <libdl/stdio.h>
#include <libdl/game.h>
#include <libdl/player.h>
#include <libdl/pad.h>
#include <libdl/music.h>

#define TRACK_RANGE_MIN (*(u8*)0x0021EC08)
#define TRACK_RANGE_MAX (*(u8*)0x0021EC0C)
#define CURRENT_TRACK (*(u16*)0x00206990)
#define TRACK_DURATION (*(u32*)0x002069A4)

int musicCurrentTrack(void)
{
    return CURRENT_TRACK;
}

int musicTrackDuration(void)
{
    return TRACK_DURATION;
}

int musicTrackRangeMin(void)
{
    return TRACK_RANGE_MIN;
}

int musicTrackRangeMax(void)
{
    return TRACK_RANGE_MAX;
}


int enableSingleplayerMusic = 1;

typedef struct MusicTrack {
	u16 LeftAudio;
	u16 RightAudio;
} MusicTrack;
MusicTrack Music[0x1f] = {};

void CampaignMusic()
{
	static int FinishedConvertingTracks = 0;
	static int AddedTracks = 0;
	static int SetupMusic = 0;

	// check to see if multiplayer tracks are loaded
	if (!musicIsLoaded())
		return;

	u32 NewTracksLocation = 0x001CF940;
	if (!FinishedConvertingTracks)
	{
		AddedTracks = 0;
		int MultiplayerSector = *(u32*)0x001CF85C;
		int Stack = 0x0023ac00;
		int WAD_Table = 0x001CE470;
		int a;
		int Offset = 0;

		// Zero out stack by the appropriate heap size (0x2a0 in this case)
		// This makes sure we get the correct values we need later on.
		memset((u32*)Stack, 0, 0x2A0);

		// Loop through each WAD ID
		for(a = 0; a < 12; a++)
		{
			Offset += 0x18;
			int WAD = *(u32*)(WAD_Table + Offset);
			// Check if Map Sector is not zero
			if (WAD != 0)
			{
				internal_wadGetSectors(WAD, 1, Stack);
				int WAD_Sector = *(u32*)(Stack + 0x4);

				// make sure WAD_Sector isn't zero
				if (WAD_Sector != 0)
				{
					printf("WAD: 0x%X\n", WAD);
					printf("WAD Sector: 0x%X\n", WAD_Sector);

					// do music stuffs~
					// Get SP 2 MP Offset for current WAD_Sector.
					int SP2MP = WAD_Sector - MultiplayerSector;
					// Remember we skip the first track because it is the start of the sp track, not the body of it.
					int b = 0;
					int Songs = Stack + 0x18;
					// while current song doesn't equal zero, then convert.
					// if it does equal zero, that means we reached the end of the list and we move onto the next batch of tracks.
					do
					{
						short Track_LeftAudio = *(u32*)(Songs + b);
						short Track_RightAudio = *(u32*)((u32)(Songs + b) + 0x8);
						int ConvertedTrack_LeftAudio = SP2MP + Track_LeftAudio;
						int ConvertedTrack_RightAudio = SP2MP + Track_RightAudio;
						// Store converted tracks for later
						Music[AddedTracks].LeftAudio = (u16)ConvertedTrack_LeftAudio;
						Music[AddedTracks].RightAudio = (u16)ConvertedTrack_RightAudio;
						// If on DreadZone Station, and first song, add 0x20 instead of 0x20
						// This fixes an offset bug.
						if (a == 0 && b == 0)
						{
							b += 0x28;
						}
						else
						{
							b += 0x20;
						}
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
		printf("Added Tracks: %d\n", AddedTracks);
	};
	

	int DefaultMultiplayerTracks = 0x0d; // This number will never change
	int StartingTrack = musicTrackRangeMin();
	int AllTracks = DefaultMultiplayerTracks + AddedTracks;
	int TotalTracks = (DefaultMultiplayerTracks - StartingTrack + 1) + AddedTracks;
	int CodeSegmentPointer = *(u32*)0x0021DA24;
	// If not in main lobby, game lobby, ect.
	if(CodeSegmentPointer != 0x01430700){
		// if TRACK_RANGE_MAX doesn't equal TotalTracks
		// if(musicTrackRangeMax() != TotalTracks){
		// 	int MusicFunctionData = CodeSegmentPointer + 0x28A0D4;
		// 	*(u16*)MusicFunctionData = AllTracks;
		// }
		if (*(u32*)NewTracksLocation == 0 || !SetupMusic)
		{
			int Track;
			for(Track = 0; Track < AddedTracks; ++Track)
			{
				u32 Left = Music[Track].LeftAudio;
				u32 Right = Music[Track].RightAudio;
				int Adder_1 = 0x10000;
				int Adder_2 = 0x20000;
				int Adder_3 = 0x30000;
				if (Track == 8)
				{
					Right += Adder_1;
				}
				else if (Track >= 9 && Track <= 31)
				{
					Left += Adder_1;
					Right += Adder_1;
				}
				else if (Track == 32)
				{
					Left += Adder_1;
					Right += Adder_2;
				}
				else if (Track >= 33 && Track <= 56)
				{
					Left += Adder_2;
					Right += Adder_2;
				}
				else if (Track >= 57)
				{
					Left += Adder_3;
					Right += Adder_3;
				}
				
				*(u32*)(NewTracksLocation) = Left;
				*(u32*)(NewTracksLocation + 0x08) = Right;
				NewTracksLocation += 0x10;
			}
			SetupMusic = 1;
		}
	}

	// If in game
	if(isInGame())
	{
		// if Cheats are Active
		if (enableSingleplayerMusic)
		{
			// if TRACK_MAX_RANGE doesn't equal TotalTracks
			// This will only happen in game if all codes are turned off and back on.
			if (*(u32*)0x0021EC0C != TotalTracks)
			{
				*(u32*)0x0021EC0C = TotalTracks;
			}
		}
		else
		{
			// Reset number of tracks to play to original 10.
			if (*(u32*)0x0021EC0C != 0x0a)
			{
				*(u32*)0x0021EC0C = 0x0a;
			}
		}
		if (*(u32*)0x002069A0 <= 0)
		{
			int CurrentTrack = musicCurrentTrack();
			int TrackDuration = musicTrackDuration();
			if ((CurrentTrack > DefaultMultiplayerTracks * 2) && (CurrentTrack != -1 && *(u32*)0x0020698C != 0) && (TrackDuration <= 0x3000))
			{
				musicTransitionTrack(0,0,0,0);
			}
		}
	}
}

int main(void)
{
	CampaignMusic();
	return 0;
}
