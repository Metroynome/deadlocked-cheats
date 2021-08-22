/*
 sp-music-to-mp Code Created by Troy "Agent Moose" Pruitt
 Allows all single player songs to be played in Multiplayer.
*/

#include <tamtypes.h>
#include <libdl/stdio.h>
#include <libdl/game.h>
#include <libdl/player.h>
#include <libdl/pad.h>
#include "music.h"

int Tracks[][2] = {
	// Multiplayer
	{0x102d2e, 0x102f6d}, // Main Lobby
	{0x1031ac, 0x103550}, // Game Lobby
	{0x1038f4, 0x103b96}, // Winning Screen
	{0x103e38, 0x1040a0}, // Loss Screen
	{0x104308, 0x104987}, // 
	{0x105006, 0x10566a}, // 
	{0x105cce, 0x1062d0}, // 
	{0x1068d2, 0x106ecf}, // 
	{0x1074cc, 0x107b73}, // 
	{0x10821a, 0x1087f3}, // Marcadia - Operation Iron Shield
	{0x108dcc, 0x10946e}, // 
	{0x109b10, 0x10a2d3}, // Metropolis
	{0x10aa96, 0x10b158}, // 
	{0x10b81a, 0x10be09}, // 

	// Veldin
	// {0xb1704, 0xb1740}, // Veldin - Kyzil Plateau --Start
	{0xb177c, 0xb1d29}, // Veldin - Kyzil Plateau
	// {0xb22d6, 0xb2394}, // Veldin - F-Sector --Start
	{0xb2452, 0xb2989}, // Veldin - F-Sector

	// Florana
	// {0xb4356, 0xb43b7}, // Florana - Nabla Forest --Start
	{0xb4418, 0xb4a19}, // Florana - Nabla Forest
	// {0xb501a, 0xb5069}, // Florana - Path of Death --Start
	{0xb50b8, 0xb5669}, // Florana - Path of Death
	// {0xb5cdf, 0xb5da4}, // Florana - Florana Tree Beast --Start
	{0xb6369, 0xb692e}, // Florana - Florana Tree Beast

	// Starship Phoenix
	// {0xb69f4, 0xb6a6f}, // Starship Phoenix --Start
	{0xb6aea, 0xb70a5}, // Starship Phoenix
	// {0xb7660, 0xb76b8}, // Starship Phoeix - VR Simulation --Start
	{0xb7710, 0xb7ce0}, // Starship Phoeix - VR Simulation
	// {0xb82b0, 0xb830e}, // Starship Phoeix - VR Gadget Training --Start
	{0xb836c, 0xb893d}, // Starship Phoeix - VR Gadget Training

	// Marcadia
	// {0xbefec, 0xbf044}, // Marcadia - Capital City --Start
	{0xbf09c, 0xbf640}, // Marcadia - Capital City
	// {0xbfbe4, 0xbfc41}, // Marcadia - Laser Defense Facility --Start
	{0xbfc9e, 0xc02bf}, // Marcadia - Laser Defense Facility
	// {0xc08e0, 0xc0932}, // Marcadia - Operation Iron Shield --Start
	// {0xc0984, 0xc0f02}, // Marcadia - Operation Iron Shield

	// Annihilation Nation
	// {0xc7ae4, 0xc7b17}, // Station Q9 - Annihilation Nation --Start
	{0xc7b4a, 0xc7ea5}, // Station Q9 - Annihilation Nation
	// {0xc8200, 0xc8257}, // Station Q9 - Deathcourse --Start
	{0xc82ae, 0xc8891}, // Station Q9 - Deathcourse
	// {0xc8e74, 0xc8ec8}, // Station Q9 - Arena Combat --Start
	{0xc8f1c, 0xc94ad}, // Station Q9 - Arena Combat
	// {0xc9a3e, 0xc9a92}, // Station Q9 - Arena Boss --Start
	{0xc9ae6, 0xca0cb}, // Station Q9 - Arena Boss

	// Aquatos
	// {0xd11bf, 0xd1217}, // Aquatos - Underwater Complex --Start
	{0xd126f, 0xd18c2}, // Aquatos - Underwater Complex
	// ----Missing: Aquatos - Underwater Tyhrranoid Base
	// ----Missing: Aquatos - Sewer

	// Tyhrranosis
	// {0xd277f, 0xd27da}, // Tyhrranosis - Landing Site --Start
	{0xd2835, 0xd2de3}, // Tyhrranosis - Landing Site
	// ----Missing: Tyhrranosis - Korgon Base
	// ----Missing: Tyhrranosis - Control Room
	// ----Missing: Tyhrranosis - Kavu Island

	// Daxx
	// {0xc2dbe, 0xc2e18}, // Daxx - Research Facility --Start
	{0xc2e72, 0xc340a}, // Daxx - Research Facility
	// {0xc46be, 0xc470c}, // Daxx - Docks Area Miniboss --Start
	{0xc475a, 0xc4d1f}, // Daxx - Docks Area Miniboss
	// ----Missing: Daxx - Weapons Facility

	// Obani Gemini
	// {0xd8d87, 0xd8df8}, // Zygan System - Obani Gemini --Start
	{0xd8e69, 0xd9420}, // Zygan System - Obani Gemini

	// Holostar Studios
	// NOTE: Studio 42 doesn't convert with the track converter.  Gotta do it manually.
	// ----Missing: Holostar Studios - Secret Agent Clank
	// ----Missing: Holostar Studios - Giant Clank
	{0xdd0ba, 0xdd112}, // Holostar Studios - Studio 42 --Start
	{0xdd16a, 0xdd73a}, // Holostar Studios - Studio 42
	// ----Missing: Holostar Studios 2 - Studio 41

	// Obani Draco
	// {0xea24b, 0xea29e}, // Zygan System - Obani Draco --Start
	{0xea2f1, 0xea908}, // Zygan System - Obani Draco
	// {0xeaf1f, 0xeaf48}, // Zygan System - Courtney Gears --Start
	{0xeaf71, 0xeb49d}, // Zygan System - Courtney Gears

	// Zeldrin Starport
	{0xd5379, 0xd5956}, // Joraal Nebula - Zeldrin Starport
	// ----Missing: Joraal Nebula - Leviathan
	// ----Missing: Joraal Nebula - Auto-Destruct Sequence

	// Metroplolis
	{0xe09c2, 0xe10c4}, // Kerwan - Metropolis
	// ----Missing: Kerwan - Operation Urban Storm

	// Aridia
	// ----Missing: Aridia - X12 Outpost

	// Qwark's Hideout
	{0xe5896, 0xe5e0f}, // Thran Asteroid - Qwark's Hideout
	// ----Missing: Thran Asteroid - Underground Caverns
	// ----Missing: Unknown 3rd Track

	// Koros
	{0xdf0c6, 0xdf69f}, // Koros - Nefarious BFG
	// ----Missing: Koros - Command Center

	// Mylon - Command Center
	// {0xec566, 0xec591}, // Mylon - Command Center --Start
	{0xec5bc, 0xecbac}, // Mylon - Command Center
	// ----Missing: Mylon - Launch Site

	// Mylon - Biobliterator
	// {0xe9180, 0xe91ce}, // Mylon - Biobliterator --Start
	{0xe921c, 0xe9806}, // Mylon - Biobliterator

	// Insomniac Museum
	// {0xf0c01, 0xf0c59}, // Insomniac Museum --Start
	{0xf0cb1, 0xf1281} // Insomniac Museum
	// {0xf18c5, 0xf1914}, // Online Lobby --Start
	// {0xf1963, 0xf1f14}, // Online Lobby
};

