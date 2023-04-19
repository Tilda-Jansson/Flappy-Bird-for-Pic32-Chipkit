# Flappy-Bird-for-Pic32-Chipkit

## Overview

This project presents an adaptation of the popular Flappy Bird game for the PIC32 chipkit, leveraging the OLED graphical display on the Basic I/O shield with an SPI bus-connection. This version features two game modes, 'Normal' and 'Nightmare', and includes a high-score list for each mode. The game can be paused and resumed during runtime, and players can enter their initials upon game completion.

In the 'Nightmare' mode, obstacles move in both the X and Y directions, making the game more challenging compared to the 'Normal' mode, where obstacles only move in the X direction.

See the video demo of the game in 'Nightmare' mode in action [**here**](video/demo.mp4)

## Features

* Two game modes: 'Normal' and 'Nightmare'
  - 'Normal' mode: Obstacles move in the X direction
  - 'Nightmare' mode: Obstacles move in both X and Y directions
* Moving object (the bird) with Y-direction movement
* Randomly generated obstacles with gradually decreasing gaps
* Game speed increases over time
* Pause and resume functionality
* High-score lists for each game mode
* Menu system for mode selection and high-score lists

## Solution

The game is controlled using the buttons on the chipkit board, and the OLED display is updated using a custom set of drawing functions. The game's speed and difficulty progression are managed using the built-in timer.

## Controls

* BTN3: Jump action for the bird
* SW4: Pause and resume the game (implemented using interrupt)
* BTN1, BTN2, BTN3, BTN4: Enter initials after game completion

## Implementation Details

* Polling is used to register button inputs for character movement in order to maintain a consistent frame rate on the display.
* Custom draw functions are used to update the icon array, which is then used by the **display_image** function to fill the page-buffers.
* The **draw_Check** function is implemented to detect bird collisions with obstacles or the ground/roof.
* The **drawTube** function is used to draw obstacle sets based on given x and y coordinates, with vertical movement in the 'Nightmare' mode.
* The built-in timer is used to control game speed and screen updates.
* All development was done using MCB32tools, and all code is written in C.
