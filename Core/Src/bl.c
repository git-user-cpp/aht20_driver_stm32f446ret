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
#include "bl.h"
#include "aht20.h"
#include "utils.h"

/*
 * holds sensor data
 */
static aht20_data_t sensor_data = {0};

/*
 * runs calibration check. if wasn't calibrated, calibrates the sensor
 */
bl_status_t bl_run_sensor(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart) {
	aht20_status_t status = AHT20_STATUS_OK;

	status = aht20_api.aht20_validate_calibration(hi2c);
	if (status != AHT20_STATUS_OK) {
#ifdef DEBUGGING
		print_error(huart, status);
#endif
		return BL_STATUS_RUN_FAILED;
	}

	return BL_STATUS_OK;
}

/*
 * processes and calculates sensor data
 */
bl_status_t bl_process_sensor_data(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart) {
	aht20_status_t status = AHT20_STATUS_OK;

	status = aht20_measure(hi2c, sensor_data.measured_data, (uint16_t)sizeof(sensor_data.measured_data));
	if (status != AHT20_STATUS_OK) {
#ifdef DEBUGGING
		print_error(huart, status);
#endif
		status = aht20_soft_reset(hi2c);
		if (status != AHT20_STATUS_OK) {
			return BL_STATUS_RUN_FAILED;
		}
	}

	aht20_calculate_measurments(sensor_data.measured_data, &sensor_data.humidity, &sensor_data.temperature_c, &sensor_data.temperature_f);

	return BL_STATUS_OK;
}

/*
 * transmits formatted data to UART
 */
void bl_uart_transmit_sensor_data(UART_HandleTypeDef *huart) {
	transmit_data(huart, sensor_data.humidity, sensor_data.temperature_c, sensor_data.temperature_f);
}
