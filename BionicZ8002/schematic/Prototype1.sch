EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "BionicZ8002 Mezzanine"
Date "2022-01-19"
Rev "1"
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
Text Label 5450 2050 0    50   ~ 0
P10
Text Label 5450 2150 0    50   ~ 0
P11
Text Label 5450 2250 0    50   ~ 0
P12
Text Label 5450 2350 0    50   ~ 0
P13
Text Label 5450 2450 0    50   ~ 0
P14
Text Label 5450 2550 0    50   ~ 0
P15
Text Label 5450 2650 0    50   ~ 0
P16
Text Label 5450 2750 0    50   ~ 0
P17
Text Label 5450 2850 0    50   ~ 0
P20
Text Label 5450 2950 0    50   ~ 0
P21
Text Label 5450 3050 0    50   ~ 0
P22
Text Label 5450 3150 0    50   ~ 0
P23
Text Label 5450 3250 0    50   ~ 0
P24
Text Label 5450 3350 0    50   ~ 0
P25
Text Label 5450 3450 0    50   ~ 0
P26
Text Label 5450 3550 0    50   ~ 0
P27
Entry Wire Line
	5700 2750 5800 2650
Entry Wire Line
	5700 2650 5800 2550
Entry Wire Line
	5700 2550 5800 2450
Entry Wire Line
	5700 2450 5800 2350
Entry Wire Line
	5700 2350 5800 2250
Entry Wire Line
	5700 2250 5800 2150
Entry Wire Line
	5700 2150 5800 2050
Entry Wire Line
	5700 2050 5800 1950
Wire Wire Line
	5350 2050 5700 2050
Wire Wire Line
	5350 2150 5700 2150
Wire Wire Line
	5350 2250 5700 2250
Wire Wire Line
	5350 2350 5700 2350
Wire Wire Line
	5350 2450 5700 2450
Wire Wire Line
	5350 2550 5700 2550
Wire Wire Line
	5350 2650 5700 2650
Wire Wire Line
	5350 2750 5700 2750
Text Label 6700 2050 2    50   ~ 0
P10
Text Label 6700 2150 2    50   ~ 0
P11
Text Label 6700 2250 2    50   ~ 0
P12
Text Label 6700 2350 2    50   ~ 0
P13
Text Label 6700 2450 2    50   ~ 0
P14
Text Label 6700 2550 2    50   ~ 0
P15
Text Label 6700 2650 2    50   ~ 0
P16
Text Label 6700 2750 2    50   ~ 0
P17
Entry Wire Line
	6450 2750 6350 2650
Entry Wire Line
	6450 2650 6350 2550
Entry Wire Line
	6450 2550 6350 2450
Entry Wire Line
	6450 2450 6350 2350
Entry Wire Line
	6450 2350 6350 2250
Entry Wire Line
	6450 2250 6350 2150
Entry Wire Line
	6450 2150 6350 2050
Entry Wire Line
	6450 2050 6350 1950
Wire Wire Line
	6800 2050 6450 2050
Wire Wire Line
	6800 2150 6450 2150
Wire Wire Line
	6800 2250 6450 2250
Wire Wire Line
	6800 2350 6450 2350
Wire Wire Line
	6800 2450 6450 2450
Wire Wire Line
	6800 2550 6450 2550
Wire Wire Line
	6800 2650 6450 2650
Wire Wire Line
	6800 2750 6450 2750
Text Label 7400 2050 0    50   ~ 0
P20
Text Label 7400 2150 0    50   ~ 0
P21
Text Label 7400 2250 0    50   ~ 0
P22
Text Label 7400 2350 0    50   ~ 0
P23
Text Label 7400 2450 0    50   ~ 0
P24
Text Label 7400 2550 0    50   ~ 0
P25
Text Label 7400 2650 0    50   ~ 0
P26
Text Label 7400 2750 0    50   ~ 0
P27
Entry Wire Line
	7750 1950 7650 2050
Entry Wire Line
	7750 2050 7650 2150
Entry Wire Line
	7750 2150 7650 2250
Entry Wire Line
	7750 2250 7650 2350
Entry Wire Line
	7750 2350 7650 2450
Entry Wire Line
	7750 2450 7650 2550
Entry Wire Line
	7750 2550 7650 2650
