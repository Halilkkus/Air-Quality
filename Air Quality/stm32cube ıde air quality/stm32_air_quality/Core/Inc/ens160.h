/*
 * ens160.h
 *
 *  Created on: Jun 11, 2023
 *      Author: Ayigitler
 */

#ifndef INC_ENS160_H_
#define INC_ENS160_H_

#include "stm32f4xx_hal.h"

#define POLY   uint8_t(0x1D)   ///< 0b00011101 = x^8+x^4+x^3+x^2+x^0 (x^8 is implicit)
#define ENS160_PART_ID   0x160   ///< ENS160 chip version

/* ENS160 register address */
#define ENS160_PART_ID_REG     0x00   ///< This 2-byte register contains the part number in little endian of the ENS160.

#define ENS160_OPMODE_REG      0x10   ///< This 1-byte register sets the Operating Mode of the ENS160.
#define ENS160_CONFIG_REG      0x11  ///< This 1-byte register configures the action of the INTn pin.
#define ENS160_COMMAND_REG     0x12   ///< This 1-byte register allows some additional commands to be executed on the ENS160.

#define ENS160_TEMP_IN_REG     0x13   ///< This 2-byte register allows the host system to write ambient temperature data to ENS160 for compensation.
#define ENS160_RH_IN_REG       0x15   ///< This 2-byte register allows the host system to write relative humidity data to ENS160 for compensation.

#define ENS160_DATA_STATUS_REG 0x20  ///< This 1-byte register indicates the current STATUS of the ENS160.

#define ENS160_DATA_AQI_REG    0x21   ///< This 1-byte register reports the calculated Air Quality Index according to the UBA.
#define ENS160_DATA_TVOC_REG   0x22   ///< This 2-byte register reports the calculated TVOC concentration in ppb.
#define ENS160_DATA_ECO2_REG   0x24   ///< This 2-byte register reports the calculated equivalent CO2-concentration in ppm, based on the detected VOCs and hydrogen.
#define ENS160_DATA_ETOH_REG   0x22   ///< This 2-byte register reports the calculated ethanol concentration in ppb.

#define ENS160_DATA_T_REG      0x30   ///< This 2-byte register reports the temperature used in its calculations (taken from TEMP_IN, if supplied).
#define ENS160_DATA_RH_REG     0x32   ///< This 2-byte register reports the relative humidity used in its calculations (taken from RH_IN if supplied).

#define ENS160_DATA_MISR_REG   0x38   ///< This 1-byte register reports the calculated checksum of the previous DATA_ read transaction (of n-bytes).

#define ENS160_GPR_WRITE_REG   0x40   ///< This 8-byte register is used by several functions for the Host System to pass data to the ENS160.
#define ENS160_GPR_READ_REG    0x48   ///< This 8-byte register is used by several functions for the ENS160 to pass data to the Host System.

/* CMD(0x12) register command */
#define ENS160_COMMAND_NOP          uint8_t(0x00)   ///< reserved. No command.
#define ENS160_COMMAND_GET_APPVER   uint8_t(0x0E)   ///< Get FW Version Command.
#define ENS160_COMMAND_CLRGPR       uint8_t(0xCC)   ///< Clears GPR Read Registers Command.

/* OPMODE(Address 0x10) register mode */
#define ENS160_SLEEP_MODE      uint8_t(0x00)   ///< DEEP SLEEP mode (low power standby).
#define ENS160_IDLE_MODE       uint8_t(0x01)   ///< IDLE mode (low-power).
#define ENS160_STANDARD_MODE   0x02   ///< STANDARD Gas Sensing Modes.

/* Convenience Macro */
#define ENS160_CONCAT_BYTES(msb, lsb)   (((uint16_t)msb << 8) | (uint16_t)lsb)   ///< Macro combines two 8-bit data into one 16-bit data


#define ENSE160_ADDR (0x53<<1) // ENSE160 sensörünün I2C adresi

typedef enum
  {
    eINTPinActiveLow = 0<<6,   /**< Active low */
    eINTPinActiveHigh = 1<<6,   /**< Active high */
  }eINTPinActive_t;

  /**
   * @enum  eINTPinMode_t
   * @brief  Interrupt pin output driving mode
   */
  typedef enum
  {
    eINTPinOD = 0<<5,   /**<  Open drain */
    eINTPinPP = 1<<5,   /**< Push / Pull */
  }eINTPinMode_t;

  /**
   * @enum  eIntGprDrdy_t
   * @brief  The status of interrupt pin when new data appear in General Purpose Read Registers
   */
  typedef enum
  {
    eIntGprDrdyDIS = 0<<3,   /**< Disable */
    eIntGprDrdyEN = 1<<3,   /**< Enable */
  }eIntGprDrdy_t;

  /**
   * @enum  eINTDataDrdy_t
   * @brief  The status of interrupt pin when new data appear in DATA_XXX
   */
  typedef enum
  {
    eINTDataDrdyDIS = 0<<1,   /**< Disable */
    eINTDataDrdyEN = 1<<1,   /**< Enable */
  }eINTDataDrdy_t;

  /**
   * @enum  eINTMode_t
   * @brief  Interrupt pin main switch mode
   */
  typedef enum
  {
    eINTModeDIS = 0,   /**< Disable */
    eINTModeEN = 1,   /**< Enable */
  }eINTMode_t;

  /**
   * @enum  eSensorStatus_t
   * @brief  The sensor status
   */
  typedef enum
  {
    eNormalOperation = 0,   /**< Normal operation */
    eWarmUpPhase = 1,   /**< Warm-Up phase */
    eInitialStartUpPhase = 2,   /**< Initial Start-Up phase */
    eInvalidOutput = 3,   /**< Invalid output */
  }eSensorStatus_t;

/************************* Sensor Status *******************************/
  /**
   * @struct sSensorStatus_t
   * @brief Sensor status flag is buffered into "DATA_STATUS (Address 0x20)" register
   * @note Register structure:
   * @n -----------------------------------------------------------------------------------
   * @n |    b7    |   b6   |    b5   |    b4   |    b3   |    b2   |    b1    |    b0    |
   * @n -----------------------------------------------------------------------------------
   * @n |  STATAS  | STATER |     reserved      |   VALIDITY FLAG   |  NEWDAT  |  NEWGPR  |
   * @n -----------------------------------------------------------------------------------
   */
  typedef struct
  {
    uint8_t   GPRDrdy: 1; /**< General purpose register data ready */
    uint8_t   dataDrdy: 1; /**< Measured data ready */
    uint8_t   validityFlag: 2; /**< 0: Normal operation, 1: Warm-Up phase, 2: Initial Start-Up phase, 3: Invalid output */
    uint8_t   reserved: 2; /**< Reserved bit */
    uint8_t   stater: 1; /**< High indicates that an error is detected. E.g. Invalid Operating Mode has been selected. */
    uint8_t   status: 1; /**< High indicates that an OPMODE is running */
  } __attribute__ ((packed)) sSensorStatus_t;


void ENSE160_Init(void);
uint8_t getAQI(void);
uint16_t getTVOC(void);
uint16_t getECO2(void);
uint16_t getID(void);

#endif /* INC_ENS160_H_ */
