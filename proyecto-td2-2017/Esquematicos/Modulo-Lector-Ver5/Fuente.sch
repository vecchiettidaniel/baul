EESchema Schematic File Version 2
LIBS:power
LIBS:device
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
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:switches
LIBS:74xgxx
LIBS:ac-dc
LIBS:actel
LIBS:allegro
LIBS:Altera
LIBS:analog_devices
LIBS:battery_management
LIBS:bbd
LIBS:brooktre
LIBS:cmos_ieee
LIBS:dc-dc
LIBS:diode
LIBS:elec-unifil
LIBS:ESD_Protection
LIBS:ftdi
LIBS:gennum
LIBS:graphic
LIBS:hc11
LIBS:ir
LIBS:Lattice
LIBS:logo
LIBS:maxim
LIBS:mechanical
LIBS:microchip_dspic33dsc
LIBS:microchip_pic10mcu
LIBS:microchip_pic12mcu
LIBS:microchip_pic16mcu
LIBS:microchip_pic18mcu
LIBS:microchip_pic32mcu
LIBS:motor_drivers
LIBS:motors
LIBS:msp430
LIBS:nordicsemi
LIBS:nxp_armmcu
LIBS:onsemi
LIBS:Oscillators
LIBS:powerint
LIBS:Power_Management
LIBS:pspice
LIBS:references
LIBS:relays
LIBS:rfcom
LIBS:sensors
LIBS:silabs
LIBS:stm8
LIBS:stm32
LIBS:supertex
LIBS:transf
LIBS:ttl_ieee
LIBS:video
LIBS:wiznet
LIBS:Worldsemi
LIBS:Xicor
LIBS:zetex
LIBS:Zilog
LIBS:HechoEnCasa
LIBS:Modulo-Lector-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 3
Title "Fuente Alimentacion (+5v),(+3v3) (+Reg)"
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L CP1 C201
U 1 1 5880C73D
P 3650 3550
F 0 "C201" H 3675 3650 50  0000 L CNN
F 1 "ADJ_100uF(25v)" H 3400 3400 50  0000 L CNN
F 2 "Capacitors_ThroughHole:C_Radial_D6.3_L11.2_P2.5" H 3650 3550 50  0001 C CNN
F 3 "" H 3650 3550 50  0000 C CNN
	1    3650 3550
	1    0    0    -1  
$EndComp
$Comp
L D_Schottky D203
U 1 1 5880C73E
P 5300 3650
F 0 "D203" H 5300 3750 50  0000 C CNN
F 1 "ADJ_1N5819" H 5300 3550 50  0000 C CNN
F 2 "Discret:D3" H 5300 3650 50  0001 C CNN
F 3 "" H 5300 3650 50  0000 C CNN
	1    5300 3650
	0    1    1    0   
$EndComp
$Comp
L INDUCTOR L202
U 1 1 5880C73F
P 5800 3400
F 0 "L202" V 5750 3400 50  0000 C CNN
F 1 "ADJ_330uH" V 5900 3400 50  0000 C CNN
F 2 "Inductors_NEOSID:Neosid_Inductor_SM-NE127" H 5800 3400 50  0001 C CNN
F 3 "" H 5800 3400 50  0000 C CNN
	1    5800 3400
	1    0    0    -1  
$EndComp
$Comp
L CP1 C203
U 1 1 5880C740
P 6200 3650
F 0 "C203" H 6225 3750 50  0000 L CNN
F 1 "ADJ_330uF(25v)" H 5950 3550 50  0000 L CNN
F 2 "Capacitors_ThroughHole:C_Radial_D8_L11.5_P3.5" H 6200 3650 50  0001 C CNN
F 3 "" H 6200 3650 50  0000 C CNN
	1    6200 3650
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR018
U 1 1 5880C742
P 8450 5350
F 0 "#PWR018" H 8450 5100 50  0001 C CNN
F 1 "GND" H 8450 5200 50  0000 C CNN
F 2 "" H 8450 5350 50  0000 C CNN
F 3 "" H 8450 5350 50  0000 C CNN
	1    8450 5350
	1    0    0    -1  
$EndComp
Text Label 2650 3250 0    60   ~ 0
VIN_7/40V
$Comp
L CONN_01X02 P201
U 1 1 5880C743
P 2750 1700
F 0 "P201" H 2750 1850 50  0000 C CNN
F 1 "POWER_IN_7-40[v]" V 2850 1700 50  0000 C CNN
F 2 "Connect:bornier2" H 2750 1700 50  0001 C CNN
F 3 "" H 2750 1700 50  0000 C CNN
	1    2750 1700
	-1   0    0    1   
$EndComp
Text HLabel 7750 3150 2    60   Input ~ 0
VCC_Reg
Text HLabel 8550 5250 2    60   Input ~ 0
GND
$Comp
L CP1 C202
U 1 1 5880C744
P 3650 5300
F 0 "C202" H 3675 5400 50  0000 L CNN
F 1 "5v_100uF(25v)" H 3300 5150 50  0000 L CNN
F 2 "Capacitors_ThroughHole:C_Radial_D6.3_L11.2_P2.5" H 3650 5300 50  0001 C CNN
F 3 "" H 3650 5300 50  0000 C CNN
	1    3650 5300
	1    0    0    -1  
$EndComp
$Comp
L D_Schottky D204
U 1 1 5880C745
P 5300 5400
F 0 "D204" H 5300 5500 50  0000 C CNN
F 1 "5v_1N5819" H 5300 5300 50  0000 C CNN
F 2 "Discret:D3" H 5300 5400 50  0001 C CNN
F 3 "" H 5300 5400 50  0000 C CNN
	1    5300 5400
	0    1    1    0   
$EndComp
$Comp
L INDUCTOR L201
U 1 1 5880C746
P 5750 5150
F 0 "L201" V 5700 5150 50  0000 C CNN
F 1 "5v_330uH" V 5850 5150 50  0000 C CNN
F 2 "Inductors_NEOSID:Neosid_Inductor_SM-NE127" H 5750 5150 50  0001 C CNN
F 3 "" H 5750 5150 50  0000 C CNN
	1    5750 5150
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR019
U 1 1 5880C747
P 4050 5650
F 0 "#PWR019" H 4050 5400 50  0001 C CNN
F 1 "GND" H 4050 5500 50  0000 C CNN
F 2 "" H 4050 5650 50  0000 C CNN
F 3 "" H 4050 5650 50  0000 C CNN
	1    4050 5650
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR020
U 1 1 5880C748
P 3650 5650
F 0 "#PWR020" H 3650 5400 50  0001 C CNN
F 1 "GND" H 3650 5500 50  0000 C CNN
F 2 "" H 3650 5650 50  0000 C CNN
F 3 "" H 3650 5650 50  0000 C CNN
	1    3650 5650
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR021
U 1 1 5880C749
P 5300 5650
F 0 "#PWR021" H 5300 5400 50  0001 C CNN
F 1 "GND" H 5300 5500 50  0000 C CNN
F 2 "" H 5300 5650 50  0000 C CNN
F 3 "" H 5300 5650 50  0000 C CNN
	1    5300 5650
	1    0    0    -1  
$EndComp
$Comp
L CP1 C204
U 1 1 5880C74A
P 6200 5400
F 0 "C204" H 6225 5500 50  0000 L CNN
F 1 "5v_330uF" H 6050 5250 50  0000 L CNN
F 2 "Capacitors_ThroughHole:C_Radial_D8_L13_P3.8" H 6200 5400 50  0001 C CNN
F 3 "" H 6200 5400 50  0000 C CNN
	1    6200 5400
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR022
U 1 1 5880C74B
P 6200 5650
F 0 "#PWR022" H 6200 5400 50  0001 C CNN
F 1 "GND" H 6200 5500 50  0000 C CNN
F 2 "" H 6200 5650 50  0000 C CNN
F 3 "" H 6200 5650 50  0000 C CNN
	1    6200 5650
	1    0    0    -1  
$EndComp
Text HLabel 7700 5000 2    60   Input ~ 0
VCC_5v
$Comp
L GND #PWR023
U 1 1 5880C74D
P 7700 5250
F 0 "#PWR023" H 7700 5000 50  0001 C CNN
F 1 "GND" H 7700 5100 50  0000 C CNN
F 2 "" H 7700 5250 50  0000 C CNN
F 3 "" H 7700 5250 50  0000 C CNN
	1    7700 5250
	1    0    0    -1  
$EndComp
$Comp
L CP1 C205
U 1 1 5880C74E
P 6550 5450
F 0 "C205" H 6575 5550 50  0000 L CNN
F 1 "3v3_100uF" H 6500 5350 50  0000 L CNN
F 2 "Capacitors_ThroughHole:C_Radial_D5_L11_P2" H 6550 5450 50  0001 C CNN
F 3 "" H 6550 5450 50  0000 C CNN
	1    6550 5450
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR024
U 1 1 5880C74F
P 6550 5600
F 0 "#PWR024" H 6550 5350 50  0001 C CNN
F 1 "GND" H 6550 5450 50  0000 C CNN
F 2 "" H 6550 5600 50  0000 C CNN
F 3 "" H 6550 5600 50  0000 C CNN
	1    6550 5600
	1    0    0    -1  
$EndComp
Text HLabel 7700 5700 2    60   Input ~ 0
VCC_3v3
$Comp
L LM2575-x U201
U 1 1 5880C755
P 4650 3600
F 0 "U201" H 4620 4050 50  0000 C CNN
F 1 "LM2575-ADJ" H 4750 3650 50  0000 C CNN
F 2 "mis-footprint:TO-220-x5-(3_2)" H 3750 3750 50  0001 C CNN
F 3 "" H 3750 3750 50  0000 C CNN
	1    4650 3600
	1    0    0    -1  
$EndComp
$Comp
L LM2575-x U202
U 1 1 5880C757
P 4650 5350
F 0 "U202" H 4620 5800 50  0000 C CNN
F 1 "LM2575-5" H 4800 5400 50  0000 C CNN
F 2 "mis-footprint:TO-220-x5-(2_3)" H 3750 5500 50  0001 C CNN
F 3 "" H 3750 5500 50  0000 C CNN
	1    4650 5350
	1    0    0    -1  
$EndComp
$Comp
L PWR_FLAG #FLG025
U 1 1 5880C758
P 3650 3000
F 0 "#FLG025" H 3650 3095 50  0001 C CNN
F 1 "PWR_FLAG" H 3650 3180 50  0000 C CNN
F 2 "" H 3650 3000 50  0000 C CNN
F 3 "" H 3650 3000 50  0000 C CNN
	1    3650 3000
	1    0    0    -1  
$EndComp
$Comp
L PWR_FLAG #FLG026
U 1 1 5880C759
P 7150 3000
F 0 "#FLG026" H 7150 3095 50  0001 C CNN
F 1 "PWR_FLAG" H 7150 3180 50  0000 C CNN
F 2 "" H 7150 3000 50  0000 C CNN
F 3 "" H 7150 3000 50  0000 C CNN
	1    7150 3000
	1    0    0    -1  
$EndComp
$Comp
L R R203
U 1 1 5880C75A
P 6950 3550
F 0 "R203" V 6950 3500 50  0000 C CNN
F 1 "ADJ-R1(1k)" V 7100 3450 50  0000 C CNN
F 2 "Discret:R3" V 6880 3550 50  0001 C CNN
F 3 "" H 6950 3550 50  0000 C CNN
	1    6950 3550
	1    0    0    -1  
$EndComp
Wire Wire Line
	4050 3400 4050 3900
Connection ~ 4050 3550
Wire Wire Line
	3650 3000 3650 3400
Wire Wire Line
	3650 3700 3650 3900
Connection ~ 3650 3250
Wire Wire Line
	5250 3400 5550 3400
Wire Wire Line
	5300 3400 5300 3500
Wire Wire Line
	5300 3800 5300 3900
Connection ~ 5300 3400
Connection ~ 6200 3400
Wire Wire Line
	6200 3800 6200 3900
Wire Wire Line
	4050 5150 4050 5650
Connection ~ 4050 5300
Wire Wire Line
	3100 5000 4050 5000
Wire Wire Line
	3650 5000 3650 5150
Wire Wire Line
	3650 5450 3650 5650
Connection ~ 3650 5000
Wire Wire Line
	5250 5150 5500 5150
Wire Wire Line
	5300 5150 5300 5250
Wire Wire Line
	5300 5550 5300 5650
Connection ~ 5300 5150
Wire Wire Line
	5250 5000 7700 5000
Connection ~ 6200 5150
Wire Wire Line
	6200 5550 6200 5650
Wire Wire Line
	6000 5150 6200 5150
Wire Wire Line
	6200 5000 6200 5250
Connection ~ 6200 5000
Wire Wire Line
	6550 5250 6700 5250
Connection ~ 6550 5250
Connection ~ 6550 5000
Wire Wire Line
	7150 3000 7150 3150
Connection ~ 7150 3150
Wire Wire Line
	6200 3150 6200 3500
Wire Wire Line
	6950 3400 6950 3250
Wire Wire Line
	6050 3400 6450 3400
Wire Wire Line
	6950 3250 5250 3250
Connection ~ 6950 3400
Wire Wire Line
	6200 3150 7750 3150
Wire Wire Line
	6750 3400 6950 3400
Wire Wire Line
	7600 5250 7700 5250
$Comp
L GNDA #PWR027
U 1 1 5880C75E
P 3650 3900
F 0 "#PWR027" H 3650 3650 50  0001 C CNN
F 1 "GNDA" H 3650 3750 50  0000 C CNN
F 2 "" H 3650 3900 50  0000 C CNN
F 3 "" H 3650 3900 50  0000 C CNN
	1    3650 3900
	1    0    0    -1  
$EndComp
$Comp
L GNDA #PWR028
U 1 1 5880C75F
P 4050 3900
F 0 "#PWR028" H 4050 3650 50  0001 C CNN
F 1 "GNDA" H 4050 3750 50  0000 C CNN
F 2 "" H 4050 3900 50  0000 C CNN
F 3 "" H 4050 3900 50  0000 C CNN
	1    4050 3900
	1    0    0    -1  
$EndComp
$Comp
L GNDA #PWR029
U 1 1 5880C760
P 5300 3900
F 0 "#PWR029" H 5300 3650 50  0001 C CNN
F 1 "GNDA" H 5300 3750 50  0000 C CNN
F 2 "" H 5300 3900 50  0000 C CNN
F 3 "" H 5300 3900 50  0000 C CNN
	1    5300 3900
	1    0    0    -1  
$EndComp
$Comp
L GNDA #PWR030
U 1 1 5880C761
P 6200 3900
F 0 "#PWR030" H 6200 3650 50  0001 C CNN
F 1 "GNDA" H 6200 3750 50  0000 C CNN
F 2 "" H 6200 3900 50  0000 C CNN
F 3 "" H 6200 3900 50  0000 C CNN
	1    6200 3900
	1    0    0    -1  
$EndComp
$Comp
L GNDA #PWR031
U 1 1 5880C762
P 6950 3900
F 0 "#PWR031" H 6950 3650 50  0001 C CNN
F 1 "GNDA" H 6950 3750 50  0000 C CNN
F 2 "" H 6950 3900 50  0000 C CNN
F 3 "" H 6950 3900 50  0000 C CNN
	1    6950 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	6950 3900 6950 3700
$Comp
L GNDA #PWR032
U 1 1 5880C763
P 3150 1750
F 0 "#PWR032" H 3150 1500 50  0001 C CNN
F 1 "GNDA" H 3150 1600 50  0000 C CNN
F 2 "" H 3150 1750 50  0000 C CNN
F 3 "" H 3150 1750 50  0000 C CNN
	1    3150 1750
	1    0    0    -1  
$EndComp
$Comp
L GNDA #PWR033
U 1 1 5880C764
P 7600 3650
F 0 "#PWR033" H 7600 3400 50  0001 C CNN
F 1 "GNDA" H 7600 3500 50  0000 C CNN
F 2 "" H 7600 3650 50  0000 C CNN
F 3 "" H 7600 3650 50  0000 C CNN
	1    7600 3650
	1    0    0    -1  
$EndComp
Text HLabel 7800 3550 2    60   Input ~ 0
GND_Reg
Wire Wire Line
	7600 3450 7600 3650
Wire Wire Line
	8550 5250 8450 5250
$Comp
L PWR_FLAG #FLG034
U 1 1 5880C765
P 7600 3450
F 0 "#FLG034" H 7600 3545 50  0001 C CNN
F 1 "PWR_FLAG" H 7600 3630 50  0000 C CNN
F 2 "" H 7600 3450 50  0000 C CNN
F 3 "" H 7600 3450 50  0000 C CNN
	1    7600 3450
	1    0    0    -1  
$EndComp
Connection ~ 7600 3550
Text Label 3100 5000 2    60   ~ 0
VIN_7/40V
$Comp
L POT RV201
U 1 1 5880C766
P 6600 3400
F 0 "RV201" V 6600 3400 50  0000 C CNN
F 1 "ADJ_POT" V 6500 3400 50  0000 C CNN
F 2 "Potentiometers:Potentiometer_Bourns_3296W_3-8Zoll_Inline_ScrewUp" H 6600 3400 50  0001 C CNN
F 3 "" H 6600 3400 50  0000 C CNN
	1    6600 3400
	0    1    1    0   
$EndComp
Wire Wire Line
	6350 3400 6350 3550
Wire Wire Line
	6350 3550 6600 3550
Connection ~ 6350 3400
Wire Wire Line
	7800 3550 7600 3550
$Comp
L CONN_01X02 P202
U 1 1 5880C767
P 8050 1700
F 0 "P202" H 8050 1850 50  0000 C CNN
F 1 "POWER_Out-(+5V)" V 8150 1700 50  0000 C CNN
F 2 "mis-footprint:Pin_1x2_Paso" H 8050 1700 50  0001 C CNN
F 3 "" H 8050 1700 50  0000 C CNN
	1    8050 1700
	-1   0    0    1   
$EndComp
Text Label 6750 5000 0    60   ~ 0
Vcc_+5V
Text Label 8250 1750 3    60   ~ 0
Vcc_+5V
$Comp
L GND #PWR035
U 1 1 5880C768
P 8600 1650
F 0 "#PWR035" H 8600 1400 50  0001 C CNN
F 1 "GND" H 8600 1500 50  0000 C CNN
F 2 "" H 8600 1650 50  0000 C CNN
F 3 "" H 8600 1650 50  0000 C CNN
	1    8600 1650
	1    0    0    -1  
$EndComp
Wire Wire Line
	8250 1650 8600 1650
$Comp
L RegLin-G-O-I U203
U 1 1 5880C769
P 7150 5250
F 0 "U203" H 7300 5100 60  0000 C CNN
F 1 "RegLin-G-O-I" H 7100 5350 60  0000 C CNN
F 2 "TO_SOT_Packages_THT:TO-220_Neutral123_Vertical" H 7150 5250 60  0001 C CNN
F 3 "" H 7150 5250 60  0000 C CNN
	1    7150 5250
	-1   0    0    -1  
$EndComp
Wire Wire Line
	7700 5700 7150 5700
Wire Wire Line
	6550 5000 6550 5300
Wire Wire Line
	8450 5250 8450 5350
$Comp
L PWR_FLAG #FLG036
U 1 1 588326BA
P 7200 4850
F 0 "#FLG036" H 7200 4945 50  0001 C CNN
F 1 "PWR_FLAG" H 7200 5030 50  0000 C CNN
F 2 "" H 7200 4850 50  0000 C CNN
F 3 "" H 7200 4850 50  0000 C CNN
	1    7200 4850
	1    0    0    -1  
$EndComp
Wire Wire Line
	7200 4850 7200 5000
Connection ~ 7200 5000
$Comp
L SW_DPST_x2 SW201
U 1 1 58837935
P 3400 1650
F 0 "SW201" H 3400 1775 50  0000 C CNN
F 1 "Power ON/OFF" H 3400 1550 50  0000 C CNN
F 2 "mis-footprint:Pin_1x2_Paso" H 3400 1650 50  0001 C CNN
F 3 "" H 3400 1650 50  0001 C CNN
	1    3400 1650
	1    0    0    -1  
$EndComp
Wire Wire Line
	2650 3250 4050 3250
Wire Wire Line
	2950 1750 3150 1750
Wire Wire Line
	2950 1650 3200 1650
$Comp
L Fuse F201
U 1 1 58838415
P 4400 1650
F 0 "F201" V 4480 1650 50  0000 C CNN
F 1 "Fuse" V 4325 1650 50  0000 C CNN
F 2 "Fuse_Holders_and_Fuses:Fuseholder5x20_horiz_SemiClosed_Casing10x25mm" H 4330 1650 50  0001 C CNN
F 3 "" H 4400 1650 50  0001 C CNN
	1    4400 1650
	0    1    1    0   
$EndComp
$Comp
L LED_ALT D202
U 1 1 588384D6
P 4700 1950
F 0 "D202" H 4700 2050 50  0000 C CNN
F 1 "PWR_ON_LED" H 4700 1850 50  0000 C CNN
F 2 "LEDs:LED-3MM" H 4700 1950 50  0001 C CNN
F 3 "" H 4700 1950 50  0001 C CNN
	1    4700 1950
	-1   0    0    1   
$EndComp
$Comp
L LED_ALT D201
U 1 1 58838585
P 4650 1350
F 0 "D201" H 4650 1450 50  0000 C CNN
F 1 "LED_FUSE" H 4650 1250 50  0000 C CNN
F 2 "LEDs:LED-3MM" H 4650 1350 50  0001 C CNN
F 3 "" H 4650 1350 50  0001 C CNN
	1    4650 1350
	-1   0    0    1   
$EndComp
$Comp
L R R202
U 1 1 58838633
P 4200 1950
F 0 "R202" V 4200 1900 50  0000 C CNN
F 1 "PWR_ON_R" V 4300 1950 50  0000 C CNN
F 2 "Discret:R3" V 4130 1950 50  0001 C CNN
F 3 "" H 4200 1950 50  0000 C CNN
	1    4200 1950
	0    1    1    0   
$EndComp
$Comp
L R R201
U 1 1 58838715
P 4150 1350
F 0 "R201" V 4150 1300 50  0000 C CNN
F 1 "PWR_Fuse_R" V 4250 1350 50  0000 C CNN
F 2 "Discret:R3" V 4080 1350 50  0001 C CNN
F 3 "" H 4150 1350 50  0000 C CNN
	1    4150 1350
	0    1    1    0   
$EndComp
Wire Wire Line
	4350 1950 4550 1950
Wire Wire Line
	4300 1350 4500 1350
Wire Wire Line
	4000 1350 3800 1350
Wire Wire Line
	3800 1350 3800 1950
Wire Wire Line
	3600 1650 4250 1650
Wire Wire Line
	4800 1350 4900 1350
Wire Wire Line
	4900 1350 4900 1650
Wire Wire Line
	4550 1650 5100 1650
Connection ~ 3800 1650
Connection ~ 4900 1650
Wire Wire Line
	3800 1950 4050 1950
$Comp
L GNDA #PWR037
U 1 1 58839572
P 5100 1950
F 0 "#PWR037" H 5100 1700 50  0001 C CNN
F 1 "GNDA" H 5100 1800 50  0000 C CNN
F 2 "" H 5100 1950 50  0000 C CNN
F 3 "" H 5100 1950 50  0000 C CNN
	1    5100 1950
	1    0    0    -1  
$EndComp
Wire Wire Line
	5100 1950 4850 1950
Text Label 5100 1650 0    60   ~ 0
VIN_7/40V
$Comp
L CONN_01X02 P203
U 1 1 58839BD5
P 7000 1700
F 0 "P203" H 7000 1850 50  0000 C CNN
F 1 "PWR_Out-Reg" V 7100 1700 50  0000 C CNN
F 2 "mis-footprint:Pin_1x2_Paso" H 7000 1700 50  0001 C CNN
F 3 "" H 7000 1700 50  0000 C CNN
	1    7000 1700
	-1   0    0    1   
$EndComp
Text Label 7200 1750 3    60   ~ 0
Vcc_Reg
Wire Wire Line
	7200 1650 7550 1650
$Comp
L GNDA #PWR038
U 1 1 58839CA1
P 7550 1650
F 0 "#PWR038" H 7550 1400 50  0001 C CNN
F 1 "GNDA" H 7550 1500 50  0000 C CNN
F 2 "" H 7550 1650 50  0000 C CNN
F 3 "" H 7550 1650 50  0000 C CNN
	1    7550 1650
	1    0    0    -1  
$EndComp
Wire Notes Line
	2050 1050 2050 6050
Wire Notes Line
	2050 6050 9050 6050
Wire Notes Line
	9050 6050 9050 1050
Wire Notes Line
	9050 1050 2050 1050
Wire Notes Line
	2050 2500 9050 2500
Wire Notes Line
	2050 4350 9050 4350
Wire Notes Line
	6050 1050 6050 2500
$EndSCHEMATC