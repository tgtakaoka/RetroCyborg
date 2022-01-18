EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "BionicP8048 Mezzanine"
Date "2022-01-18"
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
Text Label 5300 2050 0    50   ~ 0
P10
Text Label 5300 2150 0    50   ~ 0
P11
Text Label 5300 2250 0    50   ~ 0
P12
Text Label 5300 2350 0    50   ~ 0
P13
Text Label 5300 2450 0    50   ~ 0
P14
Text Label 5300 2550 0    50   ~ 0
P15
Text Label 5300 2650 0    50   ~ 0
P16
Text Label 5300 2750 0    50   ~ 0
P17
Text Label 5300 2850 0    50   ~ 0
P20
Text Label 5300 2950 0    50   ~ 0
P21
Text Label 5300 3050 0    50   ~ 0
P22
Text Label 5300 3150 0    50   ~ 0
P23
Entry Wire Line
	5550 2750 5650 2650
Entry Wire Line
	5550 2650 5650 2550
Entry Wire Line
	5550 2550 5650 2450
Entry Wire Line
	5550 2450 5650 2350
Entry Wire Line
	5550 2350 5650 2250
Entry Wire Line
	5550 2250 5650 2150
Entry Wire Line
	5550 2150 5650 2050
Entry Wire Line
	5550 2050 5650 1950
Entry Wire Line
	5750 2750 5650 2850
Entry Wire Line
	5750 2850 5650 2950
Entry Wire Line
	5750 2950 5650 3050
Entry Wire Line
	5750 3050 5650 3150
Wire Wire Line
	5200 2050 5550 2050
Wire Wire Line
	5200 2150 5550 2150
Wire Wire Line
	5200 2250 5550 2250
Wire Wire Line
	5200 2350 5550 2350
Wire Wire Line
	5200 2450 5550 2450
Wire Wire Line
	5200 2550 5550 2550
Wire Wire Line
	5200 2650 5550 2650
Wire Wire Line
	5200 2750 5550 2750
Wire Wire Line
	5200 2850 5650 2850
Wire Wire Line
	5200 2950 5650 2950
Wire Wire Line
	5200 3050 5650 3050
Wire Wire Line
	5200 3150 5650 3150
Text Label 6600 1950 2    50   ~ 0
P10
Text Label 6600 2050 2    50   ~ 0
P11
Text Label 6600 2150 2    50   ~ 0
P12
Text Label 6600 2250 2    50   ~ 0
P13
Text Label 6600 2350 2    50   ~ 0
P14
Text Label 6600 2450 2    50   ~ 0
P15
Text Label 6600 2550 2    50   ~ 0
P16
Text Label 6600 2650 2    50   ~ 0
P17
Entry Wire Line
	6350 2650 6250 2550
Entry Wire Line
	6350 2550 6250 2450
Entry Wire Line
	6350 2450 6250 2350
Entry Wire Line
	6350 2350 6250 2250
Entry Wire Line
	6350 2250 6250 2150
Entry Wire Line
	6350 2150 6250 2050
Entry Wire Line
	6350 2050 6250 1950
Entry Wire Line
	6350 1950 6250 1850
Wire Wire Line
	6700 1950 6350 1950
Wire Wire Line
	6700 2050 6350 2050
Wire Wire Line
	6700 2150 6350 2150
Wire Wire Line
	6700 2250 6350 2250
Wire Wire Line
	6700 2350 6350 2350
Wire Wire Line
	6700 2450 6350 2450
Wire Wire Line
	6700 2550 6350 2550
Wire Wire Line
	6700 2650 6350 2650
Text Label 7300 1950 0    50   ~ 0
P20
Text Label 7300 2050 0    50   ~ 0
P21
Text Label 7300 2150 0    50   ~ 0
P22
Text Label 7300 2250 0    50   ~ 0
P23
Entry Wire Line
	7650 1850 7550 1950
Entry Wire Line
	7650 1950 7550 2050
Entry Wire Line
	7650 2050 7550 2150
Entry Wire Line
	7650 2150 7550 2250
Wire Wire Line
	7200 1950 7550 1950
Wire Wire Line
	7200 2050 7550 2050
Wire Wire Line
	7200 2150 7550 2150
Wire Wire Line
	7200 2250 7550 2250
Wire Bus Line
	5650 1750 6250 1750
