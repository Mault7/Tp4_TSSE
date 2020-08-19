/*
 * max30105.c
 *
 *  Created on: 22 may. 2020
 *      Author: mauri
 */
#include "max30105.h"
#include "sapi.h"

#define MAX30105_ADDRESS 0x57 //7-bit I2C Address
#define time 100
//delay_t delayt;

// Status Registers
#define MAX30105_INTSTAT1 0x00
#define MAX30105_INTSTAT2 0x01
#define MAX30105_INTENABLE1 0x02
#define MAX30105_INTENABLE2 0x03

// FIFO Registers
#define MAX30105_FIFOWRITEPTR 0x04
#define MAX30105_FIFOOVERFLOW 0x05
#define MAX30105_FIFOREADPTR 0x06
#define MAX30105_FIFODATA 0x07

// Configuration Registers
#define MAX30105_FIFOCONFIG 0x08
#define MAX30105_MODECONFIG 0x09
#define MAX30105_PARTICLECONFIG 0x0A // Note, sometimes listed as "SPO2" config in datasheet (pg. 11)
#define MAX30105_LED1_PULSEAMP 0x0C
#define MAX30105_LED2_PULSEAMP 0x0D
#define MAX30105_LED3_PULSEAMP 0x0E
#define MAX30105_LED_PROX_AMP 0x10
#define MAX30105_MULTILEDCONFIG1 0x11
#define MAX30105_MULTILEDCONFIG2 0x12

// Die Temperature Registers
#define MAX30105_DIETEMPINT 0x1F
#define MAX30105_DIETEMPFRAC 0x20
#define MAX30105_DIETEMPCONFIG 0x21

// Proximity Function Registers
#define MAX30105_PROXINTTHRESH 0x30

// Part ID Registers
#define MAX30105_REVISIONID 0xFE
#define MAX30105_PARTID 0xFF // Should always be 0x15. Identical to MAX30102.

// MAX30105 Commands
// Interrupt configuration (pg 13, 14)
#define MAX30105_INT_A_FULL_MASK 0x7F
#define MAX30105_INT_A_FULL_ENABLE 0x80
#define MAX30105_INT_A_FULL_DISABLE 0x00

#define MAX30105_INT_DATA_RDY_MASK 0xBF
#define MAX30105_INT_DATA_RDY_ENABLE 0x40
#define MAX30105_INT_DATA_RDY_DISABLE 0x00

#define MAX30105_INT_ALC_OVF_MASK 0xDF
#define MAX30105_INT_ALC_OVF_ENABLE 0x20
#define MAX30105_INT_ALC_OVF_DISABLE 0x00

#define MAX30105_INT_PROX_INT_MASK 0xEF
#define MAX30105_INT_PROX_INT_ENABLE 0x10
#define MAX30105_INT_PROX_INT_DISABLE 0x00
#define MAX30105_INT_DIE_TEMP_RDY_MASK 0xFD
#define MAX30105_INT_DIE_TEMP_RDY_ENABLE 0x02
#define MAX30105_INT_DIE_TEMP_RDY_DISABLE 0x00

#define MAX30105_SAMPLEAVG_MASK 0x1F
#define MAX30105_SAMPLEAVG_1 0x00
#define MAX30105_SAMPLEAVG_2 0x20
#define MAX30105_SAMPLEAVG_4 0x40
#define MAX30105_SAMPLEAVG_8 0x60
#define MAX30105_SAMPLEAVG_16 0x80
#define MAX30105_SAMPLEAVG_32 0xA0

#define MAX30105_ROLLOVER_MASK 0xEF
#define MAX30105_ROLLOVER_ENABLE 0x10
#define MAX30105_ROLLOVER_DISABLE 0x00

#define MAX30105_A_FULL_MASK 0xF0

// Mode configuration commands (page 19)
#define MAX30105_SHUTDOWN_MASK 0x7F
#define MAX30105_SHUTDOWN 0x80
#define MAX30105_WAKEUP 0x00

#define MAX30105_RESET_MASK 0xBF
#define MAX30105_RESET 0x40

#define MAX30105_MODE_MASK 0xF8
#define MAX30105_MODE_REDONLY 0x02
#define MAX30105_MODE_REDIRONLY 0x03
#define MAX30105_MODE_MULTILED 0x07

// Particle sensing configuration commands (pgs 19-20)
#define MAX30105_ADCRANGE_MASK 0x9F
#define MAX30105_ADCRANGE_2048 0x00
#define MAX30105_ADCRANGE_4096 0x20
#define MAX30105_ADCRANGE_8192 0x40
#define MAX30105_ADCRANGE_16384 0x60

#define MAX30105_SAMPLERATE_MASK 0xE3
#define MAX30105_SAMPLERATE_50 0x00
#define MAX30105_SAMPLERATE_100 0x04
#define MAX30105_SAMPLERATE_200 0x08
#define MAX30105_SAMPLERATE_400 0x0C
#define MAX30105_SAMPLERATE_800 0x10
#define MAX30105_SAMPLERATE_1000 0x14
#define MAX30105_SAMPLERATE_1600 0x18
#define MAX30105_SAMPLERATE_3200 0x1C

#define MAX30105_PULSEWIDTH_MASK 0xFC
#define MAX30105_PULSEWIDTH_69 0x00
#define MAX30105_PULSEWIDTH_118 0x01
#define MAX30105_PULSEWIDTH_215 0x02
#define MAX30105_PULSEWIDTH_411 0x03

//Multi-LED Mode configuration (pg 22)
#define MAX30105_SLOT1_MASK 0xF8
#define MAX30105_SLOT2_MASK 0x8F
#define MAX30105_SLOT3_MASK 0xF8
#define MAX30105_SLOT4_MASK 0x8F

#define SLOT_NONE 0x00
#define SLOT_RED_LED 0x01
#define SLOT_IR_LED 0x02
#define SLOT_GREEN_LED 0x03
#define SLOT_NONE_PILOT 0x04
#define SLOT_RED_PILOT 0x05
#define SLOT_IR_PILOT 0x06
#define SLOT_GREEN_PILOT 0x07

#define MAX_30105_EXPECTEDPARTID 0x15

uint8_t sampleo[27];
uint8_t cont = 0;

i2cMap_t I2C0 = 0;
uint32_t clockratehz = 100000;

bool_t MAX30105Init(void)
{

	bool_t value;
	value = i2cInit(I2C0, clockratehz);
	return value;
}

uint8_t WriteRegister(uint8_t address, uint8_t reg, uint8_t value)
{
	uint8_t vec[] = {reg, value};
	uint8_t dato;
	dato = i2cWrite(I2C0, address, vec, 2, TRUE);
	return dato;
}

uint8_t ReadRegister(uint8_t address, uint8_t reg)
{
	uint8_t dato;
	dato = (i2cRead(I2C0, address, 0, 0, FALSE, &dato, 1, TRUE));
	return dato;
}

uint8_t Readmul(uint8_t address, uint8_t reg, uint8_t size, uint8_t vec[])
{

	//delay(10);
	//clearFIFO();
	uint8_t dato[size];
	uint8_t dato2;
	i2cWrite(I2C0, address, &reg, 1, TRUE);
	i2cRead(I2C0, address, 0, 0, FALSE, dato, size, TRUE);
	clearFIFO();
	/*
	dato2=dato[25];
	if (dato2 == NULL)
		return FALSE;
	for (int i = 0; i < size; i++)
	{
		vec[i] = dato[i];
	}
*/
	return TRUE;
}

uint8_t bitmask(uint8_t reg, uint8_t mask, uint8_t content)
{
	uint8_t originalContents = ReadRegister(MAX30105_ADDRESS, reg);

	// Zero-out the portions of the register we're interested in
	originalContents = originalContents & mask;

	// Change contents
	return WriteRegister(MAX30105_ADDRESS, reg, originalContents | content);
}

uint32_t longt(uint8_t vec[])
{
	uint32_t templong = 0;

	memcpy(&templong, vec, sizeof(templong));
	return templong;
}

//Begin Interrupt configuration
uint8_t getINT1(void)
{
	return (ReadRegister(MAX30105_ADDRESS, MAX30105_INTSTAT1));
}
uint8_t getINT2(void)
{
	return (ReadRegister(MAX30105_ADDRESS, MAX30105_INTSTAT2));
}

void enableAFULL(void)
{
	bitmask(MAX30105_INTENABLE1, MAX30105_INT_A_FULL_MASK, MAX30105_INT_A_FULL_ENABLE);
}
void disableAFULL(void)
{
	bitmask(MAX30105_INTENABLE1, MAX30105_INT_A_FULL_MASK, MAX30105_INT_A_FULL_DISABLE);
}

void enableDATARDY(void)
{
	bitmask(MAX30105_INTENABLE1, MAX30105_INT_DATA_RDY_MASK, MAX30105_INT_DATA_RDY_ENABLE);
}
void disableDATARDY(void)
{
	bitmask(MAX30105_INTENABLE1, MAX30105_INT_DATA_RDY_MASK, MAX30105_INT_DATA_RDY_DISABLE);
}

void enableALCOVF(void)
{
	bitmask(MAX30105_INTENABLE1, MAX30105_INT_ALC_OVF_MASK, MAX30105_INT_ALC_OVF_ENABLE);
}
void disableALCOVF(void)
{
	bitmask(MAX30105_INTENABLE1, MAX30105_INT_ALC_OVF_MASK, MAX30105_INT_ALC_OVF_DISABLE);
}

void enablePROXINT(void)
{
	bitmask(MAX30105_INTENABLE1, MAX30105_INT_PROX_INT_MASK, MAX30105_INT_PROX_INT_ENABLE);
}
void disablePROXINT(void)
{
	bitmask(MAX30105_INTENABLE1, MAX30105_INT_PROX_INT_MASK, MAX30105_INT_PROX_INT_DISABLE);
}

void enableDIETEMPRDY(void)
{
	bitmask(MAX30105_INTENABLE2, MAX30105_INT_DIE_TEMP_RDY_MASK, MAX30105_INT_DIE_TEMP_RDY_ENABLE);
}
void disableDIETEMPRDY(void)
{
	bitmask(MAX30105_INTENABLE2, MAX30105_INT_DIE_TEMP_RDY_MASK, MAX30105_INT_DIE_TEMP_RDY_DISABLE);
}

//End Interrupt configuration
/*
void softReset(void) {
	bitmask(MAX30105_MODECONFIG, MAX30105_RESET_MASK, MAX30105_RESET);

	// Poll for bit to clear, reset is then complete
	// Timeout after 100ms
	while (!delayRead(&delayt)){
		uint8_t response = ReadRegister(MAX30105_ADDRESS, MAX30105_MODECONFIG);
		if ((response & MAX30105_RESET) == 0) break; //We're done!
		delay(1);

	}
	delayWrite(&delayt,time);
}
*/

void shutDown(void)
{
	// Put IC into low power mode (datasheet pg. 19)
	// During shutdown the IC will continue to respond to I2C commands but will
	// not update with or take new readings (such as temperature)
	bitmask(MAX30105_MODECONFIG, MAX30105_SHUTDOWN_MASK, MAX30105_SHUTDOWN);
}

void wakeUp(void)
{
	// Pull IC out of low power mode (datasheet pg. 19)
	bitmask(MAX30105_MODECONFIG, MAX30105_SHUTDOWN_MASK, MAX30105_WAKEUP);
}

void setLEDMode(uint8_t mode)
{
	// Set which LEDs are used for sampling -- Red only, RED+IR only, or custom.
	// See datasheet, page 19
	bitmask(MAX30105_MODECONFIG, MAX30105_MODE_MASK, mode);
}

void setADCRange(uint8_t adcRange)
{
	// adcRange: one of MAX30105_ADCRANGE_2048, _4096, _8192, _16384
	bitmask(MAX30105_PARTICLECONFIG, MAX30105_ADCRANGE_MASK, adcRange);
}

void setSampleRate(uint8_t sampleRate)
{
	// sampleRate: one of MAX30105_SAMPLERATE_50, _100, _200, _400, _800, _1000, _1600, _3200
	bitmask(MAX30105_PARTICLECONFIG, MAX30105_SAMPLERATE_MASK, sampleRate);
}

void setPulseWidth(uint8_t pulseWidth)
{
	// pulseWidth: one of MAX30105_PULSEWIDTH_69, _188, _215, _411
	bitmask(MAX30105_PARTICLECONFIG, MAX30105_PULSEWIDTH_MASK, pulseWidth);
}

// NOTE: Amplitude values: 0x00 = 0mA, 0x7F = 25.4mA, 0xFF = 50mA (typical)
// See datasheet, page 21
void setPulseAmplitudeRed(uint8_t amplitude)
{
	WriteRegister(MAX30105_ADDRESS, MAX30105_LED1_PULSEAMP, amplitude);
}

void setPulseAmplitudeIR(uint8_t amplitude)
{
	WriteRegister(MAX30105_ADDRESS, MAX30105_LED2_PULSEAMP, amplitude);
}

void setPulseAmplitudeGreen(uint8_t amplitude)
{
	WriteRegister(MAX30105_ADDRESS, MAX30105_LED3_PULSEAMP, amplitude);
}

void setPulseAmplitudeProximity(uint8_t amplitude)
{
	WriteRegister(MAX30105_ADDRESS, MAX30105_LED_PROX_AMP, amplitude);
}

void setProximityThreshold(uint8_t threshMSB)
{
	// Set the IR ADC count that will trigger the beginning of particle-sensing mode.
	// The threshMSB signifies only the 8 most significant-bits of the ADC count.
	// See datasheet, page 24.
	WriteRegister(MAX30105_ADDRESS, MAX30105_PROXINTTHRESH, threshMSB);
}

//Given a slot number assign a thing to it
//Devices are SLOT_RED_LED or SLOT_RED_PILOT (proximity)
//Assigning a SLOT_RED_LED will pulse LED
//Assigning a SLOT_RED_PILOT will ??
void enableSlot(uint8_t slotNumber, uint8_t device)
{

	uint8_t originalContents;

	switch (slotNumber)
	{
	case (1):
		bitmask(MAX30105_MULTILEDCONFIG1, MAX30105_SLOT1_MASK, device);
		break;
	case (2):
		bitmask(MAX30105_MULTILEDCONFIG1, MAX30105_SLOT2_MASK, device << 4);
		break;
	case (3):
		bitmask(MAX30105_MULTILEDCONFIG2, MAX30105_SLOT3_MASK, device);
		break;
	case (4):
		bitmask(MAX30105_MULTILEDCONFIG2, MAX30105_SLOT4_MASK, device << 4);
		break;
	default:
		//Shouldn't be here!
		break;
	}
}

//Clears all slot assignments
void disableSlots(void)
{
	WriteRegister(MAX30105_ADDRESS, MAX30105_MULTILEDCONFIG1, 0);
	WriteRegister(MAX30105_ADDRESS, MAX30105_MULTILEDCONFIG2, 0);
}

//
// FIFO Configuration
//

//Set sample average (Table 3, Page 18)
void setFIFOAverage(uint8_t numberOfSamples)
{
	bitmask(MAX30105_FIFOCONFIG, MAX30105_SAMPLEAVG_MASK, numberOfSamples);
}

//Resets all points to start in a known state
//Page 15 recommends clearing FIFO before beginning a read
void clearFIFO(void)
{
	WriteRegister(MAX30105_ADDRESS, MAX30105_FIFOWRITEPTR, 0);
	WriteRegister(MAX30105_ADDRESS, MAX30105_FIFOOVERFLOW, 0);
	WriteRegister(MAX30105_ADDRESS, MAX30105_FIFOREADPTR, 0);
}

//Enable roll over if FIFO over flows
void enableFIFORollover(void)
{
	bitmask(MAX30105_FIFOCONFIG, MAX30105_ROLLOVER_MASK, MAX30105_ROLLOVER_ENABLE);
}

//Disable roll over if FIFO over flows
void disableFIFORollover(void)
{
	bitmask(MAX30105_FIFOCONFIG, MAX30105_ROLLOVER_MASK, MAX30105_ROLLOVER_DISABLE);
}

//Set number of samples to trigger the almost full interrupt (Page 18)
//Power on default is 32 samples
//Note it is reverse: 0x00 is 32 samples, 0x0F is 17 samples
void setFIFOAlmostFull(uint8_t numberOfSamples)
{
	bitmask(MAX30105_FIFOCONFIG, MAX30105_A_FULL_MASK, numberOfSamples);
}

//Read the FIFO Write Pointer
uint8_t getWritePointer(void)
{
	return (ReadRegister(MAX30105_ADDRESS, MAX30105_FIFOWRITEPTR));
}

//Read the FIFO Read Pointer
uint8_t getReadPointer(void)
{
	return (ReadRegister(MAX30105_ADDRESS, MAX30105_FIFOREADPTR));
}

// Die Temperature
// Returns temp in C'

float readTemperature() {

	//DIE_TEMP_RDY interrupt must be enabled
	//See issue 19: https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library/issues/19

	// Step 1: Config die temperature register to take 1 temperature sample
	WriteRegister(MAX30105_ADDRESS, MAX30105_DIETEMPCONFIG, 0x01);

	// Poll for bit to clear, reading is then complete
	// Timeout after 100ms


	while (!delayRead(&delayt))
	{
		//uint8_t response = readRegister8(_i2caddr, MAX30105_DIETEMPCONFIG); //Original way
		//if ((response & 0x01) == 0) break; //We're done!

		//Check to see if DIE_TEMP_RDY interrupt is set
		uint8_t response = ReadRegister(MAX30105_ADDRESS, MAX30105_INTSTAT2);
		if ((response & MAX30105_INT_DIE_TEMP_RDY_ENABLE) > 0) break; //We're done!
		//delay(1); //Let's not over burden the I2C bus
	}
	//delayWrite(&delayt,time);
	//TODO How do we want to fail? With what type of error?
	//? if(millis() - startTime >= 100) return(-999.0);

	// Step 2: Read die temperature register (integer)
	int8_t tempInt = ReadRegister(MAX30105_ADDRESS, MAX30105_DIETEMPINT);
	uint8_t tempFrac = ReadRegister(MAX30105_ADDRESS, MAX30105_DIETEMPFRAC); //Causes the clearing of the DIE_TEMP_RDY interrupt

	// Step 3: Calculate temperature (datasheet pg. 23)
	return (float)tempInt + ((float)tempFrac * 0.0625);
}


// Set the PROX_INT_THRESHold
void setPROXINTTHRESH(uint8_t val)
{
	WriteRegister(MAX30105_ADDRESS, MAX30105_PROXINTTHRESH, val);
}

//
// Device ID and Revision
//
uint8_t readPartID()
{
	return ReadRegister(MAX30105_ADDRESS, MAX30105_PARTID);
}

void readRevisionID()
{
	revisionID = ReadRegister(MAX30105_ADDRESS, MAX30105_REVISIONID);
}

uint8_t getRevisionID()
{
	return revisionID;
}

//Setup the sensor
//The MAX30105 has many settings. By default we select:
// Sample Average = 4
// Mode = MultiLED
// ADC Range = 16384 (62.5pA per LSB)
// Sample rate = 50
//Use the default setup if you are just getting started with the MAX30105 sensor
void setup(uint8_t powerLevel, uint8_t sampleAverage, uint8_t ledMode, int sampleRate, int pulseWidth, int adcRange)
{
	//Reset(); //Reset all configuration, threshold, and data registers to POR values

	//FIFO Configuration
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	//The chip will average multiple samples of same type together if you wish
	if (sampleAverage == 1)
		setFIFOAverage(MAX30105_SAMPLEAVG_1); //No averaging per FIFO record
	else if (sampleAverage == 2)
		setFIFOAverage(MAX30105_SAMPLEAVG_2);
	else if (sampleAverage == 4)
		setFIFOAverage(MAX30105_SAMPLEAVG_4);
	else if (sampleAverage == 8)
		setFIFOAverage(MAX30105_SAMPLEAVG_8);
	else if (sampleAverage == 16)
		setFIFOAverage(MAX30105_SAMPLEAVG_16);
	else if (sampleAverage == 32)
		setFIFOAverage(MAX30105_SAMPLEAVG_32);
	else
		setFIFOAverage(MAX30105_SAMPLEAVG_4);

	//setFIFOAlmostFull(2); //Set to 30 samples to trigger an 'Almost Full' interrupt
	enableFIFORollover(); //Allow FIFO to wrap/roll over
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	//Mode Configuration
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (ledMode == 3)
		setLEDMode(MAX30105_MODE_MULTILED); //Watch all three LED channels
	else if (ledMode == 2)
		setLEDMode(MAX30105_MODE_REDIRONLY); //Red and IR
	else
		setLEDMode(MAX30105_MODE_REDONLY); //Red only
	activeLEDs = ledMode;				   //Used to control how many bytes to read from FIFO buffer
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	//Particle Sensing Configuration
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (adcRange < 4096)
		setADCRange(MAX30105_ADCRANGE_2048); //7.81pA per LSB
	else if (adcRange < 8192)
		setADCRange(MAX30105_ADCRANGE_4096); //15.63pA per LSB
	else if (adcRange < 16384)
		setADCRange(MAX30105_ADCRANGE_8192); //31.25pA per LSB
	else if (adcRange == 16384)
		setADCRange(MAX30105_ADCRANGE_16384); //62.5pA per LSB
	else
		setADCRange(MAX30105_ADCRANGE_2048);

	if (sampleRate < 100)
		setSampleRate(MAX30105_SAMPLERATE_50); //Take 50 samples per second
	else if (sampleRate < 200)
		setSampleRate(MAX30105_SAMPLERATE_100);
	else if (sampleRate < 400)
		setSampleRate(MAX30105_SAMPLERATE_200);
	else if (sampleRate < 800)
		setSampleRate(MAX30105_SAMPLERATE_400);
	else if (sampleRate < 1000)
		setSampleRate(MAX30105_SAMPLERATE_800);
	else if (sampleRate < 1600)
		setSampleRate(MAX30105_SAMPLERATE_1000);
	else if (sampleRate < 3200)
		setSampleRate(MAX30105_SAMPLERATE_1600);
	else if (sampleRate == 3200)
		setSampleRate(MAX30105_SAMPLERATE_3200);
	else
		setSampleRate(MAX30105_SAMPLERATE_50);

	//The longer the pulse width the longer range of detection you'll have
	//At 69us and 0.4mA it's about 2 inches
	//At 411us and 0.4mA it's about 6 inches
	if (pulseWidth < 118)
		setPulseWidth(MAX30105_PULSEWIDTH_69); //Page 26, Gets us 15 bit resolution
	else if (pulseWidth < 215)
		setPulseWidth(MAX30105_PULSEWIDTH_118); //16 bit resolution
	else if (pulseWidth < 411)
		setPulseWidth(MAX30105_PULSEWIDTH_215); //17 bit resolution
	else if (pulseWidth == 411)
		setPulseWidth(MAX30105_PULSEWIDTH_411); //18 bit resolution
	else
		setPulseWidth(MAX30105_PULSEWIDTH_69);
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	//LED Pulse Amplitude Configuration
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	//Default is 0x1F which gets us 6.4mA
	//powerLevel = 0x02, 0.4mA - Presence detection of ~4 inch
	//powerLevel = 0x1F, 6.4mA - Presence detection of ~8 inch
	//powerLevel = 0x7F, 25.4mA - Presence detection of ~8 inch
	//powerLevel = 0xFF, 50.0mA - Presence detection of ~12 inch

	setPulseAmplitudeRed(powerLevel);
	setPulseAmplitudeIR(powerLevel);
	setPulseAmplitudeGreen(powerLevel);
	setPulseAmplitudeProximity(powerLevel);
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	//Multi-LED Mode Configuration, Enable the reading of the three LEDs
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	enableSlot(1, SLOT_RED_LED);
	if (ledMode > 1)
		enableSlot(2, SLOT_IR_LED);
	if (ledMode > 2)
		enableSlot(3, SLOT_GREEN_LED);
	//enableSlot(1, SLOT_RED_PILOT);
	//enableSlot(2, SLOT_IR_PILOT);
	//enableSlot(3, SLOT_GREEN_PILOT);
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	clearFIFO(); //Reset the FIFO before we begin checking the sensor
}
//
// Data Collection
//

//Tell caller how many samples are available
uint8_t available(sense_struct *psense)
{
	int8_t numberOfSamples = psense->head - psense->tail;
	if (numberOfSamples < 0)
		numberOfSamples += STORAGE_SIZE;

	return (numberOfSamples);
}

//Report the most recent red value
uint32_t getRed(sense_struct *psense)
{
	//Check the sensor for new data for 250ms
	if (safeCheck(250, psense))
		return (psense->red[psense->head]);
	else
		return (0); //Sensor failed to find new data
}

//Report the most recent IR value
uint32_t getIR(sense_struct *psense)
{
	//Check the sensor for new data for 250ms
	if (safeCheck(250, psense))
		return (psense->IR[psense->head]);
	else
		return (0); //Sensor failed to find new data
}

//Report the most recent Green value
uint32_t getGreen(sense_struct *psense)
{
	//Check the sensor for new data for 250ms
	if (safeCheck(250, psense))
		return (psense->green[psense->head]);
	else
		return (0); //Sensor failed to find new data
}

//Report the next Red value in the FIFO
uint32_t getFIFORed(sense_struct *psense)
{
	return (psense->red[psense->tail]);
}

//Report the next IR value in the FIFO
uint32_t getFIFOIR(sense_struct *psense)
{
	return (psense->IR[psense->tail]);
}

//Report the next Green value in the FIFO
uint32_t getFIFOGreen(sense_struct *psense)
{
	return (psense->green[psense->tail]);
}

//Advance the tail
void nextSample(sense_struct *psense)
{
	if (available(psense)) //Only advance the tail if new data is available
	{
		psense->tail++;
		psense->tail %= STORAGE_SIZE; //Wrap condition
	}
}

//Polls the sensor for new data
//Call regularly
//If new data is available, it updates the head and tail in the main struct
//Returns number of new samples obtained
uint16_t check(sense_struct *psense)
{
	//Read register FIDO_DATA in (3-byte * number of active LED) chunks
	//Until FIFO_RD_PTR = FIFO_WR_PTR

	uint8_t readPointer = getReadPointer();
	uint8_t writePointer = getWritePointer();

	int numberOfSamples = 0;

	//Do we have new data?
	if (readPointer != writePointer)
	{
		//Calculate the number of readings we need to get from sensor
		numberOfSamples = writePointer - readPointer;
		if (numberOfSamples < 0)
			numberOfSamples += 32; //Wrap condition

		//We now have the number of readings, now calc bytes to read
		//For this example we are just doing Red and IR (3 bytes each)
		int bytesLeftToRead = numberOfSamples * activeLEDs * 3;

		//Get ready to read a burst of data from the FIFO register

		/*
    _i2cPort->beginTransmission(MAX30105_ADDRESS);
    _i2cPort->write(MAX30105_FIFODATA);
    _i2cPort->endTransmission();
		 */
		//We may need to read as many as 288 bytes so we read in blocks no larger than I2C_BUFFER_LENGTH
		//I2C_BUFFER_LENGTH changes based on the platform. 64 bytes for SAMD21, 32 bytes for Uno.
		//Wire.requestFrom() is limited to BUFFER_LENGTH which is 32 on the Uno
		while (bytesLeftToRead > 0)
		{
			//clearFIFO();
			int toGet = bytesLeftToRead;
			if (toGet > I2C_BUFFER_LENGTH)
			{
				//If toGet is 32 this is bad because we read 6 bytes (Red+IR * 3 = 6) at a time
				//32 % 6 = 2 left over. We don't want to request 32 bytes, we want to request 30.
				//32 % 9 (Red+IR+GREEN) = 5 left over. We want to request 27.

				toGet = I2C_BUFFER_LENGTH - (I2C_BUFFER_LENGTH % (activeLEDs * 3)); //Trim toGet to be a multiple of the samples we need to read
			}

			bytesLeftToRead -= toGet;
			uint8_t size = (uint8_t)toGet;
			cont = 0;
			//Request toGet number of bytes from sensor
			//WriteRegister(MAX30105_ADDRESS, MAX30105_FIFOWRITEPTR, 0);
			//clearFIFO();
			//WriteRegister(MAX30105_ADDRESS, MAX30105_FIFOREADPTR, 0);
			//WriteRegister(MAX30105_ADDRESS, MAX30105_FIFOOVERFLOW, 0);
			Readmul(MAX30105_ADDRESS, MAX30105_FIFODATA, size, sampleo);

			while (toGet > 0)
			{

				/*
				for(int i=0;i<size;i++){

					//int value=Readmul(MAX30105_ADDRESS,MAX30105_FIFODATA,27,sampleo);
						int value=sampleo[i];

					printf("valor de dato%i  :  %i\r\n",i,value);

					}
				 */

				//delay(18);
				//delayInaccurateMs(80);

				psense->head++;				  //Advance the head of the storage struct
				psense->head %= STORAGE_SIZE; //Wrap condition

				uint32_t temp[sizeof(uint32_t)]; //Array of 4 bytes that we will convert into long
				uint32_t templong = 0;

				//Burst read three bytes - RED
				temp[3] = 0;
				temp[2] = (uint32_t)sampleo[cont] << 16;
				cont++;
				temp[1] = (uint32_t)sampleo[cont] << 8;
				cont++;
				temp[0] = (uint32_t)sampleo[cont];
				cont++;

				//while(!delayRead(&delayt));
				/*
				for(int i=0;i<200;i++){
					printf(" ");
				}
				 */

				templong |= temp[0] | temp[1] | temp[2] | temp[3];
				//printf("%i\r\n",templong);
				//printf("RED   valor de dato  :  %i\r\n",templong);
				//Convert array to long
				//memcpy(&tempLong, temp, sizeof(tempLong));
				//tempLong|= temp[3]<<24 | temp[2]<<16 | temp[1]<<8 | temp[0];
				//printf("valor de dato  %i  :  \r\n",longt(temp));
				templong &= 0x3FFFF; //Zero out all but 18 bits
				//printf("RED valor leido del sensor  %i\r\n",templong);
				//printf("RED   valor de dato  :  %i\r\n", templong);
				psense->red[psense->head] = templong; //Store this reading into the sense array

				if (activeLEDs > 1)
				{

					//Burst read three more bytes - IR
					temp[3] = 0;
					temp[2] = (uint32_t)sampleo[cont] << 16;
					cont++;
					temp[1] = (uint32_t)sampleo[cont] << 8;
					cont++;
					temp[0] = (uint32_t)sampleo[cont];
					cont++;
					/*
					for(int x=0;x<4;x++){

						printf("IR    valor de dato%i  :  %i\r\n",x,temp[x]);

					}
					 */
					templong |= temp[0] | temp[1] | temp[2] | temp[3];
					//Convert array to long
					//memcpy(&tempLong, temp, sizeof(tempLong));
					//	tempLong=temp[3]<<24|temp[2]<<16|temp[1]<<8|temp[0];
					//	printf("valor de dato%i  :  \r\n",tempLong);
					templong &= 0x3FFFF; //Zero out all but 18 bits
					//	printf("valor de dato  : %i \r\n",templong);
					psense->IR[psense->head] = templong;
					//printf("valor de dato  : %i \r\n",psense->IR[psense->head]);
				}

				if (activeLEDs > 2)
				{
					//Burst read three more bytes - Green
					temp[3] = 0;
					temp[2] = (uint32_t)sampleo[cont] << 16;
					cont++;
					temp[1] = (uint32_t)sampleo[cont] << 8;
					cont++;
					temp[0] = (uint32_t)sampleo[cont];
					cont++;
					/*
					for(int y=0;y<4;y++){

						printf("GRENN valor de dato%i  :  %i\r\n",y,temp[y]);

					}
					 */
					templong |= temp[0] | temp[1] | temp[2] | temp[3];
					//Convert array to long
					//	tempLong=temp[3]<<24|temp[2]<<16|temp[1]<<8|temp[0];
					//memcpy(&tempLong, temp, sizeof(tempLong));
					//	printf("valor de dato%i  :  \r\n",tempLong);
					templong &= 0x3FFFF; //Zero out all but 18 bits

					psense->green[psense->head] = templong;
				}

				toGet -= activeLEDs * 3;
			}

			//clearFIFO();
		} //End while (bytesLeftToRead > 0)

	} //End readPtr != writePtr

	//return (numberOfSamples); //Let the world know how much new data we found
	return TRUE;
}

//Check for new data but give up after a certain amount of time
//Returns true if new data was found
//Returns false if new data was not found
bool_t safeCheck(uint8_t maxTimeToCheck, sense_struct *psense)
{
	//delayWrite(&delayt,maxTimeToCheck);

	while (1)
	{
		//if(delayRead(&delayt)) return FALSE;
		if (check(psense) == TRUE) //We found new data!
			return (TRUE);
		//delay(1);
	}
	//return FALSE;
}
