/**
 * This example can be used together with a Teensy 2.0 or later to use the OPL2 Audio Board as a MIDI device. To
 * configure the Teensy as a MIDI device set USB Type to MIDI in the IDE using Tools > USB Type > MIDI. Once connected
 * the board should appear in the device list as 'OPL2 AUdio Board MIDI'. You can now use test the board with, for
 * example, MIDI-OX, your favorite music creation software or DosBox! 
 *
 * OPL2 board is connected as follows:
 *   Pin  8 - Reset
 *   Pin  9 - A0
 *   Pin 10 - Latch
 *   Pin 11 - Data
 *   Pin 13 - Shift
 *
 * Code by Maarten Janssen (maarten@cheerful.nl) 2018-07-02
 * Most recent version of the library can be found at my GitHub: https://github.com/DhrBaksteen/ArduinoOPL2
 */

#include <SPI.h>
#include <OPL2.h>
#include <MIDI.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

MIDI_CREATE_DEFAULT_INSTANCE();

#include <midi_drums.h>
#include <C64dmt3.h>

char voicename[16]; //Array to hold the Patch & Bank name
int k;              
int l;
int program = 0;
int up_buttonState = 0;         // current state of the up button
int up_lastButtonState = 0;     // previous state of the up button
int down_buttonState = 0;         // current state of the up button
int down_lastButtonState = 0;     // previous state of the up button
bool bPress = false;

#define disk1 0x50    //Address of 24LC512 eeprom chip
const int  Up_buttonPin   = A0;    // the pin that the pushbutton is attached to
const int  Down_buttonPin = A1;

#define MIDI_NUM_CHANNELS 16
#define MIDI_DRUM_CHANNEL 10

#define CONTROL_VOLUME          7
#define CONTROL_ALL_SOUND_OFF 120
#define CONTROL_RESET_ALL     121
#define CONTROL_ALL_NOTES_OFF 123


// LCD display defines ---------
//#define  LCD_ROWS  2
//#define  LCD_COLS  16

//Use address 0x3F for blue 20 x 4 0x27 for 16 x 2
LiquidCrystal_I2C LCD(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);


// Channel mapping to keep track of MIDI to OPL2 channel mapping.
struct ChannelMapping {
	byte midiChannel;
	byte midiNote;
	float midiVelocity;
	float op1Level;
	float op2Level;
};


OPL2 opl2;
ChannelMapping channelMap[OPL2_NUM_CHANNELS];
byte oldestChannel[OPL2_NUM_CHANNELS];
byte programMap[MIDI_NUM_CHANNELS];
float channelVolumes[MIDI_NUM_CHANNELS];


/**
 * Register MIDI event handlers and initialize.
 */
void setup() {
  pinMode( Up_buttonPin , INPUT_PULLUP);
  pinMode( Down_buttonPin , INPUT_PULLUP);

	MIDI.setHandleNoteOn(onNoteOn);
	MIDI.setHandleNoteOff(onNoteOff);
	MIDI.setHandleProgramChange(onProgramChange);
	MIDI.setHandleControlChange(onControlChange);
	MIDI.setHandleSystemReset(onSystemReset);
	onSystemReset();

	opl2.setDeepVibrato(true);
	opl2.setDeepTremolo(true);
    MIDI.begin(MIDI_CHANNEL_OMNI);

  // initialise LCD display
  LCD.begin(16, 2);
  LCD.backlight();
  LCD.clear();
  LCD.noCursor();

LCD.setCursor(0,0); //First line
  LCD.print("    SFX Sound  ");
  LCD.setCursor(0,1); //Second line
  LCD.print("     Expander     ");
delay(1500);
onProgramChange(1, 0);


}


/**
 * Read and handle MIDI events.
 */
void loop() {
	MIDI.read();
  ///Button push stuff
   checkUp();
   checkDown();

//End of button stuff

  
}


/**
 * Get a free OPL2 channel to play a note. If all channels are occupied then recycle the oldes one.
 */
