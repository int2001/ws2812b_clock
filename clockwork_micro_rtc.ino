#include "FastLED.h"

// RTC
#include <Wire.h>
#include <WireRtcLib.h>
WireRtcLib rtc; // RTC Objekt

#define DATA_PIN    5             // An PIN 5 haengt der Strip
#define LED_TYPE    WS2812        // Wir haben es mit einem WS2812 zu tun
#define COLOR_ORDER GRB           // Der die Farben in der Reihenfolge GRB entgegennimmt
#define NUM_LEDS    60            // 60 LEDs sind es in Summe
CRGB leds[NUM_LEDS];              // Ansteuern moechten wir die LEDs ueber das Array "leds"

#define BRIGHTNESS          42    // Konstante um die Helligkeit vorzugeben
#define FRAMES_PER_SECOND  120    // Delays (s.u.) zwischen jedem Update

unsigned long spos=0; // Globale Var um die Sekunden zu halten.
unsigned long o_spos=-1; // Globale Var um die Sekunden zu halten.
unsigned long millis_offset=0;

void setup()
{
  delay(4000);  // Dem mC etwas Zeit zum einpendeln geben.
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip); // LEDs initialisieren
  Wire.begin(); // i2c-Bus initialisieren (WICHTIG! Sonst tut die RTC nicht)
  rtc.begin();  // RTC initialisieren
}


void loop()
{ 
  unsigned long spos=get_spos();  // RTC abfragen
  if (o_spos != spos) {           // Neue Sekunde?
    o_spos=spos;                  // Dann alte auf neue setzen
    millis_offset=millis();       // internen Millisec.-Counter des mC in offset speichern (das ding ist sonst zu ungenau)
  }
  float hpos=(((float)spos/3600)*60/12);  // Stundenposition berechnen und auf die 60 Teilstriche (hier noch Fliesskommazahl) umrechnen
  for (byte i=0;i<NUM_LEDS;i++) { // Einmal alle LEDs durchlaufen (fuer "Mark"-LEDs)
    if (i%5 == 0) {               // Position durch 5 glatt teilbar?
     // leds[i]=CRGB(2,2,0);      // Dann SET
    }
    if (i%15 == 0) {              // Position durch 15 glatt teilbar
      leds[i]=CRGB(3,3,3);        // Dann SET
    }
  }
  
  float nbright=(float)(millis()-millis_offset)/1000;         // Berechne Sekundenbruchteil
  byte sekufraction=ceil(nbright*BRIGHTNESS);                 // Berechne aus dem Bruchteil Dimmfaktor * Helligkeit
  if (sekufraction>BRIGHTNESS) { sekufraction=BRIGHTNESS; }   // Neuer Helligkeitskorrekturwert groesser als Max-Helligkeit? Dann begrenzen
  set_led(spos%NUM_LEDS,0, BRIGHTNESS-sekufraction, BRIGHTNESS-sekufraction); // Setze Sekunde gedimmt um korrekturwert
  set_led((spos+1)%NUM_LEDS,0, sekufraction, sekufraction);                   // Setze Zukunftssekunde gedimmt um korrekturwert
  set_led((round(hpos+1)%NUM_LEDS),BRIGHTNESS/4, 0, 0);                       // Setze NACHlaufende Stunde gedimmt
  set_led((round(hpos)%NUM_LEDS),BRIGHTNESS, 0, 0);                           // Setze Stunde
  if (hpos==-1) { hpos=12; }                                                  // Rollover um 00:0 bzw. 12:00
  set_led((round(hpos-1)%NUM_LEDS),BRIGHTNESS/4, 0, 0);                       // Setze VORlaufende Stunde
  nbright=((float)spos/60)-int(spos/60);               // Fadingeffekt fuer Minuten errechnen (Fraction of Minutes)
  set_led(((spos/60))%NUM_LEDS,0, BRIGHTNESS-ceil(nbright*BRIGHTNESS), 0);    // Setze Minuten auf Helligkeit - (Helligkeit * Fraction)
  set_led(((spos/60)+1)%NUM_LEDS,0, ceil(nbright*BRIGHTNESS), 0);             // Setze Minuten+1 auf Helligkeit * Fraction
  
  FastLED.show();                                               // Und SHOWTime
  FastLED.delay(1000/FRAMES_PER_SECOND);                        // Warten

  fadeToBlackBy(leds,NUM_LEDS,5);                               // Alles was nicht neu gesetzt wurde ausfaden
}

unsigned long get_spos() {  // Returned die aktuelle Zeit seit Mitternacht in Sekunden
  WireRtcLib::tm* t = rtc.getTime();
  return ( (long(t->hour%12)*3600)+(long(t->min)*60)+(t->sec) );
}

void set_led (byte which_one, byte r,byte g, byte b) {
  if (which_one % 15 == 0) {  // This is a "Mark"-LED
    if (r<=3) { r=3; }
    if (g<=3) { g=3; }
    if (b<=3) { b=3; }
  }
  leds[which_one]=CRGB(r,g,b);
}


