EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "BionicMC146805E Mezzanine"
Date "2021-11-09"
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
NoConn ~ 3800 2000
NoConn ~ 3800 2100
NoConn ~ 3800 2200
NoConn ~ 3800 2300
NoConn ~ 3800 2400
NoConn ~ 3800 2500
NoConn ~ 3800 2600
NoConn ~ 3800 2700
Text Label 4800 2000 0    50   ~ 0
P10
Text Label 4800 2100 0    50   ~ 0
P11
Text Label 4800 2200 0    50   ~ 0
P12
Text Label 4800 2300 0    50   ~ 0
P13
Text Label 4800 2400 0    50   ~ 0
P14
Text Label 4800 2500 0    50   ~ 0
P15
Text Label 4800 2600 0    50   ~ 0
P16
Text Label 4800 2700 0    50   ~ 0
P17
Text Label 4800 2800 0    50   ~ 0
P20
Text Label 4800 2900 0    50   ~ 0
P21
Text Label 4800 3000 0    50   ~ 0
P22
Text Label 4800 3100 0    50   ~ 0
P23
Text Label 4800 3200 0    50   ~ 0
P24
Entry Wire Line
	5050 2700 5150 2600
Entry Wire Line
	5050 2600 5150 2500
Entry Wire Line
	5050 2500 5150 2400
Entry Wire Line
	5050 2400 5150 2300
Entry Wire Line
	5050 2300 5150 2200
Entry Wire Line
	5050 2200 5150 2100
Entry Wire Line
	5050 2100 5150 2000
Entry Wire Line
	5050 2000 5150 1900
Entry Wire Line
	5250 2700 5150 2800
Entry Wire Line
	5250 2800 5150 2900
Entry Wire Line
	5250 2900 5150 3000
Entry Wire Line
	5250 3000 5150 3100
Entry Wire Line
	5250 3100 5150 3200
Wire Wire Line
	4700 2000 5050 2000
Wire Wire Line
	4700 2100 5050 2100
Wire Wire Line
	4700 2200 5050 2200
Wire Wire Line
	4700 2300 5050 2300
Wire Wire Line
	4700 2400 5050 2400
Wire Wire Line
	4700 2500 5050 2500
Wire Wire Line
	4700 2600 5050 2600
Wire Wire Line
	4700 2700 5050 2700
Wire Wire Line
	4700 2800 5150 2800
Wire Wire Line
	4700 2900 5150 2900
Wire Wire Line
	4700 3000 5150 3000
Wire Wire Line
	4700 3100 5150 3100
Wire Wire Line
	4700 3200 5150 3200
Text Label 6800 1900 2    50   ~ 0
P10
Text Label 6800 2000 2    50   ~ 0
P11
Text Label 6800 2100 2    50   ~ 0
P12
Text Label 6800 2200 2    50   ~ 0
P13
Text Label 6800 2300 2    50   ~ 0
P14
Text Label 6800 2400 2    50   ~ 0
P15
Text Label 6800 2500 2    50   ~ 0
P16
Text Label 6800 2600 2    50   ~ 0
P17
Entry Wire Line
	6550 2600 6450 2500
Entry Wire Line
	6550 2500 6450 2400
Entry Wire Line
	6550 2400 6450 2300
Entry Wire Line
	6550 2300 6450 2200
Entry Wire Line
	6550 2200 6450 2100
Entry Wire Line
	6550 2100 6450 2000
Entry Wire Line
	6550 2000 6450 1900
Entry Wire Line
	6550 1900 6450 1800
Wire Wire Line
	6900 1900 6550 1900
Wire Wire Line
	6900 2000 6550 2000
Wire Wire Line
	6900 2100 6550 2100
Wire Wire Line
	6900 2200 6550 2200
Wire Wire Line
	6900 2300 6550 2300
Wire Wire Line
	6900 2400 6550 2400
Wire Wire Line
	6900 2500 6550 2500
Wire Wire Line
	6900 2600 6550 2600
Text Label 6800 2800 2    50   ~ 0
P20
Text Label 6800 2900 2    50   ~ 0
P21
Text Label 6800 3000 2    50   ~ 0
P22
Text Label 6800 3100 2    50   ~ 0
P23
Text Label 6800 3200 2    50   ~ 0
P24
Entry Wire Line
	6350 2700 6450 2800
Entry Wire Line
	6350 2800 6450 2900
Entry Wire Line
	6350 2900 6450 3000
Entry Wire Line
	6350 3000 6450 3100
Entry Wire Line
	6350 3100 6450 3200
Wire Wire Line
	6900 2800 6450 2800
Wire Wire Line
	6900 2900 6450 2900
Wire Wire Line
	6900 3000 6450 3000
Wire Wire Line
	6900 3100 6450 3100