Entry Wire Line
	7750 2650 7650 2750
Wire Wire Line
	7300 2050 7650 2050
Wire Wire Line
	7300 2150 7650 2150
Wire Wire Line
	7300 2250 7650 2250
Wire Wire Line
	7300 2350 7650 2350
Wire Wire Line
	7300 2450 7650 2450
Wire Wire Line
	7300 2550 7650 2550
Wire Wire Line
	7300 2650 7650 2650
Wire Wire Line
	7300 2750 7650 2750
Wire Wire Line
	4250 4250 4450 4250
Wire Wire Line
	4250 4500 4250 4450
Wire Wire Line
	4250 4250 4250 4200
Connection ~ 4250 4450
Connection ~ 4250 4250
$Comp
L Device:C_Small C1
U 1 1 5D0E12B4
P 4250 4350
F 0 "C1" H 4350 4400 50  0000 L CNN
F 1 "0.1u" H 4300 4250 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D3.4mm_W2.1mm_P2.50mm" H 4250 4350 50  0001 C CNN
F 3 "~" H 4250 4350 50  0001 C CNN
	1    4250 4350
	-1   0    0    -1  
$EndComp
$Comp
L power:VCC #PWR03
U 1 1 5CE117D7
P 4250 4200
F 0 "#PWR03" H 4250 4050 50  0001 C CNN
F 1 "VCC" H 4350 4300 50  0000 C CNN
F 2 "" H 4250 4200 50  0001 C CNN
F 3 "" H 4250 4200 50  0001 C CNN
	1    4250 4200
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR04
U 1 1 5CE12AA7
P 4250 4500
F 0 "#PWR04" H 4250 4250 50  0001 C CNN
F 1 "GND" H 4350 4400 50  0000 C CNN
F 2 "" H 4250 4500 50  0001 C CNN
F 3 "" H 4250 4500 50  0001 C CNN
	1    4250 4500
	-1   0    0    -1  
$EndComp
Text Label 6700 3350 2    50   ~ 0
P34
Text Label 6700 3450 2    50   ~ 0
P35
Text Label 6700 3550 2    50   ~ 0
P36
Wire Wire Line
	6450 3350 6800 3350
Wire Wire Line
	6450 3450 6800 3450
Wire Wire Line
	6450 3550 6800 3550
Entry Wire Line
	6450 3350 6350 3450
Entry Wire Line
	6450 3450 6350 3550
Entry Wire Line
	6450 3550 6350 3650
Text Label 7400 2950 0    50   ~ 0
P40
Text Label 7400 3150 0    50   ~ 0
P42
Text Label 7400 3250 0    50   ~ 0
P43
Text Label 7400 3350 0    50   ~ 0
P44
Text Label 7400 3550 0    50   ~ 0
P46
Text Label 7400 3650 0    50   ~ 0
P47
NoConn ~ 6800 3750
NoConn ~ 7300 3750
NoConn ~ 7300 4050
NoConn ~ 7300 4150
NoConn ~ 7300 2850
NoConn ~ 6800 2850
Wire Wire Line
	6800 1950 7300 1950
$Comp
L power:GND #PWR0101
U 1 1 61A3B3D0
P 8150 1950
F 0 "#PWR0101" H 8150 1700 50  0001 C CNN
F 1 "GND" H 8155 1777 50  0000 C CNN
F 2 "" H 8150 1950 50  0001 C CNN
F 3 "" H 8150 1950 50  0001 C CNN
	1    8150 1950
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR0102
U 1 1 61A51C51
P 7450 4250
F 0 "#PWR0102" H 7450 4100 50  0001 C CNN
F 1 "VCC" H 7467 4423 50  0000 C CNN
F 2 "" H 7450 4250 50  0001 C CNN
F 3 "" H 7450 4250 50  0001 C CNN
	1    7450 4250
	1    0    0    -1  
$EndComp
Wire Wire Line
	6800 2950 6450 2950
Wire Wire Line
	6800 3050 6450 3050
Wire Wire Line
	6800 3150 6450 3150
Wire Wire Line
	6800 3650 6450 3650
Text Label 6700 2950 2    50   ~ 0
P30
Text Label 6700 3050 2    50   ~ 0
P31
Text Label 6700 3150 2    50   ~ 0
P32
Text Label 6700 3650 2    50   ~ 0
P37
Entry Wire Line
	6450 2950 6350 3050
