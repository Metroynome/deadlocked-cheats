#include "music.h"

// void internal_musicPlayTrack_inGame(u64, u64, u64);
// void internal_musicPlayTrack_inLobby(u64, u64, u64);
// void internal_musicStopTrack_inGame(void);
// void internal_musicStopTrack_inLobby(void);
// void internal_musicPauseTrack_inGame(u64);
// void internal_musicPauseTrack_inLobby(u64);
// void internal_musicUnpauseTrack_inGame(void);
// void internal_musicUnpauseTrack_inLobby(void);
// void internal_musicTransitionTrack_inGame(u64, u64, u64, u64);
// void internal_musicTransitionTrack_inLobby(u64, u64, u64, u64);

/*
 * Games music volume setting.
 */
// #define MUSIC_VOLUME (*(u32*)0x00171D44)

/*
 * This is what is used to figure out where to load the tracks from.
 */
#define MUSIC_SECTOR (*(u32*)0x001F8584)

/*
 * Defines which track to start on.
 */
#define TRACK_RANGE_MIN (*(u16*)0x002435B8)

/*
 * How many tracks to play, starting at TRACK_RANGE_MIN
 */
#define TRACK_RANGE_MAX (*(u16*)0x002435BC)

#define CURRENT_TRACK (*(u16*)0x00206990)

/*
 * how long the track is.
 * Counts down to 0.  If 0, track is done playing.
 * if 0xBB80, game is loading next track.
 */
#define TRACK_DURATION (*(u32*)0x00225828)

// Grab Gameplay File pointer.  Used for checking which map you are on.
/*
 * NAME :		             musicPlayTrack
 * 
 * DESCRIPTION :
 * 			                 Plays the given Music Track
 * 
 * NOTES :                   In order to play a new track, the current track must finish
 *                           or be stopped with musicStopTrack.
 * 
 * ARGS : 
 *          TrackNumber:     Music track
 *          KeepPlaying:     0 = don't play another track when current is finished.
 *                           1 = play random track when current is finished.
 *          arg3       :     No Idea.  Default is 0x400.
 * 
 * RETURN :
 * 
 * AUTHOR :			         Troy "Agent Moose" Pruitt
 */
void musicPlayTrack(int TrackNumber, int KeepPlaying)
{
    musicStopTrack();
    void * GameplayFilePointer = (void*)(*(u32*)0x01FFFD00);
    int musicPlayTrackAddress = 0;
    switch((u32)GameplayFilePointer)
    {
        // Online Lobby
        case 0x00574F88:
            musicPlayTrackAddress = 0x005BF920; //0x005C0B70 //0x005BF920; // 0x005BFA18 <-- First;
            break;
        // Outpost X12
        case 0x0043A288:
            musicPlayTrackAddress = 0x00489BD8;
            break;
    }
    ((void (*)(u32, u32, u16))musicPlayTrackAddress)(TrackNumber, KeepPlaying, 0x400);
}

/*
 * NAME :		             musicStopTrack
 * 
 * DESCRIPTION :
 * 			                 Stops the current Music Track
 * 
 * NOTES :
 * 
 * ARGS :                    None
 * 
 * RETURN :
 * 
 * AUTHOR :			         Troy "Agent Moose" Pruitt
 */
void musicStopTrack(void)
{
    void * GameplayFilePointer = (void*)(*(u32*)0x01FFFD00);
    int musicStopTrackAddress = 0;
    switch((u32)GameplayFilePointer)
    {
        // Online Lobby
        case 0x00574F88:
            musicStopTrackAddress = 0x005BFDC8;
            break;
        case 0x0043A288:
            musicStopTrackAddress = 0x00489F88;
            break;
    }
    ((void (*)())musicStopTrackAddress)();
}

/*
 * NAME :		             musicPauseTrack
 * 
 * DESCRIPTION :
 * 			                 Pauses the current Music Track
 * 
 * NOTES :
 * 
 * ARGS :                    arg1: No clue.
 * 
 * RETURN :
 * 
 * AUTHOR :			         Troy "Agent Moose" Pruitt
 */
// void musicPauseTrack(int arg1)
// {
//     if (gameIsIn())
//     {
//         internal_musicPauseTrack_inGame(arg1);
//     }
//     else
//     {
//         internal_musicPauseTrack_inLobby(arg1);
//     }
// }

/*
 * NAME :		             musicUnpauseTrack
 * 
 * DESCRIPTION :
 * 			                 Unpauses the currently paused Music Track
 * 
 * NOTES :
 * 
 * ARGS :                    None.
 * 
 * RETURN :
 * 
 * AUTHOR :			         Troy "Agent Moose" Pruitt
 */
// void musicUnpauseTrack(void)
// {
//     if (gameIsIn())
//     {
//         internal_musicUnpauseTrack_inGame();
//     }
//     else
//     {
//         internal_musicUnpauseTrack_inLobby();
//     }
// }

/*
 * NAME :		             musicTransitionTrack
 * 
 * DESCRIPTION :
 * 			                 Transitions to another track.
 * 
 * NOTES :                   This function is better used to fade-out the current track, so it
 *                           can transition to the next track without doing a hacky job of
 *                           manually changing the music volume.
 * 
 * ARGS : 
 *          arg1     :       No Idea.
 *          CudeTrack:       Transition to this track.
 *                           This will cue the track duration and other things.
 *          arg3     :       No Idea, but if set to 0x400, cued track will play.  If set to 0, cued track will will but have no sound.
 *          arg4     :       No Idea, but if set to 0x400, cued track will play.  If set to 0, cued track will will but have no sound.
 * 
 * RETURN :
 * 
 * AUTHOR :			         Troy "Agent Moose" Pruitt
 */
void musicTransitionTrack(short arg1, short CueTrack, short arg3, short arg4)
{
    void * GameplayFilePointer = (void*)(*(u32*)0x01FFFD00);
    int musicTransitionAddress = 0;
    switch((u32)GameplayFilePointer)
    {
        // Online Lobby
        case 0x00574F88:
            musicTransitionAddress = 0x005BFC70;
            break;
        // Outpost X12
        case 0x0043A288:
            musicTransitionAddress = 0x00489E30;
            break;
        // Korgon Outpost
        case 0x00437EC8:
            musicTransitionAddress = 0x00487580;
            break;
        // Metropolis
        case 0x00437208:
            musicTransitionAddress = 0x00486998;
            break;
        // Blackwater City
        case 0x00434988:
            musicTransitionAddress = 0x004841B0;
            break;
        // Command Center
        case 0x004357C8:
            musicTransitionAddress = 0x00484368;
            break;
        // Blackwater Docks
        case 0x00438008:
            musicTransitionAddress = 0x00486BA8;
            break;
        // Aquatos Sewers
        case 0x00437348:
            musicTransitionAddress = 0x00485EE8;
            break;
        // Marcadia Palace
        case 0x00436C88:
            musicTransitionAddress = 0x00485828;
            break;
        // Bakisi Isle
        case 0x00441988:
            musicTransitionAddress = 0x004924C0;
            break;
        // Hoven Gorge
        case 0x00443448:
            musicTransitionAddress = 0x00494518;
            break;
    }
    ((void (*)(u16,u16,u16,u16))musicTransitionAddress)(arg1, CueTrack, arg3, arg4);
}

int musicIsLoaded(void)
{
    if (MUSIC_SECTOR != 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
