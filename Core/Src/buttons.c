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

#include "buttons.h"
#include <stdbool.h>

/// Time needed to debounce in milliseconds
static const uint8_t DEBOUNCE_TIME_MS = 20;
/// Time needed to make long press

// entry to button linked list
static volatile Button *button_list_entry;

/**
 * @brief checks if there is a button connected to a gpio_pin
 *
 * @param gpio_pin GPIO pin that caused interrupt
 *
 * @return Button pointer which points to a button which caused interrupt
 */
static volatile Button* check_interrupt_pin(uint16_t gpio_pin);

/**
 * @brief Checks button and returns detected button state
 *
 * @param[in]button is a button to check
 *
 * @return detected button state
 */
Button_State check_button_state(volatile Button *button) {
	Button_State checked_button_state;

	// Current state needs to be checked
	// Because interrupt on rising edge sometimes don't work properly
	GPIO_PinState  current_button_pin_state =  HAL_GPIO_ReadPin(button->gpio.port, button->gpio.pin);

	// Check if button released
	if(button->last_gpio_state == GPIO_PIN_SET || current_button_pin_state == GPIO_PIN_SET) {
		checked_button_state = BUTTON_RELEASED;
	} else {
		checked_button_state = BUTTON_SHORT_PRESS;
	}

	// save button_state
	button->button_state = checked_button_state;
	return checked_button_state;
}

/**
 * @brief detect button GPIO state change using debouncing and save time when button pressed
 *
 * @param gpio_pin is a pin that caused interrupt
 */
void read_button(uint16_t gpio_pin) {
	// Determine which button was pressed
	volatile Button* button = check_interrupt_pin(gpio_pin);
	// Check if pressed button was found
	if(button != NULL) {
		uint32_t current_time = HAL_GetTick();
		// Debounce
		if(current_time - button->press_time >= DEBOUNCE_TIME_MS && current_time - button->release_time >= DEBOUNCE_TIME_MS) {
			// Save button GPIO state
			button->last_gpio_state = HAL_GPIO_ReadPin(button->gpio.port, button->gpio.pin);
			// If button pressed save button_press_time
			if(button->last_gpio_state == GPIO_PIN_RESET) {
				button->press_time = current_time;
			} else {
				button->release_time = current_time;
			}
		}
	}
}

/**
 * @brief Initializes button and adds it to linked list
 *
 * @param[out] button button to be initialized
 *
 * @param[in] gpio_pin GPIO pin that caused interrupt
 *
 * @param gpio_pin GPIO pin button connected to
 */
void button_init(volatile Button *button, GPIO_TypeDef* gpio_port, uint16_t gpio_pin)
{
	static volatile Button *next_button = NULL;

	button->button_state = (HAL_GPIO_ReadPin(gpio_port, gpio_pin) == GPIO_PIN_RESET) ? BUTTON_RELEASED : BUTTON_SHORT_PRESS;
	button->last_gpio_state = HAL_GPIO_ReadPin(gpio_port, gpio_pin);
	button->press_time = 0;
	button->release_time = 0;

	// initialize buttons GPIO
	button->gpio.port = gpio_port;
	button->gpio.pin = gpio_pin;

	// assign next button
	button->next_button = next_button;

	// set next button pointer to a current button
	next_button = button;

	/*
	 * button_list_entry variable is static global in buttons.c file
	 * used to save the start of linked button list
	 */
	button_list_entry = button;
}

/**
 * @brief checks if there is a button connected to a gpio_pin
 *
 * @param gpio_pin GPIO pin that caused interrupt
 *
 * @return Button pointer which points to a button which caused interrupt
 */
volatile Button* check_interrupt_pin(uint16_t gpio_pin) {
	/*
	 * button_list_entry variable is static global in buttons.c file
	 * used to save the start of linked button list
	 */
	volatile Button *button_to_return = button_list_entry;

	while(button_to_return->next_button != NULL) {
		if(button_to_return->gpio.pin != gpio_pin) {
			button_to_return = button_to_return->next_button;
		}
		else {
			break;
		}
	}


	return button_to_return;
}

