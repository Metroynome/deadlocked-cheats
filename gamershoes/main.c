#include <tamtypes.h>
#include <libdl/dl.h>
#include <libdl/game.h>
#include <libdl/player.h>
#include <libdl/moby.h>
#include <libdl/math3d.h>
#include <libdl/color.h>

#define SHOE_LEFT_JOINT             (15)
#define SHOE_RIGHT_JOINT            (16)
#define SHOE_MIN_SPEED_2D           (1.5f)
#define SHOE_MAX_OFF_GROUND_TICKS   (18)
#define SHOE_COLOR_CYCLE_FRAMES     (240)
#define SHOE_PARTICLE_OPACITY       (0x80)
#define SHOE_PARTICLE_COUNT         (3)
#define SHOE_PARTICLE_SCALE         (0.55f)
#define SHOE_HEIGHT_OFFSET          (-0.18f)
#define SHOE_COLOR_ALPHA            (0x78000000)
#define SHOE_DEBUG_EXAGGERATE       (0)
#define SHOE_DEBUG_INTERVAL         (10)
#define SHOE_FALLBACK_INTERVAL      (12)
#define SHOE_AFTERGLOW_FRAMES       (90)
#define SHOE_MAX_PARTICLES          (64)
#define SHOE_PARTICLE_LIFE_FRAMES   (24)

#define ANIM_FRAME_WALK_FIRST       (38.5f)
#define ANIM_FRAME_WALK_SECOND      (12.0f)
#define ANIM_FRAME_RUN_FIRST        (12.0f)
#define ANIM_FRAME_RUN_SECOND       (1.0f)
#define ANIM_FRAME_FAST_FIRST       (8.0f)
#define ANIM_FRAME_FAST_SECOND      (21.0f)

struct PartInstance {
	char IClass;
	char Type;
	char Tex;
	char Alpha;
	u32 RGBA;
	char Rot;
	char DrawDist;
	short Timer;
	float Scale;
	VECTOR Position;
	int Update[8];
};

struct ShoeState {
	int initialized;
	int foot;
	int fallbackTimer;
	int afterglowTimer;
	int wasAirborne;
	char animSeqId;
	float prevMayaFrame;
	VECTOR lastPos;
};

static struct ShoeState shoeStates[GAME_MAX_PLAYERS];
static struct PartInstance * shoeParticles[SHOE_MAX_PARTICLES];
static unsigned char shoeParticleTimers[SHOE_MAX_PARTICLES];
static unsigned char shoeParticlePlayerIndexes[SHOE_MAX_PARTICLES];
static unsigned char shoeParticleJoints[SHOE_MAX_PARTICLES];
static int shoeParticleIndex = 0;
static int shoeColorIndex = 0;

static const u32 shoePalette[] = {
	0x780000ff, /* red */
	0x78ff0000, /* blue */
	0x780080ff, /* orange */
	0x78ff00ff, /* pink */
	0x7800ff00, /* green */
	0x78ffff00, /* cyan */
	0x78ff00a0, /* purple */
	0x7800ffff  /* yellow */
};

static struct PartInstance * spawnShoeParticle(VECTOR position, u32 color, char opacity, int idx)
{
	u32 a3 = *(u32*)0x002218E8;
	u32 t0 = *(u32*)0x002218E4;
	float f12 = *(float*)0x002218DC;
	float f1 = *(float*)0x002218E0;

	return ((struct PartInstance* (*)(VECTOR, u32, char, u32, u32, int, int, int, float))0x00533308)(position, color, opacity, a3, t0, -1, 0, 0, f12 + (f1 * idx));
}

static void destroyShoeParticle(struct PartInstance *particle)
{
	if (particle)
		((void (*)(struct PartInstance*))0x005284D8)(particle);
}

static void getMobyJointWorld(Moby *moby, int joint, VECTOR out)
{
	((void (*)(Moby*, int, VECTOR))0x004F82B8)(moby, joint, out);
}