Wire Wire Line
	6900 3200 6450 3200
Wire Bus Line
	5150 1600 6450 1600
Wire Bus Line
	5250 2550 6350 2550
Wire Wire Line
	3600 4100 3800 4100
Wire Wire Line
	3600 4350 3600 4300
Wire Wire Line
	3600 4100 3600 4050
Connection ~ 3600 4300
Connection ~ 3600 4100
$Comp
L Device:C_Small C2
U 1 1 5D0E12B4
P 3600 4200
F 0 "C2" H 3700 4250 50  0000 L CNN
F 1 "0.1u" H 3700 4150 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D3.4mm_W2.1mm_P2.50mm" H 3600 4200 50  0001 C CNN
F 3 "~" H 3600 4200 50  0001 C CNN
	1    3600 4200
	-1   0    0    -1  
$EndComp
$Comp
L power:VCC #PWR03
U 1 1 5CE117D7
P 3600 4050
F 0 "#PWR03" H 3600 3900 50  0001 C CNN
F 1 "VCC" H 3700 4150 50  0000 C CNN
F 2 "" H 3600 4050 50  0001 C CNN
F 3 "" H 3600 4050 50  0001 C CNN
	1    3600 4050
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR04
U 1 1 5CE12AA7
P 3600 4350
F 0 "#PWR04" H 3600 4100 50  0001 C CNN
F 1 "GND" H 3700 4250 50  0000 C CNN
F 2 "" H 3600 4350 50  0001 C CNN
F 3 "" H 3600 4350 50  0001 C CNN
	1    3600 4350
	-1   0    0    -1  
$EndComp
Text Label 7500 2400 0    50   ~ 0
P34
Text Label 7500 2500 0    50   ~ 0
P35
Text Label 7500 2600 0    50   ~ 0
P36
Wire Wire Line
	7750 2400 7400 2400
Wire Wire Line
	7750 2500 7400 2500
Wire Wire Line
	7750 2600 7400 2600
Text Label 4800 3700 0    50   ~ 0
P34
Text Label 4800 3800 0    50   ~ 0
P35
Text Label 4800 3900 0    50   ~ 0
P36
Text Label 4800 4000 0    50   ~ 0
P45
Text Label 4800 4100 0    50   ~ 0
P44
Text Label 3700 3100 2    50   ~ 0
P42
Text Label 3700 3400 2    50   ~ 0
P50
Text Label 3700 3500 2    50   ~ 0
P41
Text Label 3700 3600 2    50   ~ 0
P40
Text Label 3700 3800 2    50   ~ 0
P46
Wire Wire Line
	4700 3700 5250 3700
Wire Wire Line
	4700 3900 5250 3900
Entry Wire Line
	5350 3600 5250 3700
Entry Wire Line
	5350 3700 5250 3800
Entry Wire Line
	5350 3800 5250 3900
Wire Bus Line
	5350 1500 7850 1500
Entry Wire Line
	7850 2300 7750 2400
Entry Wire Line
	7850 2400 7750 2500
Entry Wire Line
	7850 2500 7750 2600
Entry Wire Line
	3050 3000 3150 3100
Entry Wire Line
	3050 3400 3150 3500
Entry Wire Line
	3050 3500 3150 3600
Entry Wire Line
	3050 3700 3150 3800
Entry Wire Line
	3050 3300 3150 3400
Wire Wire Line
	4700 4000 5350 4000
Wire Wire Line
	4700 4100 5350 4100
Entry Wire Line
	5450 3900 5350 4000
Entry Wire Line
	5450 4000 5350 4100
Text Label 7500 2800 0    50   ~ 0
P40
Text Label 7500 2900 0    50   ~ 0
P41
Text Label 7500 3000 0    50   ~ 0
P42
Text Label 7500 3100 0    50   ~ 0
P43
Text Label 7500 3200 0    50   ~ 0
P44
Text Label 7500 3300 0    50   ~ 0
P45
Text Label 7500 3400 0    50   ~ 0
P46
Text Label 7500 3500 0    50   ~ 0
P47
Wire Wire Line
	7400 2800 7850 2800
Wire Wire Line
	7400 2900 7850 2900
Wire Wire Line
	7400 3000 7850 3000
Wire Wire Line
	7400 3100 7850 3100
Wire Wire Line
	7400 3200 7850 3200
Wire Wire Line
	7400 3300 7850 3300
Wire Wire Line
	7400 3400 7850 3400
Wire Wire Line
	7400 3500 7850 3500
Entry Wire Line
	7950 2700 7850 2800
Entry Wire Line
	7950 2800 7850 2900
Entry Wire Line
	7950 2900 7850 3000
Entry Wire Line
	7950 3000 7850 3100