byte getFreeChannel(byte midiChannel) {  
	byte opl2Channel = 255;

	// Look for a free OPL2 channel.
	for (byte i = 0; i < OPL2_NUM_CHANNELS; i ++) {
		if (!opl2.getKeyOn(i)) {
			opl2Channel = i;
			break;
		}
	}

	// If no channels are free then recycle the oldest, where drum channels will be the first to be recycled. Only if
	// no drum channels are left will the actual oldest channel be recycled.
	if (opl2Channel == 255) {
		opl2Channel = oldestChannel[0];
		for (byte i = 0; i < OPL2_NUM_CHANNELS; i ++) {
			if (channelMap[oldestChannel[i]].midiChannel == MIDI_DRUM_CHANNEL) {
				opl2Channel = oldestChannel[i];
			}
		}
	}

	// Update the list of last used channels by moving the current channel to the bottom so the last updated channel
	// will move to the front of the list. If no more OPL2 channels are free then the last updated one will be recycled.
	byte i;
	for (i = 0; i < OPL2_NUM_CHANNELS && oldestChannel[i] != opl2Channel; i ++) {}

	while (i < OPL2_NUM_CHANNELS - 1) {
		byte temp = oldestChannel[i + 1];
		oldestChannel[i + 1] = oldestChannel[i];
		oldestChannel[i] = temp;
		i ++;
	}

	return opl2Channel;
}


/**
 * Set the volume of operators 1 and 2 of the given OPL2 channel according to the settings of the given MIDI channel.
 */
void setOpl2ChannelVolume(byte opl2Channel, byte midiChannel) {
	float volume = channelMap[opl2Channel].midiVelocity * channelVolumes[midiChannel];
	byte volumeOp1 = round(channelMap[opl2Channel].op1Level * volume * 63.0);
	byte volumeOp2 = round(channelMap[opl2Channel].op2Level * volume * 63.0);
	opl2.setVolume(opl2Channel, OPERATOR1, 63 - volumeOp1);
	opl2.setVolume(opl2Channel, OPERATOR2, 63 - volumeOp2);
}


/**
 * Handle a note on MIDI event to play a note.
 */
void onNoteOn(byte channel, byte note, byte velocity) {
	channel = channel % 16;

	// Treat notes with a velocity of 0 as note off.
	if (velocity == 0) {
		onNoteOff(channel, note, velocity);
		return;
	}

	// Get an available OPL2 channel and setup instrument parameters.
	byte i = getFreeChannel(channel);
	if (channel != MIDI_DRUM_CHANNEL) {
		opl2.setInstrument(i, midiInstruments[programMap[channel]]);
	} else {
		if (note >= DRUM_NOTE_BASE && note < DRUM_NOTE_BASE + NUM_MIDI_DRUMS) {
			opl2.setInstrument(i, midiDrums[note - DRUM_NOTE_BASE]);
		} else {
			return;
		}
	}

	// Register channel mapping.
	channelMap[i].midiChannel  = channel;
	channelMap[i].midiNote     = note;
	channelMap[i].midiVelocity = round(velocity / 127.0);
	channelMap[i].op1Level     = round((63 - opl2.getVolume(i, OPERATOR1)) / 63.0);
	channelMap[i].op2Level     = round((63 - opl2.getVolume(i, OPERATOR2)) / 63.0);

	// Set operator output levels based on note velocity.
	setOpl2ChannelVolume(i, channel);

	// Calculate octave and note number and play note!
	byte opl2Octave = 4;
	byte opl2Note = NOTE_C;
	if (channel != MIDI_DRUM_CHANNEL) {
		note = max(24, min(note, 119));
		opl2Octave = 1 + (note - 24) / 12;
		opl2Note   = note % 12;
	}
	opl2.playNote(i, opl2Octave, opl2Note);
}


/**
 * Handle a note off MIDI event to stop playing a note.
 */
void onNoteOff(byte channel, byte note, byte velocity) {
	channel = channel % 16;
	for (byte i = 0; i < OPL2_NUM_CHANNELS; i ++) {
		if (channelMap[i].midiChannel == channel && channelMap[i].midiNote == note) {
			opl2.setKeyOn(i, false);
			break;
		}
	}
}


/**
 * Handle instrument change on the given MIDI channel.
 */
