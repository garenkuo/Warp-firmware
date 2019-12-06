#include <stdint.h>

#include "fsl_spi_master_driver.h"
#include "fsl_port_hal.h"

#include "SEGGER_RTT.h"
#include "gpio_pins.h"
#include "warp.h"
#include "devSSD1331.h"

volatile uint8_t	inBuffer[1];
volatile uint8_t	payloadBytes[1];


/*
 *	Override Warp firmware's use of these pins and define new aliases.
 */
enum
{
	kSSD1331PinMOSI		= GPIO_MAKE_PIN(HW_GPIOA, 8),
	kSSD1331PinSCK		= GPIO_MAKE_PIN(HW_GPIOA, 9),
	kSSD1331PinCSn		= GPIO_MAKE_PIN(HW_GPIOB, 13),
	kSSD1331PinDC		= GPIO_MAKE_PIN(HW_GPIOA, 12),
	kSSD1331PinRST		= GPIO_MAKE_PIN(HW_GPIOB, 0),
};

static int
writeCommand(uint8_t commandByte)
{
	spi_status_t status;

	/*
	 *	Drive /CS low.
	 *
	 *	Make sure there is a high-to-low transition by first driving high, delay, then drive low.
	 */
	GPIO_DRV_SetPinOutput(kSSD1331PinCSn);
	OSA_TimeDelay(10);
	GPIO_DRV_ClearPinOutput(kSSD1331PinCSn);

	/*
	 *	Drive DC low (command).
	 */
	GPIO_DRV_ClearPinOutput(kSSD1331PinDC);

	payloadBytes[0] = commandByte;
	status = SPI_DRV_MasterTransferBlocking(0	/* master instance */,
					NULL		/* spi_master_user_config_t */,
					(const uint8_t * restrict)&payloadBytes[0],
					(uint8_t * restrict)&inBuffer[0],
					1		/* transfer size */,
					1000		/* timeout in microseconds (unlike I2C which is ms) */);

	/*
	 *	Drive /CS high
	 */
	GPIO_DRV_SetPinOutput(kSSD1331PinCSn);

	return status;
}



int
devSSD1331init(void)
{
	/*
	 *	Override Warp firmware's use of these pins.
	 *
	 *	Re-configure SPI to be on PTA8 and PTA9 for MOSI and SCK respectively.
	 */
	PORT_HAL_SetMuxMode(PORTA_BASE, 8u, kPortMuxAlt3);
	PORT_HAL_SetMuxMode(PORTA_BASE, 9u, kPortMuxAlt3);

	enableSPIpins();

	/*
	 *	Override Warp firmware's use of these pins.
	 *
	 *	Reconfigure to use as GPIO.
	 */
	PORT_HAL_SetMuxMode(PORTB_BASE, 13u, kPortMuxAsGpio);
	PORT_HAL_SetMuxMode(PORTA_BASE, 12u, kPortMuxAsGpio);
	PORT_HAL_SetMuxMode(PORTB_BASE, 0u, kPortMuxAsGpio);


	/*
	 *	RST high->low->high.
	 */
	GPIO_DRV_SetPinOutput(kSSD1331PinRST);
	OSA_TimeDelay(100);
	GPIO_DRV_ClearPinOutput(kSSD1331PinRST);
	OSA_TimeDelay(100);
	GPIO_DRV_SetPinOutput(kSSD1331PinRST);
	OSA_TimeDelay(100);

	/*
	 *	Initialization sequence, borrowed from https://github.com/adafruit/Adafruit-SSD1331-OLED-Driver-Library-for-Arduino
	 */
	writeCommand(kSSD1331CommandDISPLAYOFF);	// 0xAE
	writeCommand(kSSD1331CommandSETREMAP);		// 0xA0
	writeCommand(0x72);				// RGB Color
	writeCommand(kSSD1331CommandSTARTLINE);		// 0xA1
	writeCommand(0x0);
	writeCommand(kSSD1331CommandDISPLAYOFFSET);	// 0xA2
	writeCommand(0x0);
	writeCommand(kSSD1331CommandNORMALDISPLAY);	// 0xA4
	writeCommand(kSSD1331CommandSETMULTIPLEX);	// 0xA8
	writeCommand(0x3F);				// 0x3F 1/64 duty
	writeCommand(kSSD1331CommandSETMASTER);		// 0xAD
	writeCommand(0x8E);
	writeCommand(kSSD1331CommandPOWERMODE);		// 0xB0
	writeCommand(0x0B);
	writeCommand(kSSD1331CommandPRECHARGE);		// 0xB1
	writeCommand(0x31);
	writeCommand(kSSD1331CommandCLOCKDIV);		// 0xB3
	writeCommand(0xF0);				// 7:4 = Oscillator Frequency, 3:0 = CLK Div Ratio (A[3:0]+1 = 1..16)
	writeCommand(kSSD1331CommandPRECHARGEA);	// 0x8A
	writeCommand(0x64);
	writeCommand(kSSD1331CommandPRECHARGEB);	// 0x8B
	writeCommand(0x78);
	writeCommand(kSSD1331CommandPRECHARGEA);	// 0x8C
	writeCommand(0x64);
	writeCommand(kSSD1331CommandPRECHARGELEVEL);	// 0xBB
	writeCommand(0x3A);
	writeCommand(kSSD1331CommandVCOMH);		// 0xBE
	writeCommand(0x3E);
	writeCommand(kSSD1331CommandMASTERCURRENT);	// 0x87
	writeCommand(0x06);
	writeCommand(kSSD1331CommandCONTRASTA);		// 0x81
	writeCommand(0x91);
	writeCommand(kSSD1331CommandCONTRASTB);		// 0x82
	writeCommand(0x50);
	writeCommand(kSSD1331CommandCONTRASTC);		// 0x83
	writeCommand(0x7D);
	writeCommand(kSSD1331CommandDISPLAYON);		// Turn on oled panel

	/*
	 *	To use fill commands, you will have to issue a command to the display to enable them. See the manual.
	 */
	writeCommand(kSSD1331CommandFILL);
	writeCommand(0x01);

	/*
	 *	Clear Screen
	 */
	writeCommand(kSSD1331CommandCLEAR);
	writeCommand(0x00);
	writeCommand(0x00);
	writeCommand(0x5F);
	writeCommand(0x3F);



	/*
	 *	Any post-initialization drawing commands go here.
	 */

	return 0;
}