$Comp
L 0-LocalLibrary:74HCT126 U2
U 1 1 618CEFAB
P 4900 5550
F 0 "U2" H 4900 6200 50  0000 C CNN
F 1 "74HCT126" H 4900 4800 50  0000 C CNN
F 2 "Package_DIP:DIP-14_W7.62mm" H 4900 4700 50  0001 C CNN
F 3 "https://www.ti.com/lit/ds/symlink/cd74hct126.pdf" H 4900 5550 50  0001 C CNN
	1    4900 5550
	-1   0    0    -1  
$EndComp
Wire Wire Line
	5300 5950 5300 5900
Wire Wire Line
	5300 6200 5300 6150
Wire Wire Line
	5300 5950 5200 5950
$Comp
L power:GND #PWR08
U 1 1 618D4C10
P 5300 6200
F 0 "#PWR08" H 5300 5950 50  0001 C CNN
F 1 "GND" H 5400 6100 50  0000 C CNN
F 2 "" H 5300 6200 50  0001 C CNN
F 3 "" H 5300 6200 50  0001 C CNN
	1    5300 6200
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR07
U 1 1 618D4C0A
P 5300 5900
F 0 "#PWR07" H 5300 5750 50  0001 C CNN
F 1 "VCC" H 5400 6000 50  0000 C CNN
F 2 "" H 5300 5900 50  0001 C CNN
F 3 "" H 5300 5900 50  0001 C CNN
	1    5300 5900
	1    0    0    -1  
$EndComp
Wire Wire Line
	4100 4150 4300 4150
Wire Wire Line
	4100 4400 4100 4350
Wire Wire Line
	4100 4150 4100 4100
Connection ~ 4100 4350
Connection ~ 4100 4150
$Comp
L Device:C_Small C1
U 1 1 5D0E12B4
P 4100 4250
F 0 "C1" H 4200 4300 50  0000 L CNN
F 1 "0.1u" H 4150 4150 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D3.4mm_W2.1mm_P2.50mm" H 4100 4250 50  0001 C CNN
F 3 "~" H 4100 4250 50  0001 C CNN
	1    4100 4250
	-1   0    0    -1  
$EndComp
$Comp
L power:VCC #PWR03
U 1 1 5CE117D7
P 4100 4100
F 0 "#PWR03" H 4100 3950 50  0001 C CNN
F 1 "VCC" H 4200 4200 50  0000 C CNN
F 2 "" H 4100 4100 50  0001 C CNN
F 3 "" H 4100 4100 50  0001 C CNN
	1    4100 4100
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR04
U 1 1 5CE12AA7
P 4100 4400
F 0 "#PWR04" H 4100 4150 50  0001 C CNN
F 1 "GND" H 4200 4300 50  0000 C CNN
F 2 "" H 4100 4400 50  0001 C CNN
F 3 "" H 4100 4400 50  0001 C CNN
	1    4100 4400
	-1   0    0    -1  
$EndComp
Wire Wire Line
	5200 6150 5300 6150
Text Label 5350 5350 0    50   ~ 0
P53
Wire Wire Line
	5200 5050 5300 5050
$Comp
L power:VCC #PWR02
U 1 1 6193812C
P 5300 5050
F 0 "#PWR02" H 5300 4900 50  0001 C CNN
F 1 "VCC" H 5317 5223 50  0000 C CNN
F 2 "" H 5300 5050 50  0001 C CNN
F 3 "" H 5300 5050 50  0001 C CNN
	1    5300 5050
	1    0    0    -1  
$EndComp
NoConn ~ 4600 5550
NoConn ~ 4600 5750
Text Label 5350 5150 0    50   ~ 0
P46
Text Label 7300 2850 0    50   ~ 0
P40
Text Label 7300 3050 0    50   ~ 0
P42
Text Label 7300 3250 0    50   ~ 0
P44
Text Label 7300 3350 0    50   ~ 0
P45
Text Label 7300 3450 0    50   ~ 0
P46
Wire Wire Line
	7200 2850 7550 2850
Wire Wire Line
	7200 3050 7550 3050
Wire Wire Line
	7200 3250 7550 3250
Wire Wire Line
	7200 3350 7550 3350
Entry Wire Line
	7550 2850 7650 2950
Entry Wire Line
	7550 3050 7650 3150
Entry Wire Line
	7550 3250 7650 3350
Entry Wire Line
	7550 3350 7650 3450
