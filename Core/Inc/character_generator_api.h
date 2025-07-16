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
 * @file character_generator_api.h
 * @brief Public interface for the 7-segment character generator module.
 *
 * This header defines the API for initializing and controlling a 7-segment display.
 * It provides data structures and function prototypes to configure the display with
 * characters, decimal points, and brightness settings, using an SPI interface for communication.
 */

/**
 * @addtogroup Character_Generator_API
 * @{
 */

#pragma once
#include <stdint.h>
#include "main.h"
#include "driver_7_seg.h"

/**
 * @enum char_generator_status_t
 * @brief Status codes returned by character generator functions.
 *
 * Indicates the outcome of initialization or transmission operations.
 */
typedef enum
{
	CHAR_GEN_STATUS_OK = 1,				/**< Operation completed successfully. */
	CHAR_GEN_STATUS_INVALID_PARAMETERS,	/**< Invalid input parameters (e.g., NULL pointer or unsupported characters). */
	CHAR_GEN_STATUS_NOT_INITIALIZED,	/**< Initialization failed or was not performed. */
	CHAR_GEN_STATUS_NOT_TRANSMITTED,	/**< Data transmission to the display failed. */
} char_generator_status_t;

/**
 * @enum period_status
 * @brief Represents the on/off state of the decimal point (dot) segment for a digit.
 *
 * Controls whether the decimal point is displayed for each digit on the 7-segment display.
 */
typedef enum
{
	PERIOD_ON,	/**< Decimal point is on for the digit. */
	PERIOD_OFF,	/**< Decimal point is off for the digit. */
} period_status;

/**
 * @struct char_gen_config_t
 * @brief Configuration for transmitting 4 digits to the 7-segment display.
 *
 * Holds the characters to display, their decimal point statuses, and the brightness setting.
 */
typedef struct {
	char *digits;										/**< Pointer to a null-terminated string of 4 characters to display. */
	const period_status *const periods;					/**< Pointer to an array of 4 period_status values for decimal points. */
	const driver_7_seg_brightness_t *const brightness;	/**< Pointer to the brightness setting for the display. */
} char_gen_data_t;

/**
 * @struct char_gen_api_t
 * @brief API for character generator functions to initialize and transmit data to a 7-segment display.
 *
 * Provides function pointers for initializing the module and transmitting character data.
 */
typedef struct
{
	/**
	 * @brief Initializes the character generator module and its underlying 7-segment driver.
	 *
	 * Configures the SPI, timer, and GPIO settings for communication with the 7-segment display.
	 *
	 * @param hspi Pointer to the SPI handle for communication with the display.
	 * @param htim Pointer to the timer handle for timing/control signals.
	 * @param GPIOx Pointer to the GPIO port for the latch/control signal.
	 * @param GPIO_Pin The GPIO pin number for the latch/control signal.
	 * @return char_generator_status_t The initialization status:
	 *         - CHAR_GEN_STATUS_OK if successful.
	 *         - CHAR_GEN_STATUS_NOT_INITIALIZED if the driver initialization fails.
	 * @note Must be called before any transmit operations.
	 */
	char_generator_status_t (*init)(SPI_HandleTypeDef *const hspi, TIM_HandleTypeDef *const htim,
			GPIO_TypeDef *const GPIOx, const uint16_t GPIO_Pin);

	/**
	 * @brief Transmits a 4-digit configuration to the 7-segment display.
	 *
	 * Converts the 4-character string and period statuses in the configuration to 7-segment codes
	 * and sends them to the display with the specified brightness.
	 *
	 * @param config Pointer to the configuration containing the 4-character string, period statuses,
	 *               and brightness setting.
	 * @return char_generator_status_t The transmission status:
	 *         - CHAR_GEN_STATUS_OK if successful.
	 *         - CHAR_GEN_STATUS_INVALID_PARAMETERS if config is NULL, digits is not 4 characters,
	 *           or contains unsupported characters.
	 *         - CHAR_GEN_STATUS_NOT_TRANSMITTED if the driver fails to send the data.
	 * @note Supported characters are: '0'-'9', 'A', 'a', 'T', 't', 'P', 'p', '-'.
	 * @note The digits string must be exactly 4 characters long, and periods must contain 4 elements.
	 */
	char_generator_status_t (*transmit)(const char_gen_data_t *const config);
} char_gen_api_t;

/** @} */ // end of Character_Generator_API
