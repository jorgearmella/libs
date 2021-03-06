#ifndef __UART_H
#define __UART_H



#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <types.h>

#include <avr/io.h>
#include <util/setbaud.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "../asyncTypes.h"
#include "../core/ringbuffer.h"


class Uart
{
public:
	typedef void (*uart_rx_callback_t)(char);
	typedef void (*uart_tx_callback_t)(void);
	typedef void (*uart_asyncCallback_t)(void);

	typedef char (*f_reader_t)(volatile const char**);

	//Uart(void);
	Uart(RingBuffer* rx_buff);

	void putstr(const char* pstr);
	void putstr_P(const char* pstr);
	void putstrAM(const char * pstr, uart_asyncCallback_t callBack);

	char * getstr(char * pstr, uint16_t max);

	void writeA(uart_tx_callback_t callBack);
	void writeAEnd(void);
	void readA(uart_rx_callback_t callBack);
	void readAEnd(void);

	// returns true if data is waiting in the receivers register
	bool dataWaiting(void);

	// reads a byte of data from the buffer or returns -1 if not data available
	int read(void);

	// blocks until the number of bytes read is received
	void read(char * buffer, uint16_t length);

	void write(char x);
	void write(const char * buffer, uint16_t length);

	// blocks the caller until data is available
	void wait(void);

	void receiveHandler(char data);
	void transmitHandler(void);

private:
	inline char sram_read(const char** buff)
	{
		return *(*buff)++;
	}
	inline char pgm_read(const char** buff)
	{
		return pgm_read_byte((*buff)++);
	}
	static void drain_rx(void)
	{
		unsigned char data;
		while (UCSR0A & (1<<RXC0))
			data = UDR0;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	void uart_putstrAMHandler(void)
	{
		char data = pgm_read_byte(_txAsyncData++);
		if (data == 0)
		{
			writeAEnd();
			if (_txAsyncCallback)
				_txAsyncCallback();
			_txAsyncCallback = 0;
			_asyncBusy = 0;
		}		
		else
			write(data);
	}
	void uart_putstrAM(const char * pstr, uart_asyncCallback_t callBack)
	{
		while(_asyncBusy);
		
		_txAsyncCallback	= callBack;
		_txAsyncData		= pstr;
		_asyncBusy			= 1;

		writeA(callBack);

		write(pgm_read_byte(_txAsyncData++));
	}
	static inline void null_handler(void)
	{}



	RingBuffer *			_uart_rx_buff;

	uart_tx_callback_t		_uart_tx_callback;
	uart_rx_callback_t		_uart_rx_callback;
	const char*				_txAsyncData;
	uint16_t				_txAsyncLen;
	char					_asyncBusy;

	uart_asyncCallback_t	_txAsyncCallback;
	uart_asyncCallback_t	_rxAsyncCallback;
};


#endif
