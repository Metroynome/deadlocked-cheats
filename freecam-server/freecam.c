/***************************************************
 * FILENAME :		freecam.c
 * 
 * DESCRIPTION :
 * 		Free Cam entry point and logic.
 * 
 * NOTES :
 * 		Each offset is determined per app id.
 * 		This is to ensure compatibility between versions of Deadlocked/Gladiator.
 * 		
 * AUTHOR :			Troy "Agent Moose" Pruitt
 */

#include <tamtypes.h>
#include <libdl/stdio.h>
#include <libdl/game.h>
#include <libdl/player.h>
#include <libdl/pad.h>
#include <libdl/math.h>
#include <libdl/math3d.h>
#include <libdl/hud.h>
#include <libdl/ui.h>

int Active = 0; // Is Code Turned on
int ActiveTriangle = 0; // Is Triangle Held
int ToggleHUD = 0; // Is Select Held
int ShowEnterFreeCam = 0; // Is Enter Free Cam Message showing
int ShowControlsPopup = 0; // Is Controls Message Showing
int ToggleRenderAll = 0; // Is Render All on or off?
VECTOR CameraPosition,
		targetPos,
		cameraPos,
		delta;
char RenderAllData[0x280];


void MovementInputs(Player * player, PadButtonStatus * pad)
{
	VECTOR v;

	// get rotation from yaw and pitch
	float ySin = sinf(player->CameraYaw.Value);
	float yCos = cosf(player->CameraYaw.Value);
	float pSin = sinf(player->CameraPitch.Value);
	float pCos = cosf(player->CameraPitch.Value);

	// Handle Speed
	// Default Speed
	float MOVE_SPEED = 0.5;
	// L1: Fast Speed
	if ((pad->btns & PAD_L1) == 0 && (pad->btns & PAD_R1) != 0)
	{
		MOVE_SPEED = 2.0;
	}
	// R1: Slow Speed
	if ((pad->btns & PAD_R1) == 0 && (pad->btns & PAD_L1) != 0)
	{
		MOVE_SPEED = 0.12;
	}

	// Left Analog
	// Swapped library pad->ljoy with another line.
	// It adds more error for drifting analog sticks.
	float LeftAnalogH = (*(float*)0x001ee708);
	float LeftAnalogV = (*(float*)0x001ee70c);
	if((LeftAnalogV || LeftAnalogH) != 0)
	{
		float hSpeed = LeftAnalogH * MOVE_SPEED;
		float vSpeed = -LeftAnalogV * MOVE_SPEED;

		// generate vertical and horizontal vectors
		v[0] = (yCos * vSpeed) + (ySin * hSpeed);
		v[1] = (ySin * vSpeed) + (-yCos * hSpeed);
		v[2] = (pSin * -vSpeed);
		v[3] = 0;
	}
	// D-Pad Up: Forward
	if ((pad->btns & PAD_UP) == 0)
	{
		v[0] = (yCos * MOVE_SPEED);
		v[1] = (ySin * MOVE_SPEED);
	}
	// D-Pad Down: Backward
	if ((pad->btns & PAD_DOWN) == 0)
	{
		v[0] = (-yCos * MOVE_SPEED);
		v[1] = (-ySin * MOVE_SPEED);
	}
	// D-Pad Left: Strafe Left
	if ((pad->btns & PAD_LEFT) == 0)
	{
		v[0] = (-ySin * MOVE_SPEED);
		v[1] = (yCos * MOVE_SPEED);
	}
	// D-Pad Right: Strafe Right
	if ((pad->btns & PAD_RIGHT) == 0)
	{
		v[0] = (ySin * MOVE_SPEED);
		v[1] = (-yCos * MOVE_SPEED);
	}
	// L2 = Move Down
	if ((pad->btns & PAD_L2) == 0 && (pad->btns & PAD_R2) != 0)
	{
		//v[2] = (pCos * -MOVE_SPEED);
		v[2] -= MOVE_SPEED;
	}
	// R2 = Move Up
	if ((pad->btns & PAD_R2) == 0 && (pad->btns & PAD_L2) != 0)
	{
		//v[2] = (pCos * MOVE_SPEED);
		v[2] += MOVE_SPEED;
	}
	// R3: Select target for lock rotation
	if ((pad->btns & PAD_R3) == 0)
	{
		vector_copy(targetPos, CameraPosition);
	}
	// Hold Circle: lock camera
	if ((pad->btns & PAD_CIRCLE) == 0)
	{
		vector_copy(cameraPos, CameraPosition);
		vector_subtract(delta, targetPos, cameraPos);
		vector_normalize(delta, delta);
		float pitch = asinf(-delta[2]);
		float yaw = atan2f(delta[1], delta[0]);

		player->CameraPitch.Value = pitch;
		player->CameraYaw.Value = yaw;
	}

	// Add Vector to Camera Position
	vector_add(CameraPosition, CameraPosition, v);
}

