
/***************************Includes***************************/

#include <stdbool.h>
#include "sht40.h"

/**************************************************************/

/***************************Static Variables***************************/

static uint8_t writeBuffer[3] = {0x00, 0x00, 0x00}; // definiert "writeBuffer"
static uint8_t readBuffer[10] = {0x00, 0x00};       // definiert "readBuffer"

/**************************************************************/

/***************************Extern Variables***************************/
sht4x_precision_t _precision = SHT4X_HIGH_PRECISION;
sht4x_heater_t _heater = SHT4X_NO_HEATER;
/**********************************************************************/

/*
 * @Author Muhammad Ahtasham Baig
 * @Parameter None
 * @return None
 * @Description This function is used to initialize communicate with Sht40
 */

void twi_init(void)
{
  int i2c_master_port = I2C_MASTER_NUM;

  i2c_config_t conf = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = I2C_MASTER_SDA_IO,
      .scl_io_num = I2C_MASTER_SCL_IO,
      .sda_pullup_en = GPIO_PULLUP_ENABLE,
      .scl_pullup_en = GPIO_PULLUP_ENABLE,
      .master.clk_speed = I2C_MASTER_FREQ_HZ,
  };

  i2c_param_config(i2c_master_port, &conf);

  (void)i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

/*
 * @Author Muhammad Ahtasham Baig
 * @Parameter None
 * @return None
 * @Description This function is used to initialize communicate with Sht40
 */

void SHT40_Init(void)
{
  int ret;
  twi_init();

  uint8_t write_buf[2] = {SHT4x_SOFTRESET, 1};

  ret = i2c_master_write_to_device(I2C_MASTER_NUM, SHT40_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
  vTaskDelay(10 / portTICK_PERIOD_MS);
  if (ret)
  {
  }
}

/*
 * @Author Muhammad Ahtasham Baig
 * @Parameter None
 * @return float humidity
 * @Description This function is used to read buffer from sht40 to global buffer
 */

void SHT40_Read(void)
{
  uint8_t reg[2] = {0x00};
  int err_code;
  uint8_t cmd = SHT4x_NOHEAT_HIGHPRECISION;
  uint16_t duration = 10;

  if (_heater == SHT4X_NO_HEATER)
  {
    if (_precision == SHT4X_HIGH_PRECISION)
    {
      cmd = SHT4x_NOHEAT_HIGHPRECISION;
      duration = 10;
    }
    if (_precision == SHT4X_MED_PRECISION)
    {
      cmd = SHT4x_NOHEAT_MEDPRECISION;
      duration = 5;
    }
    if (_precision == SHT4X_LOW_PRECISION)
    {
      cmd = SHT4x_NOHEAT_LOWPRECISION;
      duration = 2;
    }
  }

  if (_heater == SHT4X_HIGH_HEATER_1S)
  {
    cmd = SHT4x_HIGHHEAT_1S;
    duration = 1100;
  }
  if (_heater == SHT4X_HIGH_HEATER_100MS)
  {
    cmd = SHT4x_HIGHHEAT_100MS;
    duration = 110;
  }

  if (_heater == SHT4X_MED_HEATER_1S)
  {
    cmd = SHT4x_MEDHEAT_1S;
    duration = 1100;
  }
  if (_heater == SHT4X_MED_HEATER_100MS)
  {
    cmd = SHT4x_MEDHEAT_100MS;
    duration = 110;
  }

  if (_heater == SHT4X_LOW_HEATER_1S)
  {
    cmd = SHT4x_LOWHEAT_1S;
    duration = 1100;
  }
  if (_heater == SHT4X_LOW_HEATER_100MS)
  {
    cmd = SHT4x_LOWHEAT_100MS;
    duration = 110;
  }

  reg[0] = cmd;
  reg[1] = 1;

  err_code = i2c_master_write_to_device(I2C_MASTER_NUM, SHT40_ADDR, reg, sizeof(reg), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
  vTaskDelay(duration / portTICK_PERIOD_MS);

  /* Read 6 byte from the specified address */
  err_code = i2c_master_read_from_device(I2C_MASTER_NUM, SHT40_ADDR, &readBuffer, 6, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

/*
 * @Author Muhammad Ahtasham Baig
 * @Parameter None
 * @return float temperature
 * @Description This function is used to get temperature value from sht40
 */

float SHT40_temp_GetValue(void)
{
  float tempDataAnz;

  SHT40_Read();
  tempDataAnz = readBuffer[0] * 256 + readBuffer[1]; //
  tempDataAnz = -45 + 175 * tempDataAnz / 65535;     //

  return tempDataAnz;
} //

/*
 * @Author Muhammad Ahtasham Baig
 * @Parameter None
 * @return float humidity
 * @Description This function is used to get humidity value from sht40
 */

float SHT40_humy_GetValue(void)
{                     // "SHT40_humy_GetValue"
  float pressDataAnz; // definiert "pressDataAnz"

  SHT40_Read();                                       // ruft "SHT40_Read" auf
  pressDataAnz = readBuffer[3] * 256 + readBuffer[4]; //
  pressDataAnz = -6 + 125 * pressDataAnz / 65535;     //

  if (pressDataAnz > 100)
  {                     // wenn "pressDataAnz" gr��er als "100" ist
    pressDataAnz = 100; // "pressDataAnz" gleich 100
  }                     //
  if (pressDataAnz < 0)
  {                   // wenn "pressDataAnz" kleiner als "100" ist
    pressDataAnz = 0; // "pressDataAnz" gleich 0
  }                   //

  return pressDataAnz;
}

/*
 * @Author Muhammad Ahtasham Baig
 * @Parameter None
 * @return none
 * @Description Sets Precision of sht40
 */

void setPrecision(sht4x_precision_t precision)
{
  _precision = precision;
}

/*
 * @Author Muhammad Ahtasham Baig
 * @Parameter None
 * @return none
 * @Description Gets Precision of sht40
 */
sht4x_precision_t getPrecision(void)
{
  return _precision;
}

/*
 * @Author Muhammad Ahtasham Baig
 * @Parameter None
 * @return none
 * @Description Get Heater settings of sht40
 */

sht4x_heater_t getHeater(void)
{
  return _heater;
}
/*
 * @Author Muhammad Ahtasham Baig
 * @Parameter None
 * @return none
 * @Description Set Heater settings of sht40
 */

void setHeater(sht4x_heater_t heat)
{
  _heater = heat;
}