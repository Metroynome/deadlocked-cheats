#include <tamtypes.h>
#include <libdl/dl.h>
#include <libdl/game.h>
#include <libdl/player.h>
#include <libdl/moby.h>

void wrenchReplacement(void)
{
}

int main(void)
{
	dlPreUpdate();

	if (!isInGame())
		return -1;

	wrenchReplacement();

	dlPostUpdate();

	return 0;
}
