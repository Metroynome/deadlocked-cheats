#include <tamtypes.h>
#include <libdl/stdio.h>
#include <libdl/game.h>
#include <libdl/player.h>
#include <libdl/moby.h>
#include <libdl/utils.h>
#include <libdl/math.h>
#include <libdl/math3d.h>
#include <libdl/vehicle.h>

void doTheFlapFlap(a0, a1)
{
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

		float negativef = -0.75;
		float positivef = 0.75;
		float rotModifier = 0.05;
		static int state = 0;
		float left = 0;
		float right = 0;

		float left_wing_start = *(float*)wing1;
		float right_wing_start = *(float*)wing2;
		if (state == 0) {
			if (left_wing_start < positivef){
				left = left_wing_start + rotModifier;
				right = right_wing_start - rotModifier;
			} else {
				state = 1;
			}
		}
		if (state == 1){
			if (left_wing_start > negativef){
				left = left_wing_start - rotModifier;
				right = right_wing_start + rotModifier;
			} else {
				state = 2;
			}
		}
		if (state == 2){
			if (left_wing_start < 0){
				left = left_wing_start + rotModifier;
				right = right_wing_start - rotModifier;
			} else {
				state = 0;
			}
		}
		
		*(float*)wing1 = left;
		*(float*)wing2 = right;
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
	if (!isInGame())
		return -1;
	
	hookTheFlapFlap();

	return 0;
}
