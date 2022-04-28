EESchema Schematic File Version 4
LIBS:push-push-air-cache
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Push Push Air 2"
Date "2022-04-28"
Rev "ver 2.o"
Comp "UBI Stage"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L ESP32-DEVKITC-32D:ESP32-DEVKITC-32D U1
U 1 1 623A0D71
P 5550 3850
F 0 "U1" H 5550 5017 50  0000 C CNN
F 1 "WEMOS ESP32 Board" H 5550 4926 50  0000 C CNN
F 2 "MODULE_ESP32-DEVKITC-32D" H 5550 3850 50  0001 L BNN
F 3 "" H 5550 3850 50  0001 L BNN
F 4 "Espressif Systems" H 5550 3850 50  0001 L BNN "MANUFACTURER"
F 5 "4" H 5550 3850 50  0001 L BNN "PARTREV"
	1    5550 3850
	1    0    0    -1  
$EndComp
$Comp
L Device:R R2
U 1 1 623A29FF
P 4250 3650
F 0 "R2" V 4043 3650 50  0000 C CNN
F 1 "47K" V 4134 3650 50  0000 C CNN
F 2 "" V 4180 3650 50  0001 C CNN
F 3 "~" H 4250 3650 50  0001 C CNN
	1    4250 3650
	0    1    1    0   
$EndComp
$Comp
L Device:R R1
U 1 1 623A32C7
P 4250 3350
F 0 "R1" V 4043 3350 50  0000 C CNN
F 1 "47K" V 4134 3350 50  0000 C CNN
F 2 "" V 4180 3350 50  0001 C CNN
F 3 "~" H 4250 3350 50  0001 C CNN
	1    4250 3350
	0    1    1    0   
$EndComp
$Comp
L Switch:SW_Push Ped_2
U 1 1 623A22F1
P 7450 3650
F 0 "Ped_2" V 7400 3900 50  0000 C CNN
F 1 "SW_Push" V 7500 3900 50  0000 C CNN
F 2 "" H 7450 3850 50  0001 C CNN
F 3 "~" H 7450 3850 50  0001 C CNN
	1    7450 3650
	0    1    1    0   
$EndComp
$Comp
L Switch:SW_Push Ped_1
U 1 1 623A23D3
P 7900 3250
F 0 "Ped_1" V 7854 3398 50  0000 L CNN
F 1 "SW_Push" V 7945 3398 50  0000 L CNN
F 2 "" H 7900 3450 50  0001 C CNN
F 3 "~" H 7900 3450 50  0001 C CNN
	1    7900 3250
	0    1    1    0   
$EndComp
$Comp
L Device:Battery_Cell BT1
U 1 1 623A8D9A
P 3850 3550
F 0 "BT1" H 3550 3650 50  0000 L CNN
F 1 "Battery 18650" H 3250 3550 50  0000 L CNN
F 2 "" V 3850 3610 50  0001 C CNN
F 3 "~" V 3850 3610 50  0001 C CNN
	1    3850 3550
	1    0    0    -1  
$EndComp
Wire Wire Line
	4750 3350 4400 3350
Wire Wire Line
	4400 3350 4400 3650
Connection ~ 4400 3350
Wire Wire Line
	4100 3650 3850 3650
Wire Wire Line
	4100 3350 3850 3350
Wire Wire Line
	7450 3850 7650 3850
$Comp
L power:GND #PWR?
U 1 1 623AE0F7
P 7650 3850
F 0 "#PWR?" H 7650 3600 50  0001 C CNN
F 1 "GND" H 7655 3677 50  0000 C CNN
F 2 "" H 7650 3850 50  0001 C CNN
F 3 "" H 7650 3850 50  0001 C CNN
	1    7650 3850
	1    0    0    -1  
$EndComp
Connection ~ 7650 3850
Wire Wire Line
	7650 3850 7900 3850
$Comp
L Device:LED D1
U 1 1 626A715F
P 6500 4050
F 0 "D1" H 6500 3800 50  0000 C CNN
F 1 "LED_BUILTIN" H 6350 3900 50  0000 C CNN
F 2 "" H 6500 4050 50  0001 C CNN
F 3 "~" H 6500 4050 50  0001 C CNN
	1    6500 4050
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR?
U 1 1 626A7B0B
P 6650 4050
F 0 "#PWR?" H 6650 3800 50  0001 C CNN
F 1 "GND" H 6655 3877 50  0000 C CNN
F 2 "" H 6650 4050 50  0001 C CNN
F 3 "" H 6650 4050 50  0001 C CNN
	1    6650 4050
	1    0    0    -1  
$EndComp
Wire Notes Line
	4600 2950 4600 3850
Wire Notes Line
	4600 3850 3050 3850
Wire Notes Line
	3050 3850 3050 2950
Wire Notes Line
	3050 2950 4600 2950
Text Notes 3150 3050 0    50   ~ 0
Battery charge monitor
Wire Notes Line
	2850 2600 6950 2600
Wire Notes Line
	6950 2600 6950 4950
Wire Notes Line
	6950 4950 2850 4950
Wire Notes Line
	2850 4950 2850 2600
Text Notes 2900 2550 0    50   ~ 0
WEMOS ESP32 Board with 18650 Battery Holder
Wire Wire Line
	6350 3050 7900 3050
Wire Wire Line
	7900 3450 7900 3850
Wire Wire Line
	6350 3450 7450 3450
$EndSCHEMATC
