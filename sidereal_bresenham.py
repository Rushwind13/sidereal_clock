#!/usr/bin/python
#
# Bresenham Sidereal Algorithm
#
# To simulate producing 32768 ticks per sidereal second (32857.715 ticks per solar second)
# This algorithm could produce an oscillating "clock crystal" timebase for a 
# sidereal clock circuit, based on an accurate microcontroller timer
# The microcontroller parameters can be configured as long as the constraint noted in the
# comments is met. This program is built around an Arduino ATMega 328 uC:
# 16MHz clock, 8-bit timer
#
# Algorithm developed using the Bresenham technique as described by Roman Black at
# http://www.romanblack.com/one_sec.htm
#
# My general approach was to do:
#
# bres += target_frequency
# if bres >= cpu_frequency
# 	bres -= cpu_frequency
#	tick()
#
# where target_frequency is the number of desired ticks per second (32857.715), and
# cpu_frequency is cpu ticks per second / timer overflow limit (16million / 256)
#
# I scale up the 2 frequencies (for more accuracy) using a scalar that does not allow
# either value to exceed MAX_INT (~4 billion)
#
# created 1/7/2012 by Jimbo S. Harris
# Use as you wish, but please credit my work as well as Roman Black's work.
#
#

####
#
# DEFINITIONS
#
####

# want this many ticks per second
solar_ticks = 32768.0

# sidereal seconds are this much faster than solar ones
sidereal_multiplier = 1.00273790935

# so you want this many ticks in a sidereal second
# (THIS IS THE NUMBER OF TICKS WE WANT PER SOLAR SECOND)
sidereal_ticks = solar_ticks*sidereal_multiplier
target_ticks = sidereal_ticks

# CPU speed
cpu_ticks = 16000000.0

# timer overflows after this many ticks
timer_ticks = 256.0

# which results in this many timer overflows per sec
# (THIS IS THE NUMBER OF TIMES YOU GET TO CHECK PER SECOND)
# The target ticks must be smaller than this number, or you must scale
# things so that happens
timer_overflows_per_sec = cpu_ticks / timer_ticks
max_ticks = timer_overflows_per_sec

# For a little extra accuracy, you can scale both of the numbers up
# try this bresenham scaler (want this to be set so that the largest of
# the compared numbers are less than 2^32 (about 4billion)
bresenham_scaler = 70000.0

bresenham_add = target_ticks * bresenham_scaler
bresenham_limit = max_ticks * bresenham_scaler

####
#
# TEST SCRIPT
#
####
count = 0
ticks = 0.0
output = 0
test_secs = 1000
count2 = 0
test_limit = (max_ticks * test_secs)
out_limit = (max_ticks * 10)
max_remainder = 0.0
min_remainder = 1000000.0

####
#
# PUT THIS INSIDE THE INTERRUPT ROUTINE
#
####
while count < test_limit:
	count = count + 1
	count2 = count2 + 1
	ticks = ticks + bresenham_add
	if ticks > bresenham_limit:
		ticks = ticks - bresenham_limit
		# This is where you oscillate the output clock pin

####
# Everything below here is just extra-curricular stuff I put in to make sure the 
# algorithm was working properly. It's not needed for the calculation
####
		if ticks < min_remainder: min_remainder = ticks
		if ticks > max_remainder: max_remainder = ticks
		output = output + 1
	if count2 >= out_limit:
		count2=0
		elapsed_sec = count / max_ticks
		elapsed_sid = output / solar_ticks
		extra_sid = elapsed_sid - elapsed_sec
		print "elapsed solar sec %d elapsed sidereal ticks %d extra sidereal secs %f" % (elapsed_sec,output,extra_sid)
		#print "tick! 32k %f elapsed overflows %d remainder %.02f (%f / %f)" % (output/solar_ticks,count/max_ticks, (ticks / bresenham_add), (min_remainder / bresenham_scaler) * target_ticks, target_ticks *(target_ticks - (max_remainder / bresenham_scaler)) )

print "A timer algorithm to produce 2^15 clock pulses every sidereal second."
print "Note: sidereal ticks at any power of 10 solar seconds are = to 32857.7158135808 times that power of 10"
print "Note: You get 1 extra sidereal second every 366.25 solar seconds."
