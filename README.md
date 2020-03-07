# PVCC2-AHU

This project is a subset of a larger monitoring system of air handlers,
temperature and humidity sensors, air pressure sensors, and relays for a nursing
home.  The AHU (Air Handling Unit) projects all use Cypress PSoC 5LP chips in
combination with Wemos D1 Mini (ESP8266) chips to communicate over WiFi mqtt 
messages to a Raspberry PI running Mosquitto and displaying the results using Python.

The **ds18b20**, **max31855**, **max31865**, and **one-wire** files are common
to all PSoC projects in this workspace.  Only the main.c programs along with the 
hardware design are unique to each monitoring setup.  And the only difference 
between the two types of setups is that one (AHU-1) uses MAX31865 chips and the 
other (AHU-2) uses MAX31855 chips.

Parts of these programs came from examples others have provided, and have been 
modified to run on the Cypress hardware.  There weren't a whole lot of examples 
for those chips.  They are very easy to program and use, along with being 
relatively inexpensive to purchase.  PSoC chips were chosen over STM32, LPC and 
TI chips because of their flexibility for pin assignments and component usage.

The Wemos programming is done through Arduino.  The program for the air handlers
is designed to have the PSoC 5LP chips talk to the sensors, so it just acts as 
the communications between the PSoC and the mqtt world.

This is version 2 of the monitoring system.  The major change from version 1 is 
that this uses mqtt to queue messages on the Raspberry PI, and WiFi instead of 
RS485 as the primary communications method.  RS485 is still used, but in limited 
sections of the building, and only from the PSoC to a maximum of 10 devices, 
typically 0, 1, or 3 devices at a time.

Use what you want for anything.  Much of the code was taken from other samples.

## Requirements
To compile these programs you will need Cypress' PSoC Creator running on Windows.
It's free, but doesn't run on Linux (except in a virtual machine running Windows).  
All of the flash programming can be done from the machine that is running PSoC 
Creator, using the Cypress-supplied KitProg adaptors (which come with the 
CY8CKIT-059 boards I've been using).

There are no external make files.  This is all done as projects within one 
workspace in the Cypress IDE.

The boards I've been using are the PSoC 5lp, CY8C5888LTI-LP097 chips.  They are 
probably more expensive than Atmel, but the chips are very flexible to program 
and have some good examples.

Arduino 1.8.11 is used for the Wemos D1 Mini's.  It has the following library 
requirements -
- ESP8266WiFi at version 1.0
- ESP8266HTTPClient at version 1.2
- ESP8266WebServer at version 1.0
- async-mqtt-client at version 0.8.2
- ESPAsyncTCP at version 1.2.0
- Time at version 1.6
- Timezone at version 1.2.4
- EEPROM at version 1.0
- ArduinoOTA at version 1.0
- ESP8266mDNS at version 1.2




