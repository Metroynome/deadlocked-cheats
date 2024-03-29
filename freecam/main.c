/*
 Free Cam Code Created by Troy "Agent Moose" Pruitt
*/

#include <tamtypes.h>
#include <libdl/stdio.h>
#include <libdl/game.h>
#include <libdl/player.h>
#include <libdl/pad.h>
#include <libdl/math.h>
#include <libdl/math3d.h>
#include <libdl/hud.h>

int Active = 0;
int ToggleScoreboard = 0;
int ToggleRenderAll = 0;
VECTOR CameraPosition,
		PlayerBackup,
		targetPos,
		cameraPos,
		delta;

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
	if ((LeftAnalogV || LeftAnalogH) != 0)
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
	// Hold Circle: Lock Camera
	if ((pad->btns & PAD_CIRCLE) == 0)
	{
		vector_copy(delta, CameraPosition);
		delta[2] += 1;
		vector_subtract(delta, targetPos, delta);
		float len = vector_length(delta);
		float targetYaw = atan2f(delta[1] / len, delta[0] / len);
		float targetPitch = asinf(-delta[2] / len);
		player->CameraPitch.Value = targetPitch;
		player->CameraYaw.Value = targetYaw;
	}
	
	// Add Vector to Camera Position
	vector_add(CameraPosition, CameraPosition, v);
}

void activate(Player * player, PlayerHUDFlags * hud)
{
	// Update stored camera position
	vector_copy(CameraPosition, player->CameraPos);

	// Copy Current Player Position and store it.
	vector_copy(PlayerBackup, player->PlayerPosition);

	// Let Camera go past the death barrier
	*(u32*)0x005F40DC = 0x10000006;

	// Set Respawn Timer to Zero, then negative so player can't respawn
	player->timers.resurrectWait = -1;

	// deactivate hud
	hud->Flags.Healthbar = 0;
	hud->Flags.Minimap = 0;
	hud->Flags.Weapons = 0;
	hud->Flags.Popup = 0;
	hud->Flags.NormalScoreboard = 0;
}

void deactivate(Player * player, PlayerHUDFlags * hud)
{
	// Reset Player Position
	float * PlayerCoordinates = (float*)0x0034a9a4;
	PlayerCoordinates[0] = PlayerBackup[0];
	PlayerCoordinates[1] = PlayerBackup[1];
	PlayerCoordinates[2] = PlayerBackup[2];

	// Set Camera Distance to Default
	player->CameraOffset[0] = -6;

	// Don't let Camera go past death barrier
	*(u32*)0x005F40DC = 0x10400006;

	// Reset Respawn timer
	player->timers.resurrectWait = 0;

	// Reset Occlusion
	if (*(u32*)0x00222648 == 0)
		*(u32*)0x00222648 = 2;

	// reactivate hud
	hud->Flags.Healthbar = 1;
	hud->Flags.Minimap = 1;
	hud->Flags.Weapons = 1;
	hud->Flags.Popup = 1;
	hud->Flags.NormalScoreboard = 1;
}

int main(void)
{
	// ensure we're in game
	if (!isInGame())
	{
		Active = 0;
		return -1;
	}

	// Get Local Player
	Player * player = (Player*)0x00347aa0;
	PadButtonStatus * pad = playerGetPad(player);
	PlayerHUDFlags * hud = hudGetPlayerFlags(0);

	if (!Active)
	{
		// Don't activate if player is in Vehicle
		// Activate with L1 + R1 + L3
		if (!player->Vehicle && (pad->btns & (PAD_L1 | PAD_R1 | PAD_L3)) == 0)
		{
			Active = 1;
			activate(player, hud);
		}
	}
	else if (Active)
	{
		// Deactivate with L1 + R1 + R3
		if ((pad->btns & (PAD_L1 | PAD_R1 | PAD_R3)) == 0)
		{
			Active = 0;
			deactivate(player, hud);
		}
	}
	
	if (!Active)
		return 0;

	// If start isn't open, let inputs go through.
	if ((*(u32*)0x00347E58) == 0)
	{
		// Select: Toggle Score
		if ((pad->btns & PAD_SELECT) == 0 && ToggleScoreboard == 0)
		{
			ToggleScoreboard = 1;
			hud->Flags.NormalScoreboard = !hud->Flags.NormalScoreboard;
		}
		else if (!(pad->btns & PAD_SELECT) == 0)
		{
			ToggleScoreboard = 0;
		}
		// Square: Toggle Render All
		if ((pad->btns & PAD_SQUARE) == 0 && ToggleRenderAll == 0)
		{
			ToggleRenderAll = 1;
			
			// If Occlusion equals 2 (On) change it to zero (Off)
			if (*(u32*)0x00222648 == 2)
			{
				*(u32*)0x00222648 = 0;
			}
			else
			{
				*(u32*)0x00222648 = 2;
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

	// If player isn't dead, move player to X: Zero
	if ((player->PlayerState) != 0x99)
	{
		float * PlayerCoordinates = (float*)0x0034a9a4;
		PlayerCoordinates[0] = 0;
		PlayerCoordinates[1] = PlayerBackup[1];
		PlayerCoordinates[2] = PlayerBackup[2] + 0x00100000;
	}

	// Force Hold Wrench
	player->ChangeWeaponHeldId = 1;

	// Fix Void fall bug.  This only needs to load if fallen in the void.
	// Running any other time will cause the player to keep getting deaths if not in void.
	if ((*(u8*)0x0034A078) == 0x76)
		player->UNK19[4] = 0;

	// Constanty Set Camera Distance to Zero
	player->CameraOffset[0] = 0;

	// fix death camera lock
	player->CameraPitchMin = 1.48353; // Testing: 1.57077;
	player->CameraPitchMax = -1.22173;

	return 1;
}
