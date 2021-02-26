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
int HUD = 0;
VECTOR 	CameraPosition,
		PlayerPosition,
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
	// if (pad->ljoy_v != 0x7F || pad->ljoy_h != 0x7F)
	if ((LeftAnalogV || LeftAnalogH) != 0 || (pad->btns != 0xFFFF))
	{
		float hSpeed = LeftAnalogH * MOVE_SPEED;
		float vSpeed = -LeftAnalogV * MOVE_SPEED;
		// float vSpeed = -((float)(pad->ljoy_v - 0x7F) / 128.0) * MOVE_SPEED;
		// float hSpeed = ((float)(pad->ljoy_h - 0x7F) / 128.0) * MOVE_SPEED;

		// Using ternary statements fixes a bug with movement in campaign.
		v[0] = ((pad->btns & PAD_UP) == 0) ? (yCos * MOVE_SPEED)
			: ((pad->btns & PAD_DOWN) == 0) ? (-yCos * MOVE_SPEED)
			: ((pad->btns & PAD_LEFT) == 0) ? (-ySin * MOVE_SPEED)
			: ((pad->btns & PAD_RIGHT) == 0) ? (ySin * MOVE_SPEED)
			: (yCos * vSpeed) + (ySin * hSpeed);
		v[1] = ((pad->btns & PAD_UP) == 0) ? (ySin * MOVE_SPEED)
			: ((pad->btns & PAD_DOWN) == 0) ? (-ySin * MOVE_SPEED)
			: ((pad->btns & PAD_LEFT) == 0) ? (yCos * MOVE_SPEED)
			: ((pad->btns & PAD_RIGHT) == 0) ? (-yCos * MOVE_SPEED)
			: (ySin * vSpeed) + (-yCos * hSpeed);
		v[2] = ((pad->btns & PAD_L2) == 0 && (pad->btns & PAD_R2) != 0) ? (pCos * -MOVE_SPEED)
			: ((pad->btns & PAD_R2) == 0 && (pad->btns & PAD_L2) != 0) ? (pCos * MOVE_SPEED)
			: (pSin * -vSpeed);
		v[3] = 0;
		vector_add(CameraPosition, CameraPosition, v);
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
}

void activate(Player * player, void * PlayerPointer)
{
	// Update stored camera position
	vector_copy(CameraPosition, player->CameraPos);

	// Copy Current Player Position and store it.
	vector_copy(PlayerPosition, player->PlayerPosition);

	// Let Camera go past the death barrier
	//*(u32*)0x005F40DC = 0x10000006;

	// Store Original HUD
	HUD = *(u16*)(PlayerPointer - 0x3d1e8);
	// Hide HUD
	*(u16*)(PlayerPointer - 0x3d1e8) = 0;
	// deactivate hud
	// hud->Healthbar = 0;
	// hud->Minimap = 0;
	// hud->Weapons = 0;
	// hud->Popup = 0;
	// hud->NormalScoreboard = 0;
}

void deactivate(Player * player, void * PlayerPointer)
{
	// Reset Player Position
	float * PlayerCoordinates = (float*) player->UNK24;
	PlayerCoordinates[0] = PlayerPosition[0];
	PlayerCoordinates[1] = PlayerPosition[1];
	PlayerCoordinates[2] = PlayerPosition[2];

	// Set Camera Distance to Default
	player->CameraDistance = -6;

	// Don't let Camera go past death barrier
	//*(u32*)0x005F40DC = 0x10400006;

	// Show HUD
	*(u16*)(PlayerPointer - 0x3d1e8) = HUD;
	// reactivate hud
	// hud->Healthbar = 1;
	// hud->Minimap = 1;
	// hud->Weapons = 1;
	// hud->Popup = 1;
	// hud->NormalScoreboard = 1;
}

int main(void)
{
	//void * PlayerPointer = (void*)(*(u32*)0x001eeb70);
	void * PlayerPointer = *(u32*)0x001eeb70;
	Player * player = (Player*)((u32)PlayerPointer - 0x2FEC);
	PadButtonStatus * pad = playerGetPad(player);
	if (PlayerPointer == 0 || (u32)PlayerPointer == 0x0034aa8c)
	{
		Active = 0;
		return -1;
	}

	// handle activate deactivate
	// Don't activate if player is in Vehicle
	// Activate with L1 + R1 + L3
	if (!Active && !player->Vehicle && (pad->btns & (PAD_L1 | PAD_R1 | PAD_L3)) == 0)
	{
		Active = 1;
		activate(player, PlayerPointer);
	}
	// Deactivate with L1 + R1 + R3
	else if (Active && (pad->btns & (PAD_L1 | PAD_R1 | PAD_R3)) == 0)
	{
		Active = 0;
		deactivate(player, PlayerPointer);
	}
	else if (Active && (pad->btns & PAD_TRIANGLE) == 0)
	{
		Active = 0;
		// Could just do vector_copy, but this is actualy shorter assembly.
		PlayerPosition[0] = CameraPosition[0];
		PlayerPosition[1] = CameraPosition[1];
		PlayerPosition[2] = CameraPosition[2];
		deactivate(player, PlayerPointer);
	}
	
	if (!Active)
		return 0;

	// If start isn't open, let inputs go through.
	if (*(u32*)(PlayerPointer + 0x03B8) == 0) 
	{
		// Handle All Movement Inputs
		MovementInputs(player, pad);
	}

	// Apply Camera Position
	vector_copy(player->CameraPos, CameraPosition);

	// If player isn't dead, move player to X: Zero
	if((player->PlayerState) != 0x99)
	{
		float * PlayerCoordinates = (float*) player->UNK24;
		PlayerCoordinates[0] = 0;
		PlayerCoordinates[1] = PlayerPosition[1];
		PlayerCoordinates[2] = PlayerPosition[2] + 0x00010000;
	}

	// Force Hold Wrench
	player->ChangeWeaponHeldId = 1;

	// Constanty Set Camera Distance to Zero
	player->CameraDistance = 0;

	// fix death camera lock
	player->CameraPitchMin = 1.48353;
	player->CameraPitchMax = -1.22173;

	return 1;
}