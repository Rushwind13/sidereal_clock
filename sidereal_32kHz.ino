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

#undef  TIMER0
#define TIMER1
#undef  TIMER2

// Arduino timer CTC interrupt example
// www.engblaze.com
 
// avr-libc library includes
#include <avr/io.h>
#include <avr/interrupt.h>

#define SID_TICK_PIN 13 // PORTB 5

void setup()
{
    pinMode(SID_TICK_PIN, OUTPUT);
    delay(500);
       
    // initialize Timer
    cli();        // disable global interrupts
    setup_Timer();
    sei();        // enable global interrupts:
}


void setup_Timer1()
{
    TCCR1A = 0;     // set entire TCCR1A register to 0
    TCCR1B = 0;     // same for TCCR1B
    TCNT1 = 0;      // only used for OVF
    /*
     * Bob notes:
     * So, 65536 yields 0.5hz, because it toggles the pin at count = max-ticks.
     * That makes 1.0 hz at 32,768 ticks.
     * 
     * I measured and adjusted for the interrupt routine. It came out to about 31300, but finer adjustments would 
     * take a lot more time. Instead I divided that by 100 and started much finer tuning at 313 ticks (which should be 100 hz), and adjusted.
     * 
     * This number came out to be 311 ticks producing a 100.0Hz signal. So multiplied by 0.99726958, we have 310.15083938 ticks to produce a 100.0hz sidereal time signal on pin 13.
     * 
     * Since the more ticks we give the function, the higher accuracy we can get, it might have to be "good enough" to display tenths of a second with this setup. (Though this is 3101.5083938 ticks)
     * One twentieth would be 3110 / 20 = 155.5, multiplied by 0.997 gives 155.075 ticks making a 20hz sidereal signal, so it's a much lower remainer).
     * 
     * More testing is required.
     */
    // set compare match register to desired timer count:
    //OCR1A = 311;
    
    // Jimbo notes
    // with /256 prescaler
    //OCR1A = 31250;  // "1.00s" freq on my meter
    //OCR1A = 3125;   // "100ms" freq on my meter
    //OCR1A = 312;    // "10.0ms" freq on my meter
    //OCR1A = 31335;  // "1.00s" freq on my meter (expected: 1.002)
    //OCR1A = 3133;   // "100ms" freq on my meter (expected: 100.2)
    //OCR1A = 313;    // "10.0ms" freq on my meter
    // At this point, I started running into the resolution limit
    // of my tiny screen and cursor positions. I measured the 
    // half-wavelength (rise-to-fall) at somewhere around 5.02ms
    // or 5.04ms, it was definitely above 5.0, but a bit jittery.
    // That would be about 10.4 to 10.8ms for the full wave, which 
    // is a little too long, but I think this is measurement accuracy
    // rather than timing accuracy.

    // with 0 prescaler
    //OCR1A = 0;        // I get a very jittery 4.00us freq for this (I think this is beyond my resolution limit)
    //OCR1A = 10000;    // 800Hz = 16m/2/800
    //OCR1A = 32768;    // I get a jittery 4.12ms freq for this (this seems like a more stable number, I had calculated a .004096sec = 4.096ms for this)

    OCR1A = 18181;    // 440Hz (concert A) -- play music on your Arduino!

    // 10^n Hz 
    //OCR1A = 8000;     // 1.00ms = 1kHz (facepalm, 16MHz / 2 / 1000)
    //OCR1A = 1600;     // 5kHz (800 was too jittery for me to measure)

    // 10^n Hz (sidereal)
    //OCR1A = 7977;     // expected 1kHz sidereal (@bob: please measure)
    //OCR1A = 797;      // expected 10kHz sidereal (please measure)

    // 10^n * 256 (multiples of CPU freq)
    //OCR1A = 25600;    // 3.2ms (312.5Hz)
    //OCR1A = 2560;     // 320uS = 3125Hz
    //OCR1A = 256;      // 32uS = 31250Hz

    // 2^15 sidereal and solar (divided by factors of 10^n)
    //OCR1A = 24414;    // 3.05ms (~327.68Hz) (16M/2/32k)*100
    //OCR1A = 24347;    // 3.04ms (~328.57Hz)
    //OCR1A = 2441;     // 304uS expected: 305uS = 3276.8 Hz
    //OCR1A = 2434;     // 304uS expected: 304uS = 3285.7 Hz

    
    // I guess I shouldn't be surprised that multiples of 256 yield
    // very obvious numbers, when the /256 prescaler popped those 
    // same numbers out.

    // With no prescaler, it's quite easy to pull out a lot of
    // resolution if you're looking for musical-note-speed
    // (440Hz to 1000Hz or so).

    // In any case, I think that we have a decent 1kHz solar and
    // sidereal timebase. I'm going to try it without the prescaler
    // and see if I can improve the resolution.
    
    // turn on CTC mode:
    TCCR1B |= (1 << WGM12);
    // Set CS10 bit (reset CS11 and CS12 bits) for no prescaler:
    TCCR1B |= (0 << CS12); //I had to change this bit instead of CS10, 
    TCCR1B |= (0 << CS11); // 100 = /256; 001 = 1x
    TCCR1B |= (1 << CS10);
    // enable timer overflow interrupt:
    TIMSK1 |= (1 << OCIE1A);
}
void setup_Timer() { setup_Timer1(); }

void loop()
{
    delay(1000);
}

ISR(TIMER1_COMPA_vect)
{
  //Toggles digital pin 13. I'm glad you knew about the fast I/O toggle strategy :)
    PORTB ^= B00100000;
}

