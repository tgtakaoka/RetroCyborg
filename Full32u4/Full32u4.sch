EESchema Schematic File Version 4
LIBS:Full32u4-cache
EELAYER 26 0
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
L Connector:USB_B_Micro J1
U 1 1 5CC7EC11
P 3350 3050
F 0 "J1" H 3350 3450 50  0000 C CNN
F 1 "AE-USB-MICRO" H 3000 3400 50  0000 C CNN
F 2 "User-Boards:AE-USB-MICRO-B-D" H 3500 3000 50  0001 C CNN
F 3 "~" H 3500 3000 50  0001 C CNN
	1    3350 3050
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R3
U 1 1 5CC7DDF0
P 4950 1900
F 0 "R3" H 4800 1850 50  0000 L CNN
F 1 "10k" H 4750 1950 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P1.90mm_Vertical" H 4950 1900 50  0001 C CNN
F 3 "~" H 4950 1900 50  0001 C CNN
	1    4950 1900
	1    0    0    -1  
$EndComp
Wire Wire Line
	4950 2000 4950 2050
Wire Wire Line
	4950 2050 5000 2050
Wire Wire Line
	5700 1700 5600 1700
Wire Wire Line
	5600 1700 5600 1750
Text GLabel 5500 1600 1    50   Input ~ 0
VBUS
Wire Wire Line
	5500 1600 5500 1750
$Comp
L Device:R_Small R2
U 1 1 5CC7E2AA
P 4050 3050
F 0 "R2" V 4000 2950 50  0000 C CNN
F 1 "22" V 4000 3150 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P2.54mm_Vertical" H 4050 3050 50  0001 C CNN
F 3 "~" H 4050 3050 50  0001 C CNN
	1    4050 3050
	0    1    1    0   
$EndComp
$Comp
L Device:R_Small R1
U 1 1 5CC7E343
P 3850 3150
F 0 "R1" V 3950 3100 50  0000 C CNN
F 1 "22" V 3950 3200 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P2.54mm_Vertical" H 3850 3150 50  0001 C CNN
F 3 "~" H 3850 3150 50  0001 C CNN
	1    3850 3150
	0    1    1    0   
$EndComp
Wire Wire Line
	3650 3150 3750 3150
Wire Wire Line
	3950 3150 5000 3150
Wire Wire Line
	3650 3050 3950 3050
Wire Wire Line
	4150 3050 5000 3050
$Comp
L power:GND #PWR0103
U 1 1 5CC7E6A2
P 3350 3450
F 0 "#PWR0103" H 3350 3200 50  0001 C CNN
F 1 "GND" H 3355 3277 50  0000 C CNN
F 2 "" H 3350 3450 50  0001 C CNN
F 3 "" H 3350 3450 50  0001 C CNN
	1    3350 3450
	1    0    0    -1  
$EndComp
NoConn ~ 3250 3450
NoConn ~ 3650 3250
Wire Wire Line
	3650 2850 3750 2850
Wire Wire Line
	3950 2850 4050 2850
Wire Wire Line
	3650 2550 3650 2850
Connection ~ 3650 2850
Text GLabel 5000 2850 0    50   Input ~ 0
VBUS
Wire Wire Line
	4050 2850 4050 2550
$Comp
L Device:C_Small C2
U 1 1 5CC7F12B
P 4850 3450
F 0 "C2" H 4950 3450 50  0000 L CNN
F 1 "1u" H 4700 3400 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 4850 3450 50  0001 C CNN
F 3 "~" H 4850 3450 50  0001 C CNN
	1    4850 3450
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0105
U 1 1 5CC7F17E
P 4850 3550
F 0 "#PWR0105" H 4850 3300 50  0001 C CNN
F 1 "GND" H 4700 3450 50  0000 C CNN
F 2 "" H 4850 3550 50  0001 C CNN
F 3 "" H 4850 3550 50  0001 C CNN
	1    4850 3550
	1    0    0    -1  
