EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "BionicBase Teensy 3.5"
Date "2021-11-05"
Rev "2"
Comp "Tadashi G. Takaoka"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L power:VCC #PWR01
U 1 1 5CDE46A4
P -53250 -44750
F 0 "#PWR01" H -53250 -44900 50  0001 C CNN
F 1 "VCC" H -53233 -44577 50  0000 C CNN
F 2 "" H -53250 -44750 50  0001 C CNN
F 3 "" H -53250 -44750 50  0001 C CNN
	1    -53250 -44750
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR02
U 1 1 5CE5EFDE
P 4750 4700
F 0 "#PWR02" H 4750 4550 50  0001 C CNN
F 1 "VCC" H 4650 4750 50  0000 C CNN
F 2 "" H 4750 4700 50  0001 C CNN
F 3 "" H 4750 4700 50  0001 C CNN
	1    4750 4700
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C1
U 1 1 5D080349
P 4750 4800
F 0 "C1" H 4900 4850 50  0000 L CNN
F 1 "0.1u" H 4850 4700 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D3.4mm_W2.1mm_P2.50mm" H 4750 4800 50  0001 C CNN
F 3 "~" H 4750 4800 50  0001 C CNN
	1    4750 4800
	-1   0    0    -1  
$EndComp
Text Label 4900 6200 0    50   ~ 0
P56
$Comp
L power:GND #PWR07
U 1 1 5EC2E2A0
P 4750 5150
F 0 "#PWR07" H 4750 4900 50  0001 C CNN
F 1 "GND" H 4900 5050 50  0000 C CNN
F 2 "" H 4750 5150 50  0001 C CNN
F 3 "" H 4750 5150 50  0001 C CNN
	1    4750 5150
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R6
U 1 1 5DD95AB3
P 4650 6300
F 0 "R6" V 4750 6200 50  0000 C CNN
F 1 "560" V 4750 6350 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P5.08mm_Horizontal" H 4650 6300 50  0001 C CNN
F 3 "~" H 4650 6300 50  0001 C CNN
	1    4650 6300
	0    -1   1    0   
$EndComp
Wire Wire Line
	4900 4700 4750 4700
Wire Wire Line
	4900 4900 4750 4900
Wire Wire Line
	4750 4900 4750 5000
Wire Wire Line
	4900 5000 4750 5000
Connection ~ 4750 5000
Wire Wire Line
	4750 5000 4750 5100
Wire Wire Line
	4750 5100 4900 5100
Connection ~ 4750 5100
Wire Wire Line
	4750 5100 4750 5150
$Comp
L 0-LocalLibrary:Teensy3.5 B1
U 1 1 61858745
P 5400 3500
F 0 "B1" H 5400 4867 50  0000 C CNN
F 1 "Teensy3.5" H 5400 4776 50  0000 C CNN
F 2 "Package_DIP:DIP-48_W15.24mm" H 5400 1700 50  0001 C CIN
F 3 "https://www.pjrc.com/store/teensy35.html" H 5350 4850 50  0001 C CNN
	1    5400 3500
	1    0    0    -1  
$EndComp
Connection ~ 4750 4900
Connection ~ 4750 4700
$Comp
L power:+3.3V #PWR0101
U 1 1 61C4EE52
P 4750 4550
F 0 "#PWR0101" H 4750 4400 50  0001 C CNN
F 1 "+3.3V" H 4850 4700 50  0000 C CNN
F 2 "" H 4750 4550 50  0001 C CNN
F 3 "" H 4750 4550 50  0001 C CNN
	1    4750 4550
	1    0    0    -1  
$EndComp
Wire Wire Line
	4750 4550 4900 4550
$Comp
L Device:LED_Small D1
U 1 1 626EC97A
P 4350 6300
F 0 "D1" H 4450 6250 50  0000 C CNN
F 1 "OSYL3131P" H 4350 6150 50  0000 C CNN
F 2 "LED_THT:LED_D3.0mm" V 4350 6300 50  0001 C CNN
F 3 "~" V 4350 6300 50  0001 C CNN
	1    4350 6300
	-1   0    0    -1  
$EndComp
Wire Wire Line
	4550 6300 4450 6300
Wire Wire Line
	4900 6200 4550 6200