int
devSSD1331LeftTurn(void) {
	int startCols[6] = {0x0f, 0x32, 0x37, 0x3c, 0x41, 0x46};
	int endCols[6] = {0x32, 0x37, 0x3c, 0x41, 0x46, 0x4b};
	int startRows[6] = {0x15, 0x0b, 0x10, 0x15, 0x1a, 0x1e};
	int endRows[6] = {0x29, 0x33, 0x2e, 0x29, 0x24, 0x20};

	for (int i = 0; i < 6; i++) {
		writeCommand(kSSD1331CommandDRAWRECT);
		// start row, col
		writeCommand(startCols[i]);
		writeCommand(startRows[i]);
		// end row, col
		writeCommand(endCols[i]);
		writeCommand(endRows[i]);
		// color of line
		writeCommand(0x0);
		writeCommand(0xff);
		writeCommand(0x0);
		// color of fill
		writeCommand(0x0);
		writeCommand(0xff);
		writeCommand(0x0);
	}

	OSA_TimeDelay(500);

	devSSD1331clear();

	return 0;
	// delay
}

int
devSSD1331RightTurn(void) {
	// int startCols[6] = {37, 27, 32, 37, 42, 46};
	// int endCols[6] = {57, 67, 62, 57, 52, 47};
	// int startRows[6] = {4, 34, 39, 44, 49, 54};
	// int endRows[6] = {34, 39, 44, 49, 54, 59};

	int startCols[6] = {0x25, 0x1b, 0x20, 0x25, 0x2a, 0x2e};
	int endCols[6] = {0x39, 0x43, 0x3e, 0x39, 0x34, 0x2f};
	int startRows[6] = {0x04, 0x22, 0x27, 0x2c, 0x31, 0x36};
	int endRows[6] = {0x22, 0x27, 0x2c, 0x31, 0x36, 0x3b};

	for (int i = 0; i < 6; i++) {
		writeCommand(kSSD1331CommandDRAWRECT);
		// start row, col
		writeCommand(startCols[i]);
		writeCommand(startRows[i]);
		// end row, col
		writeCommand(endCols[i]);
		writeCommand(endRows[i]);
		// color of line
		writeCommand(0x0);
		writeCommand(0xff);
		writeCommand(0x0);
		// color of fill
		writeCommand(0x0);
		writeCommand(0xff);
		writeCommand(0x0);
	}

	// writeCommand(kSSD1331CommandDRAWRECT);
	// // start row, col
	// writeCommand(0x0f);
	// writeCommand(0x15); // 21
	// // end row, col
	// writeCommand(0x32);
	// writeCommand(0x29); // 41
	// // color of line
	// writeCommand(0x0);
	// writeCommand(0x0);
	// writeCommand(0xff);
	// // color of fill
	// writeCommand(0x0);
	// writeCommand(0x0);
	// writeCommand(0xff);
	//
	// // triangle 1
	// writeCommand(kSSD1331CommandDRAWRECT);
	// // start row, col
	// writeCommand(0x32);
	// writeCommand(0x0b); // 11
	// // end row, col
	// writeCommand(0x37);
	// writeCommand(0x33); // 51
	// // color of line
	// writeCommand(0x0);
	// writeCommand(0x0);
	// writeCommand(0xff);
	// // color of fill
	// writeCommand(0x0);
	// writeCommand(0x0);
	// writeCommand(0xff);
	//
	// // triangle 2
	// writeCommand(kSSD1331CommandDRAWRECT);
	// // start row, col
	// writeCommand(0x37);
	// writeCommand(0x10); // 16
	// // end row, col
	// writeCommand(0x3c);
	// writeCommand(0x2e); // 46
	// // color of line
	// writeCommand(0x0);
	// writeCommand(0x0);
	// writeCommand(0xff);
	// // color of fill
	// writeCommand(0x0);
	// writeCommand(0x0);
	// writeCommand(0xff);
	//
	// writeCommand(kSSD1331CommandDRAWRECT);
	// // start row, col
	// writeCommand(0x3c);
	// writeCommand(0x15); // 21
	// // end row, col
	// writeCommand(0x41);
	// writeCommand(0x29); // 41
	// // color of line
	// writeCommand(0x0);
	// writeCommand(0x0);
	// writeCommand(0xff);
	// // color of fill
	// writeCommand(0x0);
	// writeCommand(0x0);
	// writeCommand(0xff);
	//
	// writeCommand(kSSD1331CommandDRAWRECT);
	// // start row, col
	// writeCommand(0x41);
	// writeCommand(0x1a); // 26
	// // end row, col
	// writeCommand(0x46);
	// writeCommand(0x24); // 36
	// // color of line
	// writeCommand(0x0);
	// writeCommand(0x0);
	// writeCommand(0xff);
	// // color of fill
	// writeCommand(0x0);
	// writeCommand(0x0);
	// writeCommand(0xff);
	//
	// writeCommand(kSSD1331CommandDRAWRECT);
	// // start row, col
	// writeCommand(0x46);
	// writeCommand(0x1e); // 30
	// // end row, col
	// writeCommand(0x4b);
	// writeCommand(0x20); // 32
	// // color of line
	// writeCommand(0x0);
	// writeCommand(0x0);
	// writeCommand(0xff);
	// // color of fill
	// writeCommand(0x0);
	// writeCommand(0x0);
	// writeCommand(0xff);

	OSA_TimeDelay(500);

	devSSD1331clear();

	return 0;
	// delay
}

