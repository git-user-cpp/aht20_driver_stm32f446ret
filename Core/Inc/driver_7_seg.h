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
#include "driver_7_seg_api.h"
#include <stdint.h>

extern const driver_7_seg_api_t api_7_seg;

/*
 Initializes the 7-segment display driver.
 */

driver_7_seg_status_t driver_7_seg_init( SPI_HandleTypeDef *const hspi, TIM_HandleTypeDef *const htim,
										 GPIO_TypeDef *const GPIOx, const uint16_t GPIO_Pin );
/*
 Sends a data buffer to the display with specified brightness levels.
 */
driver_7_seg_status_t driver_7_seg_send_buffer(const uint16_t *const data, const driver_7_seg_brightness_t *const brightness_level, const uint8_t size);