$Comp
L 0-LocalLibrary:Tact_SW SW1
U 1 1 626EBB02
P 4350 6200
F 0 "SW1" H 4350 6485 50  0000 C CNN
F 1 "Tact_SW" H 4350 6394 50  0000 C CNN
F 2 "0-LocalLibrary:Tact_SW_6x6mm" H 4350 6400 50  0001 C CNN
F 3 "~" H 4350 6400 50  0001 C CNN
	1    4350 6200
	-1   0    0    -1  
$EndComp
Wire Wire Line
	3950 6100 3950 6300
Wire Wire Line
	4000 6200 4000 6400
$Comp
L power:GND #PWR023
U 1 1 5DD6FB12
P 4000 6400
F 0 "#PWR023" H 4000 6150 50  0001 C CNN
F 1 "GND" H 4005 6227 50  0000 C CNN
F 2 "" H 4000 6400 50  0001 C CNN
F 3 "" H 4000 6400 50  0001 C CNN
	1    4000 6400
	-1   0    0    -1  
$EndComp
Wire Wire Line
	4150 6200 4000 6200
Wire Wire Line
	4250 6300 3950 6300
$Comp
L 0-LocalLibrary:Bionic_ZIF_Socket_48pin J1
U 1 1 61A16AEC
P 3100 3750
F 0 "J1" H 3250 5075 50  0000 C CNN
F 1 "Bionic_ZIF_Socket_48pin" H 3250 4984 50  0000 C CNN
F 2 "0-LocalLibrary:Aries_ZIF_socket_DIP-48-pin" H 3350 2450 50  0001 C CNN
F 3 "https://www.arieselec.com/wp-content/uploads/2020/02/10001-universal-dip-zif-test-socket.pdf" H 3100 3750 50  0001 C CNN
	1    3100 3750
	1    0    0    -1  
$EndComp
Text Label 5950 2400 0    50   ~ 0
P10
Text Label 5950 2500 0    50   ~ 0
P11
Text Label 5950 2600 0    50   ~ 0
P12
Text Label 5950 2700 0    50   ~ 0
P13
Text Label 5950 2800 0    50   ~ 0
P14
Text Label 5950 2900 0    50   ~ 0
P15
Text Label 5950 3000 0    50   ~ 0
P16
Text Label 5950 3100 0    50   ~ 0
P17
Text Label 5950 3300 0    50   ~ 0
P20
Text Label 5950 3400 0    50   ~ 0
P21
Text Label 5950 3500 0    50   ~ 0
P22
Text Label 5950 3600 0    50   ~ 0
P23
Text Label 5950 3700 0    50   ~ 0
P24
Text Label 5950 3800 0    50   ~ 0
P25
Text Label 5950 3900 0    50   ~ 0
P26
Text Label 5950 4000 0    50   ~ 0
P27
Text Label 5950 4200 0    50   ~ 0
P30
Text Label 5950 4300 0    50   ~ 0
P31
Text Label 5950 4400 0    50   ~ 0
P32
Text Label 5950 4500 0    50   ~ 0
P33
Text Label 5950 4600 0    50   ~ 0
P34
Text Label 5950 4700 0    50   ~ 0
P35
Text Label 5950 4800 0    50   ~ 0
P36
Text Label 5950 4900 0    50   ~ 0
P37
Text Label 4850 2400 2    50   ~ 0
P40
Text Label 4850 2500 2    50   ~ 0
P41
Text Label 4850 2600 2    50   ~ 0
P42
Text Label 4850 2700 2    50   ~ 0
P43
Text Label 4850 2800 2    50   ~ 0
P44
Text Label 4850 2900 2    50   ~ 0
P45
Text Label 4850 3000 2    50   ~ 0
P46
Text Label 4850 3100 2    50   ~ 0
P47
Text Label 4850 3300 2    50   ~ 0
P50
Text Label 4850 3400 2    50   ~ 0
P51
Text Label 4850 3500 2    50   ~ 0
P52
Text Label 4850 3600 2    50   ~ 0
P53
Text Label 4850 3700 2    50   ~ 0
P54
Text Label 4850 3800 2    50   ~ 0
P55
Text Label 4850 3900 2    50   ~ 0
P56
Text Label 4850 4000 2    50   ~ 0
P57
Entry Wire Line
	6150 3100 6250 3000
Entry Wire Line
	6150 3000 6250 2900
Entry Wire Line
	6150 2900 6250 2800
Entry Wire Line
	6150 2800 6250 2700
Entry Wire Line
	6150 2700 6250 2600
Entry Wire Line
	6150 2600 6250 2500
