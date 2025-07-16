/*
 * digital_thermomether
 * digital thermometer built using stm32f446ret, AHT20 sensor and multi-function shield
 * Copyright (C) 2025 Anatoliy Lizanets
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

/*
 Enum of possible statuses for the 7-segment driver.
 */
typedef enum
{
    DRIVER_7_SEG_STATUS_OK = 1,
    DRIVER_7_SEG_STATUS_NOT_INITIALIZED,
	DRIVER_7_SEG_STATUS_SEND_ERROR,
    DRIVER_7_SEG_STATUS_INVALID_PARAMETERS,
	DRIVER_7_SEG_STATUS_BUSY
} driver_7_seg_status_t;

/*
  Enum of brightness levels for the 7-segment display.
 */
typedef enum
{
	LEVEL_5_MAX = 0,
	LEVEL_4 = 1,
	LEVEL_3 = 5,
	LEVEL_2 = 15,
	LEVEL_1_MIN = 30,
	NOT_USED = 255,
} driver_7_seg_brightness_t;

/*
 * API interface which contains pointers to initialization and buffer sending functions.
 */
typedef struct
{
	 /*
	 Initializes the 7-segment display driver.
	 */
    driver_7_seg_status_t (*init)        (SPI_HandleTypeDef *const hspi, TIM_HandleTypeDef *const htim,
			 	 	 	 	 	 	 	  GPIO_TypeDef *const GPIOx, const uint16_t GPIO_Pin );
     /*
     Sends a data buffer to the display with specified brightness levels.
     */
    driver_7_seg_status_t (*send_buffer) (const uint16_t *const data, const driver_7_seg_brightness_t *const brightness_level, const uint8_t size);

} driver_7_seg_api_t;

