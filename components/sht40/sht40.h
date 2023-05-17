/* Microchip Technology Inc. and its subsidiaries.  You may use this software
 * and any derivatives exclusively with Microchip products.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
 * TERMS.
 */

/*
 * File:
 * Author:
 * Comments:
 * Revision history:
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef AB_SHT40_H
#define AB_SHT40_H

/***************************Includes***************************/
#include <string.h>
#include <stdio.h>

#include "esp_log.h"
#include "driver/i2c.h"
/**************************************************************/

/***************************Global Macros****************/

#define SHT40_ADDR 0x44

#define SHT4x_NOHEAT_HIGHPRECISION \
  0xFD /**< High precision measurement, no heater */
#define SHT4x_NOHEAT_MEDPRECISION \
  0xF6 /**< Medium precision measurement, no heater */
#define SHT4x_NOHEAT_LOWPRECISION \
  0xE0 /**< Low precision measurement, no heater */

#define SHT4x_HIGHHEAT_1S \
  0x39 /**< High precision measurement, high heat for 1 sec */
#define SHT4x_HIGHHEAT_100MS \
  0x32 /**< High precision measurement, high heat for 0.1 sec */
#define SHT4x_MEDHEAT_1S \
  0x2F /**< High precision measurement, med heat for 1 sec */
#define SHT4x_MEDHEAT_100MS \
  0x24 /**< High precision measurement, med heat for 0.1 sec */
#define SHT4x_LOWHEAT_1S \
  0x1E /**< High precision measurement, low heat for 1 sec */
#define SHT4x_LOWHEAT_100MS \
  0x15 /**< High precision measurement, low heat for 0.1 sec */

#define SHT4x_READSERIAL 0x89 /**< Read Out of Serial Register */
#define SHT4x_SOFTRESET 0x94  /**< Soft Reset */

#define I2C_MASTER_SCL_IO 22 /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO 21 /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM 0                        /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ 400000               /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0             /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0             /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 1000
/**************************************************************/

/***************************Enumerations**********************/

/** How precise (repeatable) the measurement will be */
typedef enum
{
  SHT4X_HIGH_PRECISION,
  SHT4X_MED_PRECISION,
  SHT4X_LOW_PRECISION,
} sht4x_precision_t;

/** Optional pre-heater configuration setting */
typedef enum
{
  SHT4X_NO_HEATER,
  SHT4X_HIGH_HEATER_1S,
  SHT4X_HIGH_HEATER_100MS,
  SHT4X_MED_HEATER_1S,
  SHT4X_MED_HEATER_100MS,
  SHT4X_LOW_HEATER_1S,
  SHT4X_LOW_HEATER_100MS,
} sht4x_heater_t;

/**************************************************************/

/***************************Extern Variables***************************/
extern sht4x_precision_t _precision;
extern sht4x_heater_t _heater;
/**********************************************************************/

/***************************Prototype Functions****************/

/*
 * @Author Muhammad Ahtasham Baig
 * @Parameter None
 * @return None
 * @Description This function is used to initialize communicate with Sht40
 */
void SHT40_Init(void);

/*
 * @Author Muhammad Ahtasham Baig
 * @Parameter None
 * @return float temperature
 * @Description This function is used to get temperature value from sht40
 */

float SHT40_temp_GetValue(void);

/*
 * @Author Muhammad Ahtasham Baig
 * @Parameter None
 * @return float humidity
 * @Description This function is used to get humidity value from sht40
 */
float SHT40_humy_GetValue(void);

/*
 * @Author Muhammad Ahtasham Baig
 * @Parameter None
 * @return float humidity
 * @Description This function is used to read buffer from sht40 to global buffer
 */

void SHT40_Read(void);

/*
 * @Author Muhammad Ahtasham Baig
 * @Parameter None
 * @return none
 * @Description Initializes TWI drivers for Two wire i2c
 */

void twi_init(void);

/*
 * @Author Muhammad Ahtasham Baig
 * @Parameter None
 * @return none
 * @Description Sets Precision of sht40
 */

void setPrecision(sht4x_precision_t prec);

/*
 * @Author Muhammad Ahtasham Baig
 * @Parameter None
 * @return none
 * @Description Gets Precision of sht40
 */
sht4x_precision_t getPrecision(void);

/*
 * @Author Muhammad Ahtasham Baig
 * @Parameter None
 * @return none
 * @Description Set Heater settings of sht40
 */

void setHeater(sht4x_heater_t heat);

/*
 * @Author Muhammad Ahtasham Baig
 * @Parameter None
 * @return none
 * @Description Get Heater settings of sht40
 */

sht4x_heater_t getHeater(void);

/**************************************************************/

#endif /* AB_SHT40_H */