static void trackShoeParticle(struct PartInstance *particle, int playerIndex, int joint)
{
	if (!particle)
		return;

	if (shoeParticles[shoeParticleIndex])
		destroyShoeParticle(shoeParticles[shoeParticleIndex]);

	shoeParticles[shoeParticleIndex] = particle;
	shoeParticleTimers[shoeParticleIndex] = SHOE_PARTICLE_LIFE_FRAMES;
	shoeParticlePlayerIndexes[shoeParticleIndex] = playerIndex;
	shoeParticleJoints[shoeParticleIndex] = joint;
	shoeParticleIndex = (shoeParticleIndex + 1) % SHOE_MAX_PARTICLES;
}


static void attachShoeParticleToJoint(int i)
{
	Player *player = playerGetFromIndex(shoeParticlePlayerIndexes[i]);
	VECTOR pos;

	if (!player || !player->pMoby) {
		destroyShoeParticle(shoeParticles[i]);
		shoeParticles[i] = 0;
		shoeParticleTimers[i] = 0;
		return;
	}

	getMobyJointWorld(player->pMoby, shoeParticleJoints[i], pos);
	pos[2] += SHOE_HEIGHT_OFFSET;
	pos[3] = 1.0f;
	vector_copy(shoeParticles[i]->Position, pos);
}

static void updateShoeParticles(void)
{
	int i;

	for (i = 0; i < SHOE_MAX_PARTICLES; ++i) {
		if (!shoeParticles[i])
			continue;

		attachShoeParticleToJoint(i);
		if (!shoeParticles[i])
			continue;

		if (shoeParticleTimers[i] > 0)
			--shoeParticleTimers[i];

		if (shoeParticleTimers[i] == 0) {
			destroyShoeParticle(shoeParticles[i]);
			shoeParticles[i] = 0;
		}
	}
}


static int isDeadState(int state)
{
	return state == PLAYER_STATE_DEATH
		|| state == PLAYER_STATE_DROWN
		|| state == PLAYER_STATE_DEATH_FALL
		|| state == PLAYER_STATE_DEATHSAND_SINK
		|| state == PLAYER_STATE_LAVA_DEATH
		|| state == PLAYER_STATE_ICEWATER_FREEZE
		|| state == PLAYER_STATE_ELECTRIC_DEATH
		|| state == PLAYER_STATE_ELECTRIC_DEATH_UNDER
		|| state == PLAYER_STATE_DEATH_NO_FALL
		|| state == PLAYER_STATE_WAIT_FOR_RESURRECT;
}

static int playerCanFlashShoes(Player *player)
{
	if (!player || !player->pMoby)
		return 0;

	if (isDeadState(player->state))
		return 0;

	if (player->state == PLAYER_STATE_CHARGE)
		return 0;

	return 1;
}

static int playerIsJumpState(Player *player)
{
	switch (player->state) {
	case PLAYER_STATE_FALL:
	case PLAYER_STATE_JUMP:
	case PLAYER_STATE_GLIDE:
	case PLAYER_STATE_RUN_JUMP:
	case PLAYER_STATE_LONG_JUMP:
	case PLAYER_STATE_FLIP_JUMP:
	case PLAYER_STATE_JINK_JUMP:
	case PLAYER_STATE_ROCKET_JUMP:
	case PLAYER_STATE_DOUBLE_JUMP:
	case PLAYER_STATE_HELI_JUMP:
	case PLAYER_STATE_CHARGE_JUMP:
	case PLAYER_STATE_WALL_JUMP:
	case PLAYER_STATE_WATER_JUMP:
	case PLAYER_STATE_JUMP_ATTACK:
	case PLAYER_STATE_LEDGE_JUMP:
	case PLAYER_STATE_GRIND_JUMP:
	case PLAYER_STATE_GRIND_SWITCH_JUMP:
	case PLAYER_STATE_SWING_FALL:
	case PLAYER_STATE_LAVA_JUMP:
	case PLAYER_STATE_GRIND_JUMP_TURN:
	case PLAYER_STATE_JUMP_TO_POS:
	case PLAYER_STATE_QUICKSAND_JUMP:
	case PLAYER_STATE_MAGNE_JUMP:
	case PLAYER_STATE_JUMP_BOUNCE:
	case PLAYER_STATE_MOON_JUMP:
	case PLAYER_STATE_LATCH_JUMP:
		return 1;
	default:
		return 0;
	}
}

