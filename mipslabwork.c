/* mipslabwork.c*/

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Declare all variables as global here
int prime = 1234567;
int PosX = 4;
int PosY = 16;
int length = 20;  //Length
int width = 8; //width
int space = 16; // Mellanrum
int game_state = 1;
int score =0;
int highscore[3] = {0,0,0};
char highscoreHolder[9];
int nhighscore[3] = {0,0,0};
char nhighscoreHolder[9];
int modify = 0;
int antal = 0;
int dummy;
int timedelay = 18;

volatile int* portE = (volatile int*) 0xbf886110; //initializing pointer for lights

volatile int* IEC0 = (volatile int*) 0xbf881060;
volatile int* IPC2 = (volatile int*) 0xbf8810b0;

int timeoutcount = 0;

char holder[3];
char tempHolder[3];
int tempScore;


char alphabet[27] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
int ptr1 = 0;
int ptr2 = 0;
int ptr3 = 0;
int tempcount = 0;
int f = 0;

/* Interrupt Service Routine */
void user_isr( void )
{
  int is_timer_elapsed = (IFS(0) & 0x100)>>8;

  if(is_timer_elapsed == 1){
    timeoutcount++;
    IFS(0) = IFS(0) & 0xfffffeff; //reset flag
  }

 if(((IFS(0) & 0x80000) >> 19) == 1){
  *portE += 1;
  if(game_state==0){
    paus();
  }
  IFSCLR(0) = 0x80000;
  }
}



/* initialization */
void init( void )
{

  volatile int* trisE = (volatile int*) 0xbf886100; //initializing pointer to control register for light GPIO pin.
  *trisE &= ~0xff; //setting lights as output. Port E is used for the LED Set bits 0 through 7 to 0 (output)
  *portE = 0;       //setting value of lights to 0
  TRISD |= 0xfe0;     //setting port d bit 11-5 as input

  T2CONCLR = 0x8000;
  //T2CON = 0x0; //clear timer 2 configurations
  T2CON = 0x70; // prescale 1:256

  PR2 = 1250; //  PERIOD : timer period: 1500
  TMR2 = 0x0;		//set timer value to 0 before start

  T2CONSET = 0x8000; // Start timer



  IEC(0) = 0x80000;  //enable interrupt for timer 2
  IPC(2) = 0x1f;    //set timer2 interrupt priority. set 0,1,2,3,4 to be 1 to have maximum priority for timer 2

  //External interrupt
  IECSET(0) = 0x80000;	//enable interrupt for int4, bit 19
  IPCSET(4) = 0x18000000; //set priority of switch 4

  enable_interrupt(); //call labwork.s to enable interrupts globally
  return;
}








