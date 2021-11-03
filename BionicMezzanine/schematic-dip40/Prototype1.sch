EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "BionicMezzanine DIP40"
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
Text Label 3850 4000 0    50   ~ 0
VCC
Text Label 3850 3400 0    50   ~ 0
P47
Text Label 3850 3300 0    50   ~ 0
P46
Text Label 3850 3200 0    50   ~ 0
P45
Text Label 3850 3100 0    50   ~ 0
P44
Text Label 3850 3000 0    50   ~ 0
P43
Text Label 3850 2900 0    50   ~ 0
P42
Text Label 3850 2800 0    50   ~ 0
P41
Text Label 3850 2700 0    50   ~ 0
P40
Text Label 3850 1700 0    50   ~ 0
GND
Text Label 3850 3700 0    50   ~ 0
P55
Text Label 3850 3600 0    50   ~ 0
P54
Text Label 3850 3800 0    50   ~ 0
P56
Text Label 3850 3900 0    50   ~ 0
P57
Entry Wire Line
	4100 3100 4000 3200
Entry Wire Line
	4100 3200 4000 3300
Entry Wire Line
	4100 3300 4000 3400
Entry Wire Line
	4100 2800 4000 2900
Entry Wire Line
	4100 2900 4000 3000
Entry Wire Line
	4100 3000 4000 3100
Entry Wire Line
	4100 2600 4000 2700
Entry Wire Line
	4100 2700 4000 2800
Entry Wire Line
	2000 3300 2100 3400
Entry Wire Line
	2000 3200 2100 3300
Entry Wire Line
	2000 3100 2100 3200
Entry Wire Line
	2000 3000 2100 3100
Entry Wire Line
	2000 2900 2100 3000
Entry Wire Line
	2000 2800 2100 2900
Entry Wire Line
	2000 2700 2100 2800
Entry Wire Line
	2000 2600 2100 2700
Entry Wire Line
	4100 3800 4000 3900
Entry Wire Line
	4100 3700 4000 3800
Entry Wire Line
	4100 3600 4000 3700
Entry Wire Line
	4100 3500 4000 3600
Entry Wire Line
	4100 3900 4000 4000
Entry Wire Line
	4100 1600 4000 1700
Text Label 2250 1700 2    50   ~ 0
GND
Text Label 2250 1800 2    50   ~ 0
P10
Text Label 2250 1900 2    50   ~ 0
P11
Text Label 2250 2000 2    50   ~ 0
P12
Text Label 2250 2100 2    50   ~ 0
P13
Text Label 2250 2200 2    50   ~ 0
P14
Text Label 2250 2300 2    50   ~ 0
P15
Text Label 2250 2400 2    50   ~ 0
P16
Text Label 2250 2500 2    50   ~ 0
P17
Text Label 2250 4000 2    50   ~ 0
VCC
Text Label 2250 3900 2    50   ~ 0
P53
Text Label 2250 3800 2    50   ~ 0
P52
Text Label 2250 3700 2    50   ~ 0
P51
Text Label 2250 3600 2    50   ~ 0
P50
Wire Wire Line
	2300 1800 2100 1800
Wire Wire Line
	2300 1900 2100 1900
Wire Wire Line
	2300 2000 2100 2000
Wire Wire Line
	2100 2100 2300 2100
Wire Wire Line
	2300 2200 2100 2200
Wire Wire Line
	2100 2300 2300 2300
Wire Wire Line
	2300 2400 2100 2400
Wire Wire Line
	2300 2500 2100 2500
Wire Wire Line
	2100 3900 2300 3900
Wire Wire Line
	2100 3800 2300 3800
Wire Wire Line
	2100 3700 2300 3700
Wire Wire Line
	2300 3600 2100 3600
Entry Wire Line
	2000 3800 2100 3900
Entry Wire Line
	2000 3700 2100 3800
Entry Wire Line
	2000 3600 2100 3700
Entry Wire Line
	2000 3500 2100 3600
Entry Wire Line
	2000 2400 2100 2500
Entry Wire Line
	2000 2300 2100 2400
Entry Wire Line
	2000 2200 2100 2300
Entry Wire Line
	2000 2100 2100 2200
Entry Wire Line
	2000 2000 2100 2100
Entry Wire Line
	2000 1900 2100 2000
Entry Wire Line
	2000 1800 2100 1900
Entry Wire Line
	2000 1700 2100 1800
Wire Wire Line
	2100 4000 2300 4000
Entry Wire Line
	2000 3900 2100 4000
Wire Wire Line
	2100 1700 2300 1700
Entry Wire Line
	2000 1600 2100 1700
$Comp
L 0-LocalLibrary:ZIF_Socket_40pin J4
U 1 1 61849ADD
P 7300 2600
F 0 "J4" H 7500 3600 50  0000 C CNN
F 1 "ZIF_Socket_40pin" H 7550 1450 50  0000 C CNN
F 2 "0-LocalLibrary:Aries_ZIF_socket_DIP-40-pin" H 7500 1450 50  0001 C CNN
F 3 "https://www.arieselec.com/wp-content/uploads/2020/02/10001-universal-dip-zif-test-socket.pdf" H 7300 2600 50  0001 C CNN
	1    7300 2600
	1    0    0    -1  
$EndComp
Wire Wire Line
	4000 4000 3800 4000
Wire Wire Line
	4000 3900 3800 3900
Wire Wire Line
	4000 3800 3800 3800
Wire Wire Line
	4000 3700 3800 3700
Wire Wire Line
	4000 3600 3800 3600
Wire Wire Line
	4000 1700 3800 1700
Wire Wire Line
	3800 3300 4000 3300
Wire Wire Line
	3800 3200 4000 3200
Wire Wire Line
	3800 3100 4000 3100
Wire Wire Line
	3800 3000 4000 3000
Wire Wire Line
	3800 2900 4000 2900
Wire Wire Line
	3800 2700 4000 2700
Text Label 2250 2600 2    50   ~ 0
E0
Wire Wire Line
	2300 2600 2100 2600
Entry Wire Line
	2000 2500 2100 2600
Text Label 2250 3500 2    50   ~ 0
E2
Wire Wire Line
	2300 3500 2100 3500
Entry Wire Line
	2000 3400 2100 3500
Text Label 3850 2600 0    50   ~ 0
E1
Wire Wire Line
	3800 2600 4000 2600
Entry Wire Line
	4100 2500 4000 2600
Text Label 3850 3500 0    50   ~ 0
E3
Entry Wire Line
	4100 3400 4000 3500
$Comp
L Connector_Generic:Conn_02x25_Odd_Even J2
U 1 1 618A1594
P 2500 2900
F 0 "J2" H 2550 4317 50  0000 C CNN
F 1 "Conn_02x25_Odd_Even" H 2550 1600 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_2x25_P2.54mm_Vertical" H 2500 2900 50  0001 C CNN
F 3 "~" H 2500 2900 50  0001 C CNN
	1    2500 2900
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x25_Odd_Even J3
U 1 1 618E6790
P 3500 2900
F 0 "J3" H 3550 4317 50  0000 C CNN
F 1 "Conn_02x25_Odd_Even" H 3550 1600 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_2x25_P2.54mm_Vertical" H 3500 2900 50  0001 C CNN
F 3 "~" H 3500 2900 50  0001 C CNN
	1    3500 2900
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x20_Odd_Even J5
U 1 1 619601EA
P 6800 2600
F 0 "J5" H 6850 3717 50  0000 C CNN
F 1 "Conn_02x20_Odd_Even" H 6850 3626 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_2x20_P2.54mm_Vertical" H 6800 2600 50  0001 C CNN
F 3 "~" H 6800 2600 50  0001 C CNN
	1    6800 2600
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x20_Odd_Even J6
U 1 1 61961D9B
P 8150 2600
F 0 "J6" H 8200 3717 50  0000 C CNN
F 1 "Conn_02x20_Odd_Even" H 8200 3626 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_2x20_P2.54mm_Vertical" H 8150 2600 50  0001 C CNN
F 3 "~" H 8150 2600 50  0001 C CNN
	1    8150 2600
	1    0    0    -1  
$EndComp
$Comp
L 0-LocalLibrary:BionicConnector J1
U 1 1 62848BAE
P 3000 2800
F 0 "J1" H 3050 4117 50  0000 C CNN
F 1 "BionicConnector" H 3050 4026 50  0000 C CNN
F 2 "0-LocalLibrary:DIP-48_W10.16mm" H 3050 1500 50  0001 C CNN
F 3 "https://www.arieselec.com/wp-content/uploads/2020/02/10001-universal-dip-zif-test-socket.pdf" H 3000 2800 50  0001 C CNN
	1    3000 2800
	1    0    0    -1  
$EndComp
Wire Bus Line
	4100 1400 2000 1400
Wire Wire Line
	6600 1700 7100 1700
Connection ~ 7100 1700
Wire Wire Line
	7100 1800 6600 1800
Connection ~ 7100 1800
Wire Wire Line
	6600 1900 7100 1900
Connection ~ 7100 1900
Wire Wire Line
	7100 2000 6600 2000
Connection ~ 7100 2000
Wire Wire Line
	6600 2100 7100 2100
Connection ~ 7100 2100
Wire Wire Line
	7100 2200 6600 2200
Connection ~ 7100 2200
Wire Wire Line
	6600 2300 7100 2300
Connection ~ 7100 2300
Wire Wire Line
	7100 2400 6600 2400
Connection ~ 7100 2400
Wire Wire Line
	6600 2500 7100 2500
Connection ~ 7100 2500
Wire Wire Line
	7100 2600 6600 2600
Connection ~ 7100 2600
Wire Wire Line
	6600 2700 7100 2700
Connection ~ 7100 2700
Wire Wire Line
	7100 2800 6600 2800
Connection ~ 7100 2800
Wire Wire Line
	6600 2900 7100 2900
Connection ~ 7100 2900
Wire Wire Line
	7100 3000 6600 3000
Connection ~ 7100 3000
Wire Wire Line
	6600 3100 7100 3100
Connection ~ 7100 3100
Wire Wire Line
	7100 3200 6600 3200
Connection ~ 7100 3200
Wire Wire Line
	6600 3300 7100 3300
Connection ~ 7100 3300
Wire Wire Line
	7100 3400 6600 3400
Connection ~ 7100 3400
Wire Wire Line
	6600 3500 7100 3500
Connection ~ 7100 3500
Wire Wire Line
	7100 3600 6600 3600
Connection ~ 7100 3600
Wire Wire Line
	8450 1700 7950 1700
Connection ~ 7950 1700
Wire Wire Line
	7950 1800 8450 1800
Connection ~ 7950 1800
Wire Wire Line
	8450 1900 7950 1900
Connection ~ 7950 1900
Wire Wire Line
	7950 2000 8450 2000
Connection ~ 7950 2000
Wire Wire Line
	8450 2100 7950 2100
Connection ~ 7950 2100
Wire Wire Line
	7950 2200 8450 2200
Connection ~ 7950 2200
Wire Wire Line
	8450 2300 7950 2300
Connection ~ 7950 2300
Wire Wire Line
	7950 2400 8450 2400
Connection ~ 7950 2400
Wire Wire Line
	8450 2500 7950 2500
Connection ~ 7950 2500
Wire Wire Line
	7950 2600 8450 2600
Connection ~ 7950 2600
Wire Wire Line
	8450 2700 7950 2700
Connection ~ 7950 2700
Wire Wire Line
	7950 2800 8450 2800
Connection ~ 7950 2800
Wire Wire Line
	8450 2900 7950 2900
Connection ~ 7950 2900
Wire Wire Line
	7950 3000 8450 3000
Connection ~ 7950 3000
Wire Wire Line
	8450 3100 7950 3100
Connection ~ 7950 3100
Wire Wire Line
	7950 3200 8450 3200
Connection ~ 7950 3200
Wire Wire Line
	8450 3300 7950 3300
Connection ~ 7950 3300
Wire Wire Line
	7950 3400 8450 3400
Connection ~ 7950 3400
Wire Wire Line
	8450 3500 7950 3500
Connection ~ 7950 3500
Wire Wire Line
	7950 3600 8450 3600
Connection ~ 7950 3600
Wire Wire Line
	3800 1700 3300 1700
Connection ~ 3800 1700
Connection ~ 3300 1700
Wire Wire Line
	3300 1800 3800 1800
Connection ~ 3300 1800
Wire Wire Line
	3800 1900 3300 1900
Connection ~ 3300 1900
Wire Wire Line
	3300 2000 3800 2000
Connection ~ 3300 2000
Wire Wire Line
	3800 2100 3300 2100
Connection ~ 3300 2100
Wire Wire Line
	3300 2200 3800 2200
Connection ~ 3300 2200
Wire Wire Line
	3800 2300 3300 2300
Connection ~ 3300 2300
Wire Wire Line
	3300 2400 3800 2400
Connection ~ 3300 2400
Wire Wire Line
	3800 2500 3300 2500
Connection ~ 3300 2500
Wire Wire Line
	3300 2600 3800 2600
Connection ~ 3300 2600
Connection ~ 3800 2600
Wire Wire Line
	3800 2700 3300 2700
Connection ~ 3800 2700
Connection ~ 3300 2700
Wire Wire Line
	3300 2800 3800 2800
Connection ~ 3300 2800
Connection ~ 3800 2800
Wire Wire Line
	3800 2800 4000 2800
Wire Wire Line
	3800 2900 3300 2900
Connection ~ 3800 2900
Connection ~ 3300 2900
Wire Wire Line
	3300 3000 3800 3000
Connection ~ 3300 3000
Connection ~ 3800 3000
Wire Wire Line
	3800 3100 3300 3100
Connection ~ 3800 3100
Connection ~ 3300 3100
Wire Wire Line
	3300 3200 3800 3200
Connection ~ 3300 3200
Connection ~ 3800 3200
Wire Wire Line
	3800 3300 3300 3300
Connection ~ 3800 3300
Connection ~ 3300 3300
Wire Wire Line
	3300 3400 3800 3400
Connection ~ 3300 3400
Connection ~ 3800 3400
Wire Wire Line
	3800 3400 4000 3400
Wire Wire Line
	3300 3500 3800 3500
Connection ~ 3300 3500
Connection ~ 3800 3500
Wire Wire Line
	3800 3500 4000 3500
Wire Wire Line
	3800 3600 3300 3600
Connection ~ 3800 3600
Connection ~ 3300 3600
Wire Wire Line
	3300 3700 3800 3700
Connection ~ 3300 3700
Connection ~ 3800 3700
Wire Wire Line
	3800 3800 3300 3800
Connection ~ 3800 3800
Connection ~ 3300 3800
Wire Wire Line
	3300 3900 3800 3900
Connection ~ 3300 3900
Connection ~ 3800 3900
Wire Wire Line
	3800 4000 3300 4000
Connection ~ 3800 4000
Connection ~ 3300 4000
Wire Wire Line
	3800 4100 3300 4100
Connection ~ 3300 4100
Wire Wire Line
	2300 4100 2800 4100
Connection ~ 2800 4100
Wire Wire Line
	2800 4100 3300 4100
Wire Wire Line
	2800 1700 2300 1700
Connection ~ 2800 1700
Connection ~ 2300 1700
Wire Wire Line
	2300 1800 2800 1800
Connection ~ 2300 1800
Connection ~ 2800 1800
Wire Wire Line
	2800 1900 2300 1900
Connection ~ 2800 1900
Connection ~ 2300 1900
Wire Wire Line
	2300 2000 2800 2000
Connection ~ 2300 2000
Connection ~ 2800 2000
Wire Wire Line
	2800 2100 2300 2100
Connection ~ 2800 2100
Connection ~ 2300 2100
Wire Wire Line
	2300 2200 2800 2200
Connection ~ 2300 2200
Connection ~ 2800 2200
Wire Wire Line
	2800 2300 2300 2300
Connection ~ 2800 2300
Connection ~ 2300 2300
Wire Wire Line
	2300 2400 2800 2400
Connection ~ 2300 2400
Connection ~ 2800 2400
Wire Wire Line
	2800 2500 2300 2500
Connection ~ 2800 2500
Connection ~ 2300 2500
Wire Wire Line
	2300 2600 2800 2600
Connection ~ 2300 2600
Connection ~ 2800 2600
Wire Wire Line
	2800 2700 2300 2700
Connection ~ 2800 2700
Wire Wire Line
	2300 2800 2800 2800
Connection ~ 2800 2800
Wire Wire Line
	2800 2900 2300 2900
Connection ~ 2800 2900
Wire Wire Line
	2300 3000 2800 3000
Connection ~ 2800 3000
Wire Wire Line
	2800 3100 2300 3100
Connection ~ 2800 3100
Wire Wire Line
	2300 3200 2800 3200
Connection ~ 2800 3200
Wire Wire Line
	2800 3300 2300 3300
Connection ~ 2800 3300
Wire Wire Line
	2300 3400 2800 3400
Connection ~ 2800 3400
Wire Wire Line
	2800 3500 2300 3500
Connection ~ 2800 3500
Connection ~ 2300 3500
Wire Wire Line
	2300 3600 2800 3600
Connection ~ 2300 3600
Connection ~ 2800 3600
Wire Wire Line
	2800 3700 2300 3700
Connection ~ 2800 3700
Connection ~ 2300 3700
Wire Wire Line
	2300 3800 2800 3800
Connection ~ 2300 3800
Connection ~ 2800 3800
Wire Wire Line
	2800 3900 2300 3900
Connection ~ 2800 3900
Connection ~ 2300 3900
Wire Wire Line
	2300 4000 2800 4000
Connection ~ 2300 4000
Connection ~ 2800 4000
Wire Wire Line
	2300 2700 2100 2700
Wire Wire Line
	2300 2800 2100 2800
Wire Wire Line
	2300 2900 2100 2900
Wire Wire Line
	2100 3000 2300 3000
Wire Wire Line
	2300 3100 2100 3100
Wire Wire Line
	2100 3200 2300 3200
Wire Wire Line
	2300 3300 2100 3300
Wire Wire Line
	2300 3400 2100 3400
Text Label 2250 3100 2    50   ~ 0
P34
Text Label 2250 3200 2    50   ~ 0
P35
Text Label 2250 3300 2    50   ~ 0
P36
Text Label 2250 3400 2    50   ~ 0
P37
Text Label 2250 3000 2    50   ~ 0
P33
Text Label 2250 2900 2    50   ~ 0
P32
Text Label 2250 2800 2    50   ~ 0
P31
Text Label 2250 2700 2    50   ~ 0
P30
Connection ~ 2300 2700
Connection ~ 2300 2800
Connection ~ 2300 2900
Connection ~ 2300 3000
Connection ~ 2300 3100
Connection ~ 2300 3200
Connection ~ 2300 3300
Connection ~ 2300 3400
Wire Wire Line
	3800 2500 4000 2500
Wire Wire Line
	3800 2400 4000 2400
Wire Wire Line
	3800 2300 4000 2300
Wire Wire Line
	3800 2200 4000 2200
Wire Wire Line
	3800 2100 4000 2100
Wire Wire Line
	3800 2000 4000 2000
Wire Wire Line
	3800 1900 4000 1900
Wire Wire Line
	3800 1800 4000 1800
Entry Wire Line
	4100 2200 4000 2300
Entry Wire Line
	4100 2300 4000 2400
Entry Wire Line
	4100 2400 4000 2500
Entry Wire Line
	4100 1900 4000 2000
Entry Wire Line
	4100 2000 4000 2100
Entry Wire Line
	4100 2100 4000 2200
Entry Wire Line
	4100 1700 4000 1800
Entry Wire Line
	4100 1800 4000 1900
Text Label 3850 2500 0    50   ~ 0
P27
Text Label 3850 2400 0    50   ~ 0
P26
Text Label 3850 2300 0    50   ~ 0
P25
Text Label 3850 2200 0    50   ~ 0
P24
Text Label 3850 2100 0    50   ~ 0
P23
Text Label 3850 2000 0    50   ~ 0
P22
Text Label 3850 1900 0    50   ~ 0
P21
Text Label 3850 1800 0    50   ~ 0
P20
Connection ~ 3800 1800
Connection ~ 3800 1900
Connection ~ 3800 2000
Connection ~ 3800 2100
Connection ~ 3800 2200
Connection ~ 3800 2300
Connection ~ 3800 2400
Connection ~ 3800 2500
$Comp
L 0-LocalLibrary:74HCT541 U1
U 1 1 6C48517D
P 5400 5150
F 0 "U1" H 5400 5917 50  0000 C CNN
F 1 "74HCT541" H 5400 5826 50  0000 C CNN
F 2 "Package_DIP:DIP-20_W7.62mm" H 5400 5150 50  0001 C CNN
F 3 "http://www.ti.com/lit/gpn/sn74HCT541" H 5400 5150 50  0001 C CNN
	1    5400 5150
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C1
U 1 1 6C491401
P 5750 5650
F 0 "C1" H 5842 5696 50  0000 L CNN
F 1 "0.1uF" H 5842 5605 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D3.0mm_W1.6mm_P2.50mm" H 5750 5650 50  0001 C CNN
F 3 "~" H 5750 5650 50  0001 C CNN
	1    5750 5650
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR04
U 1 1 6C492B3A
P 5750 5750
F 0 "#PWR04" H 5750 5500 50  0001 C CNN
F 1 "GND" H 5900 5650 50  0000 C CNN
F 2 "" H 5750 5750 50  0001 C CNN
F 3 "" H 5750 5750 50  0001 C CNN
	1    5750 5750
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR03
U 1 1 6C493939
P 5750 5550
F 0 "#PWR03" H 5750 5400 50  0001 C CNN
F 1 "VCC" H 5900 5650 50  0000 C CNN
F 2 "" H 5750 5550 50  0001 C CNN
F 3 "" H 5750 5550 50  0001 C CNN
	1    5750 5550
	1    0    0    -1  
$EndComp
Wire Wire Line
	5700 5550 5750 5550
Connection ~ 5750 5550
Wire Wire Line
	5750 5750 5700 5750
Connection ~ 5750 5750
$Comp
L Connector_Generic:Conn_02x10_Odd_Even J7
U 1 1 6C4C6A75
P 4500 4950
F 0 "J7" H 4550 5567 50  0000 C CNN
F 1 "Conn_02x10_Odd_Even" H 4550 5650 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_2x10_P2.54mm_Vertical" H 4500 4950 50  0001 C CNN
F 3 "~" H 4500 4950 50  0001 C CNN
	1    4500 4950
	1    0    0    -1  
$EndComp
Connection ~ 4800 4550
Wire Wire Line
	4800 4550 4300 4550
Wire Wire Line
	5100 4650 4800 4650
Connection ~ 4800 4650
Wire Wire Line
	4800 4650 4300 4650
Wire Wire Line
	5100 4750 4800 4750
Connection ~ 4800 4750
Wire Wire Line
	4800 4750 4300 4750
Wire Wire Line
	5100 4850 4800 4850
Connection ~ 4800 4850
Wire Wire Line
	4800 4850 4300 4850
Wire Wire Line
	5100 4950 4800 4950
Connection ~ 4800 4950
Wire Wire Line
	4800 4950 4300 4950
Wire Wire Line
	4300 5050 4800 5050
Connection ~ 4800 5050
Wire Wire Line
	4800 5050 5100 5050
Wire Wire Line
	4300 5150 4800 5150
Connection ~ 4800 5150
Wire Wire Line
	4800 5150 5100 5150
Wire Wire Line
	4300 5250 4800 5250
Connection ~ 4800 5250
Wire Wire Line
	4800 5250 5100 5250
Wire Wire Line
	4300 5350 4800 5350
Connection ~ 4800 5350
Wire Wire Line
	4800 5350 5100 5350
$Comp
L power:GND #PWR02
U 1 1 6C56D374
P 4900 5750
F 0 "#PWR02" H 4900 5500 50  0001 C CNN
F 1 "GND" H 4905 5577 50  0000 C CNN
F 2 "" H 4900 5750 50  0001 C CNN
F 3 "" H 4900 5750 50  0001 C CNN
	1    4900 5750
	1    0    0    -1  
$EndComp
Wire Wire Line
	4800 5450 4300 5450
Connection ~ 4800 5450
$Comp
L Connector_Generic:Conn_02x10_Odd_Even J8
U 1 1 6C58B970
P 6300 4950
F 0 "J8" H 6350 5567 50  0000 C CNN
F 1 "Conn_02x10_Odd_Even" H 6350 5650 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_2x10_P2.54mm_Vertical" H 6300 4950 50  0001 C CNN
F 3 "~" H 6300 4950 50  0001 C CNN
	1    6300 4950
	1    0    0    -1  
$EndComp
Wire Wire Line
	4900 4550 4800 4550
Wire Wire Line
	6100 4550 6600 4550
Wire Wire Line
	5700 4650 6100 4650
Connection ~ 6100 4650
Wire Wire Line
	6100 4650 6600 4650
Wire Wire Line
	6600 4750 6100 4750
Connection ~ 6100 4750
Wire Wire Line
	6100 4750 5700 4750
Wire Wire Line
	5700 4850 6100 4850
Connection ~ 6100 4850
Wire Wire Line
	6100 4850 6600 4850
Wire Wire Line
	6600 4950 6100 4950
Connection ~ 6100 4950
Wire Wire Line
	6100 4950 5700 4950
Wire Wire Line
	5700 5050 6100 5050
Connection ~ 6100 5050
Wire Wire Line
	6100 5050 6600 5050
Wire Wire Line
	6600 5150 6100 5150
Connection ~ 6100 5150
Wire Wire Line
	6100 5150 5700 5150
Wire Wire Line
	5700 5250 6100 5250
Connection ~ 6100 5250
Wire Wire Line
	6100 5250 6600 5250
Wire Wire Line
	6600 5350 6100 5350
Connection ~ 6100 5350
Wire Wire Line
	6100 5350 5700 5350
Wire Wire Line
	6100 5450 6600 5450
$Comp
L power:GND #PWR05
U 1 1 6C6B7F95
P 6600 5750
F 0 "#PWR05" H 6600 5500 50  0001 C CNN
F 1 "GND" H 6605 5577 50  0000 C CNN
F 2 "" H 6600 5750 50  0001 C CNN
F 3 "" H 6600 5750 50  0001 C CNN
	1    6600 5750
	1    0    0    -1  
$EndComp
Wire Wire Line
	6600 5450 6600 5750
Wire Wire Line
	4900 5750 5100 5750
Wire Wire Line
	4900 5650 5100 5650
Connection ~ 4900 5650
Wire Wire Line
	4900 5650 4900 5750
Wire Wire Line
	4800 5450 4900 5450
Wire Wire Line
	4900 5450 4900 5650
Connection ~ 4900 5750
Wire Bus Line
	4100 1400 4100 3900
Wire Bus Line
	2000 1400 2000 3900
$Comp
L power:VCC #PWR0101
U 1 1 6C85FE03
P 4900 4550
F 0 "#PWR0101" H 4900 4400 50  0001 C CNN
F 1 "VCC" H 4917 4723 50  0000 C CNN
F 2 "" H 4900 4550 50  0001 C CNN
F 3 "" H 4900 4550 50  0001 C CNN
	1    4900 4550
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR0102
U 1 1 6C860B29
P 6600 4550
F 0 "#PWR0102" H 6600 4400 50  0001 C CNN
F 1 "VCC" H 6617 4723 50  0000 C CNN
F 2 "" H 6600 4550 50  0001 C CNN
F 3 "" H 6600 4550 50  0001 C CNN
	1    6600 4550
	1    0    0    -1  
$EndComp
Connection ~ 6600 4550
$EndSCHEMATC
