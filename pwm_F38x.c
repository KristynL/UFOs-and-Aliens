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

volatile unsigned char pwm_count=0;
volatile unsigned int speed;
volatile unsigned int num1;
volatile unsigned int num2;
volatile char dir; 
volatile unsigned int pass1;
volatile unsigned int pass2;
volatile unsigned int pass3;
volatile unsigned int turn1;
volatile unsigned int turn2;
volatile unsigned int turn3;

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

// Uses Timer3 to delay <us> micro-seconds. 
void Timer3us(unsigned char us)
{
	unsigned char i;               // usec counter
	
	// The input for Timer 3 is selected as SYSCLK by setting T3ML (bit 6) of CKCON:
	CKCON|=0b_0100_0000;
	
	TMR3RL = (-(SYSCLK)/1000000L); // Set Timer3 to overflow in 1us.
	TMR3 = TMR3RL;                 // Initialize Timer3 for first overflow
	
	TMR3CN = 0x04;                 // Sart Timer3 and clear overflow flag
	for (i = 0; i < us; i++)       // Count <us> overflows
	{
		while (!(TMR3CN & 0x80));  // Wait for overflow
		TMR3CN &= ~(0x80);         // Clear overflow indicator
	}
	TMR3CN = 0 ;                   // Stop Timer3 and clear overflow flag
}

void waitms (unsigned int ms)
{
	unsigned int j;
	unsigned char k;
	for(j=0; j<ms; j++)
		for (k=0; k<4; k++) Timer3us(250);
}

void locked (1){
printf("             .+ydmNMMMNmdyo-   \n");              
printf("          `+mMMmyo+++++oydMMNs`   \n");           
printf("         .mMN+`           `/mMN:    \n");         
printf("        `mMm`                hMM-     \n");       
printf("        /MM+                 .MMs       \n");     
printf("        +MM:                 `MMy         \n");   
printf("        +MM:                 `MMy          \n");  
printf("       `oMMy++/-`      `-/+oooMMy           \n");
printf("    :yNMMNmddmMMMdo..smMMMmmdmNMMmy:        \n"); 
printf("  :dMMh/.      .+dMMMMd+-  ./:../hMMd:      \n"); 
printf(" oMMh.            -dd-     ./+sdy-.hMMo     \n");
printf("/MMs                            :mo sMM/    \n");
printf("mMN               .::.           .N/ NMm     \n");
printf("NMh             `dMMMMd`          ds hMN     \n");
printf("dMN`            /MMMMMM/          h-`NMd     \n");
printf("-MMy             oMMMM+             yMM-     \n");
printf(" +MMy`           /MMMM-           `yMM+      \n");
printf("  :NMm:          hMMMMs          :mMN:       \n");
printf("   `yMMh-        dmmmmd        -hMMy`        \n");
printf("     -hMMh:                  :hMMh-          \n");
printf("       -yMMm+`            `+mMMy-            \n");
printf("         .sNMNs-        -sNMNs.              \n");
printf("            /hMMmo.  .omMMh/                 \n");
printf("              .omMMddMMmo.                   \n");
printf("                 .ommo.     \n");
                      
                      }
         
void unlocked(1){
}

void Timer2_ISR (void) interrupt 5
{
	TF2H = 0; // Clear Timer2 interrupt flag
	
	pwm_count++;
	if(pwm_count>100) pwm_count=0;
	
	//OUT0=pwm_count>num1?0:1;
	//OUT1=pwm_count>num1?1:0;
	
	OUT0=pwm_count>num1?0:1;
	OUT1=pwm_count>num2?0:1;
}

void main (void)
{
	
	
	printf("\x1b[2J"); // Clear screen using ANSI escape sequence.
	printf("Square wave generator for the F38x.\r\n"
	       "Check pins P2.0 and P2.1 with the oscilloscope.\r\n");
	
	locked(1);
	do {
	printf("Please enter a 3 number password (numbers between 1-100).\n");
	scanf("%d %d %d\n", &pass1, &pass2, &pass3);
	printf("LOCKED    \n");
	} while(pass1>100 || pass2>100 || pass3>100);
	
	do{
	
	printf("To unlock, please enter your password again.\n");
	scanf("%d %d %d\n", &turn1, &turn2, &turn3);
	}while(turn1!=pass1 || turn2!=pass2 || turn3!=pass3);
	
	if(turn1==pass1 && turn2==pass2 && turn3==pass3){
	  printf("UNLOCKED  \n");
	  
	  num1 = turn1;
	  num2 = 0;
	  
	  waitms(3000);
	  
	  num2 = turn2;
	  num1 = 0;
	  
	  waitms(3000);
	  
	  num1 = turn3;
	  num2 = 0;
	  
	  waitms(3000);
	
	}
	
	while(1)
	{
	  
	  printf("Please enter a direction: R for clockwise or L for counterclockwise.\n");
	  scanf("%s", &dir);
	  printf("Please enter a speed between 0 and 100.\n");
	  scanf("%d", &speed);  
	  
	  
	    //ask user for inputs until values are within range
	  while(speed>100){
	  	printf("Please enter a speed between 0 and 100.\n");
		scanf("%d", &speed); 
		}
	
	if(dir=='R'){
	  num1 = speed;
	  num2 = 0;
	  }
	else if(dir=='L'){
	  num2 = speed;
	  num1 = 0;
	 }
	  	
	
	}
}
