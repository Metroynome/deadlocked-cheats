#include <tamtypes.h>
#include <libdl/collision.h>
#include <libdl/game.h>
#include <libdl/graphics.h>
#include <libdl/moby.h>
#include <libdl/player.h>
#include <libdl/utils.h>

#define HEARTBEAT (*(volatile u32*)0x000F4FFC)
#define GAME_ACTIVE (*(volatile int*)0x0021E1EC)
#define SCENE_LOADED (*(volatile int*)0x0022026C)
#define TFRAGS (*(Tfrag**)0x002230C0)
#define TFRAG_CNT (*(volatile int*)0x002230C4)
#define SHRUB_CNT (*(volatile int*)0x00223078)
#define SHRUB_COLORS (*(u32**)0x00223088)
#define SHRUB_XFORMS (*(u8**)0x00223084)
#define TIE_CNT (*(volatile int*)0x00223114)
#define SHRUB_CLASS_TABLE ((u8**)0x0032A720)
#define SHRUB_ACTIVE_CLASSES ((s32*)0x0032B1A0)
#define TIE_CLASS_TABLE ((u8**)0x0032D1C0)
#define TIE_ACTIVE_CLASSES ((s32*)0x003326C0)
#define LIGHT_SHRUBS_HOOK_TRANSITION 0x004C3A18
#define LIGHT_SHRUBS_HOOK_SETUP 0x004EF7E8
#define LIGHT_TIES_HOOK_TRANSITION 0x004C39D8
#define COLOR_MODE_DYNAMIC 1
#define COLOR_MODE_STATIC 2
#define COLOR_MODE COLOR_MODE_STATIC
#define CACHE_MAGIC 0xC011FAC2
#define CACHE ((PerVertexGridCache*)0x000C0000)
#define COLOR_CELL_COUNT 8192
#define MAX_TFRAGS 4096
#define MAX_SHRUBS 4096
#define MAX_TIES 4096
#define MAX_CLASS_LIST 256
#define MAX_CLASS_COLOR_WORDS 1024
#define SHRUB_CLASS_COLOR_WORDS 64
#define SCAN_GRID 81
#define SCAN_COUNT (SCAN_GRID * SCAN_GRID)
#define SCAN_SPACING 4.0f
#define CELL_SIZE 4.0f
#define CELL_INV (1.0f / CELL_SIZE)
#define SAMPLE_Z_ABOVE 18.0f
#define SAMPLE_Z_BELOW 90.0f
#define SAMPLES_PER_TICK 128
#define TFRAGS_PER_TICK 32
#define TFRAG_WORLD_SCALE (1.0f / 1024.0f)
#define MIN_TFRAG_RADIUS 20.0f
#define MAX_TFRAG_RADIUS 80.0f
#define COLOR_HOOK 0x80ff00ff
#define COLOR_GAME 0x8000ffff
#define COLOR_PLAYER 0x80ffff00
#define COLOR_HIT 0x80ffffff
#define COLOR_TINT 0x800000ff
#define COLOR_WATER 0x800000ff
#define COLOR_WALKABLE 0x8000ff00
#define COLOR_NONWALKABLE 0x8000ffff
#define COLOR_STATIC_TERRAIN 0x80009000
#define COLOR_STATIC_SHRUB 0x80008080
#define COLOR_STATIC_TIE 0x80808000
#define COLOR_STATIC_SPECIAL 0x8000ff00
#define COLOR_STATIC_WATER COLOR_WATER

typedef struct Tfrag {
    float bsphere[4];
    void *data;
    u16 lod_2_ofs;
    u16 shared_ofs;
    u16 lod_1_ofs;
    u16 lod_0_ofs;
    u16 tex_ofs;
    u16 rgba_ofs;
    u8 common_size;
    u8 lod_2_size;
    u8 lod_1_size;
    u8 lod_0_size;
    u8 lod_2_rgba_cnt;
    u8 lod_1_rgba_cnt;
    u8 lod_0_rgba_cnt;
    u8 base_only;
    u8 tex_cnt;
    u8 rgba_size;
    u8 rgba_verts_loc;
    u8 occl_index_stash;
    u8 msphere_cnt;
    u8 flags;
    s16 msphere_ofs;
    s16 light_ofs;
    s16 light_vert_start_off;
    u8 dir_lights_one;
    u8 dir_lights_upd;
    u16 point_lights;
    s16 cube_ofs;
    s16 occl_index;
    u8 vert_cnt;
    u8 tri_cnt;
    s16 mip_dist;
} Tfrag;

typedef struct ColorCell {
    s16 gx;
    s16 gy;
    u32 color;
    u8 priority;
    u8 stickyWater;
    u16 pad;
} ColorCell;

typedef struct PerVertexGridCache {
    u32 magic;
    u32 sceneLoaded;
    u32 rayCursor;
    u32 tfragCursor;
    u32 hits;
    u32 tintWrites;
    ColorCell cells[COLOR_CELL_COUNT];
} PerVertexGridCache;

typedef void (*ExecuteDrawFunctions_t)(void **list);
typedef void (*LightShrubs_t)(u16 *list);
typedef void (*LightTies_t)(void);
static ExecuteDrawFunctions_t ExecuteDrawFunctions = (ExecuteDrawFunctions_t)0x004C46B0;
static LightShrubs_t LightShrubs = (LightShrubs_t)0x0058AFE0;
static LightTies_t LightTies = (LightTies_t)0x00596780;
static int hooksInstalled;
static int shrubHookHits;
static int tieHookHits;
static int shrubClassWrites;
static int tieClassWrites;
static int tieTableWrites;
static int lastSampleTime = -1;
static int hitFlash;
static int tintCount;

static Player *getLocalPlayer(void)
{
    Player **players = playerGetAll();
    return players ? players[0] : 0;
}

static int fastFloor(float v)
{
    int i = (int)v;
    return (v < (float)i) ? i - 1 : i;
}

static int gridCoord(float v)
{
    return fastFloor(v * CELL_INV);
}

