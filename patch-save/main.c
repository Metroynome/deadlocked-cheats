/*
This is to test the MC Save feature.
====================================
*/

#include <tamtypes.h>
#include <libdl/stdio.h>
#include <libdl/graphics.h>
#include <libdl/string.h>
#include <libdl/game.h>
#include <libdl/player.h>
#include <libdl/pad.h>
#include <libdl/math.h>
#include <libdl/math3d.h>
#include <libdl/hud.h>
#include <libdl/ui.h>

#include "rpc.h"
#include <libdl/mc.h>
#include <sifcmd.h>
#include <iopheap.h>
#include <iopcontrol.h>
#include <sifrpc.h>
#include <loadfile.h>

#define NEWLIB_PORT_AWARE
#include <io_common.h>

#define MAP_FRAG_PAYLOAD_MAX_SIZE               (1024)
#define LOAD_MODULES_STATE                      (*(u8*)0x000EFF00)
#define HAS_LOADED_MODULES                      (LOAD_MODULES_STATE == 100)

#define USB_FS_ID                               (*(u8*)0x000EFF04)
#define USB_SRV_ID                              (*(u8*)0x000EFF08)

void loadModules(void);

void * usbFsModuleStart = (void*)0x000E0000;
int usbFsModuleSize = 0;
void * usbSrvModuleStart = (void*)0x000ED000;
int usbSrvModuleSize = 0;

enum MenuActionId
{
	ACTION_ERROR_LOADING_MODULES = -1,

	ACTION_MODULES_NOT_INSTALLED = 0,
	ACTION_DOWNLOADING_MODULES = 1,
	ACTION_MODULES_INSTALLED = 2,
	ACTION_NEW_MAPS_UPDATE = 3,

	ACTION_NONE = 100
};

// memory card fd
int fd;
int initialized = 0;
int actionState = ACTION_MODULES_NOT_INSTALLED;
int rpcInit = 0;

// 
char membuffer[256];

//void loadModules(void);


// paths for level specific files
char * boot = "BOOT.ELF";

// void loadModules(void)
// {
// 	if (LOAD_MODULES_STATE < 7)
// 	{
// 		printf("MASS Not Loaded! :(\n");
// 	}

// 	if (LOAD_MODULES_STATE != 100)
// 	{
// 		//
// 		SifInitRpc(0);

// 		// Load modules
// 		USB_FS_ID = SifExecModuleBuffer(usbFsModuleStart, usbFsModuleSize, 0, NULL, NULL);
// 		USB_SRV_ID = SifExecModuleBuffer(usbSrvModuleStart, usbSrvModuleSize, 0, NULL, NULL);

// 		printf("Loading MASS: %d\n", USB_FS_ID);
// 		printf("Loading USBSERV: %d\n", USB_SRV_ID);
// 	}

// 	LOAD_MODULES_STATE = 100;
// }

int readFileLength(char * path)
{
	int fd, fSize;

	// Open
	rpcUSBopen(path, FIO_O_RDONLY);
	rpcUSBSync(0, NULL, &fd);

	// Ensure the file was opened successfully
	if (fd < 0)
	{
		printf("error opening file (%s): %d\n", path, fd);
		return -1;
	}
	
	// Get length of file
	rpcUSBseek(fd, 0, SEEK_END);
	rpcUSBSync(0, NULL, &fSize);

	// Close file
	rpcUSBclose(fd);
	rpcUSBSync(0, NULL, NULL);

	return fSize;
}

int main(void)
{
	// if (LOAD_MODULES_STATE == 100)
	// {
	// 	printf("BOOT.ELF Size: %d\n", readFileLength(boot));
	// }
	if (*(u32*)0x004BC538 == 0x6F726463)
	{
		strcpy((char*)0x004BC538, "mass:/BOOT.ELF");
	}
	return 1;
}
