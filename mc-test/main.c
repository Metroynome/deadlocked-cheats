/*
This is to test the MC Save feature.
====================================
*/

#include <tamtypes.h>
#include <libdl/stdio.h>
#include <libdl/string.h>
#include <libdl/game.h>
#include <libdl/player.h>
#include <libdl/pad.h>
#include <libdl/mc.h>


int Started = 0;
char * file = "/BASCUS-97465RATCHET/patch.bin";
const char test[] = "RAWRS NOOBZORS";

int fd;

void Open()
{
	if (Started == 0)
	{
		Started = 1;
		// Port, Slot, Path, Mode
		/*
			Modes:
			Read: 1
			Write: 2
		*/
		McOpen(0, 0, file, 2);
		McSync(0, NULL, &fd);
		if (fd >= 0)
		{
			printf("\nOpened file");
		}
		// fd, offset, origin (start)
		McSeek(fd, 0xd0, 0x0);
		McSync(0, NULL, &fd);
		if (fd > 0)
		{
			printf("\nRead bytes: %x", fd);
		}
		McWrite(fd, &test, sizeof(test));
		McSync(0, NULL, &fd);
		if (fd >= 0)
		{
			printf("\nWrote bytes: %x", fd);
		}
		McClose(fd);
		McSync(0, NULL, &fd);
		if (fd >= 0)
		{
			printf("\nFile Closed!");
		}
	}
}

int main(void)
{
	PadButtonStatus * pad = (PadButtonStatus*)0x001ee600;
	if ((pad->btns & (PAD_L3)) == 0)
	{
		Started = 0;
		Open();
	}

	return 1;
}
