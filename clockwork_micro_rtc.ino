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

#define BRIGHTNESS          24    // Konstante um die Helligkeit vorzugeben
#define FRAMES_PER_SECOND  120    // Delays (s.u.) zwischen jedem Update

unsigned long spos=0; // Globale Var um die Sekunden zu halten.

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
  float hpos=(((float)spos/3600)*60/12);  // Stundenposition berechnen und auf die 60 Teilstriche (hier noch Fliesskommazahl) umrechnen
  for (byte i=0;i<NUM_LEDS;i++) { // Einmal alle LEDs durchlaufen
    if (i%5 == 0) {               // Position durch 5 glatt teilbar?
     // leds[i]=CRGB(2,2,0);      // Dann SET
    }
    if (i%15 == 0) {              // Position durch 15 glatt teilbar
      leds[i]=CRGB(2,2,2);        // Dann SET
    }
  }
  
  leds[spos%NUM_LEDS] = CRGB( 0, BRIGHTNESS, BRIGHTNESS);       // Setze Sekunden
  leds[(round(hpos+1)%NUM_LEDS)] = CRGB( BRIGHTNESS/4, 0, 0);   // Setze NACHlaufende Stunde gedimmt
  leds[(round(hpos)%NUM_LEDS)] = CRGB( BRIGHTNESS, 0, 0);       // Setze Stunde
  if (hpos==-1) { hpos=12; }                                    // Rollover um 00:0 bzw. 12:00
  leds[(round(hpos-1)%NUM_LEDS)] = CRGB( BRIGHTNESS/4, 0, 0);   // Setze VORlaufende Stunde
  leds[(spos/60)%NUM_LEDS] = CRGB( 0, BRIGHTNESS, 0);           // Setze Minuten
  
  FastLED.show();                                               // Und SHOWTime
  FastLED.delay(1000/FRAMES_PER_SECOND);                        // Warten

  fadeToBlackBy(leds,NUM_LEDS,5);                               // Alles was nicht neu gesetzt wurde ausfaden
}

unsigned long get_spos() {  // Returned die aktuelle Zeit seit Mitternacht in Sekunden
  WireRtcLib::tm* t = rtc.getTime();
  return ( (long(t->hour%12)*3600)+(long(t->min)*60)+(t->sec) );
}

