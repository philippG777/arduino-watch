Arduino-Watch
=============

This is my little 7 segment arduino-watch.
The goal was to make a watch with a very low power consumtion.

Hardware
--------
It's based on a ATmega328PU, a single CR2032-cell, some buttons, some capacitors, a resistor and a 4-digit 7-segment-display like [this](https://www.neuhold-elektronik.at/datenblatt/N1021.pdf). 

Low-Power
---------
I use a [standalone Arduino](https://www.arduino.cc/en/Tutorial/ArduinoToBreadboard) with a watch-crystal.
I disabled the brown-out-detector in the bootloader for a lower powerconsumtion.
Because of the modified bootloader, the watch has a powerconsumtion of around 1.2µA in sleepmode.
The standard booloader has a powerconsumtion of about 16µA.

Version History
---------------
* **v1.1** 18.06.2016
* **v2.0** 20.06.2016
