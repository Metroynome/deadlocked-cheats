#include <tamtypes.h>

int _InfiniteHealthMoonjump_Init = 0;
int UpdateHealthJAL = 0;
int main(void)
{
	void * GameplayFilePointer = (void*)(*(u32*)0x01FFFD00);
	if (GameplayFilePointer != 0x00574F88)
	{
        int Joker = *(u16*)0x00225982;
        if (Joker == 0xFDFB){
		    _InfiniteHealthMoonjump_Init = 1;
        }
        else if (Joker == 0xFFFD)
        {
            _InfiniteHealthMoonjump_Init = 0;
        }

        int PlayerStruct = (*(u32*)0x00225e70) - 0x430C;
        int PlayerFunctions = (u32)PlayerStruct + 0x14;
        int PlayerUpdateFunc = (*(u32*)PlayerFunctions) + 0x34;
        int UpdateHealth = ((*(u32*)PlayerUpdateFunc) + 0x68E8);
        int PlayerGravity = (u32)PlayerStruct + 0x128;
        if (_InfiniteHealthMoonjump_Init)
        {
            if (UpdateHealthJAL == 0)
                UpdateHealthJAL = *(u32*)UpdateHealth;
            
            if (*(u32*)UpdateHealth != 0)
                *(u32*)UpdateHealth = 0;

            if (Joker == 0xBFFF)
                *(float*)PlayerGravity = 0.125;
        }
        else
        {
            if (*(u32*)UpdateHealth == 0)
                *(u32*)UpdateHealth = UpdateHealthJAL;

            if (UpdateHealthJAL != 0)
                UpdateHealthJAL = 0;
        }
	}
    else
    {
        if (UpdateHealthJAL != 0)
            UpdateHealthJAL = 0;  
    }
	return 0;
}
