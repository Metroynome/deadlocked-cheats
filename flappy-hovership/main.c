#include <tamtypes.h>
#include <libdl/dl.h>
#include <libdl/player.h>
#include <libdl/pad.h>
#include <libdl/stdio.h>
#include <libdl/game.h>
#include <libdl/moby.h>
#include <libdl/utils.h>
#include <libdl/math.h>
#include <libdl/math3d.h>
#include <libdl/vehicle.h>

void doTheFlapFlap(void)
{
	// use x or not to flap!
	int Manual = 0;

	static float negativef = -0.50;
	static float positivef = 0.70;
	static float rotModifier = 0.06;
	static int state = 0;
	static float lRot = 0;
	static float rRot = 0;

	int i;
	Player ** players = playerGetAll();
	for (i = 0; i < GAME_MAX_PLAYERS; ++i) {
		Player * player = players[i];
		if (!player)
			return;
		
		u32 vehicle = *(u32*)((u32)player + 0x2ef4);
		if (!vehicle)
			return;

		Moby * moby = *(u32*)((u32)vehicle + 0x2b0);
		u32 wing1 = ((u32)moby->PVar + 0x3f0);
		u32 wing2 = ((u32)moby->PVar + 0x430);

		float left_wing = *(float*)wing1;
		float right_wing = *(float*)wing2;

		if (!Manual) {
			if (state == 0) {
				if (left_wing < positivef) {
					lRot = left_wing + rotModifier;
					rRot = right_wing - rotModifier;
				} else {
					state = 1;
				}
			}
			if (state == 1) {
				if (left_wing > negativef) {
					lRot = left_wing - rotModifier;
					rRot = right_wing + rotModifier;
				} else {
					state = 2;
				}
			}
			if (state == 2) {
				if (left_wing < 0) {
					lRot = left_wing + rotModifier;
					rRot = right_wing - rotModifier;
				} else {
					state = 0;
				}
			}
		} else {
			// PadButtonStatus * pad = (PadButtonStatus*)player->Paddata;
			if (state == 0) {
				if (playerPadGetButtonDown(player, PAD_CROSS) > 0) {
					if (left_wing > positivef) {
						lRot = left_wing + rotModifier;
						rRot = right_wing - rotModifier;
					}
				} else if (playerPadGetButtonUp(player, PAD_CROSS) > 0) {
					state = 1;
				}
			}
			if (state == 1) {
				if (left_wing < 0) {
					lRot = left_wing - rotModifier;
					rRot = right_wing + rotModifier;
				} else {
					state = 0;
				}
			}
		}
		
		*(float*)wing1 = lRot;
		*(float*)wing2 = rRot;
	}
}

void hookTheFlapFlap(void)
{
	// Hook into hovership animation
	*(u32*)0x00471af8 = 0;
	*(u32*)0x00471afc = 0;

	doTheFlapFlap();
}


int main(void)
{
	dlPreUpdate();

	if (!isInGame())
		return -1;
	
	hookTheFlapFlap();

	dlPostUpdate();

	return 0;
}