$EndComp
Wire Wire Line
	4850 3350 5000 3350
$Comp
L Device:C_Small C1
U 1 1 5CC7F360
P 4600 2750
F 0 "C1" H 4700 2800 50  0000 L CNN
F 1 "0.1u" H 4400 2700 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 4600 2750 50  0001 C CNN
F 3 "~" H 4600 2750 50  0001 C CNN
	1    4600 2750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0106
U 1 1 5CC7F3BB
P 4600 2850
F 0 "#PWR0106" H 4600 2600 50  0001 C CNN
F 1 "GND" H 4500 2750 50  0000 C CNN
F 2 "" H 4600 2850 50  0001 C CNN
F 3 "" H 4600 2850 50  0001 C CNN
	1    4600 2850
	1    0    0    -1  
$EndComp
Wire Wire Line
	4600 2650 5000 2650
$Comp
L Device:Resonator_Small Y1
U 1 1 5CC7FCC1
P 4850 2350
F 0 "Y1" V 4650 2100 50  0000 C CNN
F 1 "16MHz" V 4950 2000 50  0000 C CNN
F 2 "Crystal:Resonator-3Pin_W6.0mm_H3.0mm" H 4825 2350 50  0001 C CNN
F 3 "~" H 4825 2350 50  0001 C CNN
	1    4850 2350
	0    1    -1   0   
$EndComp
Wire Wire Line
	4950 2250 5000 2250
Wire Wire Line
	4950 2450 5000 2450
$Comp
L power:GND #PWR0107
U 1 1 5CC8021C
P 4550 2350
F 0 "#PWR0107" H 4550 2100 50  0001 C CNN
F 1 "GND" H 4450 2250 50  0000 C CNN
F 2 "" H 4550 2350 50  0001 C CNN
F 3 "" H 4550 2350 50  0001 C CNN
	1    4550 2350
	1    0    0    -1  
$EndComp
Wire Wire Line
	4550 2350 4650 2350
$Comp
L power:GND #PWR0108
U 1 1 5CC80A55
P 5600 5500
F 0 "#PWR0108" H 5600 5250 50  0001 C CNN
F 1 "GND" H 5605 5327 50  0000 C CNN
F 2 "" H 5600 5500 50  0001 C CNN
F 3 "" H 5600 5500 50  0001 C CNN
	1    5600 5500
	1    0    0    -1  
$EndComp
Wire Wire Line
	5600 5500 5600 5450
Wire Wire Line
	5500 5350 5500 5450
Wire Wire Line
	5500 5450 5600 5450
Connection ~ 5600 5450
Wire Wire Line
	5600 5450 5600 5350
$Comp
L Device:D_Schottky_Small D1
U 1 1 5CC8865A
P 3850 2850
F 0 "D1" H 3850 3055 50  0000 C CNN
F 1 "30V1A" H 3850 2964 50  0000 C CNN
F 2 "Diode_THT:D_DO-201AD_P3.81mm_Vertical_AnodeUp" V 3850 2850 50  0001 C CNN
F 3 "~" V 3850 2850 50  0001 C CNN
	1    3850 2850
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR0109
U 1 1 5CC88F5B
P 61000 -11850
F 0 "#PWR0109" H 61000 -12100 50  0001 C CNN
F 1 "GND" H 61005 -12023 50  0000 C CNN
F 2 "" H 61000 -11850 50  0001 C CNN
F 3 "" H 61000 -11850 50  0001 C CNN
	1    61000 -11850
	1    0    0    -1  
