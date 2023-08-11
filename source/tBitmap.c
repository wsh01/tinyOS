#include "tLib.h"

void tBitmapInit(tBitmap * bitmap)
{
	bitmap->bitmap = 0;
}

uint32_t tBitmapPosCount(void)
{
	return 32;
}

void tBitmapSet(tBitmap * bitmap, uint32_t pos)//��һ
{
	bitmap->bitmap |= 1 << pos;
}

void tBitmapClear(tBitmap * bitmap, uint32_t pos)//����
{
	bitmap->bitmap &= ~(1 << pos);
}

uint32_t tBitmapGetFirstSet(tBitmap * bitmap)//Ѱ�ҵ�һ����λ��λ�ã��ӵ�0λ��ʼ��
{
	static const uint8_t quickFindTable[] =									//����0000 0001��Ϊ0��0000 0010Ϊ1����0000 0011�ֱ��0����������
	{																												//���ǰ�32λ�ֳ�4�飬ÿ����16*16�ֿ��ܣ���������������ұߵ���һ
		0xff, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,		//ֵ��¼��������&0xFF�ķ�ʽ��  �����ֵ���ٲ��
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