static u32 cellHash(int gx, int gy)
{
    u32 h = (u32)(gx * 73856093) ^ (u32)(gy * 19349663);
    return h & (COLOR_CELL_COUNT - 1);
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

static u8 priorityForSurface(int id)
{
    id &= 0x0F;
    if (isWaterSurface(id)) return 3;
    if (id == COLLISION_SURFACE_NONWALKABLE_1 || id == COLLISION_SURFACE_NONWALKABLE_2 || id == COLLISION_SURFACE_NOWALK_MAGNET_WALL) return 2;
    if (id == COLLISION_SURFACE_MAGNET_WALL || id == COLLISION_SURFACE_WALKABLE_1 || id == COLLISION_SURFACE_WALKABLE_2 || id == COLLISION_SURFACE_WALKABLE_3 || id == COLLISION_SURFACE_WALKABLE_4) return 1;
    return 0;
}

static u32 colorForSurface(int id)
{
    id &= 0x0F;
    if (isWaterSurface(id)) return COLOR_WATER;
    if (id == COLLISION_SURFACE_NONWALKABLE_1 || id == COLLISION_SURFACE_NONWALKABLE_2 || id == COLLISION_SURFACE_NOWALK_MAGNET_WALL) return COLOR_NONWALKABLE;
    return COLOR_WALKABLE;
}

static void initCache(void)
{
    int i;
    if (CACHE->magic == CACHE_MAGIC && CACHE->sceneLoaded == (u32)SCENE_LOADED) return;
    CACHE->magic = CACHE_MAGIC;
    CACHE->sceneLoaded = SCENE_LOADED;
    CACHE->rayCursor = 0;
    CACHE->tfragCursor = 0;
    CACHE->hits = 0;
    CACHE->tintWrites = 0;
    for (i = 0; i < COLOR_CELL_COUNT; ++i) {
        CACHE->cells[i].gx = 0x7fff;
        CACHE->cells[i].gy = 0x7fff;
        CACHE->cells[i].color = COLOR_WALKABLE;
        CACHE->cells[i].priority = 0;
        CACHE->cells[i].stickyWater = 0;
    }
    lastSampleTime = -1;
}

static void rememberCell(float x, float y, int id)
{
    int gx = gridCoord(x);
    int gy = gridCoord(y);
    u8 priority = priorityForSurface(id);
    u32 color = colorForSurface(id);
    u32 h;
    int probe;
    if (priority == 0) return;
    h = cellHash(gx, gy);
    for (probe = 0; probe < 8; ++probe) {
        ColorCell *cell = &CACHE->cells[(h + probe) & (COLOR_CELL_COUNT - 1)];
        if (cell->gx == gx && cell->gy == gy) {
            if (cell->stickyWater && priority < 3) return;
            if (priority >= cell->priority) {
                cell->color = color;
                cell->priority = priority;
                if (priority == 3) cell->stickyWater = 1;
            }
            return;
        }
        if (cell->gx == 0x7fff) {
            cell->gx = gx;
            cell->gy = gy;
            cell->color = color;
            cell->priority = priority;
            cell->stickyWater = priority == 3;
            return;
        }
    }
}

static ColorCell *findCellAt(int gx, int gy)
{
    u32 h = cellHash(gx, gy);
    int probe;
    for (probe = 0; probe < 8; ++probe) {
        ColorCell *cell = &CACHE->cells[(h + probe) & (COLOR_CELL_COUNT - 1)];
        if (cell->gx == gx && cell->gy == gy) return cell;
        if (cell->gx == 0x7fff) return 0;
    }
    return 0;
}

static u32 lookupCellColor(float x, float y)
{
    int gx = gridCoord(x);
    int gy = gridCoord(y);
    int ox, oy;
    ColorCell *center = findCellAt(gx, gy);
    int hasWater = 0;
    int hasNonwalk = 0;
    int hasWalk = 0;
    if (center && center->priority == 3) return COLOR_WATER;
    for (oy = -1; oy <= 1; ++oy) {
        for (ox = -1; ox <= 1; ++ox) {
            ColorCell *cell = findCellAt(gx + ox, gy + oy);
            if (!cell || cell->priority == 0) continue;
            if (cell->priority == 3) hasWater = 1;
            else if (cell->priority == 2) hasNonwalk = 1;
            else if (cell->priority == 1) hasWalk = 1;
        }
    }
    if (hasWater && hasWalk) return COLOR_WATER;
    if (center && center->priority) return center->color;
    if (hasNonwalk && hasWalk) return COLOR_NONWALKABLE;
    return COLOR_WALKABLE;
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

static void sampleOne(Player *player)
{
    VECTOR from, to;
    int local, dx, dy, id;
    local = CACHE->rayCursor % SCAN_COUNT;
    getSpiralOffset(local, &dx, &dy);
    CACHE->rayCursor++;
    from[0] = player->pos[0] + dx * SCAN_SPACING;
    from[1] = player->pos[1] + dy * SCAN_SPACING;
    from[2] = player->pos[2] + SAMPLE_Z_ABOVE;
    from[3] = 1.0f;
    to[0] = from[0];
    to[1] = from[1];
    to[2] = player->pos[2] - SAMPLE_Z_BELOW;
    to[3] = 1.0f;
    if (!CollLine_Fix(from, to, COLLISION_FLAG_IGNORE_MOBY_SPECIAL_COLLIDERS | COLLISION_FLAG_INCLUDE_BACKFACE, 0, 0)) return;
    id = CollLine_Fix_GetHitCollisionId() & 0x0F;
    rememberCell(from[0], from[1], id);
    CACHE->hits++;
    hitFlash = 20;
}

static void updateSamples(Player *player)
{
    int now, i;
    now = gameGetTime();
    if (now == lastSampleTime) return;
    lastSampleTime = now;
    if (hitFlash) hitFlash--;
    for (i = 0; i < SAMPLES_PER_TICK; ++i) sampleOne(player);
}

static int saneTfrag(Tfrag *t)
{
    u32 data = (u32)t->data;
    if (!data) return 0;
    if (data < 0x00100000 || data > 0x02000000) return 0;
    if (t->rgba_ofs < 0x10) return 0;
    if (t->rgba_size <= 0) return 0;
    return 1;
}

static int approxGridCols(int words)
{
    if (words <= 16) return 4;
    if (words <= 36) return 6;
    if (words <= 64) return 8;
    if (words <= 100) return 10;
    if (words <= 144) return 12;
    return 16;
}

static int iabs(int v)
{
    return v < 0 ? -v : v;
}

static int validEntryCount(int count, int words)
{
    return count >= 4 && count <= words && count <= 192;
}

static int scoreGrid(int entries, int cols, int rows, int triCnt)
{
    int expectedTris;
    int aspectPenalty;
    if (cols < 2 || rows < 2 || cols * rows != entries) return 0x7fffffff;
    expectedTris = (cols - 1) * (rows - 1) * 2;
    aspectPenalty = iabs(cols - rows);
    return iabs(expectedTris - triCnt) * 16 + aspectPenalty;
}

static void inferGrid(Tfrag *t, int words, int *colsOut, int *rowsOut, int *periodOut)
{
    int candidates[4];
    int c, ci;
    int bestScore = 0x7fffffff;
    int bestEntries = words;
    int bestCols = approxGridCols(words);
    int bestRows = (words + bestCols - 1) / bestCols;
    candidates[0] = t->vert_cnt;
    candidates[1] = t->lod_0_rgba_cnt;
    candidates[2] = t->lod_1_rgba_cnt;
    candidates[3] = t->lod_2_rgba_cnt;
    for (ci = 0; ci < 4; ++ci) {
        int entries = candidates[ci];
        if (!validEntryCount(entries, words)) continue;
        for (c = 2; c <= 32; ++c) {
            int rows;
            int score;
            if ((entries % c) != 0) continue;
            rows = entries / c;
            if (rows > 32) continue;
            score = scoreGrid(entries, c, rows, t->tri_cnt);
            if (score < bestScore) {
                bestScore = score;
                bestEntries = entries;
                bestCols = c;
                bestRows = rows;
            }
        }
    }
    *colsOut = bestCols;
    *rowsOut = bestRows;
    *periodOut = bestEntries;
}

static void tintTfragPerEntry(Tfrag *t)
{
    u32 *rgba;
    int words, cols, rows, period, i;
    float cx, cy, r;
    if (!saneTfrag(t)) return;
    rgba = (u32*)((u32)t->data + t->rgba_ofs);
    words = ((int)t->rgba_size) * 4;
    inferGrid(t, words, &cols, &rows, &period);
    cx = t->bsphere[0] * TFRAG_WORLD_SCALE;
    cy = t->bsphere[1] * TFRAG_WORLD_SCALE;
    r = t->bsphere[3] * TFRAG_WORLD_SCALE;
    if (r < MIN_TFRAG_RADIUS) r = MIN_TFRAG_RADIUS;
    if (r > MAX_TFRAG_RADIUS) r = MAX_TFRAG_RADIUS;
    for (i = 0; i < words; ++i) {
        int vi = period > 0 ? i % period : i;
        int col = vi % cols;
        int row = vi / cols;
        float fx = cols > 1 ? ((float)col / (float)(cols - 1)) : 0.5f;
        float fy = rows > 1 ? ((float)row / (float)(rows - 1)) : 0.5f;
        float px = cx + (fx - 0.5f) * (r * 2.0f);
        float py = cy + (fy - 0.5f) * (r * 2.0f);
        rgba[i] = lookupCellColor(px, py);
    }
    CACHE->tintWrites++;
    tintCount++;
}

static void tintSomeTfrags(void)
{
    Tfrag *tfrags = TFRAGS;
    int count = TFRAG_CNT;
    int n, index;
    tintCount = 0;
    if (!tfrags || count <= 0 || count > MAX_TFRAGS) return;
    for (n = 0; n < TFRAGS_PER_TICK; ++n) {
        index = CACHE->tfragCursor % count;
        CACHE->tfragCursor++;
        tintTfragPerEntry(&tfrags[index]);
    }
}

static void tintTfragSolid(Tfrag *t, u32 color)
{
    u32 *rgba;
    int words, i;
    if (!saneTfrag(t)) return;
    rgba = (u32*)((u32)t->data + t->rgba_ofs);
    words = ((int)t->rgba_size) * 4;
    for (i = 0; i < words; ++i) rgba[i] = color;
    CACHE->tintWrites++;
    tintCount++;
}

static void tintAllTfragsStatic(void)
{
    Tfrag *tfrags = TFRAGS;
    int count = TFRAG_CNT;
    int i;
    tintCount = 0;
    if (!tfrags || count <= 0 || count > MAX_TFRAGS) return;
    for (i = 0; i < count; ++i) tintTfragSolid(&tfrags[i], COLOR_STATIC_TERRAIN);
}

static int saneBucketPointer(void *ptr)
{
    u32 addr = (u32)ptr;
    return addr >= 0x00100000 && addr < 0x02000000;
}

static void tintStaticShrubs(void)
{
    u32 *colors = SHRUB_COLORS;
    int count = SHRUB_CNT;
    int i, j;
    if (saneBucketPointer(colors) && count > 0 && count <= MAX_SHRUBS) {
        for (i = 0; i < count; ++i) {
            u32 *block = (u32*)((u32)colors + i * 0x60);
            for (j = 0; j < 24; ++j) block[j] = COLOR_STATIC_SHRUB;
        }
    }

    shrubClassWrites = 0;
    for (i = 0; i < MAX_CLASS_LIST; ++i) {
        int classId = SHRUB_ACTIVE_CLASSES[i];
        u8 *klass;
        u32 *classColors;
        if (classId < 0) break;
        if (classId >= MAX_CLASS_LIST) continue;
        klass = SHRUB_CLASS_TABLE[classId];
        if (!saneBucketPointer(klass)) continue;
        classColors = *(u32**)(klass + 0x2c);
        if (!saneBucketPointer(classColors)) continue;
        for (j = 0; j < SHRUB_CLASS_COLOR_WORDS; ++j) classColors[j] = COLOR_STATIC_SHRUB | 0x80000000;
        shrubClassWrites++;
    }
}

static void tintStaticTies(void)
{
    tieClassWrites = 0;
    tieTableWrites = 0;
}
static void lightShrubsHook(u16 *list)
{
    shrubHookHits++;
    LightShrubs(list);
    if (COLOR_MODE == COLOR_MODE_STATIC) tintStaticShrubs();
}

static void lightTiesHook(void)
{
    tieHookHits++;
    LightTies();
    if (COLOR_MODE == COLOR_MODE_STATIC) tintStaticTies();
}

static void installStaticHooks(void)
{
    if (hooksInstalled) return;
    HOOK_JAL(LIGHT_SHRUBS_HOOK_TRANSITION, &lightShrubsHook);
    HOOK_JAL(LIGHT_SHRUBS_HOOK_SETUP, &lightShrubsHook);
    hooksInstalled = 1;
}
static int isStaticShrubMoby(Moby *moby)
{
    int oClass = moby->oClass;
    return oClass == MOBY_ID_SMALL_WHITE_ROCK
        || oClass == MOBY_ID_SMALL_MAGMA_ROCK
        || oClass == MOBY_ID_BLACK_ROCK
        || oClass == MOBY_ID_BIGASS_ROCK
        || oClass == MOBY_ID_SARATHOS_MOUNTAIN
        || oClass == MOBY_ID_SARATHOS_FLOATING_ORGANISM
        || oClass == MOBY_ID_SARATHOS_LAND_BUBBLE_ORGANISM
        || oClass == MOBY_ID_VALIX_PLANT_ROOTS
        || oClass == MOBY_ID_VALIX_PLANT_STALK
        || oClass == MOBY_ID_DECORATIVE_FLAG_WITH_POLE
        || oClass == MOBY_ID_SHAAR_POLES_THEY_GO_IN_THE_HOLES_ON_THE_SIDES_OF_THE_TEMPLE;
}

static int isStaticSpecialMoby(Moby *moby)
{
    int oClass = moby->oClass;
    return oClass == MOBY_ID_SWINGSHOT_ORB
        || oClass == MOBY_ID_SWINGSHOT_REALIN_KIND;
}

static int isStaticWaterMoby(Moby *moby)
{
    int oClass = moby->oClass;
    return oClass == MOBY_ID_WATER
        || oClass == MOBY_ID_ACID;
}

static void tintMobyGlow(Moby *moby, u32 color)
{
    moby->glow_rgba = color;
    moby->modeBits |= MOBY_MODE_BIT_HAS_GLOW;
    moby->modeBits2 = (moby->modeBits2 & 0x00ff) | 0x8000;
    moby->alpha = 0x80;
    if (moby->pClass) {
        moby->pClass->glow_rgba = color;
        moby->pClass->mode_bits |= MOBY_MODE_BIT_HAS_GLOW;
        moby->pClass->mode_bits2 = (moby->pClass->mode_bits2 & 0x7f) | 0x80;
    }
}

static void tintStaticMobys(void)
{
    Moby *moby = mobyListGetStart();
    Moby *end = mobyListGetEnd();
    while (moby && moby < end) {
        if (!mobyIsDestroyed(moby)) {
            if (isStaticWaterMoby(moby)) tintMobyGlow(moby, COLOR_STATIC_WATER);
            else if (isStaticSpecialMoby(moby)) tintMobyGlow(moby, COLOR_STATIC_SPECIAL);
            else if (isStaticShrubMoby(moby)) tintMobyGlow(moby, COLOR_STATIC_SHRUB);
        }
        ++moby;
    }
}

static void runStaticColorMode(void)
{
    initCache();
    installStaticHooks();
    tintAllTfragsStatic();
    tintStaticShrubs();
    tintStaticMobys();
}
int main(void **drawList)
{
    Player *player = 0;
    HEARTBEAT = 0xC011FB00;
    if (GAME_ACTIVE && SCENE_LOADED == 1) {
        if (COLOR_MODE == COLOR_MODE_STATIC) {
            runStaticColorMode();
        } else if (COLOR_MODE == COLOR_MODE_DYNAMIC) {
            initCache();
            player = getLocalPlayer();
            if (player) updateSamples(player);
            tintSomeTfrags();
        }
    }
    ExecuteDrawFunctions(drawList);
    if (GAME_ACTIVE && SCENE_LOADED == 1 && COLOR_MODE == COLOR_MODE_STATIC) {
        tintStaticShrubs();
        }
    gfxScreenSpaceBox(0.018f, 0.36f, 0.025f, 0.030f, COLOR_HOOK);
    if (GAME_ACTIVE && SCENE_LOADED == 1) {
        gfxScreenSpaceBox(0.050f, 0.36f, 0.025f, 0.030f, COLOR_GAME);
        if (player) gfxScreenSpaceBox(0.082f, 0.36f, 0.025f, 0.030f, COLOR_PLAYER);
        if (hitFlash) gfxScreenSpaceBox(0.114f, 0.36f, 0.025f, 0.030f, COLOR_HIT);
        if (CACHE->tintWrites > 0 || tintCount > 0) gfxScreenSpaceBox(0.146f, 0.36f, 0.025f, 0.030f, COLOR_TINT);
        if (COLOR_MODE == COLOR_MODE_STATIC && SHRUB_CNT > 0) gfxScreenSpaceBox(0.178f, 0.36f, 0.025f, 0.030f, COLOR_STATIC_SHRUB);
        if (COLOR_MODE == COLOR_MODE_STATIC && shrubHookHits > 0) gfxScreenSpaceBox(0.210f, 0.36f, 0.025f, 0.030f, COLOR_STATIC_SPECIAL);
        if (COLOR_MODE == COLOR_MODE_STATIC && tieHookHits > 0) gfxScreenSpaceBox(0.242f, 0.36f, 0.025f, 0.030f, COLOR_STATIC_TIE);
        if (COLOR_MODE == COLOR_MODE_STATIC && shrubClassWrites > 0) gfxScreenSpaceBox(0.274f, 0.36f, 0.025f, 0.030f, COLOR_STATIC_SHRUB);
        if (COLOR_MODE == COLOR_MODE_STATIC && tieClassWrites > 0) gfxScreenSpaceBox(0.306f, 0.36f, 0.025f, 0.030f, COLOR_STATIC_TIE);
    }
    HEARTBEAT = 0xC011FBFF;
    return 0;
}











