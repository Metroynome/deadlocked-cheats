# Patch.bin subroutine.
## Patche.bin Settings:
if 0x01e0efff value equals following:
 - 0x00 = all codes on, no secondary
 - 0x01 = all codes off, no secondary
 - 0x02 = all codes on, use secondary codes (Automatically off)
 - 0x03 = all codes off, use secondary codes (Automatically off)

Turn all Codes On/Off: L1 + L2 + R2 + Right + Down
Turn all Secondary Codes On/Off: L1 + L2 + R2 + Left + Up

 - Starts at 01E00000
 - Offset variable: int CodesArea
    - Offset + 0x00: Infinite Health/Moonjump
	- Offset + 0x01: Mask Username
	- Offset + 0x02: Hacked Keyboard
	- Offset + 0x03: Free Cam
	- Offset + 0x04: sp-music-to-mp
	- Offset + 0x05: Follow Aimer
	- Offset + 0x06: Force G^
	- Offset + 0x07: Host Options
	- Offset + 0x08: Vehicle Select
	- Offset + 0x09: Form Party and Unkick
	- Offset + 0x0a: Max Typing Limit
	- Offset + 0x0b: More Team Colors
	- Offset + 0x0c: Infinite Chargeboot
		- If 0x01e0ef9f == 0x1, Joker is L2 + L1
	- Offset + 0x0d: Render All
	- Offset + 0x0e: Rapid Fire Weapons
	- Offset + 0x0f: Walk Through Walls
	- Offset + 0x10: Rapid Fire Vehicles
	- Offset + 0x11: Lots of Deaths
	- Offset + 0x12: No Respawn Timer
	- Offset + 0x13: Walk Fast
	- Offset + 0x14: AirWalk
	- Offset + 0x15: Flying Vehicles
	- Offset + 0x16: Surfing Vehicles
	- Offset + 0x17: Fast Vehicles
	- Offset + 0x18: Respawn Anywhere
	- Offset + 0x19: vSync
	- Offset + 0x1a: All Alpha/Omega Mods
	- Offset + 0x1b: All Skill Points
	- Offset + 0x1c: Hacked Start Menu
	- Offset + 0x1d: Cheats Menu - Weapons
	- Offset + 0x1e: Lock On Fusion
	- Offset + 0x1f: Cheats Menu - NEW GAME
	- Offset + 0x20: Cheats Menu - Fusion Aimer











================================

SETTING UP
----------

Patch.bin:
----------
Download the patch.bin file.  It contains the main code needed to run.

The "patch.bin" file replaces your current patch.bin file in your Ratchet: Deadlocked save.

It's location: mc0:/BASCUS-97465RATCHET/


OPL Cheat File
--------------
The OPL Cheat file is named "SCUS_974.65.cht"

You can edit this in Notepad or any other text edit program.

This file contains the patch.bin settings listed below.

This file is optional, but if not there, all the codes will be loaded and ran.

The file lets you choose if you want a to have a secondary list aside from the main list.


Once you set up the cheat file how you would like it, copy it to where your OPL settings are.

The file is located in your main OPL Folder in a folder called CHT
	` - mc0:/OPL/CHT
	 - mc1:/OPL/CHT
	 - mass:/OPL/CHT or mass:/CHT
	 - hdd0:/+OPL/CHT`
The file may not be already there.

How to Enable Cheats in OPL
----------------------------
1. Start OPL
2. Scroll to your Ratchet: Deadlocked game and press Triangle
3. Go to Cheat Settings
4. Set "Settings Mode" to Per Game
5. Set "Enable PS2RD Cheat Engine" to "On"
6. Leave "PS2RD Cheat Engine Mode" alone.
7. Press "Ok"
8. Press "Save Changes"
7. Load your game


ONCE GAME IS STARTED
--------------------
When you start the game, go to Multiplayer.  This is when the patch.bin file loads.

Once Multiplayer is loaded, you're cheats will be activated.

You can turn the codes On/Off by pressing: L1 + L2 + R2 + Right + Down

If you use the secondary list, you can turn those On/Off by pressing: L1 + L2 + R2 + Left + Up

================================

Patch.bin Code Setting
-----------------------
`0 = All Codes On by default
1 = All Codes Off by default
2 = All codes On by default
	- Use Secondary Codes (Automatically off)
3 = All Codes Off by defult
    - Use Secondary Codes (Automatically off)

Main Code Setting
01e0efff 00000000`

===============================

 - Turn all codes On/Off: L1 + L2 + R2 + Right + Down
 - Turn all Secondary Codes On/Off: L1 + L2 + R2 + Left + Up

===============================

SECONDARY CODES
----------------
If Main Setting is set to 2 or 3, the following codes can be selected to
turn on via the Secondary Codes joker command: L1 + L2 + R2 + Left + Up

The Secondary code list can't turn on unless the main code list is activated first.

The following is for setting which code list each code is on.

`0 = Main Code List
1 = Secondary Code List
2 = Disable Code (Also works if Main Setting is set to 0 or 1)

Infinite Health/Moonjump (R3 + R2 = On/L3 = Off)
01e0efa0 00000000
Mask Username (L2 + R2 + Select)
01e0efa1 00000000
Hacked Keyboard (Start + Select)
01e0efa2 00000000
Free Cam (L1 + R1 + L3/L1 + R1 + R3)
01e0efa3 00000000
sp-music-to-mp
01e0efa4 00000000
Follow Aimer ({L3} + {R1} or {L3} + {L2})
01e0efa5 00000000
Force G^ (L3 + R3 + L1/L3 + R3 + L2)
01e0efa6 00000000
Host Options (L2 + X: Change Team, Start: Ready Player)
01e0efa7 00000000
Vehicle Select
01e0efa8 00000000
Form Party and Unkick
01e0efa9 00000000
Max Typing Limit (L1 + R1)
01e0efaa 00000000
More Team Colors (L2 + R2)
01e0efab 00000000
Infinite Chargeboot (Hold L2)
01e0efac 00000000
Render All (Select + Left/Select + Right)
01e0efad 00000000
Rapid Fire Weapons (R3 + R1 or R3 + R1 + L2)
01e0efae 00000000
Walk Through Walls (L1 + Left/L1 + Right)
01e0efaf 00000000
Rapid Fire Vehicles (Hold R3 + R1)
01e0efb0 00000000
Lots of Deaths (L1 + Up)
01e0efb1 00000000
No Respawn Timer (Press X)
01e0efb2 00000000
Walk Fast (R3 + Left/R3 + Right)
01e0efb3 00000000
AirWalk (Hold L3)
01e0efb4 00000000
Flying Vehicles (Hold L3: High; or Hold R3: Float)
01e0efb5 00000000
Surfing Vehicles (L2 + Up/L2 + Down)
01e0efb6 00000000
Fast Vehicles (L2 + R2: Fast; L2 + R1: Faster)
01e0efb7 00000000
Respawn Anywhere (Circle + Square)
01e0efb8 00000000
vSync (R3 + Up/ R3 + Down)
01e0efb9 00000000
All Alpha/Omega Mods
01e0efba 00000000
All Skill Points
01e0efbb 00000000
Hacked Start Menu
01e0efbc 00000000
Cheats Menu - Weapons
01e0efbd 00000000
Lock On Fusion (R2 + Up/R2 + Down)
01e0efbe 00000000
Cheats Menu - New Game
01e0efbf 00000000
Cheats Menu - Fusion Aimer
01e0efc0 00000000`

===============================

Misc Codes
----------------
`Infinite Chargeboot Set to L2 + L1
01e0ef9f 00000000`