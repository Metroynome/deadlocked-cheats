#include <tamtypes.h>
#include <libdl/collision.h>
#include <libdl/game.h>
#include <libdl/graphics.h>
#include <libdl/math3d.h>
#include <libdl/player.h>

#define HEARTBEAT (*(volatile u32*)0x000F4FFC)
#define GAME_ACTIVE (*(volatile int*)0x0021E1EC)
#define SCENE_LOADED (*(volatile int*)0x0022026C)
#define CACHE_MAGIC 0xC011D6C1
#define HIT_CACHE ((HitCache*)0x000C0000)
#define HIT_CACHE_COUNT 384
#define SCAN_GRID 17
#define SCAN_COUNT (SCAN_GRID * SCAN_GRID)
#define SCAN_SPACING 8.0f
#define CELL_SIZE 8.0f
#define SAMPLE_Z_ABOVE 18.0f
#define SAMPLE_Z_BELOW 90.0f
#define SAMPLES_PER_TICK 24
#define MAX_MARKER_AGE 255
#define COLOR_HOOK 0x80ff00ff
#define COLOR_GAME 0x8000ffff
#define COLOR_PLAYER 0x80ffff00
#define COLOR_HIT 0x80ffffff
#define COLOR_SHADOW 0x80000000
#define COLOR_WATER 0x800000ff
#define COLOR_WALKABLE 0x8000ff00
#define COLOR_NONWALKABLE 0x8000ffff
#define COLOR_OTHER 0x80ffffff

typedef struct HitSample {
	VECTOR hit;
	u32 key;
	unsigned char valid;
	unsigned char age;
	unsigned char id;
	unsigned char priority;
} HitSample;

typedef struct HitCache {
	u32 magic;
	u32 sceneLoaded;
	HitSample samples[HIT_CACHE_COUNT];
} HitCache;

typedef void (*ExecuteDrawFunctions_t)(void **list);
static ExecuteDrawFunctions_t ExecuteDrawFunctions = (ExecuteDrawFunctions_t)0x004C46B0;
static int scanCursor;
static int replaceCursor;
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

static int fastFloor(float v)
{
	int i = (int)v;
	return (v < (float)i) ? i - 1 : i;
}

