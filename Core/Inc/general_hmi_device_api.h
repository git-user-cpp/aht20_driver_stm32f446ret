/*
 * digital_thermomether
 * digital thermometer built using stm32f446ret, AHT20 sensor and multi-function shield
 * Copyright (C) 2025 Andrew Kushyk, Eldar Vanin
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
#include <stdbool.h>
#include <stdint.h>

#include "main.h"

/// @brief HMI Events status codes
typedef enum {
	HMI_NO_EVENT = 0, /// HMI no event code
	HMI_SHORT_EVENT, /// HMI short event code
} HMI_Interact_Status_t;

/**
 * brief Interface for handling HMI device operation
 *
 * This structure defines functions used to
 * initialize and interact with HMI device
 */
typedef struct {
	/**
	 * @brief Initializes the HMI device
	 *
	 * @param[out] device Pointer to device instance
	 * @param[in] gpio_port GPIO port pointer
	 * @param gpio_pin GPIO pin
	 */
	void (*init)(void *device, GPIO_TypeDef* gpio_port, uint16_t gpio_pin);

	/**
	 * @brief Checks device status change
	 *
	 * @oaram[in] device Pointer to device instance
	 *
	 * @return Interaction status change
	 * @retval HMI_SHORT_EVENT On transition from deactivated state to activated state
	 * @retval HMI_LONG_EVENT On transition from activated state to long activation state
	 * @retval HMI_NO_EVENT When state doesn't change and on transition to deactivated state
	 */
	HMI_Interact_Status_t (*check_device_status_change)(void *device);

	/**
	 * @brief Checks device current status
	 *
	 * @param[in] device Pointer to device instance
	 *
	 * @return Current interaction status
	 * @retval HMI_NO_EVENT Device currently deactivated
	 * @retval HMI_SHORT_EVENT Device activated for a short time
	 * @retval HMI_LONG_EVENT Device activated for a long time
	 */
	HMI_Interact_Status_t (*check_device_current_status)(void *device);

	/**
	 * @brief Handles an external interrupt triggered by device
	 *
	 * @param gpio_pin GPIO pin that triggered the interrupt
	 */
	void (*device_interrupt_handle)(uint16_t gpio_pin);
} hmi_device_handler_t;
