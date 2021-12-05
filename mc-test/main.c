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


int fd = 0;
int Started = 0;
char * file = "/BASCUS-97465RATCHET/patch.bin";
const char test[] = "RAWRS NOOBZORS";

void Open()
{
	if (Started == 0 && *(u32*)0x001CF85C == 0x000F8D29)
	{
		Started = 1;
		char copy[0x1060];
		// Port, Slot, Path, Mode
		/*
			Modes:
			Read: 1
			Write: 2
		*/
		if(McOpen(0, 0, file, 2) == 0)
		{
			printf("\nFile opened");
		}
		else
		{
			printf("\nError opening file.");
		}
		// McSync(0, NULL, &fd);
		// if (fd >= 0)
		// {
		// 	printf("\nMcOpen: %d", fd);
		// }
		//McSeek(fd, 0xb0, file);
		//McSync(0, NULL, &fd);
		// if (fd >= 0)
		// {
		// 	printf("\nMcSeek: 0x%x", fd);
		// }
		memcpy(copy, (u8*)0x01DFF000, 0x1060); // dest, src, size
		//sprintf(&copy[0xc00], &test); // string[offset], new data
		McWrite(fd, &copy, sizeof(copy)); // fd, data, size
		McSync(0, NULL, &fd);
		// if (fd >= 0)
		// {
		// 	printf("\nMcWrite: %d", fd);
		// }
		if(McClose(fd) == 0)
		{
			printf("\nFile Closed");
		}
		else
		{
			printf("\nError closing file");
		}
		// McSync(0, NULL, &fd);
		// if (fd >= 0)
		// {
		// 	printf("\nMcClose: %d", fd);
		// }
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
