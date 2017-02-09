/*
 * Sidereal "seconds" hand - Produce a sidereal timebase on Arduino
 *
 * This sketch can either toggle pins at 1Hz or 32768Hz (to be used by another
 * Arduino for creating a whole clock), or it can display the elapsed solar and 
 * sidereal seconds to an LCD (using the Adafruit LCD library, documented below)
 *
 * Pins are documented below.
 *
 * This sketch has been tested, and uses known-good constants, but I am not 
 * guaranteeing that an out-of-the-box Arduino can create a timebase of 
 * sufficient accuracy for your project; that part is left as an exercise for 
 * the reader.
 *
 * Created by Jimbo S. Harris 1/13/2013
 * 
 * Released to the Arduino and amateur astronomy community to use as you wish,
 * but please credit my work.
 *
 * If you re-use this software, it would be nice if you let me know about it.
 * http://www.jimbo.net/circuitfarm/2013/01/sidereal-seconds-hand/
 *
 * Clear skies to you.
 */

/*
Timer reminders
Timer0:
Timer0 is a 8bit timer. 
In the Arduino world timer0 is been used for the timer functions, like delay(), millis() and micros(). If you change timer0 registers, this may influence the Arduino timer function. So you should know what you are doing. 
It uses pins 5 and 6.

Timer1:
Timer1 is a 16bit timer. 
In the Arduino world the Servo library uses timer1 on Arduino Uno (timer5 on Arduino Mega).
It uses pins 9 and 10.

Timer2:
Timer2 is a 8bit timer like timer0. 
In the Arduino work the tone() function uses timer2.
It uses pins 11 and 3.
*/

#undef  TIMER0
#define TIMER1
#undef  TIMER2

/*
####
#
# DEFINITIONS
#
####

# want this many ticks per second
int solar_ticks = 32768.0
# sidereal seconds are this much faster than solar ones
double sidereal_multiplier = 1.00273790935

# so you want this many ticks for a sidereal second
# (THIS IS THE NUMBER OF TICKS WE WANT PER SOLAR SECOND)
sidereal_ticks = solar_ticks*sidereal_multiplier
target_ticks = sidereal_ticks

# CPU speed
cpu_ticks = 16000000.0

#ifdef TIMER0
# timer overflows after this many ticks
timer_ticks = 256.0
#endif /* TIMER0 */

#ifdef TIMER1
# timer overflows after this many ticks
timer_ticks = 65536.0
#endif /* TIMER1 */

#ifdef TIMER2
# timer overflows after this many ticks
timer_ticks = 256.0
#endif /* TIMER2 */

# which results in this many timer overflows per sec
# (THIS IS THE NUMBER OF TIMES YOU GET TO CHECK PER SECOND)
# The target ticks must be smaller than this number, or you must scale
# things so that happens
timer_overflows_per_sec = cpu_ticks / timer_ticks
max_ticks = timer_overflows_per_sec

*/

//
// Pre-calculate all the big numbers to help the Arduino
//

// To get 1Hz (sidereal), add "1" (scaled) to a second tick counter everytime 
// the 32kHz (sidereal) counter goes off
// Note, using "1" made a 2Hz signal (because a 32k crystal is for toggling a 16-bit register),
// so add 2 (scaled) instead.
static unsigned long sid_hz_add = 2;
static unsigned long sid_hz_limit = 32768;

// Solar seconds are just a 1:1 ratio with the timer
static unsigned long sol_hz_add = 2;
static unsigned long sol_hz_limit = 32894;

// The rolling counters
unsigned long sid_ticks = 0;
unsigned long sid_hz_ticks = 0;
unsigned long sol_hz_ticks = 0;

// Arduino timer CTC interrupt example
// www.engblaze.com
 
// avr-libc library includes
#include <avr/io.h>
#include <avr/interrupt.h>

// 2013.02.14 Added LCD output
#include "Wire.h"
#include "LiquidCrystal.h"

/*
  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * 10K potentiometer:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 */
//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
//#define I2C
//#undef SPI
//
//#ifdef SPI
//#undef I2C
//// SPI via 595 (adafruit library)
//// Parameter order: Data (14), Clock(11), Latch(12)
//LiquidCrystal lcd(2, 4, 3);
//#endif
//
//
//#ifdef I2C
//#undef SPI
//// I2C via MCP23008 (adafruit library)
//LiquidCrystal lcd(0);
//#endif
 
#define SID_TICK_PIN 13 // PORTB 5
#define SID_HZ_PIN 10   // PORTB 2
#define SOL_HZ_PIN 8    // PORTB 0
#define COUNTER_PIN 7   // for pulseIn()

long solar_secs;
long sidereal_secs;

void setup()
{
    pinMode(SID_TICK_PIN, OUTPUT);
    pinMode(SID_HZ_PIN, OUTPUT);
    pinMode(SOL_HZ_PIN, OUTPUT);
    pinMode(COUNTER_PIN, INPUT );

    Serial.begin(9600);
 
   // set up the LCD's number of rows and columns: 
//  lcd.begin(16, 2);
//  #ifdef SPI
//  lcd.print("SPI");
//  #endif
//  #ifdef I2C
//  lcd.print("I2C");
//  #endif
  delay(500);
   // init counters
   solar_secs=0;
   sidereal_secs=0;
   
    // initialize Timer
    cli();        // disable global interrupts

    Serial.println("Calling setup_Timer()...");
    setup_Timer();
    Serial.println("return from setup_Timer()...");
    
    sei();        // enable global interrupts:
}


#ifdef TIMER1
void setup_Timer1()
{
    Serial.println( "Starting Timer1 setup..." );
    TCCR1A = 0;     // set entire TCCR1A register to 0
    TCCR1B = 0;     // same for TCCR1B

    // set compare match register to desired timer count:
    OCR1A = 600;
    //OCR1A = 15624;
    // turn on CTC mode:
    TCCR1B |= (1 << WGM12);
    // Set CS10 bit (reset CS11 and CS12 bits) for no prescaler:
    TCCR1B |= (0 << CS12);
    TCCR1B |= (0 << CS11);
    TCCR1B |= (1 << CS10);
    // enable timer overflow interrupt:
    TIMSK1 |= (1 << OCIE1A);
    Serial.println( "Timer1 setup complete." );
}
void setup_Timer() { setup_Timer1(); }
#endif /* TIMER1 */


#ifdef TIMER2
void setup_Timer2()
{
    TCCR2A = 0;     // set entire TCCR1A register to 0
    TCCR2B = 0;     // same for TCCR1B

    // set compare match register to desired timer count:
    TCNT2 = 0;
    // turn on CTC mode:
    //TCCR2B |= (1 << WGM12);
    // Set CS10 and CS12 bits for 1024 prescaler:
    TCCR2B |= (0 << CS22);
    TCCR2B |= (0 << CS21);
    TCCR2B |= (1 << CS20);
    // enable timer overflow interrupt:
    TIMSK2 |= (1 << TOIE2);
}
void setup_Timer() { setup_Timer2(); }
#endif /* TIMER2 */

#ifdef TIMER0
void setup_Timer0()
{
    
    /// Not Implemented Yet ///

    TCCR2A = 0;     // set entire TCCR1A register to 0
    TCCR2B = 0;     // same for TCCR1B

    // set compare match register to desired timer count:
    TCNT2 = 0;
    // turn on CTC mode:
    //TCCR2B |= (1 << WGM12);
    // Set CS10 and CS12 bits for 1024 prescaler:
    TCCR2B |= (0 << CS22);
    TCCR2B |= (0 << CS21);
    TCCR2B |= (1 << CS20);
    // enable timer overflow interrupt:
    TIMSK2 |= (1 << TOIE2);]

    
    /// Not Implemented Yet ///

}
void setup_Timer() { setup_Timer0(); }
#endif /* TIMER0 */

unsigned long duration;
float micro_to_sec = 1.0 / 1000000.0;
float full_wave = 2.0;
void loop()
{
    solar_secs++;
    //duration = pulseIn( COUNTER_PIN, HIGH );
    Serial.print( solar_secs );
    Serial.print(" ");
    Serial.print( sidereal_secs - solar_secs );
    Serial.print(" ");
    Serial.print( sidereal_secs );
    Serial.print("    ");
    Serial.print( sid_ticks );
    Serial.print("    ");
    //Serial.print( duration,5 );
    Serial.println();
    
    // do some crazy stuff while my LED keeps blinking
//    lcd.setCursor(0,0);
//    lcd.print("sol:");
//    lcd.print(solar_secs);
//    lcd.setCursor(0,1);
//    lcd.print("sid:");
//    lcd.print(sidereal_secs);
    delay(1000);
}

#ifdef TIMER0
ISR(TIMER2_OVF_vect)
//ISR(TIMER1_COMPA_vect)
{
    digitalWrite(SID_TICK_PIN, !digitalRead(SID_TICK_PIN));
}
#endif

ISR(TIMER1_COMPA_vect)
{
    sid_ticks = sid_ticks + 1;  
    
    // Check for 32kHz(sidereal) overflow
    if( sid_ticks >= 32894 )
    {
      // Keep the remainder
      sid_ticks = 0;//sid_ticks - 32768;
    
      // If you want 1Hz(sidereal), oscillate a digital pin here
      sidereal_secs++;
      PORTB ^= B00000100;
    }
    
    // If you want 32kHz(sidereal), oscillate a digital pin here
    PORTB ^= B00100000;
}

#ifdef TIMER2
//ISR(TIMER2_OVF_vect)
ISR(TIMER2_COMPA_vect)
{
  // test toggling pin as fast as possible
  //PORTB |= B00100000;
  //PORTB &= ~B00100000;
  PORTB ^= B00100000;
  return;
  
  // add the correct number of ticks to the 32kHz(sid) and 1Hz(sol) accumulators
  sid_ticks = sid_ticks + sid_ticks_add;
  sol_hz_ticks = sol_hz_ticks + sol_hz_add;
  
  // Check for 32kHz(sidereal) overflow
  if( sid_ticks >= sid_ticks_limit )
  {
    // Keep the remainder
    sid_ticks = sid_ticks - sid_ticks_limit;
    
    // add ticks to allow a 1Hz(sidereal) output
    sid_hz_ticks = sid_hz_ticks + sid_hz_add;
    
    // If you want 32kHz(sidereal), oscillate a digital pin here
    PORTB ^= B00100000;
  }
  
  // Check for 1Hz(sidereal) overflow
  if( sid_hz_ticks >= sid_hz_limit )
  {
    // Keep the remainder
    sid_hz_ticks = sid_hz_ticks - sid_hz_limit;
    
    // If you want 1Hz(sidereal), oscillate a digital pin here
                PORTB ^= B00000100;
                sidereal_secs++;
  }
  
  // Check for 1Hz(solar) overflow
  if( sol_hz_ticks >= sol_hz_limit )
  {
    while( sol_hz_ticks >= sol_hz_limit )
    {
      // Keep the remainder
      sol_hz_ticks = sol_hz_ticks - sol_hz_limit;
    // If you want 1Hz(solar), oscillate a digital pin here
    PORTB ^= B00000001;
    }
    
    solar_secs++;
  }
}
#endif /* TIMER2 */