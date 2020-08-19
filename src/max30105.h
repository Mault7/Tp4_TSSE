/*
 * max30105.h
 *
 *  Created on: 22 may. 2020
 *      Author: mauri
 */

#ifndef PROGRAMAS_MAX30105_INC_MAX30105_H_
#define PROGRAMAS_MAX30105_INC_MAX30105_H_

#include "sapi.h"
#define I2C_BUFFER_LENGTH 32
#define STORAGE_SIZE 4 //Each long is 4 bytes so limit this to fit on your micro


uint8_t activeLEDs; //Gets set during setup. Allows check() to calculate how many bytes to read from FIFO
uint8_t revisionID;

typedef struct Record
{
	uint32_t red[STORAGE_SIZE];
	uint32_t IR[STORAGE_SIZE];
	uint32_t green[STORAGE_SIZE];
	uint8_t head;
	uint8_t tail;
} sense_struct; //This is our circular buffer of readings from the sensor


delay_t delayt;

uint8_t ReadRegister(uint8_t address, uint8_t reg);
uint8_t WriteRegister(uint8_t address, uint8_t reg, uint8_t value);
uint8_t bitmask(uint8_t reg, uint8_t mask, uint8_t content);

bool_t MAX30105Init(void);
uint32_t getRed(sense_struct *psense); //Returns immediate red value
uint32_t getIR(sense_struct *psense); //Returns immediate IR value
uint32_t getGreen(sense_struct *psense); //Returns immediate green value
bool_t safeCheck(uint8_t maxTimeToCheck,sense_struct *psense); //Given a max amount of time, check for new data

// Configuration
void softReset();
void shutDown();
void wakeUp();

void setLEDMode(uint8_t mode);

void setADCRange(uint8_t adcRange);
void setSampleRate(uint8_t sampleRate);
void setPulseWidth(uint8_t pulseWidth);

void setPulseAmplitudeRed(uint8_t value);
void setPulseAmplitudeIR(uint8_t value);
void setPulseAmplitudeGreen(uint8_t value);
void setPulseAmplitudeProximity(uint8_t value);

void setProximityThreshold(uint8_t threshMSB);

//Multi-led configuration mode (page 22)
void enableSlot(uint8_t slotNumber, uint8_t device); //Given slot number, assign a device to slot
void disableSlots(void);

// Data Collection

//Interrupts (page 13, 14)
uint8_t getINT1(void); //Returns the main interrupt group
uint8_t getINT2(void); //Returns the temp ready interrupt
void enableAFULL(void); //Enable/disable individual interrupts
void disableAFULL(void);
void enableDATARDY(void);
void disableDATARDY(void);
void enableALCOVF(void);
void disableALCOVF(void);
void enablePROXINT(void);
void disablePROXINT(void);
void enableDIETEMPRDY(void);
void disableDIETEMPRDY(void);

//FIFO Configuration (page 18)
void setFIFOAverage(uint8_t samples);
void enableFIFORollover();
void disableFIFORollover();
void setFIFOAlmostFull(uint8_t samples);

//FIFO Reading
uint16_t check(sense_struct *psense); //Checks for new data and fills FIFO
uint8_t available(sense_struct *psense); //Tells caller how many new samples are available (head - tail)
void nextSample(sense_struct *psense); //Advances the tail of the sense array
uint32_t getFIFORed(sense_struct *psense); //Returns the FIFO sample pointed to by tail
uint32_t getFIFOIR(sense_struct *psense); //Returns the FIFO sample pointed to by tail
uint32_t getFIFOGreen(sense_struct *psense); //Returns the FIFO sample pointed to by tail

uint8_t getWritePointer(void);
uint8_t getReadPointer(void);
void clearFIFO(void); //Sets the read/write pointers to zero

//Proximity Mode Interrupt Threshold
void setPROXINTTHRESH(uint8_t val);

// Die Temperature
float readTemperature();
float readTemperatureF();

// Detecting ID/Revision
uint8_t getRevisionID();
uint8_t readPartID();

// Setup the IC with user selectable settings
void setup(uint8_t powerLevel , uint8_t sampleAverage,uint8_t ledMode , int sampleRate , int pulseWidth , int adcRange );


#endif /* PROGRAMAS_MAX30105_INC_MAX30105_H_ */
