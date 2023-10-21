#include "ens160.h"

extern I2C_HandleTypeDef hi2c1;

void ENSE160_Init(void)
{
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
 //   Error_Handler();
  }




  HAL_StatusTypeDef status= HAL_I2C_IsDeviceReady(&hi2c1, ENSE160_ADDR, 1, 100);

  if(ENS160_PART_ID==getID())
  {
	  uint8_t buf[2];
	  buf[0] = ENS160_OPMODE_REG;
	  buf[1] = ENS160_STANDARD_MODE ;
	  HAL_I2C_Master_Transmit(&hi2c1, ENSE160_ADDR, buf, 2, HAL_MAX_DELAY);

	  buf[0] = ENS160_CONFIG_REG;
	  buf[1] = 0x00 | eINTDataDrdyEN | eIntGprDrdyDIS;
	  HAL_I2C_Master_Transmit(&hi2c1, ENSE160_ADDR, buf, 2, HAL_MAX_DELAY);


		  buf[0] = ENS160_CONFIG_REG;
		  buf[1] = 0x00 | eINTDataDrdyEN | eIntGprDrdyDIS;
		  HAL_I2C_Master_Transmit(&hi2c1, ENSE160_ADDR, buf, 2, HAL_MAX_DELAY);



		  uint16_t temp = (25 + 273.15) * 64;
		  uint16_t rh = 55 * 512;
		  uint8_t buf_a[6];

		  buf_a[0] = ENS160_TEMP_IN_REG;
		  buf_a[1] = temp & 0xFF;
		  buf_a[2] = (temp & 0xFF00) >> 8;
		  buf_a[3] = rh & 0xFF;
		  buf_a[4] = (rh & 0xFF00) >> 8;
		  HAL_I2C_Master_Transmit(&hi2c1, ENSE160_ADDR, buf_a,sizeof(buf_a), HAL_MAX_DELAY);

  }

 // HAL_I2C_Master_Transmit(&hi2c1, ENSE160_ADDR, ENS160_STANDARD_MODE, 1, 1000);
 // HAL_I2C_Mem_Read(hi2c, DevAddress, MemAddress, MemAddSize, pData, Size, Timeout)
}



uint8_t getAQI(void)
{
  uint8_t data = 0;
  HAL_I2C_Mem_Read(&hi2c1, ENSE160_ADDR,ENS160_DATA_AQI_REG, 1, &data,sizeof(data), 1000);
  return data;
}

uint16_t getTVOC(void)
{
  uint8_t buf[2];
 // HAL_I2C_Master_Transmit(&hi2c1, ENSE160_ADDR, ENS160_DATA_TVOC_REG, 1, 1000);
  HAL_I2C_Mem_Read(&hi2c1, ENSE160_ADDR,ENS160_DATA_TVOC_REG, 1,buf, sizeof(buf), 1000);
  return ENS160_CONCAT_BYTES(buf[1], buf[0]);
}

uint16_t getECO2(void)
{
  uint8_t buf[2];
  HAL_I2C_Mem_Read(&hi2c1, ENSE160_ADDR,ENS160_DATA_ECO2_REG, 1, buf ,sizeof(buf), HAL_MAX_DELAY);
  return ENS160_CONCAT_BYTES(buf[1], buf[0]);
}


uint16_t getID(void)
{
  uint8_t buf[7];

//  HAL_I2C_Mem_Read(&hi2c1, ENSE160_ADDR,ENS160_PART_ID_REG, buf, 1,sizeof(buf), 100);
  HAL_I2C_Master_Transmit(&hi2c1, ENSE160_ADDR, ENS160_PART_ID_REG, 1, 1000);
  HAL_I2C_Master_Receive(&hi2c1, ENSE160_ADDR, buf,7, 100);
 // HAL_I2C_Master_Receive(hi2c, DevAddress, pData, Size, Timeout)
  return ENS160_CONCAT_BYTES(buf[1], buf[0]);
}
