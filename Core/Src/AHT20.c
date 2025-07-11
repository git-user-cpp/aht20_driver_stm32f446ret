/*
 * aht20_driver_stm32f446ret
 * driver for aht20 temperature and humidity sensor
 * Copyright (C) 2025 Andrew Kushyk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "AHT20.h"
#include <stdio.h>
#include <string.h>

/*
 * device address for AHT20
 *
 * Datasheet: AHT20 Product manuals
 * 5.3 Send command
 */
static const uint16_t DEVICE_ADDRESS = (0x38 << 1);

/*
 * get status command. is needed for sending to device after power on
 *
 * Datasheet: AHT20 Product manuals
 * 5.4 Sensor reading process, paragraph 1
 */
static uint8_t GET_STATUS = 0x71;

/*
 * array for calibration initialization
 *
 * Datasheet: AHT20 Product manuals
 * 5.4 Sensor reading process, paragraph 1
 */
static uint8_t INIT_CMD[3] = {0xbe, 0x08, 0x00};

/*
 * array for measurment initialization
 *
 * Datasheet: AHT20 Product manuals
 * 5.4 Sensor reading process, paragraph 2
 */
static uint8_t MEASURE_CMD[3] = {0xac, 0x33, 0x00};

/*
 * sends reads status_word for further calibration verification
 *
 * Datasheet: AHT20 Product manuals
 * 5.3 Send command
 */
aht20_status_t aht20_get_calibration_status(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart, uint8_t *status_word, uint16_t status_word_size) {
	if (HAL_OK != HAL_I2C_Master_Transmit(hi2c, DEVICE_ADDRESS, &GET_STATUS, (uint16_t)sizeof(GET_STATUS), HAL_MAX_DELAY)) {
		return AHT20_STATUS_NOT_TRANSMITTED;
	}

	if (HAL_OK != HAL_I2C_Master_Receive(hi2c, DEVICE_ADDRESS, status_word, status_word_size, HAL_MAX_DELAY)) {
		return AHT20_STATUS_NOT_RECEIVED;
	}

	return AHT20_STATUS_OK;
}

/*
 * checks the 3rd bit of a received variable
 *
 * Datasheet: AHT20 Product manuals
 * 5.4 Sensor reading process, paragraph 1
 */
aht20_status_t aht20_check_calibration(uint8_t status_word) {
	if (status_word & (1 << 3)) {
		return AHT20_STATUS_OK;
	} else {
		return AHT20_STATUS_NOT_CALIBRATED;
	}
}

/*
 * sends an array of integers to trigger sensor calibration
 *
 * Datasheet: AHT20 Product manuals
 * 5.4 Sensor reading process, paragraph 1
 */
aht20_status_t aht20_calibrate(I2C_HandleTypeDef *hi2c, uint8_t status_word) {
	if (HAL_OK != HAL_I2C_Master_Transmit(hi2c, DEVICE_ADDRESS, INIT_CMD, 3, HAL_MAX_DELAY)) {
		return AHT20_STATUS_NOT_TRANSMITTED;
	}

	return AHT20_STATUS_OK;
}

/*
 * sends an array of integers to trigger sensor measurment
 *
 * Datasheet: AHT20 Product manuals
 * 5.4 Sensor reading process, paragraph 2
 */
aht20_status_t aht20_measure(I2C_HandleTypeDef *hi2c, uint8_t *measured_data) {
	if (HAL_OK != HAL_I2C_Master_Transmit(hi2c, DEVICE_ADDRESS, MEASURE_CMD, 3, HAL_MAX_DELAY)) {
		return AHT20_STATUS_NOT_TRANSMITTED;
	}
	HAL_Delay(80);

	uint8_t measuring_status = 0;
	HAL_I2C_Master_Receive(hi2c, DEVICE_ADDRESS, &measuring_status, 1, HAL_MAX_DELAY);

	if(measuring_status & (1 << 7)) {
		return AHT20_STATUS_NOT_MEASURED;
	}else{
		HAL_I2C_Master_Receive(hi2c, DEVICE_ADDRESS, measured_data, 6, HAL_MAX_DELAY);
	}

	return AHT20_STATUS_OK;
}

/*
 * calculates measured_data and writes the calculation in provided variables
 *
 * Datasheet: AHT20 Product manuals
 * 6.1 Relative humidity transformation
 * 6.2 Temperature transformation
 */
void aht20_calculate_measurments(uint8_t *measured_data, float *humidity, float *temp_c, float *temp_f) {
	uint32_t raw_humidity = ((measured_data[1] << 12) | (measured_data[2] << 4) | (measured_data[3] >> 4));
	uint32_t raw_temperature = (((measured_data[3] & 0x0F) << 16) | (measured_data[4] << 8) | measured_data[5]);

	*humidity = ((float)raw_humidity * 100.0) / 1048576.0; /* 2^20 */
	*temp_c = (((float)raw_temperature * 200.0) / 1048576.0) - 50.0;
	*temp_f = *temp_c * 9.0 / 5.0 + 32.0;
}
