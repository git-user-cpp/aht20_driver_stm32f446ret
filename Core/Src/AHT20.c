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
#include <math.h>
#include <assert.h>
#include <stddef.h>

/*
 * device address for AHT20
 *
 * Datasheet: AHT20 Product manuals
 * 5.3 Send command
 */
static const uint16_t DEVICE_ADDRESS = (0x38 << 1);

/*
 * performs soft resetting of the sensor
 *
 * Datasheet: AHT20 Product manuals
 * 5.5 Soft reset
 */
static uint8_t SOFT_RESET_CMD = 0xba;

/*
 * get status command. is needed for sending to device after power on
 *
 * Datasheet: AHT20 Product manuals
 * 5.4 Sensor reading process, paragraph 1
 */
static uint8_t GET_STATUS_CMD = 0x71;

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
 * acknowledge signal
 */
static uint8_t ACK_CMD = 0x06;

/*
 * not acknowledge signal
 */
static uint8_t NACK_CMD = 0x15;

/*
 * calculates crc8 for given data
 */
static uint8_t calculate_crc(uint8_t *data);

/*
 * sends reads status_word for further calibration verification
 *
 * Datasheet: AHT20 Product manuals
 * 5.3 Send command
 */
aht20_status_t aht20_get_calibration_status(I2C_HandleTypeDef *hi2c, uint8_t *status_word, uint16_t status_word_size) {
	assert(hi2c != NULL);
	assert(status_word != NULL);

	if (HAL_OK != HAL_I2C_Master_Transmit(hi2c, DEVICE_ADDRESS, &GET_STATUS_CMD, (uint16_t)sizeof(GET_STATUS_CMD), HAL_MAX_DELAY)) {
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
	assert(hi2c != NULL);

	if (HAL_OK != HAL_I2C_Master_Transmit(hi2c, DEVICE_ADDRESS, INIT_CMD, (uint16_t)sizeof(INIT_CMD), HAL_MAX_DELAY)) {
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
aht20_status_t aht20_measure(I2C_HandleTypeDef *hi2c, uint8_t *measured_data, uint16_t measured_data_size) {
	assert(hi2c != NULL);
	assert(measured_data != NULL);

	if (HAL_OK != HAL_I2C_Master_Transmit(hi2c, DEVICE_ADDRESS, MEASURE_CMD, (uint16_t)sizeof(MEASURE_CMD), HAL_MAX_DELAY)) {
		return AHT20_STATUS_NOT_TRANSMITTED;
	}
	HAL_Delay(80);

	if (HAL_OK != HAL_I2C_Master_Receive(hi2c, DEVICE_ADDRESS, measured_data, measured_data_size, HAL_MAX_DELAY)) {
		return AHT20_STATUS_NOT_RECEIVED;
	}

	if (measured_data[0] & (1 << 7)) {
		return AHT20_STATUS_NOT_MEASURED;
	}

	uint8_t calculated_crc = calculate_crc(measured_data);
	if (calculated_crc == measured_data[6]) {
		if (HAL_OK != HAL_I2C_Master_Transmit(hi2c, DEVICE_ADDRESS, &ACK_CMD, (uint16_t)sizeof(ACK_CMD), HAL_MAX_DELAY)) {
			return AHT20_STATUS_NOT_TRANSMITTED;
		}
	} else {
		if (HAL_OK != HAL_I2C_Master_Transmit(hi2c, DEVICE_ADDRESS, &NACK_CMD, (uint16_t)sizeof(NACK_CMD), HAL_MAX_DELAY)) {
			return AHT20_STATUS_NOT_TRANSMITTED;
		}

		aht20_soft_reset(hi2c);
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
	assert(measured_data != NULL);
	assert(humidity != NULL);
	assert(temp_c != NULL);
	assert(temp_f != NULL);

	uint32_t raw_humidity = ((measured_data[1] << 12) | (measured_data[2] << 4) | (measured_data[3] >> 4));
	uint32_t raw_temperature = (((measured_data[3] & 0x0F) << 16) | (measured_data[4] << 8) | measured_data[5]);

	*humidity = ((float)raw_humidity * 100.0) / 1048576.0; /* 2^20 */
	*temp_c = (((float)raw_temperature * 200.0) / 1048576.0) - 50.0;
	*temp_f = *temp_c * 9.0 / 5.0 + 32.0;
}

/*
 * resets the sensor without turning off the power supply
 *
 * Datasheet: AHT20 Product manuals
 * 5.5 Soft reset
 */
aht20_status_t aht20_soft_reset(I2C_HandleTypeDef *hi2c) {
	assert(hi2c != NULL);

	if (HAL_OK != HAL_I2C_Master_Transmit(hi2c, DEVICE_ADDRESS, &SOFT_RESET_CMD, (uint16_t)sizeof(SOFT_RESET_CMD), HAL_MAX_DELAY)) {
		return AHT20_STATUS_NOT_TRANSMITTED;
	}

	HAL_Delay(20);
	return AHT20_STATUS_OK;
}

/*
 * calculates crc8 for given data
 */
static uint8_t calculate_crc(uint8_t *data) {
	assert(data != NULL);

    uint8_t crc = 0xFF;
    uint8_t i = 0, j = 0;

    for (; i < 6; ++i) {
        crc ^= data[i];
        for (j = 0; j < 8; ++j) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}
