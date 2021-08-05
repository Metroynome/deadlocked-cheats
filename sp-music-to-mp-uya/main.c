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

int Tracks[][2] = {
	// Multiplayer
	{0x4e9dc, 0x4ec1b}, // Main Lobby
	{0x4ee5a, 0x4f1fe}, // Game Lobby
	{0x4f5a2, 0x4f844}, // Winning Screen
	{0x4fae6, 0x4fd4e}, // Loss Screen
	{0x4ffb6, 0x50635}, // 
	{0x50cb4, 0x51318}, // 
	{0x5197c, 0x51f7e}, // 
	{0x52580, 0x52b7d}, // 
	{0x5317a, 0x53821}, // 
	{0x53ec8, 0x544a1}, // 
	{0x54a7a, 0x5511c}, // 
	{0x557be, 0x55f81}, // 
	{0x56744, 0x56e06}, // 
	{0x574c8, 0x57ab7}, // 
	// Insomniac Museum
	{0x3c8af, 0x3c907}, // --Start
	{0x3c95f, 0x3cf2f}, // 
	{0x3d573, 0x3d5c2}, // --Start
	{0x3d611, 0x3dbc2}, // 
	// Starship Phoenix
	{0x26a2, 0x271d}, // --Start
	{0x2798, 0x2d53}, // 
	{0x330e, 0x3366}, // Training Room Warm Up --Start
	{0x33be, 0x398e}, // Training Room Warm Up
	{0x3f5e, 0x3fbc}, // Training Room --Start
	{0x401a, 0x45eb}, // Training Room
	// Marcadia
	{0xac9a, 0xacf2}, // --Start
	{0xad4a, 0xb2ee}, // 
	{0xb892, 0xb8ef}, // --Start
	{0xb94c, 0xbf6d}, // 
	{0xc58e, 0xc5e0}, // --Start
	{0xc632, 0xcbb0}, // 
	// Annihilation Nation
	{0x13792, 0x137c5}, // --Start
	{0x137f8, 0x13b53}, // 
	{0x13eae, 0x13f05}, // --Start
	{0x13f5c, 0x1453f}, // 
	{0x14b22, 0x14b76}, // --Start
	{0x14bca, 0x1515b}, // 
	{0x15794, 0x15d79}, // --Start
	{0x1635e, 0x16376}, // 
	// Aquatos
	{0x1ce6d, 0x1cec5}, // --Start
	{0x1cf1d, 0x1d570}, // 
	// Tyrranosis
	{0x1e42d, 0x1e488}, // --Start
	{0x1e4e3, 0x1ea91}, // 
	// Daxx
	{0xea6c, 0xeac6}, // --Start
	{0xeb20, 0xf0b8}, // 
	{0x1036c, 0x103ba}, // --Start
	{0x10408, 0x109cd}, // 
	// Obani Gemini
	{0x24a35, 0x24aa6}, // --Start
	{0x24b17, 0x250ce}, // 
	// Blackwater City
	{0x26eeb, 0x26f3f}, // --Start
	{0x26f93, 0x27524}, // 
	// Holostar Studios
	{0x28e18, 0x293e8}, // 
	// Obani Draco
	{0x35ef9, 0x35f4c}, // --Start
	{0x35f9f, 0x365b6}, // 
	{0x36bcd, 0x36bf6}, // Courtney Gears --Start
	{0x36c1f, 0x3714b}, // Courtney Gears
	// Zeldrin Starport
	{0x21027, 0x21604}, // 
	// Metropolis
	{0x2c670, 0x2cd72}, // 
	// Crash Site
	{0x2d651, 0x2dc16}, // 
	// Aradia
	{0x2e301, 0x2e8f0}, // 
	// Quark's Hideout
	{0x31544, 0x31abd}, // 
	// Koros
	{0x2ad74, 0x2b34d}, // 
	// Command Center
	{0x38214, 0x3823f}, // --Start
	{0x3826a, 0x3885a} // 
};

int Active = 0;
int Map;
int PLAYING_TRACK = 0;

int main(void)
{
	// check to see if multiplayer tracks are loaded
	if (*(u32*)0x001F8584 != 0x000BEF23)
	{
		*(u32*)0x001F8584 = 0x000BEF23;
	}
	Active = 1;

	int DefaultMultiplayerTracks = 0x0d; // This number will never change
	int AddedTracks = (sizeof(Tracks)/sizeof(Tracks[0]));
	int StartingTrack = *(u8*)0x002435B8;
	int AllTracks = DefaultMultiplayerTracks + AddedTracks;
	// Fun fact: The math for TotalTracks is exactly how the assembly is.  Didn't mean to do it that way.  (Minus the AddedTracks part)
	int TotalTracks = (DefaultMultiplayerTracks - StartingTrack + 1) + AddedTracks;
	//int MusicDataPointer = *(u32*)0x0021DA24; // Address: 0x0This is more than just music data pointer, but it's what Im' using it for.
	//int CurrentTrack = *(u16*)0x00225828;
	int NextAddress = 0;
	for(Map = 0; Map < AddedTracks; Map++)
	{
		*(u32*)(0x001F8588 + NextAddress) = Tracks[Map][0];
		*(u32*)(0x001F8590 + NextAddress) = Tracks[Map][1];
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
	// if(MusicDataPointer != 0x01430700){
	// 	// if Last Track doesn't equal TotalTracks
	// 	if(*(u32*)0x0021EC0C != TotalTracks){
	// 		// Commented because not needed but for documumentation.
	// 		// Changes TRACK_RANGE_MIN.
	// 		// int TRACK_RANGE_MIN = MusicDataPointer + 0x28A0D0;
	// 		// *(u16*)TRACK_RANGE_MIN = 0;
	// 		int MusicFunctionData = MusicDataPointer + 0x28A0D4;
	// 		*(u16*)MusicFunctionData = AllTracks;
	// 	}
	// }

	/*
	Exmaple for choosing track

	void * PlayerPointer = (void*)(*(u32*)0x001eeb70);
	Player * player = (Player*)((u32)PlayerPointer - 0x2FEC);
	PadButtonStatus * pad = playerGetPad(player);
	if ((pad->btns & (PAD_R3 | PAD_SELECT)) == 0 && PLAYING_TRACK == 0)
	{
		PLAYING_TRACK = 1;
		musicPlayTrack(MUSIC_TRACK_GAME_LOBBY, 1);
	}
	else if (!(pad->btns & (PAD_R3 | PAD_SELECT)) == 0)
	{
		PLAYING_TRACK = 0;
	}
	*/
	// If in game
	// if (gameIsIn())
	// {
	// 	int TrackDuration = *(u32*)0x002069A4;
	// 	if (*(u32*)0x002069A0 <= 0)
	// 	{
	// 		/*
	// 			This part: (CurrentTrack != -1 && *(u32*)0x020698C != 0)
	// 			fixes a bug when switching tracks, and running the command
	// 			to set your own track or stop a track.
	// 		*/
	// 		if ((CurrentTrack > DefaultMultiplayerTracks * 2) && (CurrentTrack != -1 && *(u32*)0x020698C != 0) && (TrackDuration <= 0x3000))
	// 		{
	// 			// This techinally cues track 1 (the shortest track) with no sound to play.
	// 			// Doing this lets the current playing track to fade out.
	// 			musicTransitionTrack(0,0,0,0);
	// 		}
	// 	}
	// }
	return 1;
}