int devSSD1331Stop(void) {
	// turn this into a for loop motherfucker
	/* 95 x 63
        xxxxx    30 x 5  (17 , 16) to (24 , 46)
			 xxxxxxx   40 x 5  (24 , 11) to (31 , 51)
			xxxxxxxxx  50 x 5  (31 , 6) to (38 , 56)
		 xxxxxxxxxxx 60 x 5  (38 , 1) to (44 , 61)
		 xxxxxxxxxxx 60 x 5  (44 , 1) to (50 , 61)
		 xxxxxxxxxxx 60 x 5  (50 , 1) to (56 , 61)
		  xxxxxxxxx  50 x 5  (56 , 6) to (63 , 56)
			 xxxxxxx   40 x 5  (63 , 11) to (70 , 51)
			  xxxxx    30 x 5  (70 , 16) to (77 , 46)

	*/
	int startRows[9] = {0x10, 0x0b, 0x06, 0x01, 0x01, 0x01, 0x06, 0x0b, 0x10};
	int endRows[9] = {0x2e, 0x33, 0x38, 0x3d, 0x3d, 0x3d, 0x38, 0x33, 0x2e};
	int startCols[9] = {0x11, 0x18, 0x1f, 0x26, 0x2c, 0x32, 0x38, 0x3f, 0x46};
	int endCols[9] = {0x18, 0x1f, 0x26, 0x2c, 0x32, 0x38, 0x3f, 0x46, 0x4d};

	for (int i=0; i<9; i++) {
		// flash red for 5 seconds
		writeCommand(kSSD1331CommandDRAWRECT);
		// start row, col
		writeCommand(startCols[i]);
		writeCommand(startRows[i]);
		// end row, col
		writeCommand(endCols[i]); //
		writeCommand(endRows[i]);
		// color of line
		writeCommand(0xff);
		writeCommand(0x0);
		writeCommand(0x0);
		// color of fill
		writeCommand(0xff);
		writeCommand(0x0);
		writeCommand(0x0);
	}


	OSA_TimeDelay(5000);

	devSSD1331clear();
	return 0;
}

int devSSD1331clear(void) {
	/*
	 *	Clear Screen
	 */
	writeCommand(kSSD1331CommandCLEAR);
	writeCommand(0x00);
	writeCommand(0x00);
	writeCommand(0x5F);
	writeCommand(0x3F);
	return 0;
}
