# Commodore-64-SFX-Sound-Expander-Arduino-MIDI-V1
Arduino MIDI interface for C64 SFX Sound Expander V1.0

<img src=images/trim.jpg>


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

<img src=/images/MIDIInput.jpg>

The I2C bus is also utilised for a 16 x 2 LCD and an external EEPROM. These use the A4 and A5 pins of the Nano as per standard I2C bus usage.
Lastly there are two push buttons connected to A0 and A1 and Gnd.

In addition to the above, a C64CLK pin using D6 of the Nano is added as is D8 for a reset and D9 which is used to drive A4 of the SFX Sound Expander.

SFX Sound Expander circuit Diagram
The diagram below shows the SFX Sound Expander circuit.
It was designed to interface to the Commodore 64 Cartridge slot.
Connections are relatively straight forward, GNDs have to be connected up, the outputs of the 74HC595  are connected to D0 – D7 of the C64 connector. D8 is connected to A4 of the edge connector, Reset is connected to Reset and D6 is connected to the C64 Clock signal Phi.


<img src=/images/sfx-sch.jpg>

Software
This project is based on the Teensy MIDI example supplied with the library, the first step is to add the C64 clock signals into OPL.cpp as shown below:  

/**

 * Initialize the YM3812.
 
 */
 
void OPL2::init() {

	#if BOARD_TYPE == OPL2_BOARD_TYPE_ARDUINO
	
		SPI.begin();
		
	#else
	
		wiringPiSetup();
		
		wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED);
		
	#endif
	

	pinMode(pinLatch,   OUTPUT);
	
	pinMode(pinAddress, OUTPUT);
	
	pinMode(pinReset,   OUTPUT);
	
// Added by Sparx266

	pinMode(pinC64clk, OUTPUT);
	
// End of added by Sparx266



	digitalWrite(pinLatch,   HIGH);
	
	digitalWrite(pinReset,   HIGH);
	
	digitalWrite(pinAddress, LOW);
	
// Added by Sparx266

	digitalWrite(pinC64clk,  HIGH);
	
// End of added by Sparx266


	reset();
	
}



/**

 * Hard reset the OPL2 chip. This should be done before sending any register data to the chip.
 
 */
 
void OPL2::reset() {

	digitalWrite(pinReset, LOW);
	
//Added by Sparx266

	digitalWrite(pinC64clk,  HIGH);
	
//End of added by Spar266

	delay(1);
	
	digitalWrite(pinReset, HIGH);
	
//Added by Sparx266

digitalWrite(pinC64clk,  LOW);

//End of added by Spar266


	for(int i = 0; i < 256; i ++) {
	
		oplRegisters[i] = 0x00;
		
	}
	
}



/**

 * Send the given byte of data to the given register of the OPL2 chip.
 
 */
 
void OPL2::write(byte reg, byte data) {

	digitalWrite(pinAddress, LOW);
	
	#if BOARD_TYPE == OPL2_BOARD_TYPE_ARDUINO
	
		SPI.transfer(reg);
		
	#else
	
		wiringPiSPIDataRW(SPI_CHANNEL, &reg, 1);
		
	#endif
	
//	digitalWrite(pinLatch, LOW);

//Added by Sparx266

	digitalWrite(pinLatch, HIGH);
	
	digitalWrite(pinC64clk,  HIGH);
	
//End of added by Sparx266


	delayMicroseconds(1);
	
	//digitalWrite(pinLatch, HIGH);
	
//Added by Sparx266

	digitalWrite(pinLatch, LOW);
	
//End of added by Sparx266

delayMicroseconds(4);


	digitalWrite(pinAddress, HIGH);
	
	#if BOARD_TYPE == OPL2_BOARD_TYPE_ARDUINO
	
		SPI.transfer(data);
		
	#else
	
		wiringPiSPIDataRW(SPI_CHANNEL, &data, 1);
		
	#endif
	
	//digitalWrite(pinLatch, LOW);
	
//Added by Sparx266

	digitalWrite(pinLatch, HIGH);
	
//End of added by Spaxr266


	delayMicroseconds(1);
	
	//digitalWrite(pinLatch, HIGH);
	
//Added by Sparx266

	digitalWrite(pinLatch, LOW);
	
	digitalWrite(pinC64clk,  LOW);
	
//End of added by Sparx266

	
delayMicroseconds(23);

}



Also what’s needed is for the following to be added to OPL.h:



	#if BOARD_TYPE == OPL2_BOARD_TYPE_ARDUINO
	
		#define PIN_LATCH 10
		
		#define PIN_ADDR   9
		
		#define PIN_RESET  8
		
                #define PIN_C64clk 6
		


That’s it!


The Arduino sketch
I wanted to use the cartridge as a sound module for my modern keyboards.
The example sound banks from the original C64 software were collected and manipulated into a format useable by the Arduino.  These are in the file C64dmt3.h.
Drums I couldn’t manage to do so if any Commodore 64 whiz is able to get the drum data from the original C64 software, please feel free and I will add them later.

