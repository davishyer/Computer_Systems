// diceLib.c	03/16/2014
//*******************************************************************************
//
#include "msp430.h"
#include <stdlib.h>
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "dice.h"

volatile int WDT_Sec_Cnt;				// WDT second counter
volatile int WDT_Delay;					// WDT delay counter
volatile int WDT_Tone_Cnt;				// WDT tone counter
volatile uint16 seconds;                // seconds

//------------------------------------------------------------------------------
// dot bit positions -----------------------------------------------------------
//
//  dot bit positions:  01    02
//                      04 08 10
//                      20    40
//
const uint8 dice[] = {0x08, 0x41, 0x49, 0x63, 0x6b, 0x77};

//------------------------------------------------------------------------------
// draw die --------------------------------------------------------------------
//
//	IN:		die = 1-6 new die
//			x = left most column of die (must be divisible by 3)
//			y = bottom row of die
//
void drawDie(uint8 die, int16 x, int16 y)
{
	uint16 die_bits = dice[(unsigned int)die - 1];
	lcd_wordImage(spot_image, x, y+32, ((die_bits & 0x01) ? 1 : 0));
	lcd_wordImage(spot_image, x+30, y+32, ((die_bits & 0x02) ? 1 : 0));
	lcd_wordImage(spot_image, x, y+16, ((die_bits & 0x04) ? 1 : 0));
	lcd_wordImage(spot_image, x+15, y+16, ((die_bits & 0x08) ? 1 : 0));
	lcd_wordImage(spot_image, x+30, y+16, ((die_bits & 0x10) ? 1 : 0));
	lcd_wordImage(spot_image, x, y, ((die_bits & 0x20) ? 1 : 0));
	lcd_wordImage(spot_image, x+30, y, ((die_bits & 0x40) ? 1 : 0));
	return;
} // end drawDie


//------------------------------------------------------------------------------
// configure Watchdog
int WDT_init()
{
	WDTCTL = WDT_CTL;					// Set Watchdog interval
	WDT_Sec_Cnt = WDT_1SEC_CNT;			// set WD 1 second counter
	WDT_Delay = 0;						// reset delay counter
	WDT_Tone_Cnt = 0;					// turn off tone
	seconds = 0;						// reset seconds
	IE1 |= WDTIE;						// enable WDT interrupt
	return 0;
} // end WDT_init


//------------------------------------------------------------------------------
// configure h/w PWM for speaker
int timerB_init()
{
	P4SEL |= 0x20;						// P4.5 TB2 output
	TBR = 0;							// reset timer B
	TBCTL = TBSSEL_2 | ID_0 | MC_1;		// SMCLK, /1, UP (no interrupts)
	TBCCTL2 = OUTMOD_3;					// TB2 = set/reset
	return 0;
} // end timerB_init


//------------------------------------------------------------------------------
// output tone subroutine ------------------------------------------------------
void doTone(uint16 tone, uint16 time)
{
	while (WDT_Tone_Cnt);				// wait for tone off
	TBCCR0 = tone;						// set beep frequency/duty cycle
	TBCCR2 = tone >> 1;					// 50% duty cycle
	WDT_Tone_Cnt = time;				// turn on speaker
	return;
} // end doTone


//------------------------------------------------------------------------------
// Watchdog delay subroutine ---------------------------------------------------
void WDT_delay(uint16 delay)
{
	if (delay <= 0) return;
	WDT_Delay = delay;					// set WD decrementer
	while (WDT_Delay);					// wait for time to expire
	return;
} // end WDT_delay()


//------------------------------------------------------------------------------
// Watchdog Timer ISR ----------------------------------------------------------
#pragma vector = WDT_VECTOR
__interrupt void WDT_ISR(void)
{
	// decrement interrupts/second counter
	if (--WDT_Sec_Cnt == 0)
	{
		WDT_Sec_Cnt = WDT_1SEC_CNT;		// reset WD 1 second counter
		P4OUT ^= 0x40;					// toggle red LED
		++seconds;						// increment second counter
	}

	// decrement delay (if non-zero)
	if (WDT_Delay && (--WDT_Delay == 0));

	// decrement tone counter - turn off tone when 0
	if (WDT_Tone_Cnt && (--WDT_Tone_Cnt == 0))
	{
		TBCCR0 = 0;
	}
} // end WDT_ISR(void)


//------------------------------------------------------------------------------
// const images ----------------------------------------------------------------
const uint16 spot_image[] = { 15, 15,	// 15 x 15 spot image
  0x01ff,0xf800,0xffdf,0x001f,0x01ff,0x01ff,0x03fe,0x01ff,0xf800,0x03fe,0x001f,
  0xffc0,0x03fe,0x07df,0xffc0,0x03fe,0x07df,0x05fe,0x05fe,0x05fe,0x05fe,0x05fe,
  0xffc0,0x03fe,0x07df,0xffc0,0x03fe,0x07df,0xf800,0x03fe,0x001f,0x01ff,0x03fe,
  0x01ff,0x01ff,0xf800,0xffdf,0x001f,0x01ff
};

