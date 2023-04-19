/* mipslabfunc.c*/

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */
#include <math.h>
#include <stdio.h>
#include <string.h>

/* Declare a helper function which is local to this file */
static void num32asc( char * s, int );

#define DISPLAY_CHANGE_TO_COMMAND_MODE (PORTFCLR = 0x10)
#define DISPLAY_CHANGE_TO_DATA_MODE (PORTFSET = 0x10)

#define DISPLAY_ACTIVATE_RESET (PORTGCLR = 0x200)
#define DISPLAY_DO_NOT_RESET (PORTGSET = 0x200)

#define DISPLAY_ACTIVATE_VDD (PORTFCLR = 0x40)
#define DISPLAY_ACTIVATE_VBAT (PORTFCLR = 0x20)

#define DISPLAY_TURN_OFF_VDD (PORTFSET = 0x40)
#define DISPLAY_TURN_OFF_VBAT (PORTFSET = 0x20)

int i;
int j;
int timeoutcount;


//Increment score
void gameScore (void) {
  	score++;
}

//Checks if the timer has elapsed, increment the timeoutcount if that's the case
int timerelapsed(int timeoutcount) {
    int is_timer_elapsed = (IFS(0) & 0x100)>>8;
    if ( is_timer_elapsed == 1){
       	timeoutcount++;
       	IFS(0) = IFS(0) & 0xfffffeff; //reset flag
    }
    return timeoutcount;
}

//Clear all text from display
void display_stringClear (void) {

	int i;
	for (i = 0; i < 4; i++){
		display_string(i, "");            // "" Behövs för att resna skrärmen av tidigare display_string
	}
}

//Pause the game, activated by an interrupt from SW4
void paus(void){
	display_string(0, "  Pause");
    display_string(1, "Press btn 4");
    display_string(2, "to resume")
    display_update();

	while(1){
		if (getbtns() == 4){ //Btn 4
			display_stringClear();
			return;
	}
	}

}

//Reset the player bird
void resetPos (void){
	PosX = 4;
	PosY = 16;
}

//Display the menu
void intro (void){
	display_string(0, "    LOPPAN");
	display_string(1, "  2  normal");
	display_string(2, "  3 nightmare");
	display_string(3, "  4 high score");
	display_update();
}

//Displays the score,
void display_score(int line, char *score, char *holder) {
	int i;
	if(line < 0 || line >= 4)
		return;
	if(!score)
		return;

	for(i = 0; i < 5; i++) {
		textbuffer[line][i] = ' ';
	}

	for(i = 5; i < 9; i++) {
		if(*score) {
			textbuffer[line][i] = *score;
			score++;
		} else {
			textbuffer[line][i] = ' ';
		}
	}

	textbuffer[line][9] = ' ';
	if (*holder) {
		textbuffer[line][10] = 'b';
		textbuffer[line][11] = 'y';
	} else {
		textbuffer[line][10] = ' ';
		textbuffer[line][11] = ' ';
	}
	textbuffer[line][12] = ' ';
	for(i = 13; i < 16; i++) {
		if(*holder) {
			textbuffer[line][i] = *holder;
			holder++;
		} else {
			textbuffer[line][i] = ' ';
		}
	}

}

//Displays the high score menu
void highscoremenu (void){
	display_string(0, "  Highscorelists");
	display_string(1, "  2  normal");
	display_string(2, "  3 nightmare");
	display_string(3, "  4   back");
	display_update();
	delay(250);
	while(1){
		int btns = getbtns();
		if(btns == 1){  // NORMAL
			display_string(0, "  High score:");

			for(j=1; j<4; j++) {
				display_score(j, itoaconv(highscore[j-1]), &highscoreHolder[(j-1)*3], j);
				display_update();
			}

		}

		if(btns == 2){ //Nightmare
			display_string(0, "  High score:");

			for(j=1; j<4; j++) {
				display_score(j, itoaconv(nhighscore[j-1]), &nhighscoreHolder[(j-1)*3], j);
				display_update();
			}
		}

		if(btns == 4) {
			intro();
			return;
		}
	}
}

//Countdown, when a game is started
void countDown (void) {
    display_stringClear();
	display_string(1, "     Ready");
	display_update();
	delay( 1000 );

    display_string(2, "      Set");
    display_update();
	delay( 1000 );

	display_string(3, "      Go");
	display_update();
	delay( 1000);

	display_update();
	display_stringClear();



}