The original sounds number about 100, there are probably duplicates and also some which are not working properly.  This left me with 27 slots free.  As (near enough) the same chip is used in early PC sound cards, there are loads of sound banks available for download.  I chose .IBK files and organ sounds as that is what I wanted to use, but you are free to load your own sounds in.
The difference between the chip in the Sound Expander (YM3256) and the one in the sound cards (YM3812) is that the sound card chips had additional waveforms.  The Sound Expander has only one.  So whilst the sounds will work, they may sound different to what was expected.

The Nano is not a hugely powerful processor, as a result, I started to run out of space.  This is where the external EEPROM comes in.  It is used to store the instrument names and the Bank they originated from on the LCD display.
This required an additional sketch to be written and is included in this repository.  It needs to be run several times, un-commenting out the start address at the top and the data statements just underneath.
First time its run, un-comment the first address and the first data statement.  Upload to the Nano and run.
Comment those lines out and uncomment the ones below, flash to the Nano and repeat until it’s been done four times.

I used a 24LC512 which is overkill but it’s what I had to hand and is cheap.

Once that’s all done, you should be ready to drive your sound expander with a MIDI keyboard or computer.
If you decide to build this, I’d suggest to do it in the following stages:

1)	Connect the 74HC595, the edge connector and the Nano.  Run the demo tune sketch to see if you get sound.  If you do, move onto the next step.
2)	Connect the I2C LCD and get working
3)	Connect the EEPROM and run the programmer sketch
4)	Connect the 2 buttons
5)	Connect the MIDI input circuit and test


Construction details

<img src=/images/SFX%20Nano.jpg>
<img src=/images/NanoSFX.png>

Start with the Nano, the 74HC595 shift register and the C64 cartridge connector.
You can get the connector from a dead C64.  I found a butane blow torch and a helping pair of hands removed them quickly and easily.

Solder some female header sockets to mount the Nano, Then use a DIL socket for the 74HC595 shift register.  Observe which way is up.
Now solder the edge connector, make sure all the prongs fit through the holes.

Now start wiring up. Start by connecting the outputs from the shift register to the D0 – D7 data lines of the connector.Next connect all Gnd lines of the connector together and feed back to the ‘595 Gnd and finally to the Nano Gnd.   
Wire +5V of the edge connector to Nano Vin.
Wire I/O2 and R/W of the C64 connector to Gnd.

Wire the three lines from the Nano to the ‘595 as per below:

Nano pin    74HC595
D10        12                
D11        14                
D13        11

Wire the +5V supply to Vin on the Nano.  Also wire Pin 10 of the ‘595 to +5V and Pin 13 (‘595) to Gnd.

Wire D8 from the Nano directly to the Reset pin on the C64 connector.
Wire D9 from the Nano directly to the A4 pin on the C64 connector.
Wire D6 from the Nano to the D_Clock pin on the C64 connector.


At this stage, you should be able to run the Test tune with a genuine SFX Cartridge.
If not, double check your wiring before continuing.

Add the I2C components.
I2C is only four wires and two of them are for power.
Start with the LCD, there are many different types of LCD and you are advised to get yourself familiar with the one you have but the basic installation and testing steps will be provided here.
You’ll probably mount the display off board and will therefore use 4 male header pins and a lead.
Solder in the pins and connect them as per the connector on the LCD backpack, that way you can just make a straight through lead which is easier.
Gnd and +5V is easy.
The other two are SDA and SCL which are tapped from the Nano pins A4 and A5.
Connect the LCD and run the I2C scanner found at this page:
http://gammon.com.au/i2c

If you can see the address of the device, you may proceed, if not, go back and check what is wrong.
Now get yourself familiar with the constructor for your LCD, you will need this later on.

Next, solder in the EEPROM. Again use a DIL socket.  Pins 1,2,3,4 & 7 to Gnd. Pin 8 to +5V. Pin 5 to Nano A4 and Pin 6 to Nano A5.
Insert the EEPROM chip, make sure it’s the right way up.

Run the I2C scanner sketch again and you should see your device show up at address 0x50.  If you still have the LCD attached, you should see two devices.
If not, go back and check your wiring.

Next solder in the two push buttons, these are simply wired between Nano Pins A0, A1 and Gnd.  You may want to mount these off board so use male pin headers and a connecting lead.

You can now program the EEPROM.  There is a sketch included that needs to be run 4 times, each time with a different start address and set of Data.
Make sure you have the serial monitor open!

At this stage, load up the SFX sketch.

Now add the MIDI input circuit.
You can use a 6N138 or 6N139.  Take note of the protection diode orientation.  The little line needs to be pointing towards Pin 2 of the Opto isolator chip.
Also take care with the resistors on the output side, one is connected to Gnd the other to +5V.  Don’t forget to connect the output to the Nano Rx pin and also check that the MIDI Input socket is wired correctly.      

Connect everything up, MIDI keyboard, SFX Sound Expander, LCD, Buttons, Speaker and amplifier.
Turn on and play!

Now the MIDI circuit is done, you will have to remove the Nano each time a new sketch is uploaded.  This is why you were advised to use female header sockets.  You did use them?