Entry Wire Line
	7950 3100 7850 3200
Entry Wire Line
	7950 3200 7850 3300
Entry Wire Line
	7950 3300 7850 3400
Entry Wire Line
	7950 3400 7850 3500
Wire Bus Line
	7950 1400 5450 1400
NoConn ~ 6900 3600
NoConn ~ 6900 3700
NoConn ~ 6900 3800
NoConn ~ 6900 3900
NoConn ~ 7400 3600
NoConn ~ 7400 3700
NoConn ~ 7400 3800
NoConn ~ 7400 3900
NoConn ~ 7400 4000
NoConn ~ 7400 2700
NoConn ~ 6900 2700
$Comp
L power:GND #PWR0101
U 1 1 61A3B3D0
P 7550 1800
F 0 "#PWR0101" H 7550 1550 50  0001 C CNN
F 1 "GND" H 7555 1627 50  0000 C CNN
F 2 "" H 7550 1800 50  0001 C CNN
F 3 "" H 7550 1800 50  0001 C CNN
	1    7550 1800
	1    0    0    -1  
$EndComp
NoConn ~ 7400 1900
NoConn ~ 7400 2000
NoConn ~ 7400 2100
NoConn ~ 7400 2200
NoConn ~ 7400 2300
$Comp
L power:VCC #PWR0102
U 1 1 61A51C51
P 7550 4100
F 0 "#PWR0102" H 7550 3950 50  0001 C CNN
F 1 "VCC" H 7567 4273 50  0000 C CNN
F 2 "" H 7550 4100 50  0001 C CNN
F 3 "" H 7550 4100 50  0001 C CNN
	1    7550 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	3150 3400 3800 3400
Wire Wire Line
	3150 3500 3800 3500
Wire Wire Line
	3150 3600 3800 3600
Wire Wire Line
	3150 3800 3800 3800
Wire Bus Line
	3050 1400 5450 1400
Connection ~ 5450 1400
NoConn ~ 3800 3900
Wire Wire Line
	3150 3100 3800 3100
Wire Wire Line
	3600 4300 3800 4300
$Comp
L 0-LocalLibrary:BionicConnector J1
U 1 1 6217C56F
P 7100 2900
F 0 "J1" H 7150 4225 50  0000 C CNN
F 1 "BionicConnector" H 7150 4134 50  0000 C CNN
F 2 "0-LocalLibrary:DIP-48_W10.16mm" H 7150 1600 50  0001 C CNN
F 3 "https://www.arieselec.com/wp-content/uploads/2020/02/10001-universal-dip-zif-test-socket.pdf" H 7100 2900 50  0001 C CNN
	1    7100 2900
	1    0    0    -1  
$EndComp
Wire Wire Line
	7400 1800 7550 1800
Wire Wire Line
	7400 4100 7550 4100
$Comp
L 0-LocalLibrary:MC146805E2P U1
U 1 1 6218ACEF
P 4250 3100
F 0 "U1" H 4250 4467 50  0000 C CNN
F 1 "MC146805E2P" H 4250 4376 50  0000 C CNN
F 2 "Package_DIP:DIP-40_W15.24mm" H 4300 1750 50  0001 C CIN
F 3 "https://www.datasheetarchive.com/pdf/download.php?id=8308749ebec04603691dcc4aa7c9e5d85c5a25&type=M&term=MC146805E2P" H 4250 3100 50  0001 C CNN
	1    4250 3100
	1    0    0    -1  
$EndComp
NoConn ~ 4700 3400
NoConn ~ 6900 3300
NoConn ~ 6900 3400
NoConn ~ 6900 3500
Wire Wire Line
	4700 3800 5250 3800
Text Label 3700 3300 2    50   ~ 0
P51
Entry Wire Line
	3050 3200 3150 3300
Wire Wire Line
	3150 3300 3800 3300
Text Label 3700 3000 2    50   ~ 0
P53
Text Label 6650 4000 0    50   ~ 0
P53
Wire Wire Line
	6900 4000 6350 4000
Wire Wire Line
	6350 4000 6350 4850
Wire Wire Line
	6350 4850 3200 4850
Wire Wire Line
	3200 4850 3200 3000
Wire Wire Line
	3200 3000 3800 3000
Wire Bus Line
	5450 1400 5450 4000
Wire Bus Line
	5350 1500 5350 3800
Wire Bus Line
	7850 1500 7850 2500
Wire Bus Line
	5250 2550 5250 3100
Wire Bus Line
	6350 2550 6350 3100
Wire Bus Line
	3050 1400 3050 3700
Wire Bus Line
	7950 1400 7950 3400
Wire Bus Line
	6450 1600 6450 2500
Wire Bus Line
	5150 1600 5150 2600
$EndSCHEMATC
