#ifndef __UART_CONFIG_H__
#define __UART_CONFIG_H__

#include "stdint.h"
#include "error_types.h"

/**
 * \ingroup config
 * \defgroup configUart Конфигурация UART
 */


/**
 * \ingroup configUart
 * Число бит данных
 */
enum uart_data_bits {
	UART_DATA_BITS_5, 
	UART_DATA_BITS_6, 
	UART_DATA_BITS_7, 
	UART_DATA_BITS_8, 
	UART_DATA_BITS_9 
};

/**
 * \ingroup configUart
 * Проверяет на корректное значение
 */
#define assertCorrectDataBits(x) ( \
				   ((x) == UART_DATA_BITS_5) || \
				   ((x) == UART_DATA_BITS_6) || \
				   ((x) == UART_DATA_BITS_7) || \
				   ((x) == UART_DATA_BITS_8) || \
				   ((x) == UART_DATA_BITS_9) \
				 ) 

/**
 * \ingroup configUart
 * Число стоповых бит
 */
enum uart_stop_bits {
	UART_STOP_BITS_1,
	UART_STOP_BITS_2
}; 

/**
 * \ingroup configUart
 * Проверяет на корректное значение
 */
#define assertCorrectStopBits(x) ( \
				   ((x) == UART_STOP_BITS_1) || \
				   ((x) == UART_STOP_BITS_2) \
				 ) 

/**
 * \ingroup configUart
 * Структура настройки UART
 * \struct uart_init
 * \var uart_init::baud_rate
 * Baudrate: 0 - 115200
 * \var uart_init::data_bits
 * Число бит данных
 * \var uart_init::stop_bits
 * Число стоповых бит
 */
struct uart_init  {
	uint32_t baud_rate; 
	enum uart_data_bits data_bits;
	enum uart_stop_bits stop_bits;
};


/**
 * \ingroup configUart
 * Настройка UART
 * \param [in] init_struct Указатель на структуру настройки UART
 * \param [in] cpu_freq Частота тактирования UART
 * \return Код ошибки
 */
uint8_t  init_uart(const struct uart_init* init_struct, uint32_t cpu_freq);

/**
 * \ingroup configUart
 * Включает UART
 * \return Код ошибки
 */
uint8_t enable_uart(void);

void uart_transmit_8(uint8_t data);
void uart_transmit_16(uint16_t data);
void uart_transmit_32(uint32_t data);

#endif
