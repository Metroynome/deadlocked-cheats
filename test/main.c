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
	if (!FinishedConvertingTracks)
	{
		AddedTracks = 0;
		int MultiplayerSectorID = *(u32*)0x001CF85C;
		int Stack = 0x0023ac00;
		int Sector = 0x001CE468;
		int end_address = 0x001CEBE0;
		int Offset = 0;
		int a = 0;

		do
		{
			memset((u32*)Stack, 0, 0x26F0);
			Offset += 0x8;
			int MapSector = *(u32*)(Sector + Offset);
			if (MapSector != 0)
			{
				internal_wadGetSectors(MapSector, 1, Stack);
				int SectorID = *(u32*)(Stack);
				// levelaudiowad
				if (SectorID == 0x2a0)
				{
					printf("\nAudioWAD Address: 0x%x", (u32)(Sector + Offset));
				}
				// levelscenewad
				else if (SectorID == 0x26f0)
				{
					printf("\nSceneWAD Address: 0x%x", (u32)(Sector + Offset));
				}
				// levelwad
				else if (SectorID == 0xc68)
				{
					printf("\nLevelWAD Address: 0x%x", (u32)(Sector + Offset));
				}
				else
				{
					printf("\nUnidentified WAD:");
					printf("\nAddress: 0x%x", (u32)(Sector + Offset));
					printf("\nSize: 0x%x", *(u32*)(Sector + Offset));
				}
				a++;
			}
		} while ((u32)(Sector + Offset) <= end_address);
		memset((u32*)Stack, 0, 0x26F0);

		FinishedConvertingTracks = 1;
	};

	return 0;
}