Wire Wire Line
	5200 5450 5200 5550
Connection ~ 5200 5550
Wire Wire Line
	5200 5550 5200 5650
Connection ~ 5200 5650
Wire Wire Line
	5200 5650 5200 5750
$Comp
L power:GND #PWR06
U 1 1 61A0F5E3
P 5300 5450
F 0 "#PWR06" H 5300 5200 50  0001 C CNN
F 1 "GND" H 5305 5277 50  0000 C CNN
F 2 "" H 5300 5450 50  0001 C CNN
F 3 "" H 5300 5450 50  0001 C CNN
	1    5300 5450
	1    0    0    -1  
$EndComp
NoConn ~ 6700 3650
NoConn ~ 7200 3650
NoConn ~ 7200 3950
NoConn ~ 7200 4050
NoConn ~ 7200 2750
NoConn ~ 6700 2750
Wire Wire Line
	6700 1850 7200 1850
$Comp
L power:GND #PWR0101
U 1 1 61A3B3D0
P 8050 1850
F 0 "#PWR0101" H 8050 1600 50  0001 C CNN
F 1 "GND" H 8055 1677 50  0000 C CNN
F 2 "" H 8050 1850 50  0001 C CNN
F 3 "" H 8050 1850 50  0001 C CNN
	1    8050 1850
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR0102
U 1 1 61A51C51
P 7350 4150
F 0 "#PWR0102" H 7350 4000 50  0001 C CNN
F 1 "VCC" H 7367 4323 50  0000 C CNN
F 2 "" H 7350 4150 50  0001 C CNN
F 3 "" H 7350 4150 50  0001 C CNN
	1    7350 4150
	1    0    0    -1  
$EndComp
Wire Wire Line
	5200 5450 5300 5450
Connection ~ 5200 5450
Wire Wire Line
	5200 5250 5300 5250
Connection ~ 5300 5050
Wire Wire Line
	7200 1750 8050 1750
Wire Wire Line
	7200 1750 7200 1850
Wire Wire Line
	8050 1750 8050 1850
Entry Wire Line
	5550 4350 5650 4450
Entry Wire Line
	5550 4250 5650 4350
Wire Wire Line
	5550 4250 5200 4250
Wire Wire Line
	5200 4350 5550 4350
Text Label 5300 4350 0    50   ~ 0
P44
Text Label 5300 4250 0    50   ~ 0
P45
Wire Wire Line
	4100 4350 4300 4350
Text Label 6600 3750 2    50   ~ 0
P50
Text Label 6600 3850 2    50   ~ 0
P51
Text Label 6600 3950 2    50   ~ 0
P52
Text Label 6600 4050 2    50   ~ 0
P53
Text Label 7300 3850 0    50   ~ 0
P55
Text Label 5300 3850 0    50   ~ 0
P51
Text Label 5300 3950 0    50   ~ 0
P50
NoConn ~ 5200 3350
NoConn ~ 5200 3450
NoConn ~ 5200 3550
NoConn ~ 5200 3650
NoConn ~ 4300 2050
NoConn ~ 4300 2150
NoConn ~ 4300 2250
NoConn ~ 4300 2350
NoConn ~ 4300 2450
NoConn ~ 4300 2550
NoConn ~ 4300 2650
NoConn ~ 4300 2750
Wire Wire Line
	4300 3950 4300 4150
Connection ~ 4300 4150
Wire Wire Line
	4300 4150 4300 4250
NoConn ~ 7200 3750
Wire Wire Line
	5200 4050 5550 4050
Entry Wire Line
	5550 4050 5650 4150
Text Label 5300 4050 0    50   ~ 0
P41
Text Label 5300 4150 0    50   ~ 0
P55
NoConn ~ 7200 2350
NoConn ~ 7200 2450
NoConn ~ 7200 2550
NoConn ~ 7200 2650
NoConn ~ 6700 3250
NoConn ~ 6700 3350
NoConn ~ 6700 3450
NoConn ~ 6700 3550
Wire Wire Line
	4300 2900 3650 2900
Text Label 4200 2900 2    50   ~ 0
P40
Entry Wire Line
	3650 2900 3550 3000
Wire Wire Line
	4300 3350 3650 3350
Entry Wire Line
	3650 3350 3550 3450
Text Label 4200 3350 2    50   ~ 0
P42
Text Label 4200 3450 2    50   ~ 0
P52
Wire Wire Line
	5550 4150 5200 4150
NoConn ~ 7200 3150
Wire Wire Line
	7550 2950 7200 2950
Entry Wire Line
	7550 2950 7650 3050
Text Label 7300 2950 0    50   ~ 0
P41
NoConn ~ 7200 3550
NoConn ~ 4300 3750
Wire Wire Line
	5300 5050 5300 5250
Wire Wire Line
	5200 5150 5550 5150
$Comp
L 0-LocalLibrary:MSM80C39RS U1
U 1 1 62675BD9
P 4750 3150
F 0 "U1" H 4750 4517 50  0000 C CNN
F 1 "MSM80C39RS" H 4750 4426 50  0000 C CNN
F 2 "Package_DIP:DIP-40_W15.24mm" H 4800 1800 50  0001 C CIN
F 3 "https://datasheetspdf.com/pdf-file/515122/OKIelectroniccomponets/MSM80C48/1" H 4750 3150 50  0001 C CNN
	1    4750 3150
	1    0    0    -1  
$EndComp
$Comp
L 0-LocalLibrary:BionicConnector J1
U 1 1 62676F79
P 6900 2950
F 0 "J1" H 6950 4275 50  0000 C CNN
F 1 "BionicConnector" H 6950 4184 50  0000 C CNN
F 2 "0-LocalLibrary:DIP-48_W7.62mm" H 6950 1650 50  0001 C CNN
F 3 "https://www.arieselec.com/wp-content/uploads/2020/02/10001-universal-dip-zif-test-socket.pdf" H 6900 2950 50  0001 C CNN
	1    6900 2950
	1    0    0    -1  
$EndComp
Wire Wire Line
	7200 4150 7350 4150
Connection ~ 7200 1850
Wire Bus Line
	7650 1550 5750 1550
NoConn ~ 6700 2850
NoConn ~ 6700 2950
NoConn ~ 6700 3050
NoConn ~ 6700 3150
Wire Wire Line
	6700 4150 7200 4150
Connection ~ 7200 4150
Entry Wire Line
	5550 4150 5650 4250
Entry Wire Line
	5550 3850 5650 3950
Wire Wire Line
	5200 3850 5550 3850
Entry Wire Line
	5550 3950 5650 4050
Wire Wire Line
	5200 3950 5550 3950
Wire Wire Line
	7200 3450 7550 3450
Entry Wire Line
	7550 3450 7650 3550
Wire Wire Line
	7200 3850 7550 3850
Entry Wire Line
	7550 3850 7650 3950
Entry Wire Line
	6250 3850 6350 3750
Wire Wire Line
	6700 3750 6350 3750
Entry Wire Line
	6250 3950 6350 3850
Wire Wire Line
	6700 3850 6350 3850
Entry Wire Line
	6250 4050 6350 3950
Wire Wire Line
	6700 3950 6350 3950
Wire Wire Line
	6700 4050 6350 4050
Entry Wire Line
	6250 4150 6350 4050
Wire Bus Line
	7650 4700 6250 4700
Wire Wire Line
	5200 5350 5550 5350
Entry Wire Line
	5650 5050 5550 5150
Entry Wire Line
	5650 5250 5550 5350
Connection ~ 5650 4700
Wire Bus Line
	5650 4700 3550 4700
Wire Wire Line
	4300 3450 3650 3450
Entry Wire Line
	3650 3450 3550 3550
Wire Wire Line
	4600 5150 3850 5150
Wire Wire Line
	3850 5150 3850 3650
Wire Wire Line
	3850 3650 4300 3650
Wire Wire Line
	4300 3250 3750 3250
Wire Wire Line
	3750 5350 4600 5350
Wire Wire Line
	3750 3250 3750 5350
Connection ~ 6250 4700
Wire Bus Line
	6250 4700 5650 4700
Wire Bus Line
	5650 4700 5650 5250
Wire Bus Line
	3550 3000 3550 4700
Wire Bus Line
	7650 1550 7650 2150
Wire Bus Line
	5750 1550 5750 3050
Wire Bus Line
	6250 3850 6250 4700
Wire Bus Line
	5650 3950 5650 4700
Wire Bus Line
	7650 2950 7650 4700
Wire Bus Line
	6250 1750 6250 2550
Wire Bus Line
	5650 1750 5650 2650
$EndSCHEMATC