static u32 cellKey(float x, float y)
{
	int ix = fastFloor(x / CELL_SIZE);
	int iy = fastFloor(y / CELL_SIZE);
	return (((u32)(ix & 0xffff)) << 16) | ((u32)(iy & 0xffff));
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

static unsigned char priorityForSurface(int id)
{
	id &= 0x0F;
	if (isWaterSurface(id)) return 3;
	if (CollisionIsNonWalkable(id)) return 2;
	if (CollisionIsWalkable(id)) return 1;
	return 0;
}

static u32 colorForSurface(int id)
{
	id &= 0x0F;
	if (isWaterSurface(id)) return COLOR_WATER;
	if (CollisionIsWalkable(id)) return COLOR_WALKABLE;
	if (CollisionIsNonWalkable(id)) return COLOR_NONWALKABLE;
	return COLOR_OTHER;
}

static void initCache(void)
{
	int i;
	if (HIT_CACHE->magic == CACHE_MAGIC && HIT_CACHE->sceneLoaded == (u32)SCENE_LOADED) return;
	HIT_CACHE->magic = CACHE_MAGIC;
	HIT_CACHE->sceneLoaded = SCENE_LOADED;
	for (i = 0; i < HIT_CACHE_COUNT; ++i) HIT_CACHE->samples[i].valid = 0;
	replaceCursor = 0;
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
	return *sx >= -0.10f && *sx <= 1.10f && *sy >= -0.10f && *sy <= 1.10f;
}

static void drawScreenSquare(float sx, float sy, float size, u32 color)
{
	gfxScreenSpaceBox(sx - size * 0.5f, sy - size * 0.5f, size, size, color);
}

static void drawWorldMarker(VECTOR pos, u32 color, float size)
{
	float sx, sy;
	if (!project(pos, &sx, &sy)) return;
	drawScreenSquare(sx, sy, size * 1.30f, COLOR_SHADOW);
	drawScreenSquare(sx, sy, size, color);
}

static HitSample *findSlot(u32 key)
{
	int i;
	HitSample *s;
	for (i = 0; i < HIT_CACHE_COUNT; ++i) {
		s = &HIT_CACHE->samples[i];
		if (s->valid && s->key == key) return s;
	}
	for (i = 0; i < HIT_CACHE_COUNT; ++i) {
		s = &HIT_CACHE->samples[i];
		if (!s->valid) return s;
	}
	s = &HIT_CACHE->samples[replaceCursor % HIT_CACHE_COUNT];
	replaceCursor++;
	return s;
}

static void storeHit(u32 key)
{
	int id = CollLine_Fix_GetHitCollisionId() & 0x0F;
	unsigned char priority = priorityForSurface(id);
	HitSample *s = findSlot(key);
	int wasValid = s->valid && s->key == key;

	if (wasValid) {
		if (s->priority == 3) {
			s->age = 0;
			return;
		}
		if (priority < s->priority) {
			s->age = 0;
			return;
		}
		if (priority == s->priority) id = s->id;
	}

	copyVec(s->hit, CollLine_Fix_GetHitPosition(), 0.28f);
	s->key = key;
	s->valid = 1;
	s->age = 0;
	s->id = id;
	s->priority = priority;
	hitFlash = 20;
}

static void sampleOne(Player *player)
{
	VECTOR from, to;
	int local, dx, dy;
	u32 key;
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
	key = cellKey(from[0], from[1]);
	if (CollLine_Fix(from, to, COLLISION_FLAG_IGNORE_MOBY_SPECIAL_COLLIDERS | COLLISION_FLAG_INCLUDE_BACKFACE, 0, 0)) storeHit(key);
}

static void ageHits(void)
{
	int i;
	HitSample *s;
	for (i = 0; i < HIT_CACHE_COUNT; ++i) {
		s = &HIT_CACHE->samples[i];
		if (s->valid && s->age < 255) s->age++;
		if (s->age > MAX_MARKER_AGE) s->valid = 0;
	}
}

static void updateSamples(Player *player)
{
	int now, i;
	now = gameGetTime();
	if (now == lastSampleTime) return;
	lastSampleTime = now;
	ageHits();
	if (hitFlash) hitFlash--;
	for (i = 0; i < SAMPLES_PER_TICK; ++i) sampleOne(player);
}

static void drawHitsPass(int priority)
{
	int i;
	HitSample *s;
	float size;
	for (i = 0; i < HIT_CACHE_COUNT; ++i) {
		s = &HIT_CACHE->samples[i];
		if (!s->valid || s->priority != priority) continue;
		size = priority == 3 ? 0.045f : (priority == 2 ? 0.038f : 0.032f);
		drawWorldMarker(s->hit, colorForSurface(s->id), size);
	}
}

static void drawHits(void)
{
	drawHitsPass(0);
	drawHitsPass(1);
	drawHitsPass(2);
	drawHitsPass(3);
}

int main(void **drawList)
{
	Player *player = 0;
	HEARTBEAT = 0xC011D600;
	if (GAME_ACTIVE && SCENE_LOADED == 1) {
		initCache();
		player = getLocalPlayer();
		if (player) {
			updateSamples(player);
			drawHits();
		}
	}
	ExecuteDrawFunctions(drawList);
	gfxScreenSpaceBox(0.018f, 0.36f, 0.025f, 0.030f, COLOR_HOOK);
	if (GAME_ACTIVE && SCENE_LOADED == 1) {
		gfxScreenSpaceBox(0.050f, 0.36f, 0.025f, 0.030f, COLOR_GAME);
		if (player) {
			gfxScreenSpaceBox(0.082f, 0.36f, 0.025f, 0.030f, COLOR_PLAYER);
			if (hitFlash) gfxScreenSpaceBox(0.114f, 0.36f, 0.025f, 0.030f, COLOR_HIT);
		}
	}
	HEARTBEAT = 0xC011D6FF;
	return 0;
}
