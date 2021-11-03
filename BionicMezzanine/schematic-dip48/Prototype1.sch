EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "BionicMezzanine DIP48"
Date "2021-11-03"
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
Text Label 2250 5850 2    50   ~ 0
VCC
Text Label 2250 6150 2    50   ~ 0
GND
$Comp
L Connector_Generic:Conn_02x07_Odd_Even J8
U 1 1 619DEADD
P 3550 5250
F 0 "J8" H 3600 5767 50  0000 C CNN
F 1 "Conn_02x07_Odd_Even" H 3600 5676 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_2x07_P2.54mm_Vertical" H 3550 5250 50  0001 C CNN
F 3 "~" H 3550 5250 50  0001 C CNN
	1    3550 5250
	1    0    0    -1  
$EndComp
$Comp
L 0-LocalLibrary:74LS126 U1
U 1 1 62528091
P 2600 5450
F 0 "U1" H 2600 6217 50  0000 C CNN
F 1 "74LS126" H 2600 6126 50  0000 C CNN
F 2 "Package_DIP:DIP-14_W7.62mm" H 2600 4600 50  0001 C CNN
F 3 "https://www.ti.com/lit/ds/symlink/cd74hct126.pdf" H 2600 5450 50  0001 C CNN
	1    2600 5450
	1    0    0    -1  
$EndComp
$Comp
L 0-LocalLibrary:ZIF_Socket_48pin J4
U 1 1 626E9F26
P 6850 2100
F 0 "J4" H 7075 3425 50  0000 C CNN
F 1 "ZIF_Socket_48pin" H 7075 3334 50  0000 C CNN
F 2 "0-LocalLibrary:Aries_ZIF_socket_DIP-48-pin" H 7100 800 50  0001 C CNN
F 3 "https://www.arieselec.com/wp-content/uploads/2020/02/10001-universal-dip-zif-test-socket.pdf" H 6850 2100 50  0001 C CNN
	1    6850 2100
	1    0    0    -1  
$EndComp
$Comp
L 0-LocalLibrary:BionicConnector J1
U 1 1 6278A39E
P 2550 2100
F 0 "J1" H 2600 3417 50  0000 C CNN
F 1 "BionicConnector" H 2600 3326 50  0000 C CNN
F 2 "0-LocalLibrary:DIP-48_W10.16mm" H 2600 800 50  0001 C CNN
F 3 "https://www.arieselec.com/wp-content/uploads/2020/02/10001-universal-dip-zif-test-socket.pdf" H 2550 2100 50  0001 C CNN
	1    2550 2100
	1    0    0    -1  
$EndComp
$Comp
L 0-LocalLibrary:74HCT573 U2
U 1 1 62C3C671
P 5900 4750
F 0 "U2" H 5900 5517 50  0000 C CNN
F 1 "74HCT573" H 5900 5426 50  0000 C CNN
F 2 "Package_DIP:DIP-20_W7.62mm" H 5900 4750 50  0001 C CNN
F 3 "https://www.ti.com/lit/ds/symlink/cd74hct573.pdf" H 5900 4750 50  0001 C CNN
	1    5900 4750
	1    0    0    -1  
$EndComp
$Comp
L 0-LocalLibrary:74HCT139 U5
U 1 1 62CDDBB7
P 6150 6200
F 0 "U5" H 6150 6567 50  0000 C CNN
F 1 "74HCT139" H 6150 6476 50  0000 C CNN
F 2 "Package_DIP:DIP-16_W7.62mm" H 6150 6200 50  0001 C CNN
F 3 "https://www.ti.com/lit/ds/symlink/cd74hct139.pdf" H 6150 6200 50  0001 C CNN
	1    6150 6200
	1    0    0    -1  
$EndComp
$Comp
L 0-LocalLibrary:74HCT139 U5
U 2 1 62CE02D0
P 6150 7050
F 0 "U5" H 6150 7417 50  0000 C CNN
F 1 "74HCT139" H 6150 7326 50  0000 C CNN
F 2 "Package_DIP:DIP-16_W7.62mm" H 6150 7050 50  0001 C CNN
F 3 "https://www.ti.com/lit/ds/symlink/cd74hct139.pdf" H 6150 7050 50  0001 C CNN
	2    6150 7050
	1    0    0    -1  
$EndComp
$Comp
L 0-LocalLibrary:74HCT139 U5
U 3 1 62CE618D
P 2550 6500
F 0 "U5" H 2680 6550 50  0000 L CNN
F 1 "74HCT139" H 2680 6459 50  0000 L CNN
F 2 "Package_DIP:DIP-16_W7.62mm" H 2550 6500 50  0001 C CNN
F 3 "https://www.ti.com/lit/ds/symlink/cd74hct139.pdf" H 2550 6500 50  0001 C CNN
	3    2550 6500
	1    0    0    -1  
$EndComp
Text Label 6250 4250 0    50   ~ 0
P20
Text Label 6250 4350 0    50   ~ 0
P21
Text Label 6250 4450 0    50   ~ 0
P22
Text Label 6250 4550 0    50   ~ 0
P23
Text Label 6250 4650 0    50   ~ 0
P24
Text Label 6250 4750 0    50   ~ 0
P25
Text Label 6250 4850 0    50   ~ 0
P26
Text Label 6250 4950 0    50   ~ 0
P27
Entry Wire Line
	6500 4250 6400 4350
Entry Wire Line
	6500 4150 6400 4250
Entry Wire Line
	6500 4550 6400 4650
Entry Wire Line
	6500 4450 6400 4550
Entry Wire Line
	6500 4350 6400 4450
Entry Wire Line
	6500 4850 6400 4950
Entry Wire Line
	6500 4750 6400 4850
Entry Wire Line
	6500 4650 6400 4750
Wire Wire Line
	6200 4950 6400 4950
$Comp
L Device:C_Small C3
U 1 1 6311F082
P 6300 5250
F 0 "C3" H 6350 5300 50  0000 L CNN
F 1 "0.1uF" H 6250 5050 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D3.0mm_W1.6mm_P2.50mm" H 6300 5250 50  0001 C CNN
F 3 "~" H 6300 5250 50  0001 C CNN
	1    6300 5250
	1    0    0    -1  
$EndComp
Wire Wire Line
	6200 5150 6300 5150
$Comp
L Device:C_Small C1
U 1 1 633EF43B
P 2250 5950
F 0 "C1" H 2200 6150 50  0000 L CNN
F 1 "0.1uF" H 2150 5700 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D3.0mm_W1.6mm_P2.50mm" H 2250 5950 50  0001 C CNN
F 3 "~" H 2250 5950 50  0001 C CNN
	1    2250 5950
	-1   0    0    -1  
$EndComp
Wire Wire Line
	2300 5850 2250 5850
Text Label 6250 5150 0    50   ~ 0
VCC
Wire Wire Line
	2250 6050 2300 6050
Wire Wire Line
	2100 5850 2250 5850
Connection ~ 2250 5850
Connection ~ 2250 6050
Text Label 6400 5350 0    50   ~ 0
GND
Entry Wire Line
	6400 5150 6500 5050
Entry Wire Line
	6400 5350 6500 5250
Wire Wire Line
	6300 5150 6400 5150
Connection ~ 6300 5150
Wire Wire Line
	6300 5350 6400 5350
Connection ~ 6300 5350
Text Label 2250 6400 2    50   ~ 0
VCC
Text Label 2250 6700 2    50   ~ 0
GND
$Comp
L Device:C_Small C2
U 1 1 63DE13F9
P 2250 6500
F 0 "C2" H 2150 6700 50  0000 L CNN
F 1 "0.1uF" H 2150 6250 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D3.0mm_W1.6mm_P2.50mm" H 2250 6500 50  0001 C CNN
F 3 "~" H 2250 6500 50  0001 C CNN
	1    2250 6500
	-1   0    0    -1  
$EndComp
Wire Wire Line
	2300 6400 2250 6400
Wire Wire Line
	2250 6600 2300 6600
Wire Wire Line
	2100 6400 2250 6400
Connection ~ 2250 6400
Connection ~ 2250 6600
Wire Wire Line
	5600 5150 5500 5150
Wire Wire Line
	5400 5350 5600 5350
Wire Wire Line
	5150 5150 5300 5150
Wire Wire Line
	6200 4650 6400 4650
Wire Wire Line
	6200 4750 6400 4750
Wire Wire Line
	6200 4850 6400 4850
Wire Wire Line
	6200 4350 6400 4350
Wire Wire Line
	6200 4450 6400 4450
Wire Wire Line
	6200 4550 6400 4550
Wire Wire Line
	6200 4250 6400 4250
Wire Wire Line
	5400 5050 5400 5350
Wire Wire Line
	6200 5350 6300 5350
$Comp
L 0-LocalLibrary:74HCT573 U3
U 1 1 6662EFDD
P 7850 4750
F 0 "U3" H 7850 5517 50  0000 C CNN
F 1 "74HCT573" H 7850 5426 50  0000 C CNN
F 2 "Package_DIP:DIP-20_W7.62mm" H 7850 4750 50  0001 C CNN
F 3 "https://www.ti.com/lit/ds/symlink/cd74hct573.pdf" H 7850 4750 50  0001 C CNN
	1    7850 4750
	1    0    0    -1  
$EndComp
Text Label 8200 4250 0    50   ~ 0
P20
Text Label 8200 4350 0    50   ~ 0
P21
Text Label 8200 4450 0    50   ~ 0
P22
Text Label 8200 4550 0    50   ~ 0
P23
Text Label 8200 4650 0    50   ~ 0
P24
Text Label 8200 4750 0    50   ~ 0
P25
Text Label 8200 4850 0    50   ~ 0
P26
Text Label 8200 4950 0    50   ~ 0
P27
Entry Wire Line
	8450 4250 8350 4350
Entry Wire Line
	8450 4150 8350 4250
Entry Wire Line
	8450 4550 8350 4650
Entry Wire Line
	8450 4450 8350 4550
Entry Wire Line
	8450 4350 8350 4450
Entry Wire Line
	8450 4850 8350 4950
Entry Wire Line
	8450 4750 8350 4850
Entry Wire Line
	8450 4650 8350 4750
Wire Wire Line
	8150 4950 8350 4950
$Comp
L Device:C_Small C4
U 1 1 6662EFF4
P 8250 5250
F 0 "C4" H 8300 5300 50  0000 L CNN
F 1 "0.1uF" H 8150 5050 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D3.0mm_W1.6mm_P2.50mm" H 8250 5250 50  0001 C CNN
F 3 "~" H 8250 5250 50  0001 C CNN
	1    8250 5250
	1    0    0    -1  
$EndComp
Wire Wire Line
	8150 5150 8250 5150
Text Label 8200 5150 0    50   ~ 0
VCC
Text Label 8350 5350 0    50   ~ 0
GND
Entry Wire Line
	8350 5150 8450 5050
Entry Wire Line
	8350 5350 8450 5250
Wire Wire Line
	8250 5150 8350 5150
Connection ~ 8250 5150
Wire Wire Line
	8250 5350 8350 5350
Connection ~ 8250 5350
Wire Wire Line
	7550 5150 7450 5150
Wire Wire Line
	7350 5350 7550 5350
Wire Wire Line
	7100 5150 7250 5150
Wire Wire Line
	8150 4650 8350 4650
Wire Wire Line
	8150 4750 8350 4750
Wire Wire Line
	8150 4850 8350 4850
Wire Wire Line
	8150 4350 8350 4350
Wire Wire Line
	8150 4450 8350 4450
Wire Wire Line
	8150 4550 8350 4550
Wire Wire Line
	8150 4250 8350 4250
Wire Wire Line
	7350 5050 7350 5350
Wire Wire Line
	8150 5350 8250 5350
$Comp
L 0-LocalLibrary:74HCT573 U4
U 1 1 666962D9
P 9800 4750
F 0 "U4" H 9800 5517 50  0000 C CNN
F 1 "74HCT573" H 9800 5426 50  0000 C CNN
F 2 "Package_DIP:DIP-20_W7.62mm" H 9800 4750 50  0001 C CNN
F 3 "https://www.ti.com/lit/ds/symlink/cd74hct573.pdf" H 9800 4750 50  0001 C CNN
	1    9800 4750
	1    0    0    -1  
$EndComp
Text Label 10150 4250 0    50   ~ 0
P20
Text Label 10150 4350 0    50   ~ 0
P21
Text Label 10150 4450 0    50   ~ 0
P22
Text Label 10150 4550 0    50   ~ 0
P23
Text Label 10150 4650 0    50   ~ 0
P24
Text Label 10150 4750 0    50   ~ 0
P25
Text Label 10150 4850 0    50   ~ 0
P26
Text Label 10150 4950 0    50   ~ 0
P27
Entry Wire Line
	10400 4250 10300 4350
Entry Wire Line
	10400 4150 10300 4250
Entry Wire Line
	10400 4550 10300 4650
Entry Wire Line
	10400 4450 10300 4550
Entry Wire Line
	10400 4350 10300 4450
Entry Wire Line
	10400 4850 10300 4950
Entry Wire Line
	10400 4750 10300 4850
Entry Wire Line
	10400 4650 10300 4750
Wire Wire Line
	10100 4950 10300 4950
$Comp
L Device:C_Small C5
U 1 1 666962F0
P 10200 5250
F 0 "C5" H 10250 5300 50  0000 L CNN
F 1 "0.1uF" H 10150 5050 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D3.0mm_W1.6mm_P2.50mm" H 10200 5250 50  0001 C CNN
F 3 "~" H 10200 5250 50  0001 C CNN
	1    10200 5250
	1    0    0    -1  
$EndComp
Wire Wire Line
	10100 5150 10200 5150
Text Label 10150 5150 0    50   ~ 0
VCC
Text Label 10250 5350 0    50   ~ 0
GND
Entry Wire Line
	10300 5150 10400 5050
Entry Wire Line
	10300 5350 10400 5250
Wire Wire Line
	10200 5150 10300 5150
Connection ~ 10200 5150
Wire Wire Line
	10200 5350 10300 5350
Connection ~ 10200 5350
Wire Wire Line
	9500 5150 9400 5150
Wire Wire Line
	9300 5350 9500 5350
Wire Wire Line
	9050 5150 9200 5150
Wire Wire Line
	10100 4650 10300 4650
Wire Wire Line
	10100 4750 10300 4750
Wire Wire Line
	10100 4850 10300 4850
Wire Wire Line
	10100 4350 10300 4350
Wire Wire Line
	10100 4450 10300 4450
Wire Wire Line
	10100 4550 10300 4550
Wire Wire Line
	10100 4250 10300 4250
Wire Wire Line
	9300 5050 9300 5350
Wire Wire Line
	10100 5350 10200 5350
Wire Bus Line
	10400 3800 8450 3800
Connection ~ 6500 3800
Connection ~ 8450 3800
Wire Bus Line
	8450 3800 6500 3800
Wire Wire Line
	6500 6200 7450 6200
Wire Wire Line
	6500 6300 9400 6300
Wire Wire Line
	9400 5150 9400 6300
Wire Wire Line
	7450 5150 7450 6200
NoConn ~ 6500 6400
$Comp
L power:GND #PWR02
U 1 1 66C63D75
P 5800 7250
F 0 "#PWR02" H 5800 7000 50  0001 C CNN
F 1 "GND" H 5805 7077 50  0000 C CNN
F 2 "" H 5800 7250 50  0001 C CNN
F 3 "" H 5800 7250 50  0001 C CNN
	1    5800 7250
	1    0    0    -1  
$EndComp
Wire Wire Line
	3850 5250 3350 5250
Wire Wire Line
	3850 5450 3350 5450
Wire Wire Line
	3850 5550 3350 5550
Entry Wire Line
	2050 2700 2150 2800
Text Label 2350 2800 2    50   ~ 0
E2
Entry Wire Line
	2050 1800 2150 1900
Text Label 2350 1900 2    50   ~ 0
E0
Entry Wire Line
	2050 900  2150 1000
Entry Wire Line
	2050 3200 2150 3300
Entry Wire Line
	2050 2400 2150 2500
Entry Wire Line
	2050 2500 2150 2600
Entry Wire Line
	2050 2600 2150 2700
Entry Wire Line
	2050 2100 2150 2200
Entry Wire Line
	2050 2200 2150 2300
Entry Wire Line
	2050 2300 2150 2400
Entry Wire Line
	2050 1900 2150 2000
Entry Wire Line
	2050 2000 2150 2100
Entry Wire Line
	2050 1000 2150 1100
Entry Wire Line
	2050 1100 2150 1200
Entry Wire Line
	2050 1200 2150 1300
Entry Wire Line
	2050 1300 2150 1400
Entry Wire Line
	2050 1400 2150 1500
Entry Wire Line
	2050 1500 2150 1600
Entry Wire Line
	2050 1600 2150 1700
Entry Wire Line
	2050 1700 2150 1800
Entry Wire Line
	2050 2800 2150 2900
Entry Wire Line
	2050 2900 2150 3000
Entry Wire Line
	2050 3000 2150 3100
Entry Wire Line
	2050 3100 2150 3200
Text Label 2350 2900 2    50   ~ 0
P50
Text Label 2350 3000 2    50   ~ 0
P51
Text Label 2350 3100 2    50   ~ 0
P52
Text Label 2350 3200 2    50   ~ 0
P53
Text Label 2350 3300 2    50   ~ 0
VCC
Text Label 2350 2700 2    50   ~ 0
P27
Text Label 2350 2600 2    50   ~ 0
P26
Text Label 2350 2500 2    50   ~ 0
P25
Text Label 2350 2400 2    50   ~ 0
P24
Text Label 2350 2300 2    50   ~ 0
P23
Text Label 2350 2200 2    50   ~ 0
P22
Text Label 2350 2100 2    50   ~ 0
P21
Text Label 2350 2000 2    50   ~ 0
P20
Text Label 2350 1800 2    50   ~ 0
P17
Text Label 2350 1700 2    50   ~ 0
P16
Text Label 2350 1600 2    50   ~ 0
P15
Text Label 2350 1500 2    50   ~ 0
P14
Text Label 2350 1400 2    50   ~ 0
P13
Text Label 2350 1300 2    50   ~ 0
P12
Text Label 2350 1200 2    50   ~ 0
P11
Text Label 2350 1100 2    50   ~ 0
P10
Text Label 2350 1000 2    50   ~ 0
GND
Entry Wire Line
	3200 2700 3100 2800
Text Label 2900 2800 0    50   ~ 0
E3
Entry Wire Line
	3200 1800 3100 1900
Text Label 2900 1900 0    50   ~ 0
E1
Entry Wire Line
	3200 900  3100 1000
Entry Wire Line
	3200 3200 3100 3300
Entry Wire Line
	3200 2800 3100 2900
Entry Wire Line
	3200 2900 3100 3000
Entry Wire Line
	3200 3000 3100 3100
Entry Wire Line
	3200 3100 3100 3200
Entry Wire Line
	3200 1000 3100 1100
Entry Wire Line
	3200 1100 3100 1200
Entry Wire Line
	3200 1200 3100 1300
Entry Wire Line
	3200 1300 3100 1400
Entry Wire Line
	3200 1400 3100 1500
Entry Wire Line
	3200 1500 3100 1600
Entry Wire Line
	3200 1600 3100 1700
Entry Wire Line
	3200 1700 3100 1800
Entry Wire Line
	3200 2000 3100 2100
Entry Wire Line
	3200 1900 3100 2000
Entry Wire Line
	3200 2300 3100 2400
Entry Wire Line
	3200 2200 3100 2300
Entry Wire Line
	3200 2100 3100 2200
Entry Wire Line
	3200 2600 3100 2700
Entry Wire Line
	3200 2500 3100 2600
Entry Wire Line
	3200 2400 3100 2500
Text Label 2900 3200 0    50   ~ 0
P57
Text Label 2900 3100 0    50   ~ 0
P56
Text Label 2900 2900 0    50   ~ 0
P54
Text Label 2900 3000 0    50   ~ 0
P55
Text Label 2900 1000 0    50   ~ 0
GND
Text Label 2900 2000 0    50   ~ 0
P40
Text Label 2900 2100 0    50   ~ 0
P41
Text Label 2900 2200 0    50   ~ 0
P42
Text Label 2900 2300 0    50   ~ 0
P43
Text Label 2900 2400 0    50   ~ 0
P44
Text Label 2900 2500 0    50   ~ 0
P45
Text Label 2900 2600 0    50   ~ 0
P46
Text Label 2900 2700 0    50   ~ 0
P47
Text Label 2900 1500 0    50   ~ 0
P34
Text Label 2900 1600 0    50   ~ 0
P35
Text Label 2900 1700 0    50   ~ 0
P36
Text Label 2900 1800 0    50   ~ 0
P37
Text Label 2900 3300 0    50   ~ 0
VCC
Text Label 2900 1400 0    50   ~ 0
P33
Text Label 2900 1300 0    50   ~ 0
P32
Text Label 2900 1200 0    50   ~ 0
P31
Text Label 2900 1100 0    50   ~ 0
P30
Wire Wire Line
	1200 5550 1700 5550
Wire Wire Line
	1700 5450 1200 5450
Wire Wire Line
	1200 5350 1700 5350
Wire Wire Line
	1700 5250 1200 5250
Wire Wire Line
	1200 5150 1700 5150
Wire Wire Line
	1700 5050 1200 5050
$Comp
L Connector_Generic:Conn_02x07_Odd_Even J7
U 1 1 619DDC54
P 1400 5250
F 0 "J7" H 1450 5767 50  0000 C CNN
F 1 "Conn_02x07_Odd_Even" H 1450 5676 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_2x07_P2.54mm_Vertical" H 1400 5250 50  0001 C CNN
F 3 "~" H 1400 5250 50  0001 C CNN
	1    1400 5250
	1    0    0    -1  
$EndComp
Wire Wire Line
	1200 4950 1700 4950
Connection ~ 1700 4950
Wire Wire Line
	1700 4950 2300 4950
Connection ~ 1700 5050
Wire Wire Line
	1700 5050 2300 5050
Wire Wire Line
	2900 5050 2900 4600
Wire Wire Line
	2900 4600 1050 4600
Wire Wire Line
	1050 4600 1050 5150
Wire Wire Line
	1050 5150 1200 5150
Connection ~ 1200 5150
Wire Wire Line
	1700 5250 1800 5250
Wire Wire Line
	1800 5250 1800 5150
Wire Wire Line
	1800 5150 2300 5150
Connection ~ 1700 5250
Wire Wire Line
	2300 5250 1900 5250
Wire Wire Line
	1900 5250 1900 5350
Wire Wire Line
	1900 5350 1700 5350
Connection ~ 1700 5350
Wire Wire Line
	2900 5250 3000 5250
Wire Wire Line
	3000 5250 3000 4500
Wire Wire Line
	3000 4500 2000 4500
Wire Wire Line
	2000 4500 2000 5450
Wire Wire Line
	2000 5450 1700 5450
Connection ~ 1700 5450
Wire Wire Line
	1700 5550 1700 6050
Wire Wire Line
	1700 6050 2250 6050
Connection ~ 1700 5550
Wire Wire Line
	1700 6050 1700 6600
Wire Wire Line
	1700 6600 2250 6600
Connection ~ 1700 6050
Wire Wire Line
	2300 5350 2250 5350
Wire Wire Line
	2250 5350 2250 4450
Wire Wire Line
	2250 4450 3300 4450
Wire Wire Line
	2300 5450 2200 5450
Wire Wire Line
	2200 5450 2200 4400
Wire Wire Line
	2200 4400 3250 4400
Wire Wire Line
	3350 5050 3850 5050
Wire Wire Line
	3050 5450 2900 5450
Wire Wire Line
	3350 5150 3850 5150
Wire Wire Line
	2900 5650 3350 5650
Wire Wire Line
	3350 5650 3350 5550
Connection ~ 3350 5550
Wire Wire Line
	3050 5450 3050 5250
Wire Wire Line
	3050 5250 3350 5250
Connection ~ 3350 5250
Wire Wire Line
	3250 5150 3350 5150
Wire Wire Line
	3250 4400 3250 5150
Connection ~ 3350 5150
Wire Wire Line
	3300 5050 3350 5050
Wire Wire Line
	3300 4450 3300 5050
Connection ~ 3350 5050
Wire Wire Line
	2300 5550 2150 5550
Wire Wire Line
	2150 5550 2150 4350
Wire Wire Line
	2150 4350 3200 4350
Wire Wire Line
	3200 4350 3200 5350
Wire Wire Line
	3200 5350 3350 5350
Connection ~ 3350 5350
Wire Wire Line
	3350 5350 3850 5350
Wire Wire Line
	3350 5450 3150 5450
Wire Wire Line
	3150 5450 3150 4300
Wire Wire Line
	3150 4300 2100 4300
Wire Wire Line
	2100 4300 2100 5650
Wire Wire Line
	2100 5650 2300 5650
Connection ~ 3350 5450
Wire Wire Line
	3850 4950 3850 5050
Wire Wire Line
	3850 6250 2100 6250
Wire Wire Line
	2100 6250 2100 5850
Connection ~ 3850 5050
Wire Wire Line
	3850 5050 3850 5150
Connection ~ 3850 5150
Wire Wire Line
	3850 5150 3850 5250
Connection ~ 3850 5250
Wire Wire Line
	3850 5250 3850 5350
Connection ~ 3850 5350
Wire Wire Line
	3850 5350 3850 5450
Connection ~ 3850 5450
Wire Wire Line
	3850 5450 3850 5550
Connection ~ 3850 5550
Wire Wire Line
	3850 5550 3850 6250
Wire Wire Line
	2100 6250 2100 6400
Connection ~ 2100 6250
Connection ~ 5800 7250
Wire Wire Line
	5800 6950 5800 6400
Wire Wire Line
	5800 6950 5800 7050
Connection ~ 5800 6950
Connection ~ 5800 7050
Wire Wire Line
	5800 7050 5800 7250
NoConn ~ 6500 6950
NoConn ~ 6500 7050
NoConn ~ 6500 7150
NoConn ~ 6500 7250
Wire Wire Line
	5800 6100 5300 6100
Wire Wire Line
	5300 5150 5300 6100
Wire Wire Line
	5500 5750 6500 5750
Wire Wire Line
	5500 5150 5500 5750
Wire Wire Line
	6500 5750 6500 6100
Wire Wire Line
	7250 5600 5750 5600
Wire Wire Line
	5750 5600 5750 6200
Wire Wire Line
	5750 6200 5800 6200
Wire Wire Line
	7250 5150 7250 5600
Wire Wire Line
	1900 3300 2350 3300
Wire Wire Line
	2850 3300 3350 3300
$Comp
L Connector_Generic:Conn_01x24 J2
U 1 1 6C8FCA64
P 1700 2100
F 0 "J2" H 1618 3417 50  0000 C CNN
F 1 "Conn_01x24" H 1618 3326 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x24_P2.54mm_Vertical" H 1700 2100 50  0001 C CNN
F 3 "~" H 1700 2100 50  0001 C CNN
	1    1700 2100
	-1   0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x24 J3
U 1 1 6CEA454C
P 3550 2100
F 0 "J3" H 3500 3400 50  0000 L CNN
F 1 "Conn_01x24" H 3400 3300 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x24_P2.54mm_Vertical" H 3550 2100 50  0001 C CNN
F 3 "~" H 3550 2100 50  0001 C CNN
	1    3550 2100
	1    0    0    -1  
$EndComp
Wire Bus Line
	3200 3800 6500 3800
Wire Bus Line
	2050 700  3200 700 
Wire Wire Line
	5150 4650 5600 4650
Wire Wire Line
	5150 4750 5600 4750
Wire Wire Line
	5150 4850 5600 4850
Wire Wire Line
	5150 4950 5600 4950
Wire Wire Line
	5150 5050 5400 5050
Wire Wire Line
	5150 4250 5600 4250
Wire Wire Line
	5150 4350 5600 4350
Wire Wire Line
	5150 4450 5600 4450
Wire Wire Line
	5150 4550 5600 4550
Wire Wire Line
	7100 4650 7550 4650
Wire Wire Line
	7100 4750 7550 4750
Wire Wire Line
	7100 4850 7550 4850
Wire Wire Line
	7100 4950 7550 4950
Wire Wire Line
	7100 5050 7350 5050
Wire Wire Line
	7100 4250 7550 4250
Wire Wire Line
	7100 4350 7550 4350
Wire Wire Line
	7100 4450 7550 4450
Wire Wire Line
	7100 4550 7550 4550
Wire Wire Line
	9050 4650 9500 4650
Wire Wire Line
	9050 4750 9500 4750
Wire Wire Line
	9050 4850 9500 4850
Wire Wire Line
	9050 4950 9500 4950
Wire Wire Line
	9050 5050 9300 5050
Wire Wire Line
	9050 4250 9500 4250
Wire Wire Line
	9050 4350 9500 4350
Wire Wire Line
	9050 4450 9500 4450
Wire Wire Line
	9050 4550 9500 4550
$Comp
L Connector_Generic:Conn_01x10 J11
U 1 1 6D07780E
P 8850 4650
F 0 "J11" H 8768 5267 50  0000 C CNN
F 1 "Conn_01x10" H 8768 5176 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x10_P2.54mm_Vertical" H 8850 4650 50  0001 C CNN
F 3 "~" H 8850 4650 50  0001 C CNN
	1    8850 4650
	-1   0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x10 J10
U 1 1 6D07B50A
P 6900 4650
F 0 "J10" H 6818 5267 50  0000 C CNN
F 1 "Conn_01x10" H 6818 5176 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x10_P2.54mm_Vertical" H 6900 4650 50  0001 C CNN
F 3 "~" H 6900 4650 50  0001 C CNN
	1    6900 4650
	-1   0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x10 J9
U 1 1 6D07C66E
P 4950 4650
F 0 "J9" H 4868 5267 50  0000 C CNN
F 1 "Conn_01x10" H 4868 5176 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x10_P2.54mm_Vertical" H 4950 4650 50  0001 C CNN
F 3 "~" H 4950 4650 50  0001 C CNN
	1    4950 4650
	-1   0    0    -1  
$EndComp
Wire Wire Line
	2850 1400 3350 1400
Wire Wire Line
	2850 1500 3350 1500
Wire Wire Line
	2850 1600 3350 1600
Wire Wire Line
	2850 1700 3350 1700
Wire Wire Line
	2850 1800 3350 1800
Wire Wire Line
	2850 1000 3350 1000
Wire Wire Line
	2850 1900 3350 1900
Wire Wire Line
	2850 2000 3350 2000
Wire Wire Line
	2850 2100 3350 2100
Wire Wire Line
	2850 2200 3350 2200
Wire Wire Line
	2850 2300 3350 2300
Wire Wire Line
	2850 2400 3350 2400
Wire Wire Line
	2850 2500 3350 2500
Wire Wire Line
	2850 2600 3350 2600
Wire Wire Line
	2850 2700 3350 2700
Wire Wire Line
	2850 2800 3350 2800
Wire Wire Line
	2850 1100 3350 1100
Wire Wire Line
	2850 2900 3350 2900
Wire Wire Line
	2850 3000 3350 3000
Wire Wire Line
	2850 3100 3350 3100
Wire Wire Line
	2850 3200 3350 3200
Wire Wire Line
	2850 1200 3350 1200
Wire Wire Line
	2850 1300 3350 1300
Wire Wire Line
	1900 1000 2350 1000
Wire Wire Line
	1900 1500 2350 1500
Wire Wire Line
	1900 1600 2350 1600
Wire Wire Line
	1900 1700 2350 1700
Wire Wire Line
	1900 1800 2350 1800
Wire Wire Line
	1900 1900 2350 1900
Wire Wire Line
	1900 2000 2350 2000
Wire Wire Line
	1900 2100 2350 2100
Wire Wire Line
	1900 2200 2350 2200
Wire Wire Line
	1900 2300 2350 2300
Wire Wire Line
	1900 2400 2350 2400
Wire Wire Line
	1900 1100 2350 1100
Wire Wire Line
	1900 2500 2350 2500
Wire Wire Line
	1900 2600 2350 2600
Wire Wire Line
	1900 2700 2350 2700
Wire Wire Line
	1900 2800 2350 2800
Wire Wire Line
	1900 2900 2350 2900
Wire Wire Line
	1900 3000 2350 3000
Wire Wire Line
	1900 3100 2350 3100
Wire Wire Line
	1900 3200 2350 3200
Wire Wire Line
	1900 1200 2350 1200
Wire Wire Line
	1900 1300 2350 1300
Wire Wire Line
	1900 1400 2350 1400
Wire Bus Line
	10400 3800 10400 5250
Wire Bus Line
	8450 3800 8450 5250
Wire Bus Line
	6500 3800 6500 5250
Wire Bus Line
	2050 700  2050 3200
Wire Bus Line
	3200 700  3200 3800
$Comp
L Connector_Generic:Conn_01x24 J6
U 1 1 6D191D20
P 7700 2100
F 0 "J6" H 7780 2092 50  0000 L CNN
F 1 "Conn_01x24" H 7780 2001 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x24_P2.54mm_Vertical" H 7700 2100 50  0001 C CNN
F 3 "~" H 7700 2100 50  0001 C CNN
	1    7700 2100
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x24 J5
U 1 1 6D192F43
P 6450 2100
F 0 "J5" H 6368 3417 50  0000 C CNN
F 1 "Conn_01x24" H 6368 3326 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x24_P2.54mm_Vertical" H 6450 2100 50  0001 C CNN
F 3 "~" H 6450 2100 50  0001 C CNN
	1    6450 2100
	-1   0    0    -1  
$EndComp
$EndSCHEMATC
