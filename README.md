# Commodore-64-SFX-Sound-Expander-Arduino-MIDI-V1
Arduino MIDI interface for C64 SFX Sound Expander V1.0

<img src=“\images\DSC01459.JPG”>


This project details the build of a stand alone MIDI interface for the SFX Sound Expander cartridge originally for the Commodore 64 range of computers.

I like this cartridge, I like the cheesy sounds, I like the plastic keyboard.  What I don’t like is having to use clunky old hardware and software to use it.

After many initial versions both hardware and software, I decided to use a third party Arduino OPL2 library from DhrBaksteen.  This provided me with a “stop, do not go any further” point, which prevented feature creep and I am grateful to the author for the library and support that has been given.

Ingredients for SFX Sound Expander MIDI interface

1 x C64 Cartridge connector

1 x Arduino Nano or clone or any an Uno or Mega....

1 x I6 x 2 I2C LCD

1 x I2C EEPROM (24LC256 or 24LC512 or similar) 

1 x Sheet prototype board

1 x 6N139 opto isolator (or 6N138)

1 x 220R resistors

1 x 1K2 resistors

1 x 5K6 resistors

1 x 1N4148 diode

2 x 8 Pin DIL sockets

1 x 16 Pin DIL socket

Some female header sockets.

Some male header pins.

Hookup wire

5 pin DIN socket for MIDI connection


The circuit
I used an Arduino Nano and a 74HC595 shift register.  The 74HC595 is driven using the Arduino SPI bus on pins 10, 11 and 13.

A standard MIDI input circuit using a 6N138/9 opto-isolator is applied to the Rx pin of the Nano.