Entry Wire Line
	6150 2500 6250 2400
Entry Wire Line
	6150 2400 6250 2300
Wire Wire Line
	5900 2400 6150 2400
Wire Wire Line
	5900 2500 6150 2500
Wire Wire Line
	5900 2600 6150 2600
Wire Wire Line
	5900 2700 6150 2700
Wire Wire Line
	5900 2800 6150 2800
Wire Wire Line
	5900 2900 6150 2900
Wire Wire Line
	5900 3000 6150 3000
Wire Wire Line
	5900 3100 6150 3100
Entry Wire Line
	6250 4000 6350 3900
Entry Wire Line
	6250 3900 6350 3800
Entry Wire Line
	6250 3800 6350 3700
Entry Wire Line
	6250 3700 6350 3600
Entry Wire Line
	6250 3600 6350 3500
Entry Wire Line
	6250 3500 6350 3400
Entry Wire Line
	6250 3400 6350 3300
Entry Wire Line
	6250 3300 6350 3200
Wire Wire Line
	5900 3300 6250 3300
Wire Wire Line
	5900 3400 6250 3400
Wire Wire Line
	5900 3500 6250 3500
Wire Wire Line
	5900 3600 6250 3600
Wire Wire Line
	5900 3700 6250 3700
Wire Wire Line
	5900 3800 6250 3800
Wire Wire Line
	5900 3900 6250 3900
Wire Wire Line
	5900 4000 6250 4000
Entry Wire Line
	6350 4900 6450 4800
Entry Wire Line
	6350 4800 6450 4700
Entry Wire Line
	6350 4700 6450 4600
Entry Wire Line
	6350 4600 6450 4500
Entry Wire Line
	6350 4500 6450 4400
Entry Wire Line
	6350 4400 6450 4300
Entry Wire Line
	6350 4300 6450 4200
Entry Wire Line
	6350 4200 6450 4100
Wire Wire Line
	5900 4200 6350 4200
Wire Wire Line
	5900 4300 6350 4300
Wire Wire Line
	5900 4400 6350 4400
Wire Wire Line
	5900 4500 6350 4500
Wire Wire Line
	5900 4600 6350 4600
Wire Wire Line
	5900 4700 6350 4700
Wire Wire Line
	5900 4800 6350 4800
Wire Wire Line
	5900 4900 6350 4900
Entry Wire Line
	4550 2400 4450 2500
Entry Wire Line
	4550 2500 4450 2600
Entry Wire Line
	4550 2600 4450 2700
Entry Wire Line
	4550 2700 4450 2800
Entry Wire Line
	4550 2800 4450 2900
Entry Wire Line
	4550 2900 4450 3000
Entry Wire Line
	4550 3000 4450 3100
Entry Wire Line
	4550 3100 4450 3200
Wire Wire Line
	4900 3100 4550 3100
Wire Wire Line
	4900 3000 4550 3000
Wire Wire Line
	4900 2900 4550 2900
Wire Wire Line
	4900 2800 4550 2800
Wire Wire Line
	4900 2700 4550 2700
Wire Wire Line
	4900 2600 4550 2600
Wire Wire Line
	4900 2500 4550 2500
Wire Wire Line
	4900 2400 4550 2400
Entry Wire Line
	4650 3300 4550 3400
Entry Wire Line
	4650 3400 4550 3500
Entry Wire Line
	4650 3500 4550 3600
Entry Wire Line
	4650 3600 4550 3700
Entry Wire Line
	4650 3700 4550 3800
Entry Wire Line
	4650 3800 4550 3900
Entry Wire Line
	4650 3900 4550 4000
Entry Wire Line
	4650 4000 4550 4100
Wire Wire Line
	4900 4000 4650 4000
Wire Wire Line
	4900 3900 4650 3900
Wire Wire Line
	4900 3800 4650 3800
Wire Wire Line
	4900 3700 4650 3700
Wire Wire Line
	4900 3600 4650 3600
Wire Wire Line
	4900 3500 4650 3500
Wire Wire Line
	4900 3400 4650 3400
Wire Wire Line
	4900 3300 4650 3300
Text Label 2850 2750 2    50   ~ 0
P10
Text Label 2850 2850 2    50   ~ 0
P11
Text Label 2850 2950 2    50   ~ 0
P12
Text Label 2850 3050 2    50   ~ 0
P13
Text Label 2850 3150 2    50   ~ 0
P14
Text Label 2850 3250 2    50   ~ 0
P15
Text Label 2850 3350 2    50   ~ 0
P16
Text Label 2850 3450 2    50   ~ 0
P17
Entry Wire Line
	2650 3450 2550 3350