static int playerIsGrounded(Player *player)
{
	return !playerIsJumpState(player) && player->ground.onGood && player->ground.offGood == 0;
}

static int playerMovedEnough(struct ShoeState *state, Player *player)
{
	float dx;
	float dy;
	float dz;
	float distSqr;

	if (!state->initialized) {
		vector_copy(state->lastPos, player->pos);
		return 0;
	}

	dx = player->pos[0] - state->lastPos[0];
	dy = player->pos[1] - state->lastPos[1];
	dz = player->pos[2] - state->lastPos[2];
	distSqr = (dx * dx) + (dy * dy) + (dz * dz);
	vector_copy(state->lastPos, player->pos);

	return distSqr > 0.0004f;
}

static int mayaFrameTrigger(Player *player, float frame)
{
	return ((int (*)(float, Player*))0x005E4250)(frame, player);
}

static int didCrossFrame(struct ShoeState *state, Player *player, float frame)
{
	float current = player->anim.mayaFrm;
	float previous = state->prevMayaFrame;

	if (!state->initialized || state->animSeqId != player->pMoby->animSeqId) {
		state->initialized = 1;
		state->animSeqId = player->pMoby->animSeqId;
		state->prevMayaFrame = current;
		return 0;
	}

	if (current == previous)
		return 0;

	if (previous < current)
		return previous <= frame && frame < current;

	return frame >= previous || frame < current;
}

static void finishFrame(struct ShoeState *state, Player *player)
{
	state->prevMayaFrame = player->anim.mayaFrm;
}

static void resetShoeState(struct ShoeState *state)
{
	state->initialized = 0;
	state->fallbackTimer = 0;
	state->afterglowTimer = 0;
	state->wasAirborne = 0;
	state->animSeqId = 0;
	state->prevMayaFrame = 0;
}

static u32 nextShoeColor(void)
{
	u32 color = shoePalette[shoeColorIndex];

	shoeColorIndex = (shoeColorIndex + 1) % (sizeof(shoePalette) / sizeof(shoePalette[0]));
	return color;
}

static void spawnParticleBurst(VECTOR pos, u32 color, int playerIndex, int joint)
{
	int i;
	struct PartInstance *particle;

	pos[2] += SHOE_HEIGHT_OFFSET;
	pos[3] = 1.0f;

	for (i = 0; i < SHOE_PARTICLE_COUNT; ++i) {
		particle = spawnShoeParticle(pos, color, SHOE_PARTICLE_OPACITY, i & 3);
		if (particle)
			particle->Scale *= SHOE_PARTICLE_SCALE;
		trackShoeParticle(particle, playerIndex, joint);
	}
}

static void spawnShoeFlash(Player *player, int playerIndex, int foot)
{
	VECTOR pos;
	u32 color = nextShoeColor();
	int joint = foot ? SHOE_RIGHT_JOINT : SHOE_LEFT_JOINT;

	getMobyJointWorld(player->pMoby, joint, pos);
	spawnParticleBurst(pos, color, playerIndex, joint);
}

static void spawnDebugShoeExplosion(Player *player)
{
	VECTOR pos;
	u32 color = nextShoeColor();

	spawnShoeFlash(player, 0, 0);
	spawnShoeFlash(player, 0, 1);

	vector_copy(pos, player->pos);
	pos[2] += 0.75f;
	spawnParticleBurst(pos, color, 0, SHOE_LEFT_JOINT);
}

static int flashIfFrameTriggered(struct ShoeState *state, Player *player, int playerIndex, float frame, int foot)
{
	if (mayaFrameTrigger(player, frame) || didCrossFrame(state, player, frame)) {
		spawnShoeFlash(player, playerIndex, foot);
		state->foot = foot ^ 1;
		return 1;
	}

	return 0;
}

static void fallbackShoeFlash(struct ShoeState *state, Player *player, int playerIndex)
{
	if (++state->fallbackTimer < SHOE_FALLBACK_INTERVAL)
		return;

	state->fallbackTimer = 0;
	spawnShoeFlash(player, playerIndex, state->foot);
	state->foot ^= 1;
}