const uint16 byu4_image[] = { 75, 46,	// 75 x 46 BYU logo image
  0x06ff,0x8400,0x04f0,0x8410,0x0010,0x02ff,0x8000,0x04f0,0x8410,0x0410,0x05ff,
  0x06ff,0xfc00,0x04fe,0x039f,0x02ff,0xfb80,0x04fe,0x041f,0x05ff,0x06ff,0xfc00,
  0x04f0,0x8410,0x041f,0x02ff,0xfc00,0x04f0,0x8410,0x041f,0x05ff,0x06ff,0xfc00,
  0x0010,0x02ff,0x8000,0x459f,0x02ff,0xd7d0,0x000e,0x02ff,0x8000,0x041f,0x05ff,
  0x06ff,0xfc00,0x0010,0x02ff,0x7000,0x87df,0x01ff,0x8000,0x859f,0x03ff,0x8000,
  0x041f,0x05ff,0x06ff,0xfc00,0x0010,0x03ff,0xffce,0x000e,0xd400,0x7416,0x03ff,
  0x8000,0x041f,0x05ff,0x06ff,0xfc00,0x0010,0x03ff,0xfc00,0x0390,0xfc0e,0x041a,
  0x03ff,0x8000,0x041f,0x05ff,0x06ff,0xfc00,0x0010,0x03ff,0xfb80,0x741f,0xd7d0,
  0x000e,0x03ff,0x8000,0x041f,0x05ff,0x06ff,0xfc00,0x0010,0x03ff,0x8000,0x841f,
  0x87d0,0x04ff,0x8000,0x041f,0x05ff,0x06ff,0xfc00,0x0010,0x03ff,0x8000,0xd7da,
  0x741f,0x04ff,0x8000,0x041f,0x05ff,0x06f0,0x8410,0xfc10,0x0010,0x04ff,0xb7d0,
  0x041f,0x04ff,0x8000,0x841f,0x05f0,0x8410,0x07fe,0x8410,0x0410,0x03ff,0xfc00,
  0x0010,0x03ff,0x8400,0x8410,0x05fe,0x87df,0x841f,0x03f0,0x8410,0x87d0,0x8410,
  0xfc10,0x01fe,0x77df,0x03ff,0x8380,0x000e,0x03ff,0xffce,0x01fe,0x841f,0x04f0,
  0x8410,0x87d0,0x041f,0x03ff,0x87d0,0x01ff,0x8380,0x8410,0x87d0,0x07ff,0x8000,
  0x87da,0x8410,0x0390,0x04ff,0x87d0,0x041f,0x03ff,0x87d0,0x03ff,0xffc0,0x000e,
  0x06ff,0x8380,0x739f,0x06ff,0x87d0,0x041f,0x03ff,0x87d0,0x03ff,0xfc00,0x0010,
  0x06ff,0xfc00,0x000e,0x06ff,0x87d0,0x041f,0x03ff,0x87d0,0x03ff,0xfc00,0x039f,
  0x06ff,0x87ce,0x07ff,0x87d0,0x041f,0x03ff,0x87d0,0x03ff,0xfc00,0x041f,0x05ff,
  0x8000,0x77d0,0x07ff,0x87d0,0x041f,0x03ff,0x87d0,0x03ff,0xfc00,0x87df,0x05ff,
  0x8380,0x87df,0x07ff,0x87d0,0x041f,0x03ff,0x87d0,0x03ff,0xfc00,0x87df,0x000e,
  0x04ff,0xfc00,0x049f,0x02ff,0x8380,0x7410,0x03ff,0x87d0,0x841f,0x0010,0x02ff,
  0x87d0,0x02ff,0x8000,0xfc10,0x01fe,0x0010,0x04ff,0xffce,0x041f,0x02ff,0xfb80,
  0x769f,0x02ff,0x8400,0x87d0,0x01fe,0x0010,0x02ff,0x87d0,0x02ff,0x8000,0x01fe,
  0xffd6,0x039a,0x03ff,0x7000,0xb7d0,0x069f,0x02ff,0xd380,0x759f,0x02ff,0xfc00,
  0x87df,0xfc10,0x0010,0x02ff,0x87d0,0x02ff,0x8000,0xfd9f,0x01fe,0x041f,0x03ff,
  0x8000,0xfd9f,0x069f,0x02ff,0xe380,0x77df,0x02ff,0xfc00,0x8410,0xfc00,0x0010,
  0x02ff,0x87d0,0x02ff,0x8000,0x01fe,0xb7d6,0x041f,0x03ff,0x8000,0x01fe,0x07d6,
  0x02ff,0xfb80,0x769a,0x02ff,0xfc00,0x0010,0xfc00,0x0010,0x02ff,0x87d0,0x02ff,
  0x8000,0xb7df,0x01fe,0x041f,0x03ff,0x8000,0xfd9f,0x859f,0x02ff,0x7380,0x738e,
  0x02ff,0xfc00,0x0010,0xfc00,0x0010,0x02ff,0x87d0,0x02ff,0x8000,0x01fe,0xfd9f,
  0x041f,0x03ff,0x8000,0x02fe,0x000e,0x05ff,0xfc00,0x0010,0xfc00,0x0010,0x02ff,
  0x87d0,0x02ff,0x8000,0xfd9f,0xffd6,0x041f,0x03ff,0x8000,0xb7df,0xfd9f,0x741a,
  0x05ff,0xfc00,0x0010,0xfc00,0x0010,0x02ff,0x87d0,0x02ff,0x8000,0x01fe,0xb7df,
  0x041f,0x03ff,0x8000,0x02fe,0x7412,0x05ff,0xfc00,0x0010,0xfc00,0x0010,0x02ff,
  0x87d0,0x02ff,0x8000,0xfd9f,0xffd6,0x041f,0x03ff,0x8000,0xfd9f,0x97d6,0x0390,
  0x05ff,0xfc00,0x0010,0xfc00,0x0010,0x02ff,0x87d0,0x02ff,0x8000,0x01fe,0xfd9f,
  0x041f,0x03ff,0x8000,0xffd6,0x77df,0x000e,0x01ff,0x8380,0x7410,0x02ff,0xfc00,
  0x0010,0xfc00,0x0010,0x02ff,0x87d0,0x02ff,0x8000,0xb7df,0x01fe,0x041f,0x03ff,
  0x8000,0x01fe,0x87d6,0x02ff,0xd380,0x769f,0x02ff,0xfc00,0x8410,0xfc00,0x0010,
  0x02ff,0x87d0,0x02ff,0x8000,0x01fe,0xfd9f,0x041f,0x03ff,0x8000,0x01fe,0x041f,
  0x02ff,0xfb80,0x77df,0x02ff,0xfc00,0x87df,0xfc00,0x0010,0x02ff,0x87d0,0x02ff,
  0x8000,0xb7df,0x01fe,0x041f,0x03ff,0x8000,0xb7df,0x041f,0x02ff,0xd380,0x7696,
  0x02ff,0x8400,0x87d0,0xfc00,0x0010,0x02ff,0x8410,0x02ff,0x8000,0xfd9f,0x01fe,
  0x041f,0x03ff,0x8000,0x01fe,0x041f,0x02ff,0x8380,0x7410,0x03ff,0x87d0,0xfc00,
  0x0010,0x05ff,0x8000,0x01fe,0x841f,0x0410,0x03ff,0x8000,0x8410,0x041f,0x07ff,
  0x87d0,0xfc00,0x0010,0x05ff,0x8000,0xfd9f,0x041f,0x05ff,0x8000,0x769f,0x07ff,
  0x87d0,0xfc00,0x0010,0x05ff,0x8000,0xb7df,0x041f,0x05ff,0x8000,0x87df,0x07ff,
  0x87d0,0xfc00,0x039a,0x05ff,0xb400,0x01fe,0x041f,0x05ff,0x8000,0x87df,0x0010,
  0x06ff,0x87d0,0xd000,0x841f,0x05ff,0xffd0,0xfd9f,0x041f,0x05ff,0x8000,0x01fe,
  0x0390,0x06ff,0x87d0,0x8000,0x87d6,0x0410,0x03ff,0x8400,0xb7df,0x01fe,0x041f,
  0x05ff,0x8000,0x01fe,0x841f,0x06ff,0x87d0,0x01ff,0xffd0,0x869f,0x03f0,0x8410,
  0xfe90,0x01fe,0xfd9f,0x041f,0x05ff,0x8000,0xb7df,0x01fe,0x06f0,0x8410,0x87d0,
  0x01ff,0xd400,0xffd6,0x03fe,0xb7df,0xfd9f,0x01fe,0x041f,0x05ff,0x8000,0x01fe,
  0xffd6,0x06fe,0x87df,0x02ff,0x07f0,0x8410,0x041f,0x05ff,0x8000,0x841f,0x08f0,
  0x8410,0x08ff,0x8000,0x841f,0x06f0,0x8410,0x041f,0x08ff,0x08ff,0x8000,0x07fe,
  0x041f,0x08ff,0x08ff,0x8000,0x07f0,0x8410,0x0410,0x08ff
};
