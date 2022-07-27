#include "types.h"
#include "x86.h"
#include "defs.h"
#include "kbd.h"

int
kbdgetc(void)
{
	static int promena;
	static uint shift;
	static uchar *charcode[4] = {
		normalmap, shiftmap, ctlmap, ctlmap
	};

	uint st, data, c;
	st = inb(KBSTATP);
	if((st & KBS_DIB) == 0)
		return -1;
	data = inb(KBDATAP);

	if(data == 0xE0){
		shift |= E0ESC;
		return 0;
	} else if(data & 0x80){
		// Key released
		data = (shift & E0ESC ? data : data & 0x7F);
		shift &= ~(shiftcode[data] | E0ESC);
		return 0;
	} else if(shift & E0ESC){
		// Last character was an E0 escape; or with 0x80
		data |= 0x80;
		shift &= ~E0ESC;
	}

	shift |= shiftcode[data];
	shift ^= togglecode[data];
	c = charcode[shift & (CTL | SHIFT)][data];
	if(shift & CAPSLOCK){
		if('a' <= c && c <= 'z')
			c += 'A' - 'a';
		else if('A' <= c && c <= 'Z')
			c += 'a' - 'A';
	}

	if(shift==5 && (c=='p' || c=='P') && promena!=1)
		postavi=1;
	if(shift==5 && (c=='c' || c=='C') && promena!=1)
		promena=1;
	else if(shift==5 && (c=='c' || c=='C'))
		promena=0;
	if(promena==1)
	{
		if(c==226 || c=='w')
		{
			//Gore
			ker=5;
		}
		else if(c==227 || c=='s')
		{
			//Dole
			ker=2;
		}
		else if(c==228 || c=='a')
		{
			//Levo
			ker=3;
		}
		else if(c==229 || c=='d')
		{
			//Desno
			ker=4;
		}
		else if(c=='q')
		{
			//Zapocni selekciju
			ker=6;
		}
		else if(c=='e')
		{
			//Prekini selekciju
			ker=7;
		}
		else
		{
			ker=1;
		}
	}
	else 
	{
		ker=0;
	}
	return c;
}

void
kbdintr(void)
{
	consoleintr(kbdgetc);
}
