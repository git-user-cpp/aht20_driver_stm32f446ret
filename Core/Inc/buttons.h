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
#include "main.h"

// @brief Status codes for button press
typedef enum {
	BUTTON_RELEASED = 0, /// Button released status
	BUTTON_SHORT_PRESS, /// Button short pressed status
} Button_State;

// @brief GPIO port and pin for button
typedef struct {
	GPIO_TypeDef* port; /// GPIO port
	uint16_t pin; /// GPIO pin
} Button_Gpio;

// @brief Grouped button data
typedef struct Button {
	uint32_t press_time; /// Last button press time
	uint32_t release_time; /// Last button release time
	Button_State button_state; /// Last saved button state
	GPIO_PinState last_gpio_state; /// Last saved pin state
	Button_Gpio gpio; /// Button GPIO
	volatile struct Button *next_button; /// Pointer to next button in linked list
} Button;

/**
 * @brief Initializes button and adds it to linked list
 *
 * @param[out] button button to be initialized
 *
 * @param[in] gpio_pin GPIO pin that caused interrupt
 *
 * @param gpio_pin GPIO pin button connected to
 */
void button_init(volatile Button *button, GPIO_TypeDef* gpio_port, uint16_t gpio_pin);

/**
 * @brief Checks button and returns detected button state
 *
 * @param[in]button is a button to check
 *
 * @return detected button state
 */
Button_State check_button_state(volatile Button *button);

/**
 * @brief detect button GPIO state change using debouncing and save time when button pressed
 *
 * @param gpio_pin is a pin that caused interrupt
 */
void read_button(uint16_t gpio_pin);