void activate(Player * player, PlayerHUDFlags * hud)
{
	// Update stored camera position
	vector_copy(CameraPosition, player->CameraPos);

	// Let Camera go past the death barrier
	*(u32*)0x005F40DC = 0x10000006;

	// deactivate hud
	hud->Flags.Flags.Healthbar = 0;
	hud->Flags.Flags.Minimap = 0;
	hud->Flags.Flags.Weapons = 0;
	hud->Flags.Flags.Popup = 0;
	hud->Flags.Flags.NormalScoreboard = 0;
}

void deactivate(Player * player, PlayerHUDFlags * hud)
{
	// Set Camera Distance to Default
	player->CameraDistance = -6;

	// Don't let Camera go past death barrier
	*(u32*)0x005F40DC = 0x10400006;

	// Reset Render Data/Function
	if (*(u32*)0x004D7168 == 0x03e00008)
	{
		*(u32*)0x004C0760 = 0x0C135C40;
		*(u32*)0x004C0878 = 0x0C135BD8;
		*(u32*)0x004C09E0 = 0x0C135C40;
		*(u32*)0x004C0A50 = 0x0C135C40;
		*(u32*)0x004D7168 = 0x78A20000;
		*(u32*)0x004D716C = 0x20A50010;
		// If off, grab RenderAllData and set it to render data.
		memcpy((u8*)0x00240A40, RenderAllData, 0x280);
	}
	RenderAllData[0x280] = 0;

	// reactivate hud
	hud->Flags.Flags.Healthbar = 1;
	hud->Flags.Flags.Minimap = 1;
	hud->Flags.Flags.Weapons = 1;
	hud->Flags.Flags.Popup = 1;
	hud->Flags.Flags.NormalScoreboard = 1;
}

