#include <Wire.h>
byte a;

//word address = 0;
//word address = 0x350;
//word address = 0x6A0;
//word address = 0x800;
//word address = 0xB40;
word address = 0xE40;

// const char data[] = {"Piano1          EPiano1         Brass_3         Wah_Brass       BellBrass       Trumpet         Horn            Trombone        Tuba            Strings1        Strings2MTA     PluckedA        SynthStr        Violin          Cello           Bagpipes        Piccalo         Clarinet        Oboe            JazzFlute       Saxophone       Harmonica       EPiano3         Organ1A         Organ2A         Organ3A         ReedOrgan       PipeBrass       Pipeorg1        Harpschd1       Harpschd2       Clavichord      Celeste         ToyPiano        Accordian       BluesGuitar     SpaniGuitar     DistGuitar      ElecBass1       WalkBass        Banjo           Lute            Harp            JawHarp1        JawHarp2        Sitar           Vibes2          Glock2          Koto2           SteelDrum       Marimba         Xylophone       TubeBells       "};//string that needs to be on eeprom
// const char data[] = {"Synth1          Synth3          Synbass1        Synbass2        SynthLead       Syntharpsi      Voice           MeatyJob        NiceSound       Chimes          Guitar          Vibraphone      Brass           StringsMT       Piano           Organ1          Organ2          Harpsi          Flute           Synth1A         Synth2A         Synth3A         Banjo           Mellow          SpaceBell       Strings2        Plucked         Raindrops       Cosmicblow      SweetFlute      Alien           Glock           Synthbass       EPiano_2        BassMoto        MetalHarp       Voice1          Strings3        Plucked2        PlinkStr        SpaceBell2      PhilGlass       SitarSax        Whistle         Synth4          Fantasy         Borgan1         Borgan2         Borgan3         Borgan4         Borgan5         Borgan6         ELOrgan2        "};
//const char data[] = {"Human3          Org9            Organ1or        ORGAN13         ORGAN2          ORGAN2A         ORGAN3          ORGAN5          ORGANFD1        ORGANK          ORGANVL         ORGNOR01        ORGNOR02        ORGNOR03        ORGNOR04        ORGPEDAL        DOCORG1         DOCORG2         DOCORG3         DOCORG          --EMPTY--       --EMPTY--                       "};
//const char data[] = {"C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             "};
//const char data[] = {"C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             C64             "};
const char data[] = {"ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           ORGAN           --EMPTY--       --EMPTY--       "};





void setup()
{
  Wire.begin();
  Serial.begin(115200);
 Serial.println("Ready");
  
  
  for(int i = 0; i < strlen(data); i++) 
    {
    Serial.print("Eeprom Address = ");
    Serial.print(address, HEX);
    Serial.print("  ");
    Serial.print("Data= ");
    Serial.print(data[i]);
 a =(data[i]);
 Serial.print("  ");
 Serial.println(a);
// Actually write Data to Chip
  Wire.beginTransmission(0x50);
  Wire.write((int)highByte(address));
  Wire.write((int)lowByte(address));
  Wire.write(a);
  Wire.endTransmission();
  delay(5);
  address = address + 1;// move to next eeprom address
    
    }
 Serial.print("Ready for next run - Change address and data");   
}

void loop()
{

}