static void updateShoeAfterglow(struct ShoeState *state, Player *player, int playerIndex)
{
	if (state->afterglowTimer <= 0)
		return;

	--state->afterglowTimer;
	fallbackShoeFlash(state, player, playerIndex);
}

static void updatePlayerShoes(Player *player, int playerIndex)
{
	struct ShoeState *state = &shoeStates[playerIndex];

#if SHOE_DEBUG_EXAGGERATE
	if (player && player->pMoby && !isDeadState(player->state) && ((gameGetTime() + playerIndex) % SHOE_DEBUG_INTERVAL) == 0) {
		spawnDebugShoeExplosion(player);
	}
	return;
#endif

	if (!playerCanFlashShoes(player)) {
		resetShoeState(state);
		return;
	}

	if (!state->initialized) {
		state->initialized = 1;
		vector_copy(state->lastPos, player->pos);
		return;
	}

	if (!playerIsGrounded(player)) {
		state->wasAirborne = 1;
		state->fallbackTimer = 0;
		state->afterglowTimer = 0;
		state->animSeqId = player->pMoby->animSeqId;
		state->prevMayaFrame = player->anim.mayaFrm;
		vector_copy(state->lastPos, player->pos);
		return;
	}

	if (state->wasAirborne) {
		state->wasAirborne = 0;
		state->fallbackTimer = 0;
		state->afterglowTimer = SHOE_AFTERGLOW_FRAMES;
		state->animSeqId = player->pMoby->animSeqId;
		state->prevMayaFrame = player->anim.mayaFrm;
		vector_copy(state->lastPos, player->pos);
		spawnShoeFlash(player, playerIndex, 0);
		spawnShoeFlash(player, playerIndex, 1);
		return;
	}

	if (!playerMovedEnough(state, player)) {
		updateShoeAfterglow(state, player, playerIndex);
		return;
	}

	state->afterglowTimer = SHOE_AFTERGLOW_FRAMES;

	if (player->stateType != 1) {
		fallbackShoeFlash(state, player, playerIndex);
		return;
	}

	if (player->state == PLAYER_STATE_WALK) {
		if (player->timers.state < 0xf) {
			fallbackShoeFlash(state, player, playerIndex);
			return;
		}

		if (player->timers.state == 0x16)
			spawnShoeFlash(player, playerIndex, 0);
	}

	switch (player->pMoby->animSeqId) {
	case 5:
	case 6:
		if (!flashIfFrameTriggered(state, player, playerIndex, ANIM_FRAME_WALK_FIRST, 0) &&
			!flashIfFrameTriggered(state, player, playerIndex, ANIM_FRAME_WALK_SECOND, 1))
			fallbackShoeFlash(state, player, playerIndex);
		break;
	case 7:
	case 0x56:
	case 0x5c:
	case 0x5e:
	case 0x60:
		if (!flashIfFrameTriggered(state, player, playerIndex, ANIM_FRAME_RUN_FIRST, 0) &&
			!flashIfFrameTriggered(state, player, playerIndex, ANIM_FRAME_RUN_SECOND, 1))
			fallbackShoeFlash(state, player, playerIndex);
		break;
	case 8:
	case 0x5b:
	case 0x5d:
	case 0x5f:
	case 0x61:
		if (!flashIfFrameTriggered(state, player, playerIndex, ANIM_FRAME_FAST_FIRST, 0) &&
			!flashIfFrameTriggered(state, player, playerIndex, ANIM_FRAME_FAST_SECOND, 1))
			fallbackShoeFlash(state, player, playerIndex);
		break;
	default:
		fallbackShoeFlash(state, player, playerIndex);
		return;
	}

	finishFrame(state, player);
}

void gamershoes(void)
{
	int i;

	updateShoeParticles();

	for (i = 0; i < GAME_MAX_PLAYERS; ++i) {
		updatePlayerShoes(playerGetFromIndex(i), i);
	}
}

int main(void)
{
	dlPreUpdate();

	if (!isInGame())
		return -1;

	gamershoes();

	dlPostUpdate();

	return 0;
}