int main(void)
{
	// ensure we're in game
	if (!gameIsIn())
	{
		Active = 0;
		return -1;
	}

	// Get Local Player
	Player * player = (Player*)0x00347aa0;
	PadButtonStatus * pad = playerGetPad(player);
	PlayerHUDFlags * hud = hudGetPlayerFlags(0);

	// Check to see if player is dead
	if ((player->PlayerState) == 0x99)
	{
		//Show Enter Free Cam message if not active
		if (!ShowEnterFreeCam && !Active)
		{
			ShowEnterFreeCam = 1;
			uiShowHelpPopup(player->LocalPlayerIndex, "Press \x13 to enter Spectate mode.  Press \x12 to enter Free Cam", 6);
		}
		// Handle Activation/Deactivation
		if ((pad->btns & PAD_TRIANGLE) == 0 && ActiveTriangle == 0)
		{
			ActiveTriangle = 1;
			if(!Active)
			{
				Active = 1;
				activate(player, hud);
			}
			else if (Active)
			{
				Active = 0;
				deactivate(player, hud);
			}
		}
		else if ((pad->btns & PAD_TRIANGLE) != 0)
		{
			ActiveTriangle = 0;
		}
		// If Square is pressed, deactivate.  Pressing Square activates Spectate Mode.
		if ((pad->btns & PAD_SQUARE) == 0)
		{
			Active = 0;
			deactivate(player, hud);
		}
	}
	// if Player respspspawns, deactivate
	else if ((player->PlayerState) != 0x99 && Active)
	{
		Active = 0;
		deactivate(player, hud);
	}
	
	// If Not active, don't run anything past this point.
	if (!Active)
		return 0;

	// Show Controls Message if active and hasn't shown already.
	if (!ShowControlsPopup)
	{
		ShowControlsPopup = 1;
		uiShowHelpPopup(player->LocalPlayerIndex, "Controls: Move: \x18 Speed: \x14 (Fast)/\x15 (Slow) Height: \x16 (Down)/\x17 (Up) Toggle HUD: \x1E", 7);
	}

	// If Start isn't open, let inputs go through
	if ((*(u32*)0x00347E58) == 0)
	{
		// Select: Toggle HUD
		if ((pad->btns & PAD_SELECT) == 0 && ToggleHUD == 0)
		{
			ToggleHUD = 1;
			// if Popup and Scoreboard are showing
			if (hud->Flags.Popup & hud->Flags.NormalScoreboard)
			{
				// Hide Popup
				hud->Flags.Popup = !hud->Flags.Popup;
			}
			// if Popup and Scoreboard are hidden
			else if (!hud->Flags.Popup & !hud->Flags.NormalScoreboard)
			{
				// Show Popup
				hud->Flags.Popup = !hud->Flags.Popup;
				// Show Scoreboard
				hud->Flags.NormalScoreboard = !hud->Flags.NormalScoreboard;
			}
			// if Popup is hidden and Scoreboard is showing
			else if (!hud->Flags.Popup & hud->Flags.NormalScoreboard)
			{
				// Hide Scoreboard
				hud->Flags.NormalScoreboard = !hud->Flags.NormalScoreboard;
			}
		}
		// if Select is no longer held
		else if ((pad->btns & PAD_SELECT) != 0)
		{
			ToggleHUD = 0;
		}
		// Square: Toggle Render All
		if ((pad->btns & PAD_SQUARE) == 0 && ToggleRenderAll == 0)
		{
			ToggleRenderAll = 1;
			
			// if render function has not been modified, then do so.
			if (*(u32*)0x004D7168 != 0x03e00008)
			{
				// Copy Render Data and save it.
				memcpy(RenderAllData, (u8*)0x00240A40, 0x280);

				// Turn off render functions
				*(u32*)0x004C0760 = 0x00000000;
				*(u32*)0x004C0878 = 0x00000000;
				*(u32*)0x004C09E0 = 0x00000000;
				*(u32*)0x004C0A50 = 0x00000000;
				*(u32*)0x004D7168 = 0x03e00008;
				*(u32*)0x004D716C = 0x00000000;

				// Set render data to -1.
				memset((u8*)0x00240A40, 0xff, 0x280);
			}
			else
			{
				// If Off, turn functions back to normal.
				*(u32*)0x004C0760 = 0x0C135C40;
				*(u32*)0x004C0878 = 0x0C135BD8;
				*(u32*)0x004C09E0 = 0x0C135C40;
				*(u32*)0x004C0A50 = 0x0C135C40;
				*(u32*)0x004D7168 = 0x78A20000;
				*(u32*)0x004D716C = 0x20A50010;

				// If off, grab RenderAllData and set it to render data.
				memcpy((u8*)0x00240A40, RenderAllData, 0x280);
			}
		}
		else if (!(pad->btns & PAD_SQUARE) == 0)
		{
			ToggleRenderAll = 0;
		}
		// Handle All Movement Inputs
		MovementInputs(player, pad);
	}
	// Apply Camera Position
	vector_copy(player->CameraPos, CameraPosition);

	// Fix Void fall bug.  This only needs to load if fallen in the void.
	// Running any other time will cause the player to keep getting deaths if not in void.
	if ((*(u8*)0x0034A078) == 0x76)
		player->UNK19[4] = 0;

	// Constanty Set Camera Distance to Zero
	player->CameraDistance = 0;

	// fix death camera lock
	player->CameraPitchMin = 1.48353;
	player->CameraPitchMax = -1.22173;

	return 1;
}
