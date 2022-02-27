#include <tamtypes.h>
#include <libdl/stdio.h>
#include <libdl/string.h>
#include <libdl/game.h>
#include <libdl/player.h>
#include <libdl/pad.h>

int Nomnom[] = {PAD_UP, PAD_UP, PAD_DOWN, PAD_DOWN, PAD_LEFT, PAD_RIGHT, PAD_LEFT, PAD_RIGHT, PAD_CROSS, PAD_CIRCLE};
int index = 0;
int Konomnomnomi(Button)
{
	if (index >= sizeof(Nomnom))
	{
		return 1;
	}
	else if ((Button & (Nomnom[index])) == 0 && index != -1)
	{
		index++;
	}
	else if ((Button & (Nomnom[index])) != 0 && Button != 0xffff)
	{
		index = -1;
	}
}

int main(void)
{
	PadButtonStatus * pad = (PadButtonStatus*)0x001ee600;
	if ((pad->btns & (PAD_L2)) == 0)
	{
		if (Konomnomnomi(pad->btns))
		{
			printf("\nYou did it!");
		}
	} else if (index == -1)
	{
		index = 0;
	}
	
	return 1;
}