void onProgramChange(byte channel, byte program) {
	programMap[channel % 16] = min(program, 127);
word  eeaddress;
if (channel==1){
LCD.clear();
LCD.setCursor(0,0);
// Put read EEPROM read here:
char rdata;
unsigned int eeaddress = 0;
int i;
eeaddress = (program * 16); 
   LCD.setCursor(0, 0);
   for (i = 0; i < 16; i++)
   {
   
   voicename[k]=(readEEPROM(disk1, eeaddress));
   eeaddress++;
   LCD.print(voicename);
   }

   eeaddress=(eeaddress+2032);
  LCD.setCursor(0, 1);
   for (i = 0; i < 16; i++)
   {
   voicename[k]=(readEEPROM(disk1, eeaddress));
   eeaddress++;
   LCD.print(voicename);
   }
LCD.setCursor(13, 1);
LCD.print(program,DEC);
}
}


/**
 * Handle MIDI control changes on the given channel.
 */
void onControlChange(byte channel, byte control, byte value) {
	channel = channel % 16;

	switch (control) {

		// Change volume of a MIDI channel. (Limited to 0.8 to prevent clipping)
		case CONTROL_VOLUME: {
			channelVolumes[channel] = min(value / 127.0, 0.8);
			for (byte i = 0; i < OPL2_NUM_CHANNELS; i ++) {
				if (channelMap[i].midiChannel == channel && opl2.getKeyOn(i)) {
					setOpl2ChannelVolume(i, channel);
				}
			}
			break;
		}

		// Reset all controller values.
		case CONTROL_RESET_ALL:
			for (byte i = 0; i < MIDI_NUM_CHANNELS; i ++) {
				channelVolumes[channel] = 0.8;
			}
		break;

		// Immediately silence all channels.
		// Intentionally cascade into CONTROL_ALL_NOTES_OFF!
		case CONTROL_ALL_SOUND_OFF:
			for (byte i = 0; i < OPL2_NUM_CHANNELS; i ++) {
				opl2.setRelease(i, OPERATOR1, 0);
				opl2.setRelease(i, OPERATOR2, 0);
			}

		// Silence all MIDI channels.
		case CONTROL_ALL_NOTES_OFF: {
			for (byte i = 0; i < OPL2_NUM_CHANNELS; i ++) {
				if (channelMap[i].midiChannel == channel) {
					onNoteOff(channelMap[i].midiChannel, channelMap[i].midiNote, 0);
				}
			}
			break;
		}

		// Ignore any other MIDI controls.
		default:
			break;
	}
}


/**
 * Handle full system reset.
 */
void onSystemReset() {
	opl2.init();

	// Silence all channels and set default instrument.
	for (byte i = 0; i < OPL2_NUM_CHANNELS; i ++) {
		opl2.setKeyOn(i, false);
		opl2.setInstrument(i, midiInstruments[0]);
		oldestChannel[i] = i;
	}

	// Reset default MIDI player parameters.
	for (byte i = 0; i < MIDI_NUM_CHANNELS; i ++) {
		programMap[i] = 0;
		channelVolumes[i] = 0.8;
	}
}

byte readEEPROM(int deviceaddress, unsigned int eeaddress ) 
{
  byte rdata = 0xFF;
 
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8));   // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();
 
  Wire.requestFrom(deviceaddress,1);
 
  if (Wire.available()) rdata = Wire.read();
 
  return rdata;
}

void checkUp()
{
  up_buttonState = digitalRead(Up_buttonPin);
  // compare the buttonState to its previous state
  if (up_buttonState != up_lastButtonState) {
    // if the state has changed, increment the counter
    if (up_buttonState == LOW) {
        bPress = true;
      // if the current state is HIGH then the button went from off to on:
if (program == 127) {
  (program=0);
  } else
      program++;
    } 
 //   Serial.println(program);
    // Delay a little bit to avoid bouncing
    delay(50);
  // save the current state as the last state, for next time through the loop
  up_lastButtonState = up_buttonState;
  onProgramChange(1, program);
  }


}

void checkDown()
{
  down_buttonState = digitalRead(Down_buttonPin);
  // compare the buttonState to its previous state
  if (down_buttonState != down_lastButtonState) {
    // if the state has changed, increment the counter
    if (down_buttonState == LOW) {
       bPress = true;
      // if the current state is HIGH then the button went from off to on:
  
  if (program == 0) {
    (program=127);
  }else
      program--;
//      Serial.println(program);

    }
// Delay a little bit to avoid bouncing
delay(50);
// save the current state as the last state, for next time through the loop
down_lastButtonState = down_buttonState;
onProgramChange(1, program);
  }
  

}



