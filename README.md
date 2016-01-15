# hcsr05
Low level program for using the hcsr05 distance sensor in C

The HCSR05 uses a timer in capture mode to measure the distance
between an object and itself. The measure can then be filtered using
an average filter and displayed on a terminal through serial communication.



This project uses an Atmega2560, though it can be ported to any microcontroller with the appropiate Timer.

