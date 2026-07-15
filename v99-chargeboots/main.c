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
#include <libdl/time.h>
#include <libdl/color.h>

/*
 * FUN_005D2930 is the hero post/draw FX function. The chargeboot flame block
 * is entered when hero->state == PLAYER_STATE_CHARGE (0x7d):
 *   0x005D4E48: li v0, 0x7d
 *   0x005D4E4C: bnel v1, v0, 0x005D5184  // skip if not charge state
 *
 * The block draws two boot flames. It uses the joint id table near 0x00220500
 * and PTR_arr_Player_vChargeBootFlameOfs_002204fc, then calls FUN_004CCF98
 * from 0x005D515C to draw each flame quad.
 *
 * Shape constants:
 *   0x005D5044: LUI for length scale, original 2.5 (0x3C014020)
 *   0x005D5050: LUI for width/base, original 0.25 (0x3C013E80)
 *   Width math:  (phase * -0.2 + 0.25) * scale
 *   Length math: phase * 2.5 * scale + 0.5
 *
 * Color constants, format AABBGGRR:
 *   0x005D504C/0x005D5060: color1 LUI/ORI, original 0xA0305070
 *   0x005D5058/0x005D5068: color2 LUI/ORI, original 0x904080FF
 *
 * Other useful flame knobs:
 *   0x005D4F98: li s3, 3         // 4 quads per boot
 *   0x005D5154: li a2, 0x15      // texture id passed to FUN_004CCF98
 *   0x005D515C: jal FUN_004CCF98 // draw flame quad
 */
#define CB_FLAME_COLOR1_LUI_ADDR      (0x005D504C)
#define CB_FLAME_COLOR1_ORI_ADDR      (0x005D5060)
#define CB_FLAME_COLOR2_LUI_ADDR      (0x005D5058)
#define CB_FLAME_COLOR2_ORI_ADDR      (0x005D5068)
#define CB_FLAME_LENGTH_LUI_ADDR      (0x005D5044)
#define CB_FLAME_WIDTH_LUI_ADDR       (0x005D5050)
#define CB_FLAME_COLOR1_ORIGINAL      (0xA0305070)
#define CB_FLAME_COLOR2_ORIGINAL      (0x904080FF)
#define CB_FLAME_LENGTH_ORIGINAL_OP   (0x3C014020)
#define CB_FLAME_WIDTH_ORIGINAL_OP    (0x3C013E80)
#define CB_FLAME_LENGTH_LARGE_OP      (0x3C014070)
#define CB_FLAME_WIDTH_LARGE_OP       (0x3C013EC0)

static u32 dimColor(u32 color, int alpha, int mul, int div)
{
	u32 r = (color & 0xff) * mul / div;
	u32 g = ((color >> 8) & 0xff) * mul / div;
	u32 b = ((color >> 16) & 0xff) * mul / div;

	return (alpha << 24) | (b << 16) | (g << 8) | r;
}

static void writeColorConstant(u32 luiAddr, u32 oriAddr, u32 color, int reg)
{
	u32 hi = (color >> 16) & 0xffff;
	u32 lo = color & 0xffff;

	POKE_U32(luiAddr, 0x3c000000 | (reg << 16) | hi);
	POKE_U32(oriAddr, 0x34000000 | (reg << 21) | (reg << 16) | lo);
}

static void patchFlameShape(int enabled)
{
	POKE_U32(CB_FLAME_LENGTH_LUI_ADDR, enabled ? CB_FLAME_LENGTH_LARGE_OP : CB_FLAME_LENGTH_ORIGINAL_OP);
	POKE_U32(CB_FLAME_WIDTH_LUI_ADDR, enabled ? CB_FLAME_WIDTH_LARGE_OP : CB_FLAME_WIDTH_ORIGINAL_OP);
}

static int isAnyPlayerChargebooting(void)
{
	int i;
	Player **players = playerGetAll();

	for (i = 0; i < GAME_MAX_PLAYERS; ++i) {
		Player *player = players[i];
		if (player && player->pMoby && player->state == PLAYER_STATE_CHARGE)
			return 1;
	}

	return 0;
}

void v99Chargeboots(void)
{
	static int wasChargebooting = 0;

	if (isAnyPlayerChargebooting()) {
		if (!wasChargebooting)
			patchFlameShape(1);

		u32 rgb = colorLevel99(0, 360);
		u32 color1 = dimColor(rgb, 0x70, 3, 4);
		u32 color2 = dimColor(rgb, 0x48, 1, 2);

		writeColorConstant(CB_FLAME_COLOR1_LUI_ADDR, CB_FLAME_COLOR1_ORI_ADDR, color1, 4);
		writeColorConstant(CB_FLAME_COLOR2_LUI_ADDR, CB_FLAME_COLOR2_ORI_ADDR, color2, 5);
		wasChargebooting = 1;
	} else if (wasChargebooting) {
		writeColorConstant(CB_FLAME_COLOR1_LUI_ADDR, CB_FLAME_COLOR1_ORI_ADDR, CB_FLAME_COLOR1_ORIGINAL, 4);
		writeColorConstant(CB_FLAME_COLOR2_LUI_ADDR, CB_FLAME_COLOR2_ORI_ADDR, CB_FLAME_COLOR2_ORIGINAL, 5);
		patchFlameShape(0);
		wasChargebooting = 0;
	}
}

int main(void)
{
	dlPreUpdate();

	if (!isInGame())
		return -1;
	
	v99Chargeboots();

	dlPostUpdate();

	return 0;
}
