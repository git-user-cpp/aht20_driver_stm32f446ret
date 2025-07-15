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

#include "utils.h"
#include <stdio.h>
#include <string.h>

/*
 * prints error message via UART
 */
void print_error(UART_HandleTypeDef *huart, aht20_status_t status) {
    char debug_msg[64] = "\0";
    if (status == AHT20_STATUS_NOT_TRANSMITTED) {
        sprintf(debug_msg, "I2C initialization transmit error: 0x71\r\n");
    } else if (status == AHT20_STATUS_NOT_RECEIVED) {
        sprintf(debug_msg, "I2C initialization receive error: status_word\r\n");
    } else if (status == AHT20_STATUS_NOT_MEASURED) {
        sprintf(debug_msg, "I2C device couldn't perform measuring\r\n");
    } else if (status == AHT20_STATUS_OK) {
        return; // No action needed
    } else {
        sprintf(debug_msg, "Unknown error\r\n");
    }
    HAL_UART_Transmit(huart, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
}