int Map;
short CurrentTrack = 0;
short NextTrack = 0;
int StartSound = 0;

int main(void)
{
	// check to see if multiplayer tracks are loaded
	if (*(u32*)0x001F8584 != 0xABD1)
	{
		// Uses custom sector value.
		*(u32*)0x001F8584 = 0xABD1;
	}

	int DefaultMultiplayerTracks = 0x0d; // This number will never change
	int AddedTracks = (sizeof(Tracks)/sizeof(Tracks[0]));
	int TRACK_RANGE_MIN = *(u32*)0x002435B8;
	int AllTracks = DefaultMultiplayerTracks + AddedTracks;
	// Fun fact: The math for TotalTracks is exactly how the assembly is.  Didn't mean to do it that way.  (Minus the AddedTracks part)
	int TotalTracks = (DefaultMultiplayerTracks - TRACK_RANGE_MIN + 1) + AddedTracks;
	int NextAddress = 0;
	
	// We overwrite all the data from multiplayer tracks due to also changing the track sector.
	// The multiplayer tracks were converted using a custom sector value.
	for(Map = 0; Map < AddedTracks; Map++)
	{
		*(u32*)(0x001F8588 + NextAddress) = Tracks[Map][0];
		*(u32*)(0x001F8590 + NextAddress) = Tracks[Map][1];
		NextAddress += 0x10;
	}

	int MusicDataPointer = *(u32*)0x01FFFD00;
	if (MusicDataPointer != 0x00574F88)
	{
		if(*(u32*)0x002435BC != TotalTracks){
			int TRACK_RANGE_MAX = (u32)MusicDataPointer + 0x1A8;
			*(u16*)TRACK_RANGE_MAX = AllTracks;
		}
	}

	// //Exmaple for choosing track
	// PadButtonStatus * pad = (PadButtonStatus*)0x00225980;
	// // L3: Previous Sound
	// if ((pad->btns & PAD_L3) == 0 && PLAYING_TRACK == 0)
	// {
	// 	// Setting PLAYING_TRACK to 1 will make it so the current playing sound will play once.
	// 	PLAYING_TRACK = 1;
	// 	StartSound -= 0x1; // Subtract 1 from StartSound
	// 	musicPlayTrack(StartSound * 2, 1); // Play Sound
	// 	//printf("Sound Byte: 0x%x\n", StartSound); // print ID of sound played.
	// }
	// // R3: Next Sound
	// if ((pad->btns & PAD_R3) == 0 && PLAYING_TRACK == 0)
	// {
	// 	PLAYING_TRACK = 1;
	// 	StartSound += 0x1;
	// 	musicPlayTrack(StartSound * 2, 1);
	// 	//printf("Sound Byte: 0x%x\n", StartSound);
	// }
	// // Select: Transition Track
	// if ((pad->btns & PAD_SELECT) == 0 && PLAYING_TRACK == 0)
	// {
	// 	PLAYING_TRACK = 1;
	// 	musicTransitionTrack(0,0,0,0);
	// 	//musicPlayTrack(StartSound * 2, 1);
	// 	//printf("Sound Byte: 0x%x\n", StartSound);
	// }
	// // If neither of the above are pressed, PLAYING_TRACK = 0.
	// if (!(pad->btns & PAD_L3) == 0 && !(pad->btns & PAD_R3) == 0 && !(pad->btns & PAD_CIRCLE) == 0)
	// {
	// 	PLAYING_TRACK = 0;
	// }

	// if music !== 0 and TRACK_RANGE_MIN == 4 (TRACK_RANGE_MIN will only equal 4 in game)
	if (musicIsLoaded() && TRACK_RANGE_MIN == 4)
	{
		short Track = *(u16*)0x00225828; // Next Track to play  // In Deadlocked this is the CurrentTrack.  In UYA this is the next playing track.
		short Status = *(u16*)0x0022582E; // 8 = Start of game, 4 = Playing, 5 = Next Qued Track
		short Status2 = *(u16*)0x00225838; // 0x2 = Currently playing, 0x3 = Transitioning track, 0xA = Switching to Next Track.
		// If Status is 8 and both Current Track and Next Track equal zero
		if (Status == 8 && CurrentTrack == 0 && NextTrack == 0)
		{
			// Set CurrentTrack to Track.  This will make it so we know which was is currently playing.
			// The game automatically sets the track variable to the next track to play after the music starts.
			CurrentTrack = Track;
		}
		// If Status is 4 (meaning music starts playing) and only NextTrack is zero.
		else if (Status == 4 && NextTrack == 0)
		{
			// Set NextTrack to Track value.
			NextTrack = Track;
		}
		// If NextTrack does not equal the Track, that means that the song has switched.
		// So we have to move the NextTrack value into the CurrentTrack value, because it is now
		// playing that track.  Then we set the NextTrack to the Track value.
		else if (NextTrack != Track)
		{
			CurrentTrack = NextTrack;
			NextTrack = Track;
		}
		int TrackDuration = *(u32*)0x0022583C;
		// If CurrentTrack is ger than the default Multiplayer tracks
		// and if CurrentTrack does not equal -1
		// and if the track duration is below 0x3000
		// and if Status2 is 2, or Current Playing
		if ((CurrentTrack > DefaultMultiplayerTracks * 2) && CurrentTrack != -1 && (TrackDuration <= 0x3000) && Status2 == 2)
		{
			// This technically cues track 1 (the shortest track) with no sound to play.
			// Doing this lets the current playing track to fade out.
			musicTransitionTrack(0,0,0,0);
		}
	}
	return 1;
}
