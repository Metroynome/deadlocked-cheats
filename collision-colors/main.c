#include <tamtypes.h>
#include <libdl/collision.h>
#include <libdl/game.h>
#include <libdl/graphics.h>
#include <libdl/math3d.h>
#include <libdl/player.h>

#define HEARTBEAT (*(volatile u32*)0x000F4FFC)
#define GAME_ACTIVE (*(volatile int*)0x0021E1EC)
#define SCENE_LOADED (*(volatile int*)0x0022026C)
#define SCAN_GRID 13
#define SCAN_COUNT (SCAN_GRID * SCAN_GRID)
#define HIT_CACHE ((HitSample*)0x000C0000)
#define HIT_CACHE_COUNT SCAN_COUNT
#define SCAN_SPACING 8.0f
#define SAMPLE_Z_ABOVE 18.0f
#define SAMPLE_Z_BELOW 70.0f
#define SAMPLES_PER_TICK 12
#define MAX_MARKER_AGE 240
#define COLOR_HOOK 0x90ff00ff
#define COLOR_GAME 0x9000ffff
#define COLOR_PLAYER 0x90ffff00
#define COLOR_HIT 0x90ffffff
#define COLOR_WATER 0xD00000ff
#define COLOR_WALKABLE 0xD000ff00
#define COLOR_NONWALKABLE 0xD000ffff
#define COLOR_OTHER 0xD0ffffff

typedef struct HitSample {
	VECTOR hit;
	unsigned char valid;
	unsigned char age;
	unsigned char id;
	unsigned char pad0;
} HitSample;

typedef void (*ExecuteDrawFunctions_t)(void **list);
static ExecuteDrawFunctions_t ExecuteDrawFunctions = (ExecuteDrawFunctions_t)0x004C46B0;
static int scanCursor;
static int lastSampleTime = -1;
static int hitFlash;

static Player *getLocalPlayer(void)
{
	Player **players = playerGetAll();
	return players ? players[0] : 0;
}

static void copyVec(VECTOR dst, float *src, float zOfs)
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2] + zOfs;
	dst[3] = 1.0f;
}

static int isWaterSurface(int id)
{
	id &= 0x0F;
	return id == COLLISION_SURFACE_SWIMMABLE_WATER
		|| id == COLLISION_SURFACE_WATER
		|| id == COLLISION_SURFACE_LETHAL_WATER
		|| id == COLLISION_SURFACE_LETHAL_WATER_ICE_CUBE
		|| id == COLLISION_SURFACE_WATER_TRAIL;
}

static u32 colorForSurface(int id)
{
	id &= 0x0F;
	if (isWaterSurface(id)) return COLOR_WATER;
	if (CollisionIsWalkable(id)) return COLOR_WALKABLE;
	if (CollisionIsNonWalkable(id)) return COLOR_NONWALKABLE;
	return COLOR_OTHER;
}

static void getSpiralOffset(int n, int *dx, int *dy)
{
	int ring = 0, span, prev, side, offset;
	while (n >= (2 * ring + 1) * (2 * ring + 1)) ring++;
	if (ring == 0) { *dx = 0; *dy = 0; return; }
	span = 2 * ring + 1;
	prev = (span - 2) * (span - 2);
	side = span - 1;
	offset = n - prev;
	if (offset < side) { *dx = -ring + offset; *dy = -ring; }
	else if (offset < side * 2) { *dx = ring; *dy = -ring + (offset - side); }
	else if (offset < side * 3) { *dx = ring - (offset - side * 2); *dy = ring; }
	else { *dx = -ring; *dy = ring - (offset - side * 3); }
}

static int project(VECTOR pos, float *sx, float *sy)
{
	int x, y;
	if (!gfxWorldSpaceToScreenSpace(pos, &x, &y)) return 0;
	*sx = x / (float)SCREEN_WIDTH;
	*sy = y / (float)SCREEN_HEIGHT;
	return *sx >= -0.05f && *sx <= 1.05f && *sy >= -0.05f && *sy <= 1.05f;
}

static void drawWorldDot(VECTOR pos, u32 color, float size)
{
	float sx, sy;
	if (!project(pos, &sx, &sy)) return;
	gfxScreenSpaceBox(sx - size * 0.5f, sy - size * 0.5f, size, size, color);
}

static void storeHit(int index)
{
	HitSample *s = &HIT_CACHE[index % HIT_CACHE_COUNT];
	copyVec(s->hit, CollLine_Fix_GetHitPosition(), 0.18f);
	s->valid = 1;
	s->age = 0;
	s->id = CollLine_Fix_GetHitCollisionId() & 0x0F;
	hitFlash = 20;
}

static void sampleOne(Player *player)
{
	VECTOR from, to;
	int local, dx, dy;
	local = scanCursor % SCAN_COUNT;
	getSpiralOffset(local, &dx, &dy);
	scanCursor++;
	from[0] = player->pos[0] + dx * SCAN_SPACING;
	from[1] = player->pos[1] + dy * SCAN_SPACING;
	from[2] = player->pos[2] + SAMPLE_Z_ABOVE;
	from[3] = 1.0f;
	to[0] = from[0];
	to[1] = from[1];
	to[2] = player->pos[2] - SAMPLE_Z_BELOW;
	to[3] = 1.0f;
	if (CollLine_Fix(from, to, COLLISION_FLAG_IGNORE_MOBY_SPECIAL_COLLIDERS | COLLISION_FLAG_INCLUDE_BACKFACE, 0, 0)) storeHit(local);
}

static void ageHits(void)
{
	int i;
	for (i = 0; i < HIT_CACHE_COUNT; ++i) {
		if (HIT_CACHE[i].valid && HIT_CACHE[i].age < 255) HIT_CACHE[i].age++;
		if (HIT_CACHE[i].age > MAX_MARKER_AGE) HIT_CACHE[i].valid = 0;
	}
}

static void drawHitsPass(int waterPass)
{
	int i, isWater;
	for (i = 0; i < HIT_CACHE_COUNT; ++i) {
		if (!HIT_CACHE[i].valid) continue;
		isWater = isWaterSurface(HIT_CACHE[i].id);
		if (!!waterPass != !!isWater) continue;
		drawWorldDot(HIT_CACHE[i].hit, colorForSurface(HIT_CACHE[i].id), isWater ? 0.036f : 0.030f);
	}
}

static void drawHits(void)
{
	drawHitsPass(0);
	drawHitsPass(1);
}

int main(void **drawList)
{
	Player *player;
	int now, i;
	HEARTBEAT = 0xC011D100;
	ExecuteDrawFunctions(drawList);
	gfxScreenSpaceBox(0.018f, 0.36f, 0.025f, 0.030f, COLOR_HOOK);
	if (GAME_ACTIVE && SCENE_LOADED == 1) {
		gfxScreenSpaceBox(0.050f, 0.36f, 0.025f, 0.030f, COLOR_GAME);
		player = getLocalPlayer();
		if (player) {
			gfxScreenSpaceBox(0.082f, 0.36f, 0.025f, 0.030f, COLOR_PLAYER);
			if (hitFlash) gfxScreenSpaceBox(0.114f, 0.36f, 0.025f, 0.030f, COLOR_HIT);
			now = gameGetTime();
			if (now - lastSampleTime) {
				lastSampleTime = now;
				ageHits();
				if (hitFlash) hitFlash--;
				for (i = 0; i < SAMPLES_PER_TICK; ++i) sampleOne(player);
			}
			drawHits();
		}
	}
	HEARTBEAT = 0xC011D1FF;
	return 0;
}
