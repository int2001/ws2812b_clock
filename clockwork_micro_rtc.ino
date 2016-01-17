/*
 * Connect Data-In of Strip to PIN5
 * Connect Power of WS2812 to own supply, don't forget to connect GND to mC AND Strip
 * Connect RTC-SDA to PIN2, RTC-SCL to PIN3
 */
#include "FastLED.h"              // FastLED-Lib

#include <Wire.h>                 // i2c-Lib
#include <WireRtcLib.h>           // RTC-Lib
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
  rtc.begin();                    // RTC init
}


void loop()
{ 
  unsigned long spos=get_spos();  // Ask RTC for time
  if (o_spos != spos) {           // New second?
    o_spos=spos;                  // Set old to new
    millis_offset=millis();       // save internal millisec.-Counter of the mC to offset 
  }
  for (byte i=0;i<NUM_LEDS;i++) { // Loop all LEDs once (for the "Mark"-LEDs)
    if (i%15 == 0) {              // Position modulus 15 equals 0?
      leds[i]=CRGB(3,3,3);        // then SET
    }
  }
  
  // Hours
  float hpos=(((float)spos/3600)*60/12);                                      // Calculate Hourposition for the 60 divisors as floatingpoint
  set_led((round(hpos+1)%NUM_LEDS),BRIGHTNESS/4, 0, 0);                       // SET hour after the current (as dimmed)
  set_led((round(hpos)%NUM_LEDS),BRIGHTNESS, 0, 0);                           // SET hour
  if (hpos==-1) { hpos=12; }                                                  // Rollover at 00:00 / 12:00
  set_led((round(hpos-1)%NUM_LEDS),BRIGHTNESS/4, 0, 0);                       // SET hour before the current (as dimmed)
  
  // Minutes
  float nbright=((float)spos/60)-int(spos/60);                                // Calculate fading for Minutes (Fraction)
  nbright=(float)1-(cos8((float)nbright*127)/(float)255);                     // Calculate cosinusvalue based on fraction for smoother fading (for only one pi)
  byte fraction=ceil(nbright*BRIGHTNESS);                                     // Save our new fraction
  if (fraction>BRIGHTNESS) { fraction=BRIGHTNESS; }                           // Result greater than max-brightness? Cut it to max-brightness
  set_led(((spos/60))%NUM_LEDS,0, BRIGHTNESS-fraction, 0);                    // Set Minute to BRIGHTNESS - (BRIGHTNESS * Fraction)
  set_led(((spos/60)+1)%NUM_LEDS,0, fraction, 0);                             // Set Minute+1 to (BRIGHTNESS * Fraction)

  // Seconds
  nbright=(float)(millis()-millis_offset)/1000;                               // Do the same for the seconds, but based on our millis()
  nbright=(float)1-(cos8((float)nbright*127)/(float)255);
  fraction=ceil(nbright*BRIGHTNESS);                                 
  if (fraction>BRIGHTNESS) { fraction=BRIGHTNESS; }                           // Result greater than max-brightness? Cut it to max-brightness
  set_led(spos%NUM_LEDS,0, 0, BRIGHTNESS-fraction); 
  set_led((spos+1)%NUM_LEDS,0, 0, fraction);
  
  FastLED.show();                                               // And SHOWTime
  FastLED.delay(1000/FRAMES_PER_SECOND);                        // Wait
  for(int i = 0; i < NUM_LEDS; i++) { leds[i] = CRGB(0,0,0); }  // Set all LEDs off
}

unsigned long get_spos() {                                      // Returns timeticks after midnight in seconds
  WireRtcLib::tm* t = rtc.getTime();
  return ( (long(t->hour%12)*3600)+(long(t->min)*60)+(t->sec) );
}

void set_led (byte which_one, byte r,byte g, byte b) {
  leds[which_one] += CRGB(r,g,b);                               // Add new color to old one
}


