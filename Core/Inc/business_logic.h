/*
 * digital_thermomether
 * digital thermometer built using stm32f446ret, AHT20 sensor and multi-function shield
 * Copyright (C) 2025 Andrew Kushyk, Andrii Ostapchuk
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

#include "main.h"

/*
 * return statuses for business logic
 */
typedef enum {
	BL_STATUS_OK = 1,
	BL_STATUS_RUN_FAILED,
} bl_status_t;

/*
 * initializes buttons
 */
bl_status_t bl_init_buttons(void);

/*
 * runs calibration check. if wasn't calibrated, calibrates the sensor
 */
bl_status_t bl_run_sensor(I2C_HandleTypeDef *hi2c);

/*
 * processes and calculates sensor data
 */
bl_status_t bl_process_sensor_data(I2C_HandleTypeDef *hi2c);

/*
 * transmits formatted data to display via SPI
 */
void bl_spi_transmit_sensor_data(void);

/*
 * interrupt callback
 */
void HAL_GPIO_EXTI_Callback(uint16_t gpio_pin);
