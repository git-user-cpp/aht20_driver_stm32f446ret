/*
 * digital_thermomether
 * digital thermometer built using stm32f446ret, AHT20 sensor and multi-function shield
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


/**
 * @file character_generator.h
 * @brief Declares the character generator module for 7-segment displays.
 *
 * This header provides the interface for initializing and controlling a 7-segment display.
 * It declares functions to initialize the module and transmit character data, using the API
 * defined in character_generator_api.h. The module converts ASCII characters to 7-segment
 * codes and manages decimal point and brightness settings for display output.
 */

/**
 * @addtogroup Character_Generator
 * @{
 */

#pragma once
#include "character_generator_api.h"

/**
 * @brief Extern declaration of the character generator API structure.
 *
 * Provides access to the module's function pointers for initialization and data transmission.
 * This variable is defined in character_generator.c and should be used to call the module's
 * functions from the application code.
 */
extern const char_gen_api_t api_char_gen;

/**
 * @brief Initializes the character generator module.
 *
 * Configures the 7-segment driver with the provided SPI, timer, and GPIO settings for communication
 * with the display hardware.
 *
 * @param[in] hspi Pointer to the SPI handle for communication with the display.
 * @param[in] htim Pointer to the timer handle for timing/control signals.
 * @param[in] GPIOx Pointer to the GPIO port for the latch/control signal.
 * @param[in] GPIO_Pin The GPIO pin number for the latch/control signal.
 * @return char_generator_status_t Status of initialization:
 *         - CHAR_GEN_STATUS_OK: Initialization successful.
 *         - CHAR_GEN_STATUS_NOT_INITIALIZED: Driver initialization failed.
 * @pre The driver API (`api_7_seg`) must be properly configured.
 * @note Must be called before any transmit operations.
 * @see char_gen_transmit
 */
char_generator_status_t char_gen_init(SPI_HandleTypeDef *const hspi, TIM_HandleTypeDef *const htim,
                                      GPIO_TypeDef *const GPIOx, const uint16_t GPIO_Pin);

/**
 * @brief Transmits a 4-digit configuration to the 7-segment display.
 *
 * Converts ASCII characters and period statuses from the configuration into 16-bit data for the
 * 7-segment display, where the upper 8 bits contain segment data (including decimal point) and
 * the lower 8 bits select the digit position (bit index set to 1). The data is sent via the driver API.
 *
 * @param[in] config Pointer to the configuration containing 4 digits and brightness setting.
 * @return char_generator_status_t Status of transmission:
 *         - CHAR_GEN_STATUS_OK: Transmission successful.
 *         - CHAR_GEN_STATUS_INVALID_PARAMETERS: `config` is NULL or contains invalid characters.
 *         - CHAR_GEN_STATUS_NOT_TRANSMITTED: Driver failed to send data.
 * @pre char_gen_init must be called successfully prior to transmission.
 * @note Supported characters: '0'-'9', 'A', 'a', 'T', 't', 'P', 'p', '-'.
 * @note The `config->digits` array must contain exactly 4 elements.
 * @note The `config->periods` array must contain valid `period_status` values (PERIOD_ON or PERIOD_OFF).
 * @see char_gen_init
 */
char_generator_status_t char_gen_transmit(const char_gen_data_t *const config);

/** @} */ // end of Character_Generator