Entry Wire Line
	6450 3050 6350 3150
Entry Wire Line
	6450 3150 6350 3250
Entry Wire Line
	6450 3650 6350 3750
Wire Wire Line
	7300 1850 8150 1850
Wire Wire Line
	7300 1850 7300 1950
Wire Wire Line
	8150 1850 8150 1950
Text Label 4350 2650 2    50   ~ 0
P36
Text Label 4350 2350 2    50   ~ 0
P33
Text Label 4350 2950 2    50   ~ 0
P52
Text Label 4350 3450 2    50   ~ 0
P42
Text Label 4350 3550 2    50   ~ 0
P44
Text Label 4350 2250 2    50   ~ 0
P32
Text Label 5450 4450 0    50   ~ 0
P47
Text Label 5450 3950 0    50   ~ 0
P35
Wire Wire Line
	4250 4450 4450 4450
Text Label 4350 3050 2    50   ~ 0
P51
Text Label 6700 3950 2    50   ~ 0
P51
Text Label 6700 4050 2    50   ~ 0
P52
Text Label 6700 4150 2    50   ~ 0
P53
Text Label 7400 3850 0    50   ~ 0
P54
Text Label 7400 3950 0    50   ~ 0
P55
Text Label 5450 4150 0    50   ~ 0
P40
Text Label 5450 4250 0    50   ~ 0
P45
Text Label 4350 2150 2    50   ~ 0
P31
Text Label 4350 2050 2    50   ~ 0
P30
Wire Wire Line
	6800 3250 6450 3250
Text Label 6700 3250 2    50   ~ 0
P33
Entry Wire Line
	6450 3250 6350 3350
$Comp
L 0-LocalLibrary:BionicConnector J1
U 1 1 627DC9E5
P 7000 3050
F 0 "J1" H 7050 4375 50  0000 C CNN
F 1 "BionicConnector" H 7050 4284 50  0000 C CNN
F 2 "0-LocalLibrary:DIP-48_W7.62mm" H 7050 1750 50  0001 C CNN
F 3 "https://www.arieselec.com/wp-content/uploads/2020/02/10001-universal-dip-zif-test-socket.pdf" H 7000 3050 50  0001 C CNN
	1    7000 3050
	1    0    0    -1  
$EndComp
Connection ~ 7300 1950
Wire Wire Line
	7300 4250 7450 4250
Wire Wire Line
	6800 4250 7300 4250
Connection ~ 7300 4250
Text Label 5450 4350 0    50   ~ 0
P41
Text Label 5450 3850 0    50   ~ 0
P34
Entry Wire Line
	5700 4150 5800 4250
Text Label 4350 3750 2    50   ~ 0
P55
Wire Wire Line
	4450 2050 3650 2050
Entry Wire Line
	3650 2050 3550 2150
Entry Wire Line
	3650 2150 3550 2250
Wire Wire Line
	4450 2150 3650 2150
Wire Wire Line
	5350 4250 5700 4250
Wire Wire Line
	5350 4350 5700 4350
Wire Wire Line
	5350 4450 5700 4450
Entry Wire Line
	5700 4250 5800 4350
Entry Wire Line
	5700 4350 5800 4450
Entry Wire Line
	5700 4450 5800 4550
Wire Wire Line
	7300 3850 7650 3850
Wire Wire Line
	7300 3950 7650 3950
Entry Wire Line
	7650 3850 7750 3950
Entry Wire Line
	7650 3950 7750 4050
Wire Wire Line
	6800 3950 6450 3950
Wire Wire Line
	6800 4050 6450 4050
Wire Wire Line
	6800 4150 6450 4150
Entry Wire Line
	6450 3950 6350 4050
Entry Wire Line
	6450 4050 6350 4150
Connection ~ 5800 4750
Wire Wire Line
	5350 4150 5700 4150
Wire Wire Line
	7300 3650 7650 3650
Wire Wire Line
	7300 3350 7650 3350
Wire Wire Line
	7300 3150 7650 3150
Wire Wire Line
	7300 2950 7650 2950