$EndComp
Text GLabel 6200 2050 2    50   Input ~ 0
PB0
Text GLabel 6200 2150 2    50   Input ~ 0
PB1
Text GLabel 6200 2250 2    50   Input ~ 0
PB2
Text GLabel 6200 2350 2    50   Input ~ 0
PB3
Text GLabel 6200 2450 2    50   Input ~ 0
PB4
Text GLabel 6200 2550 2    50   Input ~ 0
PB5
Text GLabel 6200 2650 2    50   Input ~ 0
PB6
Text GLabel 6200 2750 2    50   Input ~ 0
PB7
Text GLabel 6200 2950 2    50   Input ~ 0
PC6
Text GLabel 6200 3050 2    50   Input ~ 0
PC7
Text GLabel 6200 3250 2    50   Input ~ 0
PD0
Text GLabel 6200 3350 2    50   Input ~ 0
PD1
Text GLabel 6200 3450 2    50   Input ~ 0
PD2
Text GLabel 6200 3550 2    50   Input ~ 0
PD3
Text GLabel 6200 3650 2    50   Input ~ 0
PD4
Text GLabel 6200 3750 2    50   Input ~ 0
PD5
Text GLabel 6200 3850 2    50   Input ~ 0
PD6
Text GLabel 6200 3950 2    50   Input ~ 0
PD7
Text GLabel 6200 4250 2    50   Input ~ 0
PE6
Text GLabel 6200 4450 2    50   Input ~ 0
PF0
Text GLabel 6200 4550 2    50   Input ~ 0
PF1
Text GLabel 6200 4650 2    50   Input ~ 0
PF4
Text GLabel 6200 4750 2    50   Input ~ 0
PF5
Text GLabel 6200 4850 2    50   Input ~ 0
PF6
Text GLabel 6200 4950 2    50   Input ~ 0
PF7
Wire Wire Line
	4800 2050 4950 2050
Connection ~ 4950 2050
Wire Wire Line
	5700 1700 5700 1750
$Comp
L power:PWR_FLAG #FLG0101
U 1 1 5CC92E6F
P 4000 1950
F 0 "#FLG0101" H 4000 2025 50  0001 C CNN
F 1 "PWR_FLAG" H 4000 2123 50  0000 C CNN
F 2 "" H 4000 1950 50  0001 C CNN
F 3 "~" H 4000 1950 50  0001 C CNN
	1    4000 1950
	1    0    0    1   
$EndComp
Text GLabel 7850 2950 0    50   Input ~ 0
PE6
Text GLabel 7850 2550 0    50   Input ~ 0
PB0
Text GLabel 7850 3350 0    50   Input ~ 0
PB1
Text GLabel 7850 2650 0    50   Input ~ 0
PB2
Text GLabel 7850 3450 0    50   Input ~ 0
PB3
Text GLabel 8650 2750 2    50   Input ~ 0
PB6
Text GLabel 7850 3750 0    50   Input ~ 0
PD0
Text GLabel 8650 3750 2    50   Input ~ 0
PD1
Text GLabel 7850 3650 0    50   Input ~ 0
PD2
Text GLabel 8650 3650 2    50   Input ~ 0
PD3
Text GLabel 8650 3350 2    50   Input ~ 0
PD4
Text GLabel 7850 3550 0    50   Input ~ 0
PD5
Text GLabel 8650 3550 2    50   Input ~ 0
PD6
Text GLabel 8650 3250 2    50   Input ~ 0
PD7
Text GLabel 7850 2450 0    50   Input ~ 0
PF0
Text GLabel 8650 2350 2    50   Input ~ 0
PF1
Text GLabel 7850 2350 0    50   Input ~ 0
PF4
Text GLabel 8650 2450 2    50   Input ~ 0
PF5
Text GLabel 7850 2250 0    50   Input ~ 0
PF6
Text GLabel 8650 2550 2    50   Input ~ 0
PF7
Text GLabel 8650 2650 2    50   Input ~ 0
PC7
Text GLabel 8650 3050 2    50   Input ~ 0
PC6
Text GLabel 7850 2750 0    50   Input ~ 0
PB7
Text GLabel 8650 3150 2    50   Input ~ 0
PB5
Text GLabel 8650 2850 2    50   Input ~ 0
PB4
Text GLabel 4800 2050 0    50   Input ~ 0
RESET
Text GLabel 7850 2850 0    50   Input ~ 0
RESET
Text GLabel 7850 3250 0    50   Input ~ 0
VBUS
Text GLabel 7850 3150 0    50   Input ~ 0
GND
Text GLabel 8650 3450 2    50   Input ~ 0
GND
Text GLabel 8650 2950 2    50   Input ~ 0
PE2
Text GLabel 6200 4150 2    50   Input ~ 0
PE2
Text GLabel 3650 2550 1    50   Input ~ 0
VBUS
NoConn ~ 7850 3050
$Comp
L Connector_Generic:Conn_01x16 J2
U 1 1 5CEADB7C
P 8050 2950
F 0 "J2" H 8000 3750 50  0000 L CNN
F 1 "Conn_01x16" H 7600 3850 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x16_P2.54mm_Vertical" H 8050 2950 50  0001 C CNN
F 3 "~" H 8050 2950 50  0001 C CNN
	1    8050 2950
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x16 J3
U 1 1 5CEADC42
P 8450 2950
F 0 "J3" H 8400 3750 50  0000 L CNN
F 1 "Conn_01x16" H 8000 3850 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x16_P2.54mm_Vertical" H 8450 2950 50  0001 C CNN
F 3 "~" H 8450 2950 50  0001 C CNN
	1    8450 2950
	-1   0    0    -1  