Entry Wire Line
	2650 3350 2550 3250
Entry Wire Line
	2650 3250 2550 3150
Entry Wire Line
	2650 3150 2550 3050
Entry Wire Line
	2650 3050 2550 2950
Entry Wire Line
	2650 2950 2550 2850
Entry Wire Line
	2650 2850 2550 2750
Entry Wire Line
	2650 2750 2550 2650
Wire Wire Line
	2900 2750 2650 2750
Wire Wire Line
	2900 2850 2650 2850
Wire Wire Line
	2900 2950 2650 2950
Wire Wire Line
	2900 3050 2650 3050
Wire Wire Line
	2900 3150 2650 3150
Wire Wire Line
	2900 3250 2650 3250
Wire Wire Line
	2900 3350 2650 3350
Wire Wire Line
	2900 3450 2650 3450
Entry Wire Line
	3850 3450 3950 3350
Entry Wire Line
	3850 3350 3950 3250
Entry Wire Line
	3850 3250 3950 3150
Entry Wire Line
	3850 3150 3950 3050
Entry Wire Line
	3850 3050 3950 2950
Entry Wire Line
	3850 2950 3950 2850
Entry Wire Line
	3850 2850 3950 2750
Entry Wire Line
	3850 2750 3950 2650
Wire Wire Line
	3600 2750 3850 2750
Wire Wire Line
	3600 2850 3850 2850
Wire Wire Line
	3600 2950 3850 2950
Wire Wire Line
	3600 3050 3850 3050
Wire Wire Line
	3600 3150 3850 3150
Wire Wire Line
	3600 3250 3850 3250
Wire Wire Line
	3600 3350 3850 3350
Wire Wire Line
	3600 3450 3850 3450
Entry Wire Line
	3950 4350 4050 4250
Entry Wire Line
	3950 4250 4050 4150
Entry Wire Line
	3950 4150 4050 4050
Entry Wire Line
	3950 4050 4050 3950
Entry Wire Line
	3950 3950 4050 3850
Entry Wire Line
	3950 3850 4050 3750
Entry Wire Line
	3950 3750 4050 3650
Entry Wire Line
	3950 3650 4050 3550
Wire Wire Line
	3600 3650 3950 3650
Wire Wire Line
	3600 3750 3950 3750
Wire Wire Line
	3600 3850 3950 3850
Wire Wire Line
	3600 3950 3950 3950
Wire Wire Line
	3600 4050 3950 4050
Wire Wire Line
	3600 4150 3950 4150
Wire Wire Line
	3600 4250 3950 4250
Wire Wire Line
	3600 4350 3950 4350
Entry Wire Line
	2550 4350 2450 4250
Entry Wire Line
	2550 4250 2450 4150
Entry Wire Line
	2550 4150 2450 4050
Entry Wire Line
	2550 4050 2450 3950
Entry Wire Line
	2550 3950 2450 3850
Entry Wire Line
	2550 3850 2450 3750
Entry Wire Line
	2550 3750 2450 3650
Entry Wire Line
	2550 3650 2450 3550
Wire Wire Line
	2900 3650 2550 3650
Wire Wire Line
	2900 3750 2550 3750
Wire Wire Line
	2900 3850 2550 3850
Wire Wire Line
	2900 3950 2550 3950
Wire Wire Line
	2900 4050 2550 4050
Wire Wire Line
	2900 4150 2550 4150
Wire Wire Line
	2900 4250 2550 4250
Wire Wire Line
	2900 4350 2550 4350
Text Label 2850 3650 2    50   ~ 0
P30
Text Label 2850 3750 2    50   ~ 0
P31
Text Label 2850 3850 2    50   ~ 0
P32
Text Label 2850 3950 2    50   ~ 0
P33
Text Label 2850 4050 2    50   ~ 0
P34
Text Label 2850 4150 2    50   ~ 0
P35
Text Label 2850 4250 2    50   ~ 0
P36
Text Label 2850 4350 2    50   ~ 0
P37
Text Label 3650 2750 0    50   ~ 0
P20
Text Label 3650 2850 0    50   ~ 0
P21
Text Label 3650 2950 0    50   ~ 0
P22
Text Label 3650 3050 0    50   ~ 0
P23
Text Label 3650 3150 0    50   ~ 0
P24
Text Label 3650 3250 0    50   ~ 0
P25
Text Label 3650 3350 0    50   ~ 0
P26
Text Label 3650 3450 0    50   ~ 0
P27
Text Label 3650 3650 0    50   ~ 0
P40
Text Label 3650 3750 0    50   ~ 0
P41
Text Label 3650 3850 0    50   ~ 0
P42
Text Label 3650 3950 0    50   ~ 0
P43
Text Label 3650 4050 0    50   ~ 0
P44
Text Label 3650 4150 0    50   ~ 0
P45
Text Label 3650 4250 0    50   ~ 0
P46
Text Label 3650 4350 0    50   ~ 0
P47
Text Label 2850 4550 2    50   ~ 0
P50
Text Label 2850 4650 2    50   ~ 0
P51
Text Label 2850 4750 2    50   ~ 0
P52
Text Label 2850 4850 2    50   ~ 0
P53
Text Label 3650 4550 0    50   ~ 0
P54
Text Label 3650 4650 0    50   ~ 0
P55
Text Label 3650 4750 0    50   ~ 0
P56
Text Label 3650 4850 0    50   ~ 0
P57
Entry Wire Line
	2650 4550 2550 4650
Entry Wire Line
	2650 4650 2550 4750
Entry Wire Line
	2650 4750 2550 4850
Entry Wire Line
	2650 4850 2550 4950
Entry Wire Line
	3850 4550 3950 4650
Entry Wire Line
	3850 4650 3950 4750
Entry Wire Line
	3850 4750 3950 4850
Entry Wire Line
	3850 4850 3950 4950
Wire Wire Line
	3600 4850 3850 4850
Wire Wire Line
	3600 4750 3850 4750
Wire Wire Line
	3600 4650 3850 4650
Wire Wire Line
	3600 4550 3850 4550
Wire Wire Line
	2900 4850 2650 4850
Wire Wire Line
	2900 4750 2650 4750
Wire Wire Line
	2900 4650 2650 4650
Wire Wire Line
	2900 4550 2650 4550
Wire Bus Line
	2550 5200 3950 5200
Wire Bus Line
	4550 4500 4300 4500
Wire Bus Line
	4300 4500 4300 5200
Wire Bus Line
	4300 5200 3950 5200
Connection ~ 3950 5200
Wire Bus Line
	4450 3400 4050 3400
Wire Bus Line
	6250 1950 2550 1950
Wire Bus Line
	3950 2050 6350 2050
Text Label 4900 6300 0    50   ~ 0
P57
Wire Wire Line
	4750 6300 4900 6300
Wire Bus Line
	6450 1850 2450 1850
Text Label 3650 2650 0    50   ~ 0
GND
Wire Wire Line
	3650 2650 3600 2650
Connection ~ 3600 2650
Wire Wire Line
	3600 2650 2900 2650
Text Label 3650 4950 0    50   ~ 0
VCC
Wire Wire Line
	3650 4950 3600 4950
Connection ~ 3600 4950
Wire Wire Line
	3600 4950 2900 4950
NoConn ~ 3600 4450
NoConn ~ 2900 4450
NoConn ~ 3600 3550
NoConn ~ 2900 3550
Wire Bus Line
	2550 4650 2550 5200
Wire Bus Line
	3950 4650 3950 5200
Wire Bus Line
	4550 3400 4550 4500
Wire Bus Line
	4450 2500 4450 3400
Wire Bus Line
	4050 3400 4050 4250
Wire Bus Line
	6250 1950 6250 3000
Wire Bus Line
	2550 1950 2550 3350
Wire Bus Line
	3950 2050 3950 3350
Wire Bus Line
	6350 2050 6350 3900
Wire Bus Line
	6450 1850 6450 4800
Wire Bus Line
	2450 1850 2450 4300
$Comp
L power:+3.3V #PWR?
U 1 1 618A0B8B
P 3950 6100
F 0 "#PWR?" H 3950 5950 50  0001 C CNN
F 1 "+3.3V" H 3965 6273 50  0000 C CNN
F 2 "" H 3950 6100 50  0001 C CNN
F 3 "" H 3950 6100 50  0001 C CNN
	1    3950 6100
	1    0    0    -1  
$EndComp
$EndSCHEMATC
