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
{0x101a10, 0x101c4f}, // Main Lobby
{0x101e8e, 0x102232}, // Game Lobby
{0x1025d6, 0x102878}, // Winning Screen
{0x102b1a, 0x102d82}, // Loss Screen
{0x102fea, 0x103669}, // 
{0x103ce8, 0x10434c}, // 
{0x1049b0, 0x104fb2}, // 
{0x1055b4, 0x105bb1}, // 
{0x1061ae, 0x106855}, // 
{0x106efc, 0x1074d5}, // Marcadia - Operation Iron Shield
{0x107aae, 0x108150}, // 
{0x1087f2, 0x108fb5}, // Metropolis
{0x109778, 0x109e3a}, // 
{0x10a4fc, 0x10aaeb} // 
};

int Active = 0;
int Map;
int PLAYING_TRACK = 0;
int StartSound = 0;
//int TRACK_RANGE_MAX = 0;

int main(void)
{
	// check to see if multiplayer tracks are loaded
	if (*(u32*)0x001F8584 != 0xBEEF)
	{
		// Uses custom sector value.
		*(u32*)0x001F8584 = 0xBEEF;
	}

	int DefaultMultiplayerTracks = 0x0d; // This number will never change
	int AddedTracks = (sizeof(Tracks)/sizeof(Tracks[0]));
	int TRACK_RANGE_MIN = *(u32*)0x002435B8;
	int AllTracks = DefaultMultiplayerTracks + AddedTracks;
	// Fun fact: The math for TotalTracks is exactly how the assembly is.  Didn't mean to do it that way.  (Minus the AddedTracks part)
	int TotalTracks = (DefaultMultiplayerTracks - TRACK_RANGE_MIN + 1) + AddedTracks;
	int CurrentTrack = *(u16*)0x00225828;
	int NextAddress = 0;
	
	// We overwrite all the data from multiplayer tracks due to also changing the track sector.
	// The multiplayer tracks were converted over to Florana's sector value.
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
