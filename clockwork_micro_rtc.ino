#include "FastLED.h"

// RTC
#include <Wire.h>
#include <WireRtcLib.h>
WireRtcLib rtc;                   // RTC Object

#define DATA_PIN    5             // Strip is attached to PIN5
#define LED_TYPE    WS2812        // We have a WS2812
#define COLOR_ORDER GRB           // Colorordering of the Strip is GRB 
#define NUM_LEDS    60            // We have 60 LED's
CRGB leds[NUM_LEDS];              // Our Control-Array is called "leds"

#define BRIGHTNESS          42    // Brightness Constant
#define FRAMES_PER_SECOND  120    // Delays (s.u.) between updates

unsigned long spos=0;             // Global Var to hold seconds
unsigned long o_spos=-1;          // Global Var to hold previous seconds
unsigned long millis_offset=0;    // Global var to hold our calculated offset (see loop)

void setup()
{
  delay(4000);                    // Give the controler some Time to warm-up
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip); // initialize the LEDs
  Wire.begin();                   // init the i2c-Bus (IMPORTANT! Without this, there's RTC-Trouble :)
  rtc.begin();                    // RTC initi
}


void loop()
{ 
  unsigned long spos=get_spos();  // Ask RTC for time
  if (o_spos != spos) {           // New second?
    o_spos=spos;                  // Set old to new
    millis_offset=millis();       // save internal millisec.-Counter of the mC to offset 
  }
  for (byte i=0;i<NUM_LEDS;i++) { // Loop all LEDs once (for the "Mark"-LEDs)
    if (i%5 == 0) {               // Position modulus 5 equals 0?
     // leds[i]=CRGB(2,2,0);      // then SET
    }
    if (i%15 == 0) {              // Position modulus 15 equals 0?
      leds[i]=CRGB(3,3,3);        // then SET
    }
  }
  
  float hpos=(((float)spos/3600)*60/12);                                      // Calculate Hourposition for the 60 divisors as floatingpoint
  set_led((round(hpos+1)%NUM_LEDS),BRIGHTNESS/4, 0, 0);                       // SET hour after the current (as dimmed)
  set_led((round(hpos)%NUM_LEDS),BRIGHTNESS, 0, 0);                           // SET hour
  if (hpos==-1) { hpos=12; }                                                  // Rollover at 00:00 / 12:00
  set_led((round(hpos-1)%NUM_LEDS),BRIGHTNESS/4, 0, 0);                       // SET hour before the current (as dimmed)
  float nbright=((float)spos/60)-int(spos/60);                                // Calculate fading for Minutes (Fraction)
  set_led(((spos/60))%NUM_LEDS,0, BRIGHTNESS-ceil(nbright*BRIGHTNESS), 0);    // Set Minute to BRIGHTNESS - (BRIGHTNESS * Fraction)
  set_led(((spos/60)+1)%NUM_LEDS,0, ceil(nbright*BRIGHTNESS), 0);             // Set Minute+1 to (BRIGHTNESS * Fraction)
  
  nbright=(float)(millis()-millis_offset)/1000;                               // Do the same for the seconds, but based on our millis()
  byte sekufraction=ceil(nbright*BRIGHTNESS);                                 
  if (sekufraction>BRIGHTNESS) { sekufraction=BRIGHTNESS; }             
  set_led(spos%NUM_LEDS,0, 0, BRIGHTNESS-sekufraction); 
  set_led((spos+1)%NUM_LEDS,0, 0, sekufraction);
  
  FastLED.show();                                               // And SHOWTime
  FastLED.delay(1000/FRAMES_PER_SECOND);                        // Wait
  for(int i = 0; i < NUM_LEDS; i++) { leds[i] = CRGB(0,0,0); }
}

unsigned long get_spos() {  // Returns timeticks after midnight in seconds
  WireRtcLib::tm* t = rtc.getTime();
  return ( (long(t->hour%12)*3600)+(long(t->min)*60)+(t->sec) );
}

void set_led (byte which_one, byte r,byte g, byte b) {
  if (which_one % 15 == 0) {  // This is a "Mark"-LED
    if (r<=3) { r=3; }
    if (g<=3) { g=3; }
    if (b<=3) { b=3; }
  }
  leds[which_one] += CRGB(r,g,b); // Add new color to old one
}