//Draw a pixel on the x,y coordinate on the display
void drawPixel (int x, int y){
if(x<128){

	if(y<0 | x<0){ //När rören passerar skärmen
		x= -90;
		y= -90;
	}

	if(y>= 8 && y<16){
		y=y-8;
		x = x + 128;

	}

	if(y>= 16 && y<24){
		y=y-16;
		x = x +128*2;
	}

	if(y>= 24 && y<32){
		y=y-24;
		x = x +128*3;
	}

	if(y==0){
		icon[x] = icon[x] & ~1;
	}

	if(y>0 && y<8) { //  i rätt page, kolumn y=0 redan täckt fall
			icon[x] = icon[x] & ~(1 << y);

	}
}
}

//Same functionality as drawPixel, but checks that the pixel it writes to isn't already white. Game over if it is
void draw_Check(int x, int y){

	if(y>= 8 && y<16){
		y=y-8;
		x = x + 128;
	}

	if(y>= 16 && y<24){
		y=y-16;
		x = x +128*2;
	}

	if(y>= 24 && y<32){
		y=y-24;
		x = x +128*3;
	}

	if(y==0){
		if ((icon[x] & 1) == 0){
			game_state = 2;
		}
		icon[x] = icon[x] & ~1;
		}

	if(y>0 && y<8) { // i rätt page, kolumn y=0 redan täckt fall
		for(i=1; i<8; i++){
            if(y==i){
				if ((icon[x] & (1 << i)) == 0){ // kollar om biten var vit sedan innan. I så fall krock.
							game_state = 2;
				}
				icon[x] = icon[x] & ~(1 << i);
			}
		}
	}
}

//Clear the entire screen buffer
void clearScreen(void){
	int i = 0;
	for(i = 0; i<512;i++){
		icon[i] = 255;
	}
}

//Draw a tube
void drawTube(int x, int y){
	int n;
	int k;
	int z;
	int y2 = y+length+space;
	for(n = 0; n<width; n++){ //tak
		drawPixel(x+n,y);
		drawPixel(x+n,y2);
	}
	for(k = 0; k<length; k++){
		drawPixel(x,y+k);
		drawPixel(x+width, y+k);
		drawPixel(x,y2+k);
		drawPixel(x+width, y2+k);
	}
	drawPixel(x+width, y+length);
	drawPixel(x+width, y2+length);

	for(z = 0; z<width; z++){ //golv
		drawPixel(x+z,y+length);
		drawPixel(x+z,y2+length);
	}
}

//Draw a bird
void drawBird(int PosX, int PosY){
	draw_Check(PosX+1,PosY);
	draw_Check(PosX, PosY+1);
	draw_Check(PosX,PosY);
	draw_Check(PosX+1, PosY+1);
}

//Draw the top and bottom line
void drawTopAndBottomLine(void){
	int i;
	for(i = 0; i < 128; i++){
		drawPixel(i, 0);
		drawPixel(i, 31);
	}
}

void intswap (int *a, int *b) {
	int t;
	t = *b;
	*b = *a;
	*a = t;
}








/* quicksleep:
   A simple function to create a small delay.
   Very inefficient use of computing resources,
   but very handy in some special cases. */
void quicksleep(int cyc) {
	int i;
	for(i = cyc; i > 0; i--);
}

/* tick:
   Add 1 to time in memory, at location pointed to by parameter.
   Time is stored as 4 pairs of 2 NBCD-digits.
   1st pair (most significant byte) counts days.
   2nd pair counts hours.
   3rd pair counts minutes.
   4th pair (least significant byte) counts seconds.
   In most labs, only the 3rd and 4th pairs are used. */
void tick( unsigned int * timep )
{
  /* Get current value, store locally */
  register unsigned int t = * timep;
  t += 1; /* Increment local copy */

  /* If result was not a valid BCD-coded time, adjust now */

  if( (t & 0x0000000f) >= 0x0000000a ) t += 0x00000006;
  if( (t & 0x000000f0) >= 0x00000060 ) t += 0x000000a0;
  /* Seconds are now OK */

  if( (t & 0x00000f00) >= 0x00000a00 ) t += 0x00000600;
  if( (t & 0x0000f000) >= 0x00006000 ) t += 0x0000a000;
  /* Minutes are now OK */

  if( (t & 0x000f0000) >= 0x000a0000 ) t += 0x00060000;
  if( (t & 0x00ff0000) >= 0x00240000 ) t += 0x00dc0000;
  /* Hours are now OK */

  if( (t & 0x0f000000) >= 0x0a000000 ) t += 0x06000000;
  if( (t & 0xf0000000) >= 0xa0000000 ) t = 0;
  /* Days are now OK */

  * timep = t; /* Store new value */
}

