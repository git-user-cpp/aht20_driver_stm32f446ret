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

#pragma once

/*
 * enum for status returns
 */
typedef enum {
	AHT20_STATUS_OK = 1,
	AHT20_STATUS_NOT_TRANSMITTED,
	AHT20_STATUS_NOT_RECEIVED,
	AHT20_STATUS_NOT_MEASURED,
} aht20_status_t;

/*
 * api for aht20 sensor
 */
typedef struct {
	aht20_status_t (*aht20_validate_calibration) (I2C_HandleTypeDef *hi2c);
	aht20_status_t (*measure) (I2C_HandleTypeDef *hi2c, uint8_t *measured_data, uint16_t measured_data_size);
	void (*calculate_measurments) (uint8_t *measured_data, float *humidity, float *temp_c, float *temp_f);
	aht20_status_t (*soft_reset) (I2C_HandleTypeDef *hi2c);
} aht20_sensor_api_t;
