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
#include "main.h"
#include "aht20_api.h"

/*
 * struct for holding measurment data
 */
typedef struct {
	uint8_t measured_data[7];
	float humidity;
	float temperature_c;
	float temperature_f;
} aht20_data_t;

/*
 * making api public
 */
extern const aht20_sensor_api_t aht20_api;

/*
 * sends reads status_word for further calibration verification
 *
 * Datasheet: AHT20 Product manuals
 * 5.3 Send command
 */
aht20_status_t aht20_validate_calibration(I2C_HandleTypeDef *hi2c);

/*
 * sends an array of integers to trigger sensor measurment
 *
 * Datasheet: AHT20 Product manuals
 * 5.4 Sensor reading process, paragraph 2
 */
aht20_status_t aht20_measure(I2C_HandleTypeDef *hi2c, uint8_t *measured_data, uint16_t measured_data_size);

/*
 * resets the sensor without turning off the power supply
 *
 * Datasheet: AHT20 Product manuals
 * 5.5 Soft reset
 */
aht20_status_t aht20_soft_reset(I2C_HandleTypeDef *hi2c);

/*
 * calculates measured_data and writes the calculation in provided variables
 *
 * Datasheet: AHT20 Product manuals
 * 6.1 Relative humidity transformation
 * 6.2 Temperature transformation
 */
void aht20_calculate_measurments(uint8_t *measured_data, float *humidity, float *temp_c, float *temp_f);
