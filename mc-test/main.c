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
const char test[0x50] = "RAWRS NOOBZORS";

int fd;

void Open()
{
	if (Started == 0 && *(u32*)0x001CF85C == 0x000F8D29)
	{
		Started = 1;
		char copy[0x10000];
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
		memcpy(copy, (u8*)0x01DFF000, 0x10000); // dest, src, size
		sprintf(&copy[0xc00], &test); // string[offset], new data
		McWrite(fd, &copy, 0x10000); // fd, data, size
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
		//Started = 0;
		Open();
	}

	return 1;
}
