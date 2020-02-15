EESchema Schematic File Version 2
LIBS:opto
LIBS:power
LIBS:device
LIBS:switches
LIBS:relays
LIBS:motors
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:AHU_1-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L CY8C5888LTI-LP097 U1
U 1 1 5D73DB77
P 5650 3600
F 0 "U1" H 5650 117 60  0001 C CNN
F 1 "CY8C5888LTI-LP097" H 5650 4200 60  0001 C CNN
F 2 "Oddities:CY8CKIT-059" H 5650 3600 60  0001 C CNN
F 3 "" H 5650 3600 60  0001 C CNN
	1    5650 3600
	1    0    0    -1  
$EndComp
Wire Wire Line
	4250 2700 3800 2700
Wire Wire Line
	3800 2700 3800 3300
Wire Wire Line
	4250 3000 3800 3000
Connection ~ 3800 3000
Wire Wire Line
	4250 3200 3800 3200
Connection ~ 3800 3200
$Comp
L GND #PWR01
U 1 1 5D73DBFE
P 3800 3300
F 0 "#PWR01" H 3800 3050 50  0001 C CNN
F 1 "GND" H 3800 3150 50  0000 C CNN
F 2 "" H 3800 3300 50  0001 C CNN
F 3 "" H 3800 3300 50  0001 C CNN
	1    3800 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	7050 2900 7500 2900
Wire Wire Line
	7500 2900 7500 3100
Wire Wire Line
	7500 3100 7050 3100
$Comp
L GND #PWR02
U 1 1 5D73DC24
P 7500 3100
F 0 "#PWR02" H 7500 2850 50  0001 C CNN
F 1 "GND" H 7500 2950 50  0000 C CNN
F 2 "" H 7500 3100 50  0001 C CNN
F 3 "" H 7500 3100 50  0001 C CNN
	1    7500 3100
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR03
U 1 1 5D73DC48
P 5350 5300
F 0 "#PWR03" H 5350 5050 50  0001 C CNN
F 1 "GND" H 5350 5150 50  0000 C CNN
F 2 "" H 5350 5300 50  0001 C CNN
F 3 "" H 5350 5300 50  0001 C CNN
	1    5350 5300
	1    0    0    -1  
$EndComp
Wire Wire Line
	4250 2900 3800 2900
Connection ~ 3800 2900
$EndSCHEMATC