Text Label 7400 3050 0    50   ~ 0
P41
Text Label 7400 3450 0    50   ~ 0
P45
Wire Wire Line
	7300 3050 7650 3050
Wire Wire Line
	7300 3450 7650 3450
Text Label 4350 3850 2    50   ~ 0
P54
Text Label 4350 2750 2    50   ~ 0
P37
Entry Wire Line
	3650 2250 3550 2350
Entry Wire Line
	3650 2350 3550 2450
Entry Wire Line
	3650 2650 3550 2750
Entry Wire Line
	3650 2750 3550 2850
Wire Wire Line
	3650 2250 4450 2250
Wire Wire Line
	3650 2350 4450 2350
Wire Wire Line
	3650 2650 4450 2650
Wire Wire Line
	3650 2750 4450 2750
Entry Wire Line
	7650 3450 7750 3550
Entry Wire Line
	7650 3050 7750 3150
Entry Wire Line
	7650 2950 7750 3050
Entry Wire Line
	7650 3150 7750 3250
Entry Wire Line
	7650 3350 7750 3450
Entry Wire Line
	7650 3650 7750 3750
Text Label 5550 5350 0    50   ~ 0
P43
Text Label 5550 5550 0    50   ~ 0
P53
Connection ~ 5450 5250
Wire Wire Line
	5450 5450 5450 5250
Wire Wire Line
	5350 5450 5450 5450
Connection ~ 5350 5650
Wire Wire Line
	5350 5650 5450 5650
Wire Wire Line
	4750 5150 3950 5150
Wire Wire Line
	5450 5050 5450 5250
Connection ~ 5450 5050
Wire Wire Line
	5350 5250 5450 5250
$Comp
L power:GND #PWR06
U 1 1 61A0F5E3
P 5450 5650
F 0 "#PWR06" H 5450 5400 50  0001 C CNN
F 1 "GND" H 5600 5600 50  0000 C CNN
F 2 "" H 5450 5650 50  0001 C CNN
F 3 "" H 5450 5650 50  0001 C CNN
	1    5450 5650
	1    0    0    -1  
$EndComp
Wire Wire Line
	5350 5650 5350 5750
Text Label 5550 5150 0    50   ~ 0
P46
$Comp
L power:VCC #PWR02
U 1 1 6193812C
P 5450 5050
F 0 "#PWR02" H 5450 4900 50  0001 C CNN
F 1 "VCC" H 5467 5223 50  0000 C CNN
F 2 "" H 5450 5050 50  0001 C CNN
F 3 "" H 5450 5050 50  0001 C CNN
	1    5450 5050
	1    0    0    -1  
$EndComp
Wire Wire Line
	5350 5050 5450 5050
Wire Wire Line
	5350 6150 5450 6150
$Comp
L power:VCC #PWR07
U 1 1 618D4C0A
P 5450 5900
F 0 "#PWR07" H 5450 5750 50  0001 C CNN
F 1 "VCC" H 5550 5950 50  0000 C CNN
F 2 "" H 5450 5900 50  0001 C CNN
F 3 "" H 5450 5900 50  0001 C CNN
	1    5450 5900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR08
U 1 1 618D4C10
P 5450 6200
F 0 "#PWR08" H 5450 5950 50  0001 C CNN
F 1 "GND" H 5550 6100 50  0000 C CNN
F 2 "" H 5450 6200 50  0001 C CNN
F 3 "" H 5450 6200 50  0001 C CNN
	1    5450 6200
	1    0    0    -1  
$EndComp
Wire Wire Line
	5450 5950 5350 5950
Wire Wire Line
	5450 6200 5450 6150
Wire Wire Line
	5450 5950 5450 5900
$Comp
L 0-LocalLibrary:74HCT126 U2
U 1 1 618CEFAB
P 5050 5550
F 0 "U2" H 5050 6200 50  0000 C CNN
F 1 "74HCT126" H 5050 4800 50  0000 C CNN
F 2 "Package_DIP:DIP-14_W7.62mm" H 5050 4700 50  0001 C CNN
F 3 "https://www.ti.com/lit/ds/symlink/cd74hct126.pdf" H 5050 5550 50  0001 C CNN
	1    5050 5550
	-1   0    0    -1  
$EndComp
Wire Wire Line
	4450 2950 3650 2950
