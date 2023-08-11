#include "tLib.h"

void tBitmapInit(tBitmap * bitmap)
{
	bitmap->bitmap = 0;
}

uint32_t tBitmapPosCount(void)
{
	return 32;
}

void tBitmapSet(tBitmap * bitmap, uint32_t pos)//ÖÃÒ»
{
	bitmap->bitmap |= 1 << pos;
}

void tBitmapClear(tBitmap * bitmap, uint32_t pos)//ÖÃÁã
{
	bitmap->bitmap &= ~(1 << pos);
}

uint32_t tBitmapGetFirstSet(tBitmap * bitmap)//Ñ°ÕÒµÚÒ»¸öÖÃÎ»µÄÎ»ÖÃ£¨´ÓµÚ0Î»¿ªÊ¼£©
{
	static const uint8_t quickFindTable[] =									//ÁÐÈç0000 0001ÔòÎª0£¬0000 0010Îª1£¬µ«0000 0011ÓÖ±ä³É0£¬ÒÔÏÂÀàÍÆ
	{																												//¾ÍÊÇ°Ñ32Î»·Ö³É4×é£¬Ã¿×éÓÐ16*16ÖÖ¿ÉÄÜ£¬ÓÃÊý×é°ÑËüÃÇ×îÓÒ±ßµÄÖÃÒ»
		0xff, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,		//Öµ¼ÇÂ¼ÆðÀ´£¬ÓÃ&0xFFµÄ·½Ê½Çó³  öµ¥×éÖµ£¬ÔÙ²é±í
		4,	  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		5, 	 	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4, 		0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		6, 		0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4, 		0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		5, 		0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4, 		0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		7, 		0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4, 		0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		5, 		0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4, 		0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		6, 		0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4, 		0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		5,	 	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
		4, 		0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 };
	if(bitmap->bitmap & 0xFF)
	{
		return quickFindTable[bitmap->bitmap & 0xFF];
	}
	else if (bitmap->bitmap & 0xFF00)
	{
		return quickFindTable[(bitmap->bitmap >> 8) & 0xFF] + 8;
	}
	else if (bitmap->bitmap & 0xFF0000)
	{
		return quickFindTable[(bitmap->bitmap >> 16) & 0xFF] + 16;
	}
	else if (bitmap->bitmap & 0xFF000000)
	{
		return quickFindTable[(bitmap->bitmap >> 24) & 0xFF] + 24;
	}
	else 
	{
		return tBitmapPosCount();
	}
}


