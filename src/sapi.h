#include <stdio.h>
#include "stdint.h"
#include <string.h>
#include "stdbool.h"

#define TRUE 1
#define FALSE 0

//typedef's comunicacion i2c
typedef uint8_t i2cMap_t;
typedef uint8_t bool_t;
//

//typedef's delay
typedef uint64_t tick_t;

typedef uint32_t delay_t;
//

//funcion de inicializacion con comunicacion i2c
bool_t i2cInit(i2cMap_t i2cNumber, uint32_t clockRateHz);
//

//funcion de escritura de datos en los registros de comunicacion i2c
bool_t i2cWrite(i2cMap_t i2cNumber,
                uint8_t i2cSlaveAddress,
                uint8_t *transmitDataBuffer,
                uint16_t transmitDataBufferSize,
                bool_t sendWriteStop);
//

//funcion de lectura de datos de los registros de comuncacion i2c
bool_t i2cRead(i2cMap_t i2cNumber,
               uint8_t i2cSlaveAddress,
               uint8_t *dataToReadBuffer,
               uint16_t dataToReadBufferSize,
               bool_t sendWriteStop,
               uint8_t *receiveDataBuffer,
               uint16_t receiveDataBufferSize,
               bool_t sendReadStop);
//

//funciones para manejo de delay's no bloqueantes
void delayInit( delay_t * delay, tick_t duration );
bool_t delayRead( delay_t * delay );
void delayWrite( delay_t * delay, tick_t duration );
//