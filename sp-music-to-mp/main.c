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

#define TRACK_RANGE_MIN (0x0021EC08)
#define TRACK_RANGE_MAX (0x0021EC0C)
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
static char AdderTracks[] = {};


void CheckFirstBits(int Track, int LeftAudio, int RightAudio)
{
	// Get First 2 bytes of the track data
	LeftAudio = (LeftAudio >> 10);
	RightAudio = (RightAudio >> 10);
	if (LeftAudio == RightAudio)
	{
		AdderTracks[LeftAudio] = Track;
	}
	else
	{
		AdderTracks[LeftAudio] = Track;
		AdderTracks[RightAudio] = Track;
	}
}
void CampaignMusic(void)
{
	static int FinishedConvertingTracks = 0;
	static int AddedTracks = 0;
	static int SetupMusic = 0;
	static short Music[0x1f][2] = {};

	u32 CodeSegmentPointer = *(u32*)0x0021DA24;
	if (!musicIsLoaded() || CodeSegmentPointer == 0x01430700 || !enableSingleplayerMusic)
		return;

	u32 NewTracksLocation = 0x001CF940;
	if (!FinishedConvertingTracks) {
		AddedTracks = 0;
		int MultiplayerSector = *(u32*)0x001CF85C;
		int Stack = 0x0023ac00;
		int WAD_Table = 0x001CE470;
		int a;
		int Offset = 0;

		// Zero out stack by the appropriate heap size
		memset((u32*)Stack, 0, 0x2A0);

		// Loop through each WAD ID
		for(a = 0; a < 12; a++) {
			Offset += 0x18;
			int WAD = *(u32*)(WAD_Table + Offset);
			// Check if Map Sector is not zero
			if (WAD != 0) {
				internal_wadGetSectors(WAD, 1, Stack);
				int WAD_Sector = *(u32*)(Stack + 0x4);

				// make sure WAD_Sector isn't zero
				if (WAD_Sector != 0) {
					DPRINTF("WAD: 0x%X\n", WAD);
					DPRINTF("WAD Sector: 0x%X\n", WAD_Sector);
					// Get SP 2 MP Offset for current WAD_Sector.
					int SP2MP = WAD_Sector - MultiplayerSector;
					// Remember we skip the first track because it is the start of the sp track, not the body of it.
					int b = 0;
					int Songs = Stack + 0x18;
					// while current song doesn't equal zero, then convert.
					// if it does equal zero, that means we reached the end of the list and we move onto the next batch of tracks.
					do
					{
						int Track_LeftAudio = *(u32*)(Songs + b);
						int Track_RightAudio = *(u32*)((u32)(Songs + b) + 0x8);
						int ConvertedTrack_LeftAudio = SP2MP + Track_LeftAudio;
						int ConvertedTrack_RightAudio = SP2MP + Track_RightAudio;
						// Checks the first 16 bits of track for 0, 1, 2, ect.
						// CheckFirstBits(AddedTracks, ConvertedTrack_LeftAudio, ConvertedTrack_RightAudio);
						// Store converted tracks for later
						Music[AddedTracks][0] = (u16)ConvertedTrack_LeftAudio;
						Music[AddedTracks][1] = (u16)ConvertedTrack_RightAudio;
						// If on DreadZone Station, and first song, add 0x20 instead of 0x20
						// This fixes an offset bug.
						b = (a == 0 && b == 0) ? b + 0x28 : b + 0x20;
						AddedTracks++;
					}
					while (*(u32*)(Songs + b) != 0);
				} else {
					Offset -= 0x18;
					a--;
				}
			} else {
				a--;
			}
		// Zero out stack to finish the job.
		memset((u32*)Stack, 0, 0x2A0);
		}

		FinishedConvertingTracks = 1;
		DPRINTF("Added Tracks: %d\n", AddedTracks);
	};
	

	int DefaultMultiplayerTracks = 0x0d; // This number will never change
	int StartingTrack = *(u32*)musicTrackRangeMin();
	int AllTracks = DefaultMultiplayerTracks + AddedTracks;
	int TotalTracks = (DefaultMultiplayerTracks - StartingTrack + 1) + AddedTracks;
		// if TRACK_RANGE_MAX doesn't equal TotalTracks
	if (*(u32*)musicTrackRangeMax() != TotalTracks) {
		int MusicFunctionData = CodeSegmentPointer + 0x28A0D4;
		*(u16*)MusicFunctionData = AllTracks;
	}
	if (*(u32*)NewTracksLocation == 0 || !SetupMusic) {
		int Track;
		for(Track = 0; Track < AddedTracks; ++Track) {
			u32 Left = (u16)Music[Track][0];
			u32 Right = (u16)Music[Track][1];
			u32 Add1 = 0x10000;
			u32 Add2 = 0x20000;
			u32 Add3 = 0x30000;
			if (Track == 8) {
				Right += Add1;
			} else if (Track >= 9 && Track <= 31) {
				Left += Add1;
				Right += Add1;
			} else if (Track == 32) {
				Left += Add1;
				Right += Add2;
			} else if (Track >= 33 && Track <= 56) {
				Left += Add2;
				Right += Add2;
			} else if (Track >= 57) {
				Left += Add3;
				Right += Add3;
			}
			
			*(u32*)(NewTracksLocation) = Left;
			*(u32*)(NewTracksLocation + 0x08) = Right;
			NewTracksLocation += 0x10;
		}
		SetupMusic = 1;
	}

	// If in game
	if (isInGame()) {
		// if Cheats are Active
		if (enableSingleplayerMusic) {
			// if TRACK_MAX_RANGE doesn't equal TotalTracks
			// This will only happen in game if all codes are turned off and back on.
			if (*(u32*)musicTrackRangeMax() != TotalTracks)
				*(u32*)musicTrackRangeMax() = TotalTracks;
		} else {
			// Reset number of tracks to play to original 10.
			if (*(u32*)musicTrackRangeMax() != 0x0a)
				*(u32*)musicTrackRangeMax() = 0x0a;
		}
		if (*(u32*)0x002069A0 <= 0) {
			int CurrentTrack = musicCurrentTrack();
			int TrackDuration = musicTrackDuration();
			if ((CurrentTrack > DefaultMultiplayerTracks * 2) && (CurrentTrack != -1 && *(u32*)0x0020698C != 0) && (TrackDuration <= 0x3000))
				musicTransitionTrack(0,0,0,0);
		}
	}
}

int main(void)
{
	CampaignMusic();
	return 0;
}