$EndComp
Text GLabel 8650 2250 2    50   Input ~ 0
VCC
Wire Wire Line
	5600 1600 5600 1700
Connection ~ 5600 1700
$Comp
L power:VCC #PWR01
U 1 1 5CEB09AF
P 4000 1950
F 0 "#PWR01" H 4000 1800 50  0001 C CNN
F 1 "VCC" H 4017 2123 50  0000 C CNN
F 2 "" H 4000 1950 50  0001 C CNN
F 3 "" H 4000 1950 50  0001 C CNN
	1    4000 1950
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR02
U 1 1 5CEB09EF
P 4050 2550
F 0 "#PWR02" H 4050 2400 50  0001 C CNN
F 1 "VCC" H 4067 2723 50  0000 C CNN
F 2 "" H 4050 2550 50  0001 C CNN
F 3 "" H 4050 2550 50  0001 C CNN
	1    4050 2550
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR04
U 1 1 5CEB0F9D
P 5600 1600
F 0 "#PWR04" H 5600 1450 50  0001 C CNN
F 1 "VCC" H 5617 1773 50  0000 C CNN
F 2 "" H 5600 1600 50  0001 C CNN
F 3 "" H 5600 1600 50  0001 C CNN
	1    5600 1600
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR03
U 1 1 5CEB0FDD
P 4950 1800
F 0 "#PWR03" H 4950 1650 50  0001 C CNN
F 1 "VCC" H 4967 1973 50  0000 C CNN
F 2 "" H 4950 1800 50  0001 C CNN
F 3 "" H 4950 1800 50  0001 C CNN
	1    4950 1800
	1    0    0    -1  
$EndComp
$Comp
L MCU_Microchip_ATmega:ATmega32U4-AU U1
U 1 1 5CEB112E
P 5600 3550
F 0 "U1" H 5150 5300 50  0000 C CNN
F 1 "ATmega32U4-AU" V 5600 3550 50  0000 C CNN
F 2 "User-Boards:AE-QFP44RP8-DIP" H 5600 3550 50  0001 C CIN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7766-8-bit-AVR-ATmega16U4-32U4_Datasheet.pdf" H 5600 3550 50  0001 C CNN
	1    5600 3550
	1    0    0    -1  
$EndComp
Text GLabel 4150 2850 2    50   Input ~ 0
VCC
Wire Wire Line
	4150 2850 4050 2850
Connection ~ 4050 2850
$EndSCHEMATC
