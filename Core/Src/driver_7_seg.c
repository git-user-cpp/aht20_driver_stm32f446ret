/*
 * digital_thermomether
 * digital thermometer built using stm32f446ret, AHT20 sensor and multi-function shield
 * Copyright (C) 2025 Anatoliy Lizanets
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

#include "driver_7_seg.h"

/*
  Driver configuration structure
 */

typedef struct
{
	GPIO_TypeDef* CS_GPIO_Port;
	uint16_t CS_GPIO_Pin;
	SPI_HandleTypeDef *hspi;
	TIM_HandleTypeDef *htim;
	driver_7_seg_status_t initialized;
} driver_7_seg_config_t;

/*
  Constant defining the number of segments
 */
typedef enum
{
	NUMBER_OF_SEGMENTS = 4,
} driver_7_seg_constant_t;

/*
  Structure holding segment data and brightness control (skip buffer) for all segments.
 */
typedef struct
{
    uint16_t data      [NUMBER_OF_SEGMENTS];
    uint8_t  skip_buff [NUMBER_OF_SEGMENTS];
} driver_7_seg_segment_buffer_t;

/*
  Structure holding double buffers and skip counters for the 7-segment display driver.
 */
typedef struct
{
	driver_7_seg_segment_buffer_t buf0;
	driver_7_seg_segment_buffer_t buf1;
    uint8_t skip_counter [NUMBER_OF_SEGMENTS];
} driver_7_seg_display_buffers_t;

/*
  State machine states for data transfer.
 */
typedef enum
{
	STATE_PREPARE,
	STATE_START_SENDING,
	STATE_WAIT_SPI,
	STATE_POST,
	STATE_WAIT
} driver_7_seg_transfer_state_t;

/*
 API instance linking public functions to driver interface.
 */

const driver_7_seg_api_t api_7_seg =
{
		.init          = driver_7_seg_init,
		.send_buffer   = driver_7_seg_send_buffer
};

/*
  Global driver configuration instance
 */
static driver_7_seg_config_t config  = {0};
/*
  Structure of sending buffers
 */
static driver_7_seg_display_buffers_t buffers = {0};

/*     State variables:
       Flag set by SPI complete callback
       Flag indicating new buffer available
       Indicates currently active buffer
*/
static volatile uint8_t spi_tranfer_complete = 0;
static volatile uint8_t new_buffer_ready     = 0;
static uint8_t      current_active_buf_index = 0;


/*
 Initializes the 7-segment display driver.
 */
driver_7_seg_status_t driver_7_seg_init( SPI_HandleTypeDef *const hspi, TIM_HandleTypeDef *const htim,
										 GPIO_TypeDef *const GPIOx, const uint16_t GPIO_Pin )
{

	if ( hspi == NULL || htim == NULL || GPIOx == NULL )
	{
		return DRIVER_7_SEG_STATUS_NOT_INITIALIZED;
	}

	HAL_GPIO_WritePin( config.CS_GPIO_Port, config.CS_GPIO_Pin, GPIO_PIN_SET );

	config.CS_GPIO_Port = GPIOx;
	config.CS_GPIO_Pin  = GPIO_Pin;
	config.hspi         = hspi;
	config.htim         = htim;

	for ( uint8_t i = 0; i < NUMBER_OF_SEGMENTS; i++ )
	{
		buffers.buf0.skip_buff[i] = NOT_USED;
	}

	if ( HAL_OK != HAL_TIM_Base_Start_IT(config.htim) )
	{
		return DRIVER_7_SEG_STATUS_NOT_INITIALIZED;
	}

	config.initialized = DRIVER_7_SEG_STATUS_OK;

	return DRIVER_7_SEG_STATUS_OK;
}

/*
 Sends a data buffer to the display with specified brightness levels.
 */
driver_7_seg_status_t driver_7_seg_send_buffer ( const uint16_t *const data, const driver_7_seg_brightness_t *const brightness_level, const uint8_t size )
{
	if ( DRIVER_7_SEG_STATUS_OK != config.initialized )
	{
		return DRIVER_7_SEG_STATUS_NOT_INITIALIZED;
	}

	if ( data == NULL || brightness_level == NULL || size != NUMBER_OF_SEGMENTS )
	{
		return DRIVER_7_SEG_STATUS_INVALID_PARAMETERS;
	}

	while ( new_buffer_ready != 0 ) {}

	if (current_active_buf_index == 0)
	{
		for ( uint8_t i = 0; i < NUMBER_OF_SEGMENTS; i++)
		{
			buffers.buf1.data[i]      = data[i];
			buffers.buf1.skip_buff[i] = (uint8_t)brightness_level[i];
		}
	}
	else
	{
		for( uint8_t i = 0; i < NUMBER_OF_SEGMENTS; i++)
		{
			buffers.buf0.data[i]      = data[i];
			buffers.buf0.skip_buff[i] = (uint8_t)brightness_level[i];
		}

	}

	new_buffer_ready = 1;

	return DRIVER_7_SEG_STATUS_OK;
}

/*
 Periodic timer callback for driving the display
 Timer Frequency [Hz] = Timer_Clock / ((Prescaler + 1) * (AutoReload + 1)) =
 Timer_Clock = 84 Mhz;
 Prescaler = 150;
 AutoReload = 1;
 Timer Period ≈ 3.595 microseconds
 */
void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef *htim )
{
	if ( htim->Instance == TIM6 )
	{
		static driver_7_seg_transfer_state_t state = STATE_PREPARE;

		switch (state)
		{
		case STATE_PREPARE:
			if (new_buffer_ready == 1)
			{
				current_active_buf_index ^= 1;
				new_buffer_ready = 0;
			}
			static uint16_t *data_buf = NULL;
			static uint8_t  *skip_buf  = NULL;

			data_buf = (current_active_buf_index == 0) ? buffers.buf0.data      : buffers.buf1.data;
			skip_buf = (current_active_buf_index == 0) ? buffers.buf0.skip_buff : buffers.buf1.skip_buff;

			static uint8_t current_segment_index = 0;

			HAL_GPIO_WritePin(config.CS_GPIO_Port, config.CS_GPIO_Pin, GPIO_PIN_RESET);
			state = STATE_START_SENDING;
			break;

		case STATE_START_SENDING:
			spi_tranfer_complete = 0;

			if ( skip_buf[current_segment_index] != NOT_USED && buffers.skip_counter[current_segment_index] >= skip_buf[current_segment_index])
			{
				HAL_SPI_Transmit_IT(config.hspi, (uint8_t*)&data_buf[current_segment_index], 1);

				buffers.skip_counter[current_segment_index] = 0;
			}
			else
			{
				static const uint16_t void_data = 0;

				HAL_SPI_Transmit_IT(config.hspi, (uint8_t*)&void_data, 1);

				buffers.skip_counter[current_segment_index]++;
			}
			state = STATE_WAIT_SPI;
			break;

		case STATE_WAIT_SPI:
			if (spi_tranfer_complete != 0)
			{
				state = STATE_POST;

			}
			break;

		case STATE_POST:
			HAL_GPIO_WritePin(config.CS_GPIO_Port, config.CS_GPIO_Pin, GPIO_PIN_SET);
			state = STATE_WAIT;
			break;

		case STATE_WAIT:
			current_segment_index = ( current_segment_index + 1 ) % NUMBER_OF_SEGMENTS;
			state = STATE_PREPARE;
			break;
		}

	}
}

/*
 SPI transmission complete callback.
 */

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi == config.hspi)
	{
		spi_tranfer_complete = 1;
	}
}
