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
	{0x4e9dc, 0x4ec1b}, // 0x00: Main Lobby
	{0x4ee5a, 0x4f1fe}, // 0x02: Game Lobby
	{0x4f5a2, 0x4f844}, // 0x04: Winning Screen
	{0x4fae6, 0x4fd4e}, // 0x06: Loss Screen
	{0x4ffb6, 0x50635}, // 0x08: 
	{0x50cb4, 0x51318}, // 0x0A: 
	{0x5197c, 0x51f7e}, // 0x0C: 
	{0x52580, 0x52b7d}, // 0x0E: 
	{0x5317a, 0x53821}, // 0x10: 
	{0x53ec8, 0x544a1}, // 0x12: Marcadia - Operation Iron Shield
	{0x54a7a, 0x5511c}, // 0x14: 
	{0x557be, 0x55f81}, // 0x16: Metropolis
	{0x56744, 0x56e06}, // 0x18: 
	{0x574c8, 0x57ab7}, // 0x1A: 

	// Florana


	// Insomniac Museum
	//{0x3c8af, 0x3c907}, // --Start
	{0x3c95f, 0x3cf2f}, // 0x1C:  Insomniac Museum
	//{0x3d573, 0x3d5c2}, // --Start
	//{0x3d611, 0x3dbc2}, // Online Lobby

	// Starship Phoenix
	//{0x26a2, 0x271d}, // Starship Phoenix --Start
	{0x2798, 0x2d53}, // 0x1E: Starship Phoenix
	//{0x330e, 0x3366}, // Starship Phoeix - VR Simulation --Start
	{0x33be, 0x398e}, // 0x20: Starship Phoeix - VR Simulation
	//{0x3f5e, 0x3fbc}, // Starship Phoeix - VR Gadget Training --Start
	{0x401a, 0x45eb}, // 0x22: Starship Phoeix - VR Gadget Training

	// Marcadia
	//{0xac9a, 0xacf2}, // Marcadia - Capital City --Start
	{0xad4a, 0xb2ee}, // 0x24: Marcadia - Capital City
	//{0xb892, 0xb8ef}, // Marcadia - Laser Defense Facility --Start
	{0xb94c, 0xbf6d}, // 0x26: Marcadia - Laser Defense Facility
	//{0xc58e, 0xc5e0}, // Marcadia - Operation Iron Shield --Start
	// {0xc632, 0xcbb0}, // 0x28: Marcadia - Operation Iron Shield
	
	// Station Q9
	//{0x13792, 0x137c5}, // Station Q9 - Annihilation Nation --Start
	{0x137f8, 0x13b53}, // 0x2A: Station Q9 - Annihilation Nation (Multiplayer Main Menu version 2)
	//{0x13eae, 0x13f05}, // Station Q9 - Deathcourse --Start
	{0x13f5c, 0x1453f}, // 0x2C: Station Q9 - Deathcourse
	//{0x14b22, 0x14b76}, // Station Q9 - Arena Combat --Start
	{0x14bca, 0x1515b}, // 0x2E: Station Q9 - Arena Combat
	//(0x156ec, 0x15740}, // Station Q9 - Arena Boss --Start
	{0x15794, 0x15d79}, // Station Q9 - Arena Boss
	//{0x1635e, 0x16376}, // Growl
	
	// Aquatos
	//{0x1ce6d, 0x1cec5}, // Aquatos - Underwater Complex --Start
	{0x1cf1d, 0x1d570}, // 0x30: Aquatos - Underwater Complex
	// ----Missing: Aquatos - Underwater Tyhrranoid Base
	// ----Missing: Aquatos - Sewer
	
	// Tyrranosis
	//{0x1e42d, 0x1e488}, // Tyhrranosis - Landing Site --Start
	{0x1e4e3, 0x1ea91}, // 0x32: Tyhrranosis - Landing Site
	// ----Missing: Tyhrranosis - Korgon Base
	// ----Missing: Tyhrranosis - Control Room
	// ----Missing: Tyhrranosis - Kavu Island
	
	// Daxx
	//{0xea6c, 0xeac6}, // Daxx - Research Facility --Start
	{0xeb20, 0xf0b8}, // 0x34: Daxx - Research Facility
	//{0x1036c, 0x103ba}, // Daxx - Docks Area Miniboss --Start
	{0x10408, 0x109cd}, // 0x36: Daxx - Docks Area Miniboss
	// ----Missing: Daxx - Weapons Facility
	
	// Obani Gemini
	//{0x24a35, 0x24aa6}, // Zygan System - Obani Gemini --Start
	{0x24b17, 0x250ce}, // 0x38: Zygan System - Obani Gemini
	// ----Missing: Zygan System - Obani Gemini Pollux
	
	// Blackwater City
	//{0x26eeb, 0x26f3f}, // Rilgar - Blackwater City --Start
	{0x26f93, 0x27524}, // 0x3A: Rilgar - Blackwater City
	// Holostar Studios
	// ----Missing: Holostar Studios - Secret Agent Clank
	// ----Missing: Holostar Studios - Giant Clank
	{0x28e18, 0x293e8}, // 0x3C: Holostar Studios - Studio 42
	// ----Missing: Holostar Studios 2 - Studio 41
	
	// Zygan System
	//{0x35ef9, 0x35f4c}, // Zygan System - Obani Draco --Start
	{0x35f9f, 0x365b6}, // 0x3E: Zygan System - Obani Draco
	//{0x36bcd, 0x36bf6}, // Zygan System - Courtney Gears --Start
	{0x36c1f, 0x3714b}, // 0x40: Zygan System - Courtney Gears
	
	// Joraal Nebula
	{0x21027, 0x21604}, // 0x42: Joraal Nebula - Zeldrin Starport
	// ----Missing: Joraal Nebula - Leviathan
	// ----Missing: Joraal Nebula - Auto-Destruct Sequence
	
	// Kerwan
	{0x2c670, 0x2cd72}, // Kerwan - Metropolis --------Duplicate
	// ----Missing: Kerwan - Operation Urban Storm
	// Crash Site
	{0x2d651, 0x2dc16}, // 0x48: Zeldrin - Crash Site --------Duplicate
	
	// Aridia
	// ----Missing: Aridia - X12 Outpost
	
	// Quark's Hideout
	{0x2e301, 0x2e8f0}, // 0x4A: Thran Asteroid - Qwark's Hideout
	// ----Missing: Thran Asteroid - Underground Caverns
	{0x31544, 0x31abd}, // 
	
	// Koros
	{0x2ad74, 0x2b34d}, // 0x4C: Koros - Nefarious BFG
	// ----Missing: Koros - Command Center
	
	// Mylon
	//{0x38214, 0x3823f}, // Mylon - Command Center --Start
	{0x3826a, 0x3885a} // Mylon - Command Center
	// ----Missing: Mylon - Launch Site
	// ----Missing: Mylon - Biobliterator
};

