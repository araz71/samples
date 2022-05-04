
/* Includes */
#include "stm32f0xx.h"

uint64_t tick_cntr = 0;

#define SCK_HIGH					GPIOA->BSRR |= (1 << 9)
#define SCK_LOW						GPIOA->BRR |= (1 << 9)

#define SDO_HIGH					GPIOA->BSRR |= (1 << 10)
#define SDO_LOW						GPIOA->BRR |= (1 << 10)

// Main Settings
#define	TM1637_BRIGHTNESS_MAX		(7)
#define	TM1637_POSITION_MAX			(4)

// TM1637 commands
#define	TM1637_CMD_SET_DATA			0x40
#define	TM1637_CMD_SET_ADDR			0xC0
#define	TM1637_CMD_SET_DSIPLAY		0x80

// TM1637 data settings (use bitwise OR to contruct complete command)
#define	TM1637_SET_DATA_WRITE		0x00 // write data to the display register
#define	TM1637_SET_DATA_READ		0x02 // read the key scan data
#define	TM1637_SET_DATA_A_ADDR		0x00 // automatic address increment
#define	TM1637_SET_DATA_F_ADDR		0x04 // fixed address
#define	TM1637_SET_DATA_M_NORM		0x00 // normal mode
#define	TM1637_SET_DATA_M_TEST		0x10 // test mode

// TM1637 display control command set (use bitwise OR to consruct complete command)
#define	TM1637_SET_DISPLAY_OFF		0x00 // off
#define	TM1637_SET_DISPLAY_ON		0x08 // on

static uint8_t _config = TM1637_SET_DISPLAY_ON | TM1637_BRIGHTNESS_MAX;

const uint8_t _digit2segments[] =
{
	0x3F, // 0
	0x06, // 1
	0x5B, // 2
	0x4F, // 3
	0x66, // 4
	0x6D, // 5
	0x7D, // 6
	0x07, // 7
	0x7F, // 8
	0x6F  // 9
};

void write(uint8_t data)
{
	uint8_t i, ack;

	for (i = 0; i < 8; ++i, data >>= 1) {
		SCK_LOW;
		for (int j = 0; j < 0x0FF; j++);

		if (data & 0x01) {
			SDO_HIGH;
		} else {
			SDO_LOW;
		}
		SCK_HIGH;
		for (int j = 0; j < 0x0FF; j++);
	}

	SCK_HIGH;
	for (int j = 0; j < 0x0FF; j++);
	SCK_LOW;
	for (int j = 0; j < 0x0FF; j++);

	SCK_HIGH;
	SDO_HIGH;
}

void start()
{
	SDO_HIGH;
	SCK_HIGH;
	for (int j = 0; j < 0x0FF; j++);
	SDO_LOW;
	for (int j = 0; j < 0x0FF; j++);
	SDO_LOW;
	for (int j = 0; j < 0x0FF; j++);
}

void stop()
{
	SCK_LOW;
	for (int j = 0; j < 0x0FF; j++);
	SDO_LOW;
	for (int j = 0; j < 0x0FF; j++);

	SCK_HIGH;
	for (int j = 0; j < 0x0FF; j++);
	SDO_HIGH;
	for (int j = 0; j < 0x0FF; j++);
}

void TM1637_send_command(const uint8_t value)
{
	start();
	write(value);
	stop();
}

void TM1637_send_config(const uint8_t enable, const uint8_t brightness)
{
	_config = (enable ? TM1637_SET_DISPLAY_ON : TM1637_SET_DISPLAY_OFF) |
		(brightness > TM1637_BRIGHTNESS_MAX ? TM1637_BRIGHTNESS_MAX : brightness);

	TM1637_send_command(TM1637_CMD_SET_DSIPLAY | _config);
}

void TM1637_display_segments(const uint8_t position, const uint8_t segments)
{
	TM1637_send_command(TM1637_CMD_SET_DATA | TM1637_SET_DATA_F_ADDR);
	start();
	write(TM1637_CMD_SET_ADDR | (position & (TM1637_POSITION_MAX - 1)));
	write(segments);
	stop();
}

void TM1637_set_brightness(const uint8_t value)
{
	TM1637_send_config(_config & TM1637_SET_DISPLAY_ON,
		value & TM1637_BRIGHTNESS_MAX);
}

void SysTick_Handler()
{
	tick_cntr++;
}

int main(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	SysTick_Config(8000);
	NVIC_EnableIRQ(SysTick_IRQn);

	GPIOA->MODER |= GPIO_MODER_MODER9_0;
	GPIOA->MODER |= GPIO_MODER_MODER10_0;
	GPIOA->MODER |= GPIO_MODER_MODER4_0;
	SDO_HIGH;
	SCK_HIGH;

	while(tick_cntr < 1000);

	TM1637_send_config(1, 1);

	for (int i = 0; i < 4; i++)
		TM1637_display_segments(i, 0);

	tick_cntr = 0;
	for (int i = 0; i < 10; i++) {
		TM1637_display_segments(0, _digit2segments[i]);
		TM1637_display_segments(1, _digit2segments[i]);
		TM1637_display_segments(2, _digit2segments[i]);
		TM1637_display_segments(3, 0x79);//_digit2segments[i]);
		GPIOA->BSRR |= (1 << 4);
		while (tick_cntr != 500);
		tick_cntr = 0;
		GPIOA->BRR |= (1 << 4);
		while (tick_cntr != 500);
		tick_cntr = 0;
	}
	while(1);
}
