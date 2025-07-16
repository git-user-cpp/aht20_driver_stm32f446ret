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

#include "button_hmi_api.h"

/**
 * @brief Initializes button and adds it to linked list
 *
 * @param[out] button button to be initialized
 *
 * @param[in] gpio_pin GPIO pin that caused interrupt
 *
 * @param gpio_pin GPIO pin button connected to
 */
static void b_hmi_init(void *button_ptr, GPIO_TypeDef* gpio_port, uint16_t gpio_pin);

/**
 * @brief Checks device status change
 *
 * @oaram[in] device Pointer to device instance
 *
 * @return Interaction status change
 * @retval HMI_SHORT_EVENT On transition from deactivated state to activated state
 * @retval HMI_LONG_EVENT On transition from activated state to long activation state
 * @retval HMI_NO_EVENT On transition to deactivated state
 */
static HMI_Interact_Status_t b_hmi_check_button_status_change(void *button_ptr);

/**
 * @brief Checks button current status
 *
 * @param[in] device Pointer to button instance
 *
 * @return Current interaction status
 * @retval HMI_NO_EVENT Button currently deactivated
 * @retval HMI_SHORT_EVENT Button activated for a short time
 * @retval HMI_LONG_EVENT Button activated for a long time
 */
static HMI_Interact_Status_t b_hmi_check_button_current_state(void *button_ptr);

hmi_device_handler_t button_hmi_api = {
		.init = b_hmi_init,
		.check_device_status_change = b_hmi_check_button_status_change,
		.check_device_current_status= b_hmi_check_button_current_state,
		.device_interrupt_handle = read_button
};

/**
 * @brief Convert Button_State status to HMI_Interact_Status_t status
 *
 * @param button_status Button_State to be converted
 *
 * @return converted HMI_Interact_Status_t
 * @retval HMI_NO_EVENT Variable button_status is equal BUTTON_RELEASED
 * @retval HMI_SHORT_EVENT Variable button_status is equal BUTTON_SHORT_PRESS
 * @retval HMI_LONG_EVENT Variable button_status is equal BUTTON_LONG_PRESS
 */
static HMI_Interact_Status_t convert_to_hmi_status(Button_State button_status);

/**
 * @brief Initializes button and adds it to linked list
 *
 * @param[out] button button to be initialized
 *
 * @param[in] gpio_pin GPIO pin that caused interrupt
 *
 * @param gpio_pin GPIO pin button connected to
 */
static void b_hmi_init(void *button_ptr, GPIO_TypeDef* gpio_port, uint16_t gpio_pin) {
	volatile Button *button = (volatile Button*) button_ptr;
	button_init(button, gpio_port, gpio_pin);
}

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
static HMI_Interact_Status_t b_hmi_check_button_status_change(void *button_ptr)
{
	HMI_Interact_Status_t result = HMI_NO_EVENT;
	volatile Button *button = (volatile Button*) button_ptr;

	Button_State previous_button_state = button -> button_state;
	Button_State current_button_state = check_button_state(button);

	// Check transition from released to pressed state
	if(previous_button_state == BUTTON_RELEASED && current_button_state == BUTTON_SHORT_PRESS) {
		result = HMI_SHORT_EVENT;
	}

	// Save current button state
	button->button_state = current_button_state;
	return result;
}

/**
 * @brief Checks button current status
 *
 * @param[in] device Pointer to button instance
 *
 * @return Current interaction status
 * @retval HMI_NO_EVENT Button currently deactivated
 * @retval HMI_SHORT_EVENT Button activated for a short time
 * @retval HMI_LONG_EVENT Button activated for a long time
 */
static HMI_Interact_Status_t b_hmi_check_button_current_state(void *button_ptr) {
	volatile Button *button = (volatile Button*) button_ptr;
	return convert_to_hmi_status(button->button_state);
}

/**
 * @brief Convert Button_State status to HMI_Interact_Status_t status
 *
 * @param button_status Button_State to be converted
 *
 * @return converted HMI_Interact_Status_t
 * @retval HMI_NO_EVENT Variable button_status is equal BUTTON_RELEASED
 * @retval HMI_SHORT_EVENT Variable button_status is equal BUTTON_SHORT_PRESS
 * @retval HMI_LONG_EVENT Variable button_status is equal BUTTON_LONG_PRESS
 */
static HMI_Interact_Status_t convert_to_hmi_status(Button_State button_status) {
	HMI_Interact_Status_t result = HMI_NO_EVENT;
	switch(button_status) {
		case BUTTON_RELEASED:
			result = HMI_NO_EVENT;
			break;
		case BUTTON_SHORT_PRESS:
			result = HMI_SHORT_EVENT;
			break;
	}
	return result;
}