Wire Wire Line
	4450 3050 3650 3050
Wire Wire Line
	4450 3450 3650 3450
Wire Wire Line
	4450 3550 3650 3550
Wire Wire Line
	4450 3850 3650 3850
Entry Wire Line
	3650 2950 3550 3050
Entry Wire Line
	3650 3050 3550 3150
Entry Wire Line
	3650 3450 3550 3550
Entry Wire Line
	3650 3550 3550 3650
Entry Wire Line
	3650 3750 3550 3850
Wire Wire Line
	3650 3750 4450 3750
Entry Wire Line
	3650 3850 3550 3950
Wire Bus Line
	3550 4750 5800 4750
Wire Wire Line
	3850 5350 4750 5350
Wire Wire Line
	4750 5550 3750 5550
NoConn ~ 4750 5750
Wire Bus Line
	5800 4750 6350 4750
Connection ~ 6350 4750
Wire Bus Line
	6350 4750 7750 4750
NoConn ~ 6800 3850
$Comp
L 0-LocalLibrary:Z8002 U1
U 1 1 622A9F73
P 4900 3250
F 0 "U1" H 4900 4650 50  0000 C CNN
F 1 "Z8002" V 4900 3250 50  0000 C CNN
F 2 "Package_DIP:DIP-40_W15.24mm" H 4950 1900 50  0001 C CIN
F 3 "https://pdf1.alldatasheet.com/datasheet-pdf/view/1283760/ZILOG/Z8002.html" H 4900 3250 50  0001 C CNN
	1    4900 3250
	1    0    0    -1  
$EndComp
Wire Wire Line
	5350 3550 5700 3550
Wire Wire Line
	5350 3450 5700 3450
Wire Wire Line
	5350 3350 5700 3350
Wire Wire Line
	5350 3250 5700 3250
Wire Wire Line
	5350 3150 5700 3150
Wire Wire Line
	5350 3050 5700 3050
Wire Wire Line
	5350 2950 5700 2950
Wire Wire Line
	5350 2850 5700 2850
Entry Wire Line
	5800 3450 5700 3550
Entry Wire Line
	5800 3350 5700 3450
Entry Wire Line
	5800 3250 5700 3350
Entry Wire Line
	5800 3150 5700 3250
Entry Wire Line
	5800 3050 5700 3150
Entry Wire Line
	5800 2950 5700 3050
Entry Wire Line
	5800 2850 5700 2950
Entry Wire Line
	5800 2750 5700 2850
Entry Wire Line
	5800 3950 5700 3850
Entry Wire Line
	5800 4050 5700 3950
Wire Wire Line
	5350 3850 5700 3850
Wire Wire Line
	5350 3950 5700 3950
Wire Wire Line
	5350 5150 5700 5150
Wire Wire Line
	5700 5350 5350 5350
Wire Wire Line
	5350 5550 5700 5550
Entry Wire Line
	5800 5050 5700 5150
Entry Wire Line
	5800 5250 5700 5350
Entry Wire Line
	5800 5450 5700 5550
Wire Wire Line
	4450 4050 3950 4050
Wire Wire Line
	3950 4050 3950 5150
Wire Wire Line
	4450 3350 3850 3350
Wire Wire Line
	3850 3350 3850 5350
Wire Wire Line
	3750 3150 3750 5550
Wire Wire Line
	3750 3150 4450 3150
Wire Bus Line
	5800 1650 6350 1650
Connection ~ 6350 1650
Wire Bus Line
	6350 1650 7750 1650
Entry Wire Line
	6450 4150 6350 4250
Entry Wire Line
	7650 3250 7750 3350
Entry Wire Line
	7650 3550 7750 3650
Wire Wire Line
	7300 3250 7650 3250
Wire Wire Line
	7300 3550 7650 3550
Wire Bus Line
	5800 4750 5800 5450
Wire Bus Line
	5800 3950 5800 4750
Wire Bus Line
	7750 1650 7750 2650
Wire Bus Line
	6350 1650 6350 2650
Wire Bus Line
	6350 3050 6350 4750
Wire Bus Line
	7750 3050 7750 4750
Wire Bus Line
	3550 2150 3550 4750
Wire Bus Line
	5800 1650 5800 3450
$EndSCHEMATC
