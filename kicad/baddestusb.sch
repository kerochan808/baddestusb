EESchema Schematic File Version 4
EELAYER 30 0
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
Text GLabel 2000 1500 0    50   Input ~ 0
LITE
Text GLabel 2000 1750 0    50   Input ~ 0
MISO
Text GLabel 2000 2000 0    50   Input ~ 0
SCK
Text GLabel 2000 2250 0    50   Input ~ 0
MOSI
Text GLabel 2000 2500 0    50   Input ~ 0
TFT_CS
Text GLabel 2000 2750 0    50   Input ~ 0
CARD_CS
Text GLabel 2000 3000 0    50   Input ~ 0
DC
Text GLabel 2000 3250 0    50   Input ~ 0
RESET
Text GLabel 2000 3500 0    50   Input ~ 0
VCC
Text GLabel 2000 3750 0    50   Input ~ 0
GND
Text Notes 1100 1300 0    50   ~ 0
ST7735 TFT/SD Breakout
Text GLabel 4000 1250 1    50   Input ~ 0
GP16
Text GLabel 4250 1250 1    50   Input ~ 0
GP18
Text GLabel 4500 1250 1    50   Input ~ 0
GP19
Text Notes 3850 850  0    50   ~ 0
RPi Pico
Wire Wire Line
	2000 1750 4000 1750
Wire Wire Line
	4000 1250 4000 1750
Wire Wire Line
	4250 1250 4250 2000
Wire Wire Line
	4500 1250 4500 2250
Wire Wire Line
	2000 2250 4500 2250
Wire Wire Line
	2000 2000 4250 2000
Text GLabel 3000 1250 1    50   Input ~ 0
GP7
Text GLabel 3250 1250 1    50   Input ~ 0
GP8
Text GLabel 3500 1250 1    50   Input ~ 0
GP9
Text GLabel 3750 1250 1    50   Input ~ 0
GP10
Text GLabel 4750 1250 1    50   Input ~ 0
GP20
Text GLabel 5000 1250 1    50   Input ~ 0
GP21
Text GLabel 5250 1250 1    50   Input ~ 0
GP22
Text GLabel 5500 1250 1    50   Input ~ 0
3v3
Text GLabel 5750 1250 1    50   Input ~ 0
GND
Wire Wire Line
	3000 1250 3000 2750
Wire Wire Line
	3000 2750 2000 2750
Wire Wire Line
	3250 1250 3250 3000
Wire Wire Line
	3250 3000 2000 3000
Wire Wire Line
	3500 1250 3500 3250
Wire Wire Line
	3500 3250 2000 3250
Wire Wire Line
	3750 1250 3750 2500
Wire Wire Line
	3750 2500 2000 2500
Wire Wire Line
	5500 1250 5500 3500
Wire Wire Line
	5500 3500 2000 3500
Wire Wire Line
	5750 3750 2000 3750
Text GLabel 2750 1250 1    50   Input ~ 0
GP6
Wire Wire Line
	2750 1250 2750 1500
Wire Wire Line
	2750 1500 2000 1500
Text Notes 6550 1300 0    50   ~ 0
Buttons
$Comp
L Switch:SW_Push SW0
U 1 1 61A0F503
P 6700 2000
F 0 "SW0" H 6700 2285 50  0000 C CNN
F 1 "Button 0" H 6700 2194 50  0000 C CNN
F 2 "" H 6700 2200 50  0001 C CNN
F 3 "~" H 6700 2200 50  0001 C CNN
	1    6700 2000
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_Push SW1
U 1 1 61A10786
P 6700 2500
F 0 "SW1" H 6700 2785 50  0000 C CNN
F 1 "Button 1" H 6700 2694 50  0000 C CNN
F 2 "" H 6700 2700 50  0001 C CNN
F 3 "~" H 6700 2700 50  0001 C CNN
	1    6700 2500
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_Push SW2
U 1 1 61A11AE7
P 6700 3000
F 0 "SW2" H 6700 3285 50  0000 C CNN
F 1 "Button 2" H 6700 3194 50  0000 C CNN
F 2 "" H 6700 3200 50  0001 C CNN
F 3 "~" H 6700 3200 50  0001 C CNN
	1    6700 3000
	1    0    0    -1  
$EndComp
Wire Wire Line
	4750 1250 4750 2000
Wire Wire Line
	4750 2000 6500 2000
Wire Wire Line
	5000 1250 5000 2500
Wire Wire Line
	5000 2500 6500 2500
Wire Wire Line
	5250 1250 5250 3000
Wire Wire Line
	5250 3000 6500 3000
Wire Wire Line
	5750 1250 5750 3500
Wire Wire Line
	6900 2000 7250 2000
Wire Wire Line
	7250 2000 7250 2500
Wire Wire Line
	7250 2500 6900 2500
Wire Wire Line
	7250 2500 7250 3000
Wire Wire Line
	7250 3000 6900 3000
Connection ~ 7250 2500
Wire Wire Line
	5750 3500 7250 3500
Wire Wire Line
	7250 3500 7250 3000
Connection ~ 5750 3500
Wire Wire Line
	5750 3500 5750 3750
Connection ~ 7250 3000
$EndSCHEMATC
