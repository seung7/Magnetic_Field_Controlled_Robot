//  square.c: Uses timer 2 interrupt to generate a square wave in pin
//  P2.0 and a 75% duty cycle wave in pin P2.1
//  Copyright (c) 2010-2015 Jesus Calvino-Fraga
//  ~C51~

#include <C8051F38x.h>
#include <stdlib.h>
#include <stdio.h>

#define SYSCLK    48000000L // SYSCLK frequency in Hz
#define BAUDRATE  115200L   // Baud rate of UART in bps

#define OUT0 P2_0
#define OUT1 P2_1
#define OUT2 P2_2
#define OUT3 P2_3

#define CURSOR_ON "\x1b[?25h"
#define CURSOR_OFF "\x1b[?25l"
#define CLEAR_SCREEN "\x1b[2J"
#define GOTO_YX "\x1B[%d;%dH"
#define CLR_TO_END_LINE "\x1B[K"


volatile unsigned char pwm_count=0;
volatile unsigned char proceed;
volatile unsigned int PWM_1=0;
volatile unsigned int PWM_2=0;
volatile unsigned int PWM_3=0;
volatile unsigned int PWM_4=0;

volatile int DRV = 0;
volatile int MODE = 0;
volatile int turningdirection = 0;


char _c51_external_startup (void)
{
	PCA0MD&=(~0x40) ;    // DISABLE WDT: clear Watchdog Enable bit
	VDM0CN=0x80; // enable VDD monitor
	RSTSRC=0x02|0x04; // Enable reset on missing clock detector and VDD

	// CLKSEL&=0b_1111_1000; // Not needed because CLKSEL==0 after reset
	#if (SYSCLK == 12000000L)
		//CLKSEL|=0b_0000_0000;  // SYSCLK derived from the Internal High-Frequency Oscillator / 4 
	#elif (SYSCLK == 24000000L)
		CLKSEL|=0b_0000_0010; // SYSCLK derived from the Internal High-Frequency Oscillator / 2.
	#elif (SYSCLK == 48000000L)
		CLKSEL|=0b_0000_0011; // SYSCLK derived from the Internal High-Frequency Oscillator / 1.
	#else
		#error SYSCLK must be either 12000000L, 24000000L, or 48000000L
	#endif
	OSCICN |= 0x03; // Configure internal oscillator for its maximum frequency

	// Configure UART0
	SCON0 = 0x10; 
#if (SYSCLK/BAUDRATE/2L/256L < 1)
	TH1 = 0x10000-((SYSCLK/BAUDRATE)/2L);
	CKCON &= ~0x0B;                  // T1M = 1; SCA1:0 = xx
	CKCON |=  0x08;
#elif (SYSCLK/BAUDRATE/2L/256L < 4)
	TH1 = 0x10000-(SYSCLK/BAUDRATE/2L/4L);
	CKCON &= ~0x0B; // T1M = 0; SCA1:0 = 01                  
	CKCON |=  0x01;
#elif (SYSCLK/BAUDRATE/2L/256L < 12)
	TH1 = 0x10000-(SYSCLK/BAUDRATE/2L/12L);
	CKCON &= ~0x0B; // T1M = 0; SCA1:0 = 00
#else
	TH1 = 0x10000-(SYSCLK/BAUDRATE/2/48);
	CKCON &= ~0x0B; // T1M = 0; SCA1:0 = 10
	CKCON |=  0x02;
#endif
	TL1 = TH1;      // Init Timer1
	TMOD &= ~0xf0;  // TMOD: timer 1 in 8-bit autoreload
	TMOD |=  0x20;                       
	TR1 = 1; // START Timer1
	TI = 1;  // Indicate TX0 ready
	
	// Configure the pins used for square output
	P2MDOUT|=0b_0000_0011;
	P0MDOUT |= 0x10; // Enable UTX as push-pull output
	XBR0     = 0x01; // Enable UART on P0.4(TX) and P0.5(RX)                     
	XBR1     = 0x40; // Enable crossbar and weak pull-ups

	// Initialize timer 2 for periodic interrupts
	TMR2CN=0x00;   // Stop Timer2; Clear TF2;
	CKCON|=0b_0001_0000;
	TMR2RL=(-(SYSCLK/(2*48))/(100L)); // Initialize reload value
	TMR2=0xffff;   // Set to reload immediately
	ET2=1;         // Enable Timer2 interrupts
	TR2=1;         // Start Timer2

	EA=1; // Enable interrupts
	
	return 0;
}


void Timer2_ISR (void) interrupt 5
{
	TF2H = 0; // Clear Timer2 interrupt flag
	
	pwm_count++;
	if(pwm_count>100) pwm_count=0;
	
	OUT0=pwm_count>PWM_1?0:1;
	OUT1=pwm_count>PWM_2?0:1;
	OUT2=pwm_count>PWM_3?0:1;
	OUT3=pwm_count>PWM_4?0:1;
}


void main (void)
{
	printf("\x1b[2J"); // Clear screen using ANSI escape sequence.

	while(1)
	{
	PWM_1 = 0;
	PWM_2 = 0;
	PWM_3 = 0;
	PWM_4 = 0;
	printf(GOTO_YX, 1,1);
	printf("\rto go back and forth press '1', to turn press '2':\n");
	scanf("%d", &MODE);
	if (MODE==2)
	{
	printf("\rto turn right press '1', to turn left press '2':\n");
	scanf("%d", &turningdirection);
		
		if(turningdirection==1)
		{
		 PWM_1 = 0;
		 PWM_2 = 50;
		 PWM_3 = 50;
		 PWM_4 = 0;
		 }
		 
		else if (turningdirection==2)
		{PWM_1 = 50;
		 PWM_2 = 0;
		 PWM_3 = 0;
		 PWM_4 = 50;
		 }
		
		else
		{
		PWM_1 = 0;
		PWM_2 = 0;
		PWM_3 = 0;
		PWM_4 = 0;
		}
		printf("\nType '0' and press enter to end turning: ");
		scanf("%s", &proceed);
		printf("\x1b[2J");
		
	
	}
	else if (MODE==1)
	{
		printf("\rPlease Input PWM Range 100 to -100 (positive-forward,negative-backward): ");
		scanf("%d", &DRV);
		PWM_1=0;
		PWM_2=0;
		PWM_3=0;
		PWM_4=0;
		
		if(DRV < -100 || DRV > 100)
			printf("Value Not Within Range");
		else if(DRV > 0)
		{
			PWM_1 = 0;
			PWM_2 = DRV;
			PWM_3 = 0;
			PWM_4 = DRV;
	
		

		
		}
		else if(DRV < 0)
		
		{
			PWM_1 = -DRV;
			PWM_2 = 0;
			PWM_3 = -DRV;
			PWM_4 = 0;
		
		}
		else
		{
			PWM_1 = 0;
			PWM_2 = 0;
			PWM_3 = 0;
			PWM_4 = 0;
		}
	
	
		printf("\nType '0' and press enter to end PWM: ");
		scanf("%s", &proceed);
		printf("\x1b[2J");
	}
	}
}
