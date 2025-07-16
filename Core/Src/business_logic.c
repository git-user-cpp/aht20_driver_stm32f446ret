/*
 * digital_thermomether
 * digital thermometer built using stm32f446ret, AHT20 sensor and multi-function shield
 * Copyright (C) 2025 Andrew Kushyk, Andrii Ostapchuk
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

#include "business_logic.h"
#include "aht20.h"
#include "character_generator.h"
#include "button_hmi_api.h"
#include <stdio.h>

/*
 * holds event statuses
 */
typedef enum{
	EVENT_NONE,
	EVENT_BUTTON_A_SHORT,
	EVENT_BUTTON_B_SHORT,
}SystemEvent;

/*
 * holds display main states
 */
typedef enum{
	MAIN_STATE_DISPLAY_C,
	MAIN_STATE_DISPLAY_F,
	MAIN_STATE_DISPLAY_H,
	MAIN_STATE_ERROR_DISPLAY
}MainState;

/*
 * states config
 */
typedef struct {
	MainState currentMainState;
}bl_config;

/*
 * buttons definition
 */
static Button buttonA;
static Button buttonB;

/*
 * config declaration
 */
static bl_config config = {
		.currentMainState = MAIN_STATE_DISPLAY_C,
};

/*
 * holds sensor data
 */
static aht20_data_t sensor_data = {0};

/*
 * detects buttons interrupts
 */
static SystemEvent detect_events(void);

/*
 * initializes buttons
 */
bl_status_t bl_init_buttons(void) {
	button_hmi_api.init(&buttonA, BUTTON_S1_GPIO_Port, BUTTON_S1_Pin);
	button_hmi_api.init(&buttonB, BUTTON_S2_GPIO_Port, BUTTON_S2_Pin);

	return BL_STATUS_OK;
}

/*
 * runs calibration check. if wasn't calibrated, calibrates the sensor
 */
bl_status_t bl_run_sensor(I2C_HandleTypeDef *hi2c) {
	aht20_status_t status = AHT20_STATUS_OK;

	status = aht20_api.aht20_validate_calibration(hi2c);
	if (status != AHT20_STATUS_OK) {
		return BL_STATUS_RUN_FAILED;
	}

	return BL_STATUS_OK;
}

/*
 * processes and calculates sensor data
 */
bl_status_t bl_process_sensor_data(I2C_HandleTypeDef *hi2c) {
	aht20_status_t status = AHT20_STATUS_OK;

	status = aht20_measure(hi2c, sensor_data.measured_data, (uint16_t)sizeof(sensor_data.measured_data));
	if (status != AHT20_STATUS_OK) {
		status = aht20_soft_reset(hi2c);
		if (status != AHT20_STATUS_OK) {
			return BL_STATUS_RUN_FAILED;
		}
	}

	aht20_calculate_measurments(sensor_data.measured_data, &sensor_data.humidity, &sensor_data.temperature_c, &sensor_data.temperature_f);

	return BL_STATUS_OK;
}

/*
 * hardcoding data to transmit (bad approach)
 */
char t_data[5] = "";
period_status periods[4] = {PERIOD_OFF, PERIOD_OFF, PERIOD_ON, PERIOD_OFF};
driver_7_seg_brightness_t brightness[4] = {LEVEL_5_MAX, LEVEL_5_MAX, LEVEL_5_MAX, LEVEL_5_MAX};
char_gen_data_t data = {
		.digits = "",
		.periods = periods,
		.brightness = brightness,
};

/*
 * transmits formatted data to spi 7 seg display on pressing buttons
 */
void bl_spi_transmit_sensor_data(void) {
	SystemEvent event = detect_events();

	switch(config.currentMainState) {
	case MAIN_STATE_DISPLAY_C:
		snprintf(t_data, sizeof(t_data), "C%03d", (int16_t)(sensor_data.temperature_c * 10));
		data.digits = t_data;
		api_char_gen.transmit(&data);

		if(event == EVENT_BUTTON_A_SHORT) {
			config.currentMainState = MAIN_STATE_DISPLAY_F;
		} else if(event == EVENT_BUTTON_B_SHORT) {
			config.currentMainState = MAIN_STATE_DISPLAY_H;
		}
		break;
	case MAIN_STATE_DISPLAY_F:
		snprintf(t_data, sizeof(t_data), "F%03d", (int16_t)(sensor_data.temperature_f * 10));
		data.digits = t_data;
		api_char_gen.transmit(&data);

		if(event == EVENT_BUTTON_A_SHORT) {
			config.currentMainState = MAIN_STATE_DISPLAY_H;
		} else if(event == EVENT_BUTTON_B_SHORT) {
			config.currentMainState = MAIN_STATE_DISPLAY_C;
		}
		break;
	case MAIN_STATE_DISPLAY_H:
		snprintf(t_data, sizeof(t_data), "H%03d", (int16_t)(sensor_data.humidity * 10));
		data.digits = t_data;
		api_char_gen.transmit(&data);

		if(event == EVENT_BUTTON_A_SHORT) {
			config.currentMainState = MAIN_STATE_DISPLAY_C;
		} else if(event == EVENT_BUTTON_B_SHORT) {
			config.currentMainState = MAIN_STATE_DISPLAY_F;
		}
		break;
	case MAIN_STATE_ERROR_DISPLAY:
		snprintf(t_data, sizeof(t_data), "----");
		data.digits = t_data;
		api_char_gen.transmit(&data);
		break;
	}
}

/*
 * detects buttons events
 */
static SystemEvent detect_events(void) {

	HMI_Interact_Status_t stateOnReleaseA = button_hmi_api.check_device_status_change(&buttonA);
	HMI_Interact_Status_t stateOnReleaseB = button_hmi_api.check_device_status_change(&buttonB);
	HMI_Interact_Status_t currentStateB = button_hmi_api.check_device_current_status(&buttonB);

	SystemEvent detected_event = EVENT_NONE;

	if (stateOnReleaseA == HMI_SHORT_EVENT) {
		detected_event = EVENT_BUTTON_A_SHORT;
	}

	if (detected_event == EVENT_NONE) {
		if (stateOnReleaseB == HMI_SHORT_EVENT) {
			detected_event = EVENT_BUTTON_B_SHORT;
		} else if (currentStateB == HMI_NO_EVENT) {
			detected_event = EVENT_NONE;
		}
	}

	return detected_event;
}

void HAL_GPIO_EXTI_Callback(uint16_t gpio_pin) {
	button_hmi_api.device_interrupt_handle(gpio_pin);
}