/* display_debug
   A function to help debugging.

   After calling display_debug,
   the two middle lines of the display show
   an address and its current contents.

   There's one parameter: the address to read and display.

   Note: When you use this function, you should comment out any
   repeated calls to display_image; display_image overwrites
   about half of the digits shown by display_debug.
*/
void display_debug( volatile int * const addr )
{
  display_string( 1, "Addr" );
  display_string( 2, "Data" );
  num32asc( &textbuffer[1][6], (int) addr );
  num32asc( &textbuffer[2][6], *addr );
  display_update();
}

// Skickar en byte med data till chipkittets grafikkrets
uint8_t spi_send_recv(uint8_t data) {
	while(!(SPI2STAT & 0x08));
	SPI2BUF = data;
	while(!(SPI2STAT & 1));
	return SPI2BUF;
}

void display_init(void) {
        DISPLAY_CHANGE_TO_COMMAND_MODE;
	quicksleep(10);
	DISPLAY_ACTIVATE_VDD;
	quicksleep(1000000);

	spi_send_recv(0xAE);
	DISPLAY_ACTIVATE_RESET;
	quicksleep(10);
	DISPLAY_DO_NOT_RESET;
	quicksleep(10);

	spi_send_recv(0x8D);
	spi_send_recv(0x14);

	spi_send_recv(0xD9);
	spi_send_recv(0xF1);

	DISPLAY_ACTIVATE_VBAT;
	quicksleep(10000000);

	spi_send_recv(0xA1);
	spi_send_recv(0xC8);

	spi_send_recv(0xDA);
	spi_send_recv(0x20);

	spi_send_recv(0xAF);
}

void display_string(int line, char *s) {
	int i;
	if(line < 0 || line >= 4)
		return;
	if(!s)
		return;

	for(i = 0; i < 16; i++)
		if(*s) {
			textbuffer[line][i] = *s;
			s++;
		} else
			textbuffer[line][i] = ' ';
}

void display_image(int x, const uint8_t *data) {
	int i, j;

	for(i = 0; i < 4; i++) {
		DISPLAY_CHANGE_TO_COMMAND_MODE;

		spi_send_recv(0x22); // 0x22 är 'set page number' indikerar vilken pagebuffer som ska fyllas med data
		spi_send_recv(i);

    //Skicka två bytes för att indikera positionering
		spi_send_recv(x & 0xF);
		spi_send_recv(0x10 | ((x >> 4) & 0xF));

		DISPLAY_CHANGE_TO_DATA_MODE;

    //Fyller upp pagebuffern med data
		for(j = 0; j < 128; j++)
			spi_send_recv(~data[i*128 + j]);
	}
}

void display_update(void) {
	int i, j, k;
	int c;
	for(i = 0; i < 4; i++) {
		DISPLAY_CHANGE_TO_COMMAND_MODE;
		spi_send_recv(0x22);
		spi_send_recv(i);

		spi_send_recv(0x0);
		spi_send_recv(0x10);

		DISPLAY_CHANGE_TO_DATA_MODE;

		for(j = 0; j < 16; j++) {
			c = textbuffer[i][j];
			if(c & 0x80)
				continue; //continues with the next iteration

			for(k = 0; k < 8; k++)
				spi_send_recv(font[c*8 + k]);
		}
	}
}

/* Helper function, local to this file.
   Converts a number to hexadecimal ASCII digits. */
static void num32asc( char * s, int n )
{
  int i;
  for( i = 28; i >= 0; i -= 4 )
    *s++ = "0123456789ABCDEF"[ (n >> i) & 15 ];
}

/*
 * nextprime
 *
 * Return the first prime number larger than the integer
 * given as a parameter. The integer must be positive.
 */
