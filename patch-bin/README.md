# Patch.bin subroutine.
## Patche.bin Settings:
if 0x000FFFFF value equals following:
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
	
	The following are not added in CheatDevice.txt:
	- Offset + 0x09: Form Party and Unkick
	- Offset + 0x0a: Max Typing Limit
	- Offset + 0x0b: More Team Colors
	- Offset + 0x0c: Infinite Chargeboot
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
	- Offset + 0x1f: Cheats Menu - End Game
	- Offset + 0x20: Cheats Menu - Fusion Aimer