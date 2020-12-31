
    Welcome to PSP-Magic

Written by Ludovic.Jacomme also known as Zx-81 (zx81.zx81@gmail.com)

1. Introduction
   ------------

  PSP-Magic is a PSP version of the famous mechanical drawing toy called
  "Magic Screen".
  See http://en.wikipedia.org/wiki/Etch-A-Sketch for details.

  This package is under GPL Copyright, read COPYING file for
  more information about it.

  Big thanks to Pako for his very nice music !
  (see http://www.jamendo.com/en/artist/pako)

  Many thanks to my friends Manu & Buzz, who encouraged me 
  and for their help, advices and beta testing.


2. Installation
   ------------

  Unzip the zip file, and copy the content of the directory fw5.x or fw1.5
  (depending of the version of your firmware) on the psp/game directory.

  It has been developped on linux for Firmware 5.0-M33

  For any comments or questions on this version, please visit 
  http://zx81.zx81.free.fr or http://zx81.dcemu.co.uk


3. Magic Screen commands :
  ----------------------

  Cross       Draw mode (on/off)
  Triangle    Erase mode (on/off)
  Cursor      Move on the magic screen
  Analog      Move on the magic screen
  Circle      Increment step
  Square      Decrement step
  Select      Menu

4. Magic Screen Menu
   ------------

  In the menu window, press Start to open/close the 
  On-Screen keyboard

  The On-Screen Keyboard of "Danzel" and "Jeff Chen"

  Use Analog stick to choose one of the 9 squares, and
  use Triangle, Square, Cross and Circle to choose one
  of the 4 letters of the highlighted square.

  While the virtual keyboard is displayed, you can still
  use the digital pad :

  Left    Move cursor left
  Right   Move cursor right
  Up      Move cursor to the beginning of the line
  Down    Return
  Select  Disable virtual keyboard
  Start   Disable virtual keyboard

  Snapshots can be taken using the "Image" menu option. 
  Screenshots are then saved in scr folder.
  
  Using Load/Save menu, all drawings can be saved/loaded
  with a dedicated file format (.mag files). 
  It can be useful to periodically save your drawing
  because there is no "undo" command.


4. COMPILATION
   ------------

  It has been developped under Linux using gcc with PSPSDK. 
  To rebuild the homebrew run the Makefile in the src archive.

  Waitting for nice drawings,
  
         Zx