#define PRIME_FALSE   0     /* Constant to help readability. */
#define PRIME_TRUE    1     /* Constant to help readability. */
int nextprime( int inval )
{
   register int perhapsprime = 0; /* Holds a tentative prime while we check it. */
   register int testfactor; /* Holds various factors for which we test perhapsprime. */
   register int found;      /* Flag, false until we find a prime. */

   if (inval < 3 )          /* Initial sanity check of parameter. */
   {
     if(inval <= 0) return(1);  /* Return 1 for zero or negative input. */
     if(inval == 1) return(2);  /* Easy special case. */
     if(inval == 2) return(3);  /* Easy special case. */
   }
   else
   {
     /* Testing an even number for primeness is pointless, since
      * all even numbers are divisible by 2. Therefore, we make sure
      * that perhapsprime is larger than the parameter, and odd. */
     perhapsprime = ( inval + 1 ) | 1 ;
   }
   /* While prime not found, loop. */
   for( found = PRIME_FALSE; found != PRIME_TRUE; perhapsprime += 2 )
   {
     /* Check factors from 3 up to perhapsprime/2. */
     for( testfactor = 3; testfactor <= (perhapsprime >> 1) + 1; testfactor += 1 )
     {
       found = PRIME_TRUE;      /* Assume we will find a prime. */
       if( (perhapsprime % testfactor) == 0 ) /* If testfactor divides perhapsprime... */
       {
         found = PRIME_FALSE;   /* ...then, perhapsprime was non-prime. */
         goto check_next_prime; /* Break the inner loop, go test a new perhapsprime. */
       }
     }
     check_next_prime:;         /* This label is used to break the inner loop. */
     if( found == PRIME_TRUE )  /* If the loop ended normally, we found a prime. */
     {
       return( perhapsprime );  /* Return the prime we found. */
     }
   }
   return( perhapsprime );      /* When the loop ends, perhapsprime is a real prime. */
}

/*
 * itoa
 *
 * Simple conversion routine
 * Converts binary to decimal numbers
 * Returns pointer to (static) char array
 *
 * The integer argument is converted to a string
 * of digits representing the integer in decimal format.
 * The integer is considered signed, and a minus-sign
 * precedes the string of digits if the number is
 * negative.
 *
 * This routine will return a varying number of digits, from
 * one digit (for integers in the range 0 through 9) and up to
 * 10 digits and a leading minus-sign (for the largest negative
 * 32-bit integers).
 *
 * If the integer has the special value
 * 100000...0 (that's 31 zeros), the number cannot be
 * negated. We check for this, and treat this as a special case.
 * If the integer has any other value, the sign is saved separately.
 *
 * If the integer is negative, it is then converted to
 * its positive counterpart. We then use the positive
 * absolute value for conversion.
 *
 * Conversion produces the least-significant digits first,
 * which is the reverse of the order in which we wish to
 * print the digits. We therefore store all digits in a buffer,
 * in ASCII form.
 *
 * To avoid a separate step for reversing the contents of the buffer,
 * the buffer is initialized with an end-of-string marker at the
 * very end of the buffer. The digits produced by conversion are then
 * stored right-to-left in the buffer: starting with the position
 * immediately before the end-of-string marker and proceeding towards
 * the beginning of the buffer.
 *
 * For this to work, the buffer size must of course be big enough
 * to hold the decimal representation of the largest possible integer,
 * and the minus sign, and the trailing end-of-string marker.
 * The value 24 for ITOA_BUFSIZ was selected to allow conversion of
 * 64-bit quantities; however, the size of an int on your current compiler
 * may not allow this straight away.
 */
#define ITOA_BUFSIZ ( 24 )
char * itoaconv( int num )
{
  register int i, sign;
  static char itoa_buffer[ ITOA_BUFSIZ ];
  static const char maxneg[] = "-2147483648";

  itoa_buffer[ ITOA_BUFSIZ - 1 ] = 0;   /* Insert the end-of-string marker. */
  sign = num;                           /* Save sign. */
  if( num < 0 && num - 1 > 0 )          /* Check for most negative integer */
  {
    for( i = 0; i < sizeof( maxneg ); i += 1 )
    itoa_buffer[ i + 1 ] = maxneg[ i ];
    i = 0;
  }
  else
  {
    if( num < 0 ) num = -num;           /* Make number positive. */
    i = ITOA_BUFSIZ - 2;                /* Location for first ASCII digit. */
    do {
      itoa_buffer[ i ] = num % 10 + '0';/* Insert next digit. */
      num = num / 10;                   /* Remove digit from number. */
      i -= 1;                           /* Move index to next empty position. */
    } while( num > 0 );
    if( sign < 0 )
    {
      itoa_buffer[ i ] = '-';
      i -= 1;
    }
  }
  /* Since the loop always sets the index i to the next empty position,
   * we must add 1 in order to return a pointer to the first occupied position. */
  return( &itoa_buffer[ i + 1 ] );
}
