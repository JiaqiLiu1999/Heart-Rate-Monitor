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

int writeCommand(uint8_t commandByte)
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
	SEGGER_RTT_WriteString(0, "\r\n\tDone with initialization sequence...\n");

	/*
	 *	To use fill commands, you will have to issue a command to the display to enable them. See the manual.
	 */
	writeCommand(kSSD1331CommandFILL);
	writeCommand(0x01);
	SEGGER_RTT_WriteString(0, "\r\n\tDone with enabling fill...\n");
	/*
	 *	Clear Screen
	 */
	writeCommand(kSSD1331CommandCLEAR);
	writeCommand(0x00);
	writeCommand(0x00);
	writeCommand(0x5F);
	writeCommand(0x3F);
	SEGGER_RTT_WriteString(0, "\r\n\tDone with screen clear...\n");

	/*
	  #coursework 2
	 */
    writeCommand(kSSD1331CommandCONTRASTB);         // 0x82
    writeCommand(0x80); //128 

	writeCommand(kSSD1331CommandDRAWRECT);		// 0x22
	writeCommand(0x00);
	writeCommand(0x00);
	writeCommand(0x5F);
	writeCommand(0x3F);
    writeCommand(0x00);
	writeCommand(0x3F);
	writeCommand(0x00);
	writeCommand(0x00);
    writeCommand(0x3F);
    writeCommand(0x00);

	SEGGER_RTT_WriteString(0, "\r\n\tDone with draw rectangle...\n");
	
	//writeCommand(kSSD1331CommandCLEAR);
	//writeCommand(0x00);
	//writeCommand(0x00);
	//writeCommand(0x5F);
	//writeCommand(0x3F);
	//SEGGER_RTT_WriteString(0, "\r\n\tclear\n");

	devSSD1331DrawChar("Hello", 5); //Welcome Initialization
	OSA_TimeDelay(100);
	devSSD1331ClearWithBg();
	devSSD1331ClearCoord();
	devSSD1331DrawChar("HR\nMonitor", 10);
	OSA_TimeDelay(100);
	//devSSD1331ClearCoord();
	//devSSD1331DrawChar("Place A\nFinger", 14);
	//OSA_TimeDelay(100);

	
	return 0;
}

void devSSD1331ClearWithBg() // This function can be used to clear the number or char but keeping the background color
{
	writeCommand(kSSD1331CommandCLEAR);
	writeCommand(0x00);
	writeCommand(0x00);
	writeCommand(0x5F);
	writeCommand(0x3F);
	writeCommand(kSSD1331CommandFILL);
	writeCommand(0x01);
	writeCommand(kSSD1331CommandCONTRASTB);         // 0x82
    writeCommand(0x80); //128 
	writeCommand(kSSD1331CommandDRAWRECT);		// 0x22
	writeCommand(0x00);
	writeCommand(0x00);
	writeCommand(0x5F);
	writeCommand(0x3F);
    writeCommand(0x00);
	writeCommand(0x3F);
	writeCommand(0x00);
	writeCommand(0x00);
    writeCommand(0x3F);
    writeCommand(0x00);
}

void devSSD1331DrawChar(char* data,uint8_t num_data)
{

    uint8_t i;
 

    for( i=0; i<num_data; i++) // Seperate the string into individual characters to be presented on OLED one by one
    {
    	SEGGER_RTT_printf(0, " data = %c ", data[i]);
    	char data_print = data[i];
    	if (data_print == '\n') // Set the coordinates for x and y when the string is required to be printed in a new line
    	{
    		coor_y += 18; 
    		coor_x = 5;
    	}
        else
        {
       		DrawPoint(coor_x, coor_y, ((int)(data_print)-32)); // 32 as the offset according to ASCII table
      		coor_x += 10; // Each character has a x/width value of 8 and 2 more pixels to seperate them.
      	}
    } 
}

int Pow(uint8_t a,uint8_t b) //power function
{
	uint32_t result=1;
	while(b--)
	{
	  result*=a;
	}
	return result;
}

void devSSD1331DrawNum(int data,uint8_t num_data) // print number
{

    uint8_t i;
 

    for( i=0; i<num_data; i++) 
    {
    	int data_print = ((data/Pow(10,(3-i-1)))%10); // seperate a long integer into single numbers from 0-9 
    	SEGGER_RTT_printf(0, " data = %d ", data_print);
        if(data_print==0)
			{
				DrawPoint(coor_x, coor_y, ((int)('0')-32)); 
      		}
			else 
			{
				DrawPoint(coor_x, coor_y, ((int)(data_print+'0')-32)); //+'0' to convert int to char
			}
      		coor_x += 10;
    } 
}

void DrawPoint(uint8_t x,uint8_t y, int dataValue)
{

    int i,j;
    unsigned char Temp=0;
    i = 0;
    j = 0;
    for(i=0; i<16; i++) 
    {
    	Temp = asc2[dataValue][i];
    	//SEGGER_RTT_printf(0, " data = %x ", Temp);
    	uint8_t line_length = 0; 
    	for(j=16; j>0; j--) 
    	{ 
 
    		//SEGGER_RTT_printf(0, " line_length = %d ", line_length);
   			if((Temp & 0x80)==0x80) // from the MSB to read
    		{
    			line_length++;
 			}
    		else if(line_length>0)
    		{

    			if (i<8) //upper bit of the character
    			{
    				DrawPixel(coor_x+i, coor_y+j, coor_x+i, coor_y+j + line_length, 0xf800);//0xf800=red
                    //Black:0x0000, Grey:0x7BEF, Red: 0xF800, Green:0x07E0, Blue: 0x001F, Yellow:0xFFE0, White:0xFFFF
    				line_length = 0;
    			}
    			else // lower bit of the character
    			{
    				DrawPixel(coor_x+i-8, coor_y+j+8, coor_x+i-8, coor_y+j+8 + line_length, 0xf800);
    				line_length = 0;
    			}
    		}
    			Temp = Temp << 1;

    	}
    }
    //coor_x += (16);
}

void DrawPixel(uint8_t x_start,uint8_t y_start,uint8_t x_end,uint8_t y2_end,uint16_t color)
{
	if  ( x_start > 95 ) x_start = 95; //size of the display window
    if  ( y_start > 63 ) y_start = 63;
    if  ( x_end > 95 ) x_end = 95;
    if  ( y2_end > 63 ) y2_end = 63;

    writeCommand(kSSD1331CommandFILL);
    writeCommand(0x0);
    writeCommand(kSSD1331CommandDRAWLINE);
    writeCommand(x_start);
    writeCommand(y_start);
    writeCommand(x_end);
    writeCommand(y2_end);
    writeCommand((((color>>11)&0x1F)<<1));
    writeCommand(((color>>5)&0x3F));
    writeCommand(((color&0x1F)<<1));
    
}
void devSSD1331ClearCoord() // reset the coordinates
{
	coor_x=5;
	coor_y=5;
}
void devSSD1331NextChar()
{
	coor_x=35;
	coor_y=5;
}



