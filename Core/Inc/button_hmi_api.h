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

#include "general_hmi_device_api.h"
#include "buttons.h"

/**
 * @brief HMI device handler for button devices
 *
 * This structure uses hmi_device_handler_t interface
 * to define functions to initialize and interact with
 * button device
 *
 * - `b_hmi_init` - Initializes the button GPIO
 * - `b_hmi_check_button_state_change` - Checks button state on button state change
 * - `b_hmi_check_button_current_state` - Get current button state
 * - `read_button` - Handles GPIO interrupt triggered by button
 *
 * These functions are static and decribed in button_hmi_api.c file
 *
 * @see hmi_device_handler_t
 */
extern hmi_device_handler_t button_hmi_api;
