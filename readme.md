# Worm for Metro M4

Worm game like snake to show my kids some coding

I HAVE KEPT THIS SIMPLE FOR A 10 YEAR OLD!

## Hardware 
Adafruit M4 Metro
Adafruit RGB display with their Library single 64x32 uni
4 Axis joystick using bounce2 lib

## Getting Started
Make sure your HUB75 RGB display is working with adafruit test programs in arduino
hook up the joystick
open in arduino and compile - once you have got the libs downloaded
play!

## Advanced debugging
Personally I am using Visual GDB plugin for heavy work - I find VGDB very good for debugging non-standard arduino type hardware - with a Segger J-link - well worth the license fee

*NOTE if using VGDB You will need to modify the file "M4_Metro_Worm_P5_VGDB.vgdbproj" and change the <SketchSubdirectory> to where you have it installed*

Normally I try to import to Atmel Studio 7.0 but the SAMD51 arduino import is not working - maybe ask microchip/atmel about this... maybe MPLAB X involved?

### About
move arround and eat pixels???