int Active = 0;
int Map;
int PLAYING_TRACK = 0;
int StartSound = 0;
//int TRACK_RANGE_MAX = 0;

int main(void)
{
	// check to see if multiplayer tracks are loaded
	if (*(u32*)0x001F8584 != 0x000BEF23)
	{
		*(u32*)0x001F8584 = 0x000BEF23;
	}

	int DefaultMultiplayerTracks = 0x0d; // This number will never change
	int AddedTracks = (sizeof(Tracks)/sizeof(Tracks[0]));
	int TRACK_RANGE_MIN = *(u32*)0x002435B8;
	int AllTracks = DefaultMultiplayerTracks + AddedTracks;
	// Fun fact: The math for TotalTracks is exactly how the assembly is.  Didn't mean to do it that way.  (Minus the AddedTracks part)
	int TotalTracks = (DefaultMultiplayerTracks - TRACK_RANGE_MIN + 1) + AddedTracks;
	int CurrentTrack = *(u16*)0x00225828;
	int NextAddress = 0;
	for(Map = 0; Map < AddedTracks; Map++)
	{
		*(u32*)(0x001F8588 + NextAddress) = Tracks[Map][0];
		*(u32*)(0x001F8590 + NextAddress) = Tracks[Map][1];
		NextAddress += 0x10;
	}

	void * GameplayPointer = (void*)(*(u32*)0x00240410);
	int MusicDataPointer = *(u32*)((u32)GameplayPointer + 0x8c);
	if (GameplayPointer != 0x014FD000 && *(u32*)GameplayPointer == 0x80)
	{
		if(*(u32*)0x002435BC != TotalTracks){
			int TRACK_RANGE_MAX = (*(u32*)MusicDataPointer) + 0x1A8;
			*(u16*)TRACK_RANGE_MAX = AllTracks;
		}
	}

	//Exmaple for choosing track
	PadButtonStatus * pad = (PadButtonStatus*)0x00225980;
	// L3: Previous Sound
	if ((pad->btns & PAD_L3) == 0 && PLAYING_TRACK == 0)
	{
		// Setting PLAYING_TRACK to 1 will make it so the current playing sound will play once.
		PLAYING_TRACK = 1;
		StartSound -= 0x1; // Subtract 1 from StartSound
		musicPlayTrack(StartSound * 2, 1); // Play Sound
		if (*(u16*)0x0022582E == 0x3) *(u16*)0x0022582E = 0x5;
		//printf("Sound Byte: 0x%x\n", StartSound); // print ID of sound played.
	}
	// R3: Next Sound
	if ((pad->btns & PAD_R3) == 0 && PLAYING_TRACK == 0)
	{
		PLAYING_TRACK = 1;
		StartSound += 0x1;
		musicPlayTrack(StartSound * 2, 1);
		if (*(u16*)0x0022582E == 0x3) *(u16*)0x0022582E = 0x5;
		//printf("Sound Byte: 0x%x\n", StartSound);
	}
	// Select: Replay Sound
	if ((pad->btns & PAD_SELECT) == 0 && PLAYING_TRACK == 0)
	{
		PLAYING_TRACK = 1;
		musicPlayTrack(StartSound * 2, 1);
		if (*(u16*)0x0022582E == 0x3) *(u16*)0x0022582E = 0x5;
		//printf("Sound Byte: 0x%x\n", StartSound);
	}
	// If neither of the above are pressed, PLAYING_TRACK = 0.
	if (!(pad->btns & PAD_L3) == 0 && !(pad->btns & PAD_R3) == 0 && !(pad->btns & PAD_CIRCLE) == 0)
	{
		PLAYING_TRACK = 0;
	}

	// If in game
	if (musicIsLoaded())
	{
		int TrackDuration = *(u32*)0x0022583C;
		if ((CurrentTrack > DefaultMultiplayerTracks * 2) && CurrentTrack != -1 && (TrackDuration <= 0x3000) && *(u16*)0x0022582E == 0x4)
		{
			// This techinally cues track 1 (the shortest track) with no sound to play.
			// Doing this lets the current playing track to fade out.
			musicTransitionTrack(0,0,0,0);
		}
	}
	return 1;
}
