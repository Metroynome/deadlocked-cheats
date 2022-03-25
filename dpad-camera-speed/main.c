#include <tamtypes.h>
#include <libdl/stdio.h>
#include <libdl/game.h>
#include <libdl/player.h>
#include <libdl/pad.h>

void DPadCameraLogic(void)
{
	// v0: Loaded Camera Speed
	// a0: Camera speed Address
	// a2: Max Camera speed

	//=================TRY 1:::::::
	asm (
		// Load Pad 1
		"lui	$t6, 0x001F\n"
		"addiu 	$t6, $t6, 0xE682\n"
		"lh	$t6, 0x0($t6)\n"
		// Load Jokers
		"addiu 	$t7, $zero, 0xFF7F\n" // Left
		"addiu 	$t8, $zero, 0xFFDF\n" // Right
		"addiu 	$t9, $zero, 0xBFFF\n" // Cross
		// if d-pad Left, Subtract
		"addiu 	$t5, $zero, -0x8\n"
		"beq 	$t6, $t7, _DoNewSpeed\n"
		// if d-pad Right, Add
		"addiu 	$t5, $zero, 0x8\n"
		"beq 	$t6, $t8, _DoNewSpeed\n"
		//if Cross, Add
		"addiu 	$t5, $zero, 0x8\n"
		"beq 	$t6, $t9, _DoNewSpeed\n"

		"_DoNewSpeed:"
		"add 	$t5, $t5, $v0\n"
		// if less than zero, save Zero (Can save Max Speed to Cycle Around)
		"slti	$t6, $t5, 0xffff\n"
		"sw		$zero, 0x0($a0)\n"
		"bne 	$t6, $zero, _exitDpadCamera\n"

		// if greater than max speed, save Max Speed
		"slt 	$t6, $a2, $t5\n"
		"sw 	$a2, 0x0($a0)\n"
		"bne 	$t6, $zero, _exitDpadCamera\n"
		
		// Everything In Between
		"sw 	$t5, 0x0($a0)\n"
		
		"_exitDpadCamera:"
		"jr 	$ra\n"
		"nop\n"
	);






	//==================TRY 2:::::::
	// asm (
		// DOESNT GO TO 100 IF ON 78 >:(
	// 	// Load Pad 1
	// 	"lui	$t6, 0x001F\n"
	// 	"addiu 	$t6, $t6, 0xE682\n"
	// 	"lh	$t6, 0x0($t6)\n"
	// 	// Load Jokers
	// 	"addiu 	$t7, $zero, 0xFF7F\n" // Left
	// 	"addiu 	$t8, $zero, 0xFFDF\n" // Right
	// 	"addiu 	$t9, $zero, 0xBFFF\n" // Cross
	// 	// if d-pad Left, Subtract
	// 	"addiu 	$t5, $zero, -0x8\n"
	// 	"beq 	$t6, $t7, _DoNewSpeed\n"
	// 	// if d-pad Right, Add
	// 	"addiu 	$t5, $zero, 0x8\n"
	// 	"beq 	$t6, $t8, _DoNewSpeed\n"
	// 	//if Cross, Add
	// 	"addiu 	$t5, $zero, 0x8\n"
	// 	"beq 	$t6, $t9, _DoNewSpeed\n"


	// 	"_DoNewSpeed:"
	// 	"add 	$t5, $t5, $v0\n"
	// 	// if less than zero, save Max Speed
	// 	"addiu 	$t9, $zero, -0x1\n"
	// 	"slt	$t6, $t9, $t5\n"
	// 	"nop\n"
	// 	"beq 	$t6, $zero, _GoToMax\n"

	// 	"addiu 	$t9, $a2, 0x1\n"
	// 	"slt 	$t6, $t9, $t5\n"
	// 	"nop\n"
	// 	"bne 	$t6, $zero, _GoToZero\n"

	// 	"add	$t5, $zero, $t5\n"
	// 	"beq	$zero, $zero, _exitDpadCamera\n"

	// 	"_GoToMax:"
	// 	"add	$t5, $zero, $a2\n"
	// 	"beq	$zero, $zero, _exitDpadCamera\n"

	// 	"_GoToZero:"
	// 	"add	$t5, $zero, $zero\n"

	// 	// Everything Else!
	// 	"_exitDpadCamera:"
	// 	"sw 	$t5, 0x0($a0)\n"
	// 	"jr 	$ra\n"
	// );



	//====================TRY 3::::::::
	// asm (
	// "	lui	    $t6,0x001F;"
	// "	lhu	    $t6,-0x197E($t6);"
	// "	andi	$t7,$t6,0x0080;"
	// "   addiu	$t5,$zero,-0x8;"
	// "	beq	    $t7,$zero,_JUMPHERE;"
	// "	andi	$t7,$t6,0x4020;"
	// "	addiu	$t5,$zero,0x8;"
	// "	xori	$t6,$t7,0x4020;"
	// "	movz	$t5,$zero,$t6;"
	// "_JUMPHERE:"
	// "	addiu	$t8,$zero,0x40;"
	// "	addu	$t9,$v0,$t5;"
	// "	slti	$t1,$t9,0x0040;"
	// "	slt	    $t5,$zero,$t9;"
	// "	movz	$t9,$zero,$t5;"
	// "	xori	$t4,$t1,0x0000;"
	// "	movz	$t9,$t8,$t4;"
	// "   sw	    $t9,0x0($a0);"
	// "	jr	    $ra;"
	// );
}

int main(void)
{
	if (gameIsIn())
	{
		// Which Start Menu option is Selected
		int StartMenuSelection = *(u32*)0x0030DE40;
		// if Camera Speed Option is Selected, Patch Menu
		if (StartMenuSelection == 3 && *(u32*)0x00389CA8 != 0x00548F7C)
		{
			*(u32*)0x00389CA8 = 0x00548F7C; // d-pad Left
			*(u32*)0x00389CAC = 0x00548F7C; // d-pad Right
		}
		// if Camera Speed Option is not Selected, Unpatch Menu
		else if (StartMenuSelection != 3 && *(u32*)0x00389CA8 != 0x00548FDC)
		{
			*(u32*)0x00389CA8 = 0x00548FDC; // d-pad Left
			*(u32*)0x00389CAC = 0x00548FDC; // d-pad Right
		}
		// Patch Camera Speed Logic with mine.
		if (*(u32*)0x00561BCC == 0x14460003)
		{
			*(u32*)0x00561BCC = 0x0C000000 | ((u32)(&DPadCameraLogic) / 4);
			*(u32*)0x00561BD0 = 0;
			*(u32*)0x00561BD8 = 0;
		}
	}
	return 0;
}