/* This function is called repetitively from the main program */
void game( void )
{

  timeoutcount = timerelapsed(timeoutcount);

  if (timeoutcount == timedelay) {

  timeoutcount = 0;

  int i;
  int j;
  if (game_state == 2){                    //2 = Game over
    //Reset all variables that were modified in the previous game-sequence
    timedelay = 18;
    space = 16;
    antal = 0;
    tempcount = 0;
	  ptr1 = 0;
	  ptr2 = 0;
    ptr3 = 0;
    f = 1;

    display_string(1, "GAME OVER LOSER");
    display_update();
    delay(5000);
    display_string(0, "  your score:");     // Display score
    display_score(1, itoaconv(score), "", 0);
    display_string(2, "     name:");
    display_update();

	  while(f) { //Select name, each button corresponds to one pointer that points to the alphabet char array.

		  timeoutcount = timerelapsed(timeoutcount);

		  if (timeoutcount == 30) {
			  int btns = getbtns();

			  if (btns == 4) {
          ptr1 = (ptr1 + 1) % 27;
				  textbuffer[3][5] = alphabet[ptr1];
			  }
			  if (btns == 2) {
          ptr2 = (ptr2 + 1) % 27;
				  textbuffer[3][6] = alphabet[ptr2];
			  }
        if (btns == 1) {
          ptr3 = (ptr3 + 1) % 27;
          textbuffer[3][7] = alphabet[ptr3];
        }
			  display_update();
			  if (btns == 8) {
				  holder[0] = alphabet[ptr1];
				  holder[1] = alphabet[ptr2];
          holder[2] = alphabet[ptr3];
          f = 0;
			  }
			  timeoutcount = 0;
		  }
	  }
    display_stringClear();
    if(modify == 1) { //Checks if nightmare mode was being played, thus using the nightmare highscores and names to update
      for(i=0; i<3; i++) {
        if(score > nhighscore[i]) {

        intswap(&score, &nhighscore[i]);


        tempHolder[0] = nhighscoreHolder[i*3];
        tempHolder[1] = nhighscoreHolder[i*3+1];
        tempHolder[2] = nhighscoreHolder[i*3+2];

        nhighscoreHolder[i*3] = holder[0];
        nhighscoreHolder[i*3+1] = holder[1];
        nhighscoreHolder[i*3+2] = holder[2];

        holder[0] = tempHolder[0];
        holder[1] = tempHolder[1];
        holder[2] = tempHolder[2];
      }
    }
    }
    else {  //Normal mode highscore updating
      for(i=0; i<3; i++) {
      if(score > highscore[i]) {
        intswap(&score, &highscore[i]);

        tempHolder[0] = highscoreHolder[i*3];
        tempHolder[1] = highscoreHolder[i*3+1];
        tempHolder[2] = highscoreHolder[i*3+2];
        highscoreHolder[i*3] = holder[0];
        highscoreHolder[i*3+1] = holder[1];
        highscoreHolder[i*3+2] = holder[2];
        holder[0] = tempHolder[0];
        holder[1] = tempHolder[1];
        holder[2] = tempHolder[2];
      }
    }
    }
    display_string(0, "  High score:");
    // Display the highscore
    for(j=1; j<4; j++) {
      if(modify == 1) { //Nightmare highscore
        display_score(j, itoaconv(nhighscore[j-1]), &nhighscoreHolder[(j-1)*3]);
      }
      else { //Normal highscore
        display_score(j, itoaconv(highscore[j-1]), &highscoreHolder[(j-1)*3]);
      }
    }

    display_update();
    delay(5000);
    display_stringClear();   // clear the display from text

    // reset all tubes
    for(i=0; i<4*2; i++){
      tubePos[i]=tubePosReset[i];
    }

    resetPos();       //bird reset x- and y-positions
    clearScreen();
    delay(2000);
    display_image(0, icon);
    display_stringClear();
    display_update();
    //Reset score and game mode
    score = 0;
    modify = 0;
    game_state = 1;              // Gamestate 1 = Go to main menu

  }
  //Main Menu/Start Screen
  if (game_state == 1){
    display_stringClear();
      intro();
    while(game_state == 1){
      timeoutcount = timerelapsed(timeoutcount);
      if(timeoutcount == timedelay) {
        timeoutcount = 0;
        int btns = getbtns();

        switch(btns) {
          case 1:
            if (dummy == 0) {
            srand ((unsigned) (TMR2 & 0x3fff)); //Seed the random generator, use the dummy to check that is run only once.
            dummy++;
            }
            btns = 0;
            countDown();
            game_state = 0;
            break;

          case 2:
            if (dummy == 0) {
              srand ((unsigned) (TMR2 & 0x3fff)); //Seed the random generator
              dummy++;
            }
            btns = 0;
            modify = 1;
            countDown();
            game_state = 0;
            break;

          case 4:
            display_stringClear();
            highscoremenu();
            delay(250);
            break;
        }


      }
    }

  }

clearScreen();
drawTopAndBottomLine ();

// Decrement the space between tubes and the speed of the game for every 150 pixels the user travels,
// making the game progressively harder
if(score % 50 == 0 && space > 10) {
  space = space - 1;
  timedelay = timedelay - 1;
}

// Move obstacles
for(i = 0 ; i< 8; i=i+2){


  if (tubePos[i] <= width*(-1)){
    tubePos[i]=128;
    tubePos[i+1] = 0;
  }

  if (tubePos[i+1] == 0){
    tubePos[i+1] = (-8-rand() % 8); //Random y value between -8 and -15
  }
  drawTube(tubePos[i],tubePos[i+1]);
  tubePos[i]= tubePos[i]-1;   // move tube 1 pixel to left

  // Nightmare mode, makes the tubes move in the y-direction aswell
  if (modify==1){
    if(upsidedown[i/2] == 1 && tubePos[i+1] == -15) {
      upsidedown[i/2] = 0;
    }
    if(upsidedown[i/2] == 0 && tubePos[i+1] == -8) {
      upsidedown[i/2] = 1;
    }
    if(upsidedown[i/2] == 0){
      tubePos[i+1] = tubePos[i+1]+1;
    }
    if(upsidedown[i/2] == 1){
      tubePos[i+1] = tubePos[i+1]-1;
    }
  }
}


// Gravitation, grativation is bigger in nightmare (modify == 1)
if (modify==1){
  PosY += 2;
}
else{
PosY += 1;
}


if (antal > 0) {
  PosY -= 3;
  antal = antal-1;
}

// When btn 3 is pressed, antal becomes 3. Each loop antal is bigger than 0, move up the player 3 pixels and decrement antal to simulate a jump
if ( getbtns() == 2 ){
      antal = 3;
}

//Draw the player with the new coordinates, display the new image, and update the score.
drawBird(PosX, PosY);
display_image(0, icon);
gameScore();

  }
}
