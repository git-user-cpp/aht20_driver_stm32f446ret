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
 * @file character_generator.c
 * @brief Implements character generation and transmission logic for 7-segment displays.
 *
 * This module provides functions to initialize and control a 7-segment display by converting ASCII characters
 * to segment patterns and transmitting them via an SPI interface. It uses a lookup table to map supported
 * characters to their 7-segment encodings and handles period (dot) control for each digit.
 */

/**
 * @addtogroup Character_Generator
 * @{
 */

#include <assert.h>
#include <stddef.h>
#include "driver_7_seg.h"
#include "character_generator.h"

/**
 * @brief Number of segments (digits) in the display.
 *
 * Defines the number of digits (4) supported by the 7-segment display.
 */
static const uint8_t DIGIT_NUM = 4;

/**
 * @enum chars
 * @brief Defines 7-segment display encodings for supported characters.
 *
 * Each enum value represents an 8-bit pattern for a character on a 7-segment display:
 * - Bit 7: Decimal point (0 = on, 1 = off).
 * - Bits 6-0: Segments G, F, E, D, C, B, A (0 = on, 1 = off).
 *
 * Segment layout:
 * @verbatim
 *     --- A ---
 *    |         |
 *    F         B
 *    |         |
 *     --- G ---
 *    |         |
 *    E         C
 *    |         |
 *     --- D ---
 * @endverbatim
 *
 * @note Uses `uint8_t` as the underlying type (GNU99 or later) to ensure 1-byte size.
 */
typedef enum : uint8_t	/* despite of the fact that it shows syntax error it's legal in GNU99 and later standards */
{
    INVALID_CHAR = 0xFF, 	/**< 		  Invalid character. All segments off (11111111). */
    ZERO    	 = 0xC0, 	/**< '0': 	  Segments A, B, C, D, E, F on (11000000). */
    ONE     	 = 0xF9, 	/**< '1': 	  Segments B, C on (11111001). */
    TWO     	 = 0xA4, 	/**< '2': 	  Segments A, B, D, E, G on (10100100). */
    THREE   	 = 0xB0,	/**< '3': 	  Segments A, B, C, D, G on (10110000). */
    FOUR    	 = 0x99, 	/**< '4': 	  Segments B, C, F, G on (10011001). */
    FIVE    	 = 0x92, 	/**< '5': 	  Segments A, C, D, F, G on (10010010). */
    SIX     	 = 0x82, 	/**< '6': 	  Segments A, C, D, E, F, G on (10000010). */
    SEVEN   	 = 0xF8, 	/**< '7': 	  Segments A, B, C on (11111000). */
    EIGHT   	 = 0x80, 	/**< '8': 	  All segments on (10000000). */
    NINE    	 = 0x90, 	/**< '9': 	  Segments A, B, C, D, F, G on (10010000). */
	H_CHAR		 = 0x89, 	/** 10001001 */
	F_CHAR		 = 0x8E,	/** 10001110 */
	C_CHAR		 = 0xC6,	/** 11000110 */
    DASH    	 = 0xBF, 	/**< '-': 	  Segment G on (10111111). */
} chars;

/**
 * @struct char_mapping_t
 * @brief Maps an ASCII character to its 7-segment display code.
 *
 * Defines a packed structure to associate an ASCII character with its corresponding 7-segment code.
 *
 * @note Size: 2 bytes (1 byte for `ch`, 1 byte for `code` due to `__attribute__((packed))`).
 */
typedef struct
{
    char ch; 	/**< ASCII character to display (e.g., '0', 'A', 't'). */
    chars code;	/**< Corresponding 7-segment code from the chars enum. */
} char_mapping_t;

/**
 * @brief Lookup table for ASCII characters and their 7-segment encodings.
 *
 * Maps supported ASCII characters to their 7-segment display codes.
 * Supported characters: '0'-'9', 'A', 'a', 'T', 't', 'P', 'p', '-'.
 *
 * @note Size: 34 bytes (17 entries × 2 bytes per entry).
 */
static const char_mapping_t char_mappings[] =
{
    {'0', ZERO},
    {'1', ONE},
    {'2', TWO},
    {'3', THREE},
    {'4', FOUR},
    {'5', FIVE},
    {'6', SIX},
    {'7', SEVEN},
    {'8', EIGHT},
    {'9', NINE},
	{'H', H_CHAR},
	{'h', H_CHAR},
	{'F', F_CHAR},
	{'f', F_CHAR},
	{'C', C_CHAR},
	{'c', C_CHAR},
    {'-', DASH},
};

/**
 * @brief Character generator API structure.
 *
 * Provides function pointers for initializing and transmitting data to the 7-segment display.
 */
const char_gen_api_t api_char_gen =
{
    .init = char_gen_init,			/**< Pointer to initialization function. */
    .transmit = char_gen_transmit,	/**< Pointer to transmission function. */
};

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
                                      GPIO_TypeDef *const GPIOx, const uint16_t GPIO_Pin)
{
	assert(hspi != NULL);
	assert(htim != NULL);
	assert(GPIOx != NULL);

    if (api_7_seg.init(hspi, htim, GPIOx, GPIO_Pin) != DRIVER_7_SEG_STATUS_OK)
    {
        return CHAR_GEN_STATUS_NOT_INITIALIZED;
    }

    return CHAR_GEN_STATUS_OK;
}

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
char_generator_status_t char_gen_transmit(const char_gen_data_t *const config)
{
    assert(config != NULL);

    uint16_t out_data[DIGIT_NUM];

    for (uint8_t i = 0; i < DIGIT_NUM; ++i)
    {
        chars code = INVALID_CHAR;
        for (size_t j = 0; j < (sizeof(char_mappings) / sizeof(char_mappings[0])); ++j)
        {
            if (char_mappings[j].ch == config->digits[i])
            {
                code = char_mappings[j].code;
                break;
            }
        }

        uint8_t segment_data = (uint8_t)code;
        if (config->periods[i] == PERIOD_ON)
        {
            segment_data &= ~(1 << 7);
        }

        out_data[i] = (uint16_t)(segment_data << 8) | (1 << i);
    }

    if (api_7_seg.send_buffer(out_data, config->brightness, DIGIT_NUM) != DRIVER_7_SEG_STATUS_OK)
    {
        return CHAR_GEN_STATUS_NOT_TRANSMITTED;
    }

    return CHAR_GEN_STATUS_OK;
}

/** @} */ // end of Character_Generator
