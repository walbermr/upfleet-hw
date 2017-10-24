/*
 * Copyright (c) 2016, NXP B.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of  NXP B.V. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!
 * @file main.c
 *
 * Console Control for OL2385 based on Sigfox SDK SW driver.
 *
 * This example enables user to control OL2385 using console interface.
 * Sigfox SDK SW driver is used to access/communicate with OL2385 device.
 *
 * Note that not all of the commands are implemented in this version.
 */
#include <string.h>
/* SDK */
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "fsl_uart.h"

/* Sigfox */
#include "sf/sf.h"
#include "sf_cmd.h"

/*!
 * @brief Gets source clock for SPI peripheral.
 */
#define GET_SPI_MODULE_CLK() \
    (CLOCK_GetFreq(kCLOCK_BusClk))

union Pos {  // union consegue definir vários tipos de dados na mesma posição de memória
	char b[4];
	float f;
};

/*!
 * @brief Initializes SIGFOX device and SW driver.
 *
 * @param drvData Driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t SetupSigfoxDriver(sf_drv_data_t *drvData);

/*!
 * @brief Application entry point.
 */
int main(void)
{
    uint8_t ch[] = "000000000";
    uart_config_t config;

    status_t serialStatus = kStatus_Success;

	char desgaste = 5;
    union Pos lon, lat;
	lat.f = -8.056157;
	lon.f = -34.951114;



    status_t status = kStatus_Success;
    sf_drv_data_t sfDrvData;            /* Sigfox driver data needed by the
                                           driver's functions. */
    sf_spi_cmd_t cmd;					/* Command for OL2385. */

    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /*
     * config.baudRate_Bps = 9600U;
     * config.parityMode = kUART_ParityDisabled;
     * config.stopBitCount = kUART_OneStopBit;
     * config.txFifoWatermark = 0;
     * config.rxFifoWatermark = 1;
     * config.enableTx = false;
     * config.enableRx = false;
     */
    UART_GetDefaultConfig(&config);
    config.baudRate_Bps = 9600U;
    config.enableTx = true;
    config.enableRx = true;
    //CLOCK_GetBusClkFreq();
    PRINTF("%d\n\r", CLOCK_GetFreq(kCLOCK_BusClk));
    //PRINTF("%d\n\r", CLOCK_GetFreq(kCLOCK_PlatClk));
	//PRINTF("%d\n\r", CLOCK_GetFreq(kCLOCK_BusClk));
	//PRINTF("%d\n\r", CLOCK_GetFreq(kCLOCK_FlexBusClk));
    //PRINTF("%d\n\r", CLOCK_GetFreq(kCLOCK_FlashClk));
    //PRINTF("%d\n\r", CLOCK_GetFreq(kCLOCK_FastPeriphClk));
    //PRINTF("%d\n\r", CLOCK_GetFreq(kCLOCK_PllFllSelClk));

    UART_Init(UART2, &config, CLOCK_GetFreq(kCLOCK_BusClk));
    //UART_EnableInterrupts(UART2, true);

    PRINTF("Serial test.\r\n");

    while (1)
    {
        serialStatus = UART_ReadBlocking(UART2, &ch, 12);

        if(serialStatus == kStatus_Success)
        {
            PRINTF("Data received: %X\r\n", ch);
            //UART_WriteBlocking(UART2, &ch, 1);
        }
        else
        {
        	switch(serialStatus)
        	{
        		case kStatus_UART_RxHardwareOverrun:
        			PRINTF("RX HW OVERRUN\r\n");
        			break;
        		case kStatus_UART_NoiseError:
        			PRINTF("NOISE ERROR FLAG\r\n");
        			break;
        		case kStatus_UART_FramingError:
        			PRINTF("FRAMING ERROR\r\n");
        			break;
        		case kStatus_UART_ParityError:
        			PRINTF("PARITY ERROR\r\n");
        			break;
        	}
        	PRINTF("Serial error: %d\r\n", serialStatus);
        }

    }


    PRINTF("=============================\r\n");
    PRINTF("Sigfox OL2385 Console Control\r\n");
    PRINTF("=============================\r\n");


    /* Initialize SPI, GPIOs and Sigfox driver, */
    status = SetupSigfoxDriver(&sfDrvData);
    if (status == kStatus_Success)
    {
        PRINTF("An error occurred in SetupSigfoxDriver (%d)\r\n", status);
    }
    else
    {
    	ProcessCommand(&sfDrvData, (sf_spi_cmd_t) 1);
        PrintCommands();

        while (1) {
        	cmd = GetUserCommand();

        	if (!cmd)
        	{
        	    PRINTF("Shutting down command processing.\r\n");
        		break;
        	}
        	else
        	{
                status = ProcessCommand(&sfDrvData, cmd);
                if (status != kStatus_Success)
                {
                    PRINTF("Processing of the command failed with error code: %d\r\n", status);
                }
                else
                {
                    PRINTF("Command was successfully executed.\r\n");
                }
                PRINTF("\r\n");
        	}
        }
    }

    for (;;)
    { /* Infinite loop to avoid leaving the main function */
        __asm("NOP");
        /* something to use as a breakpoint stop while looping */
    }
}

static status_t SetupSigfoxDriver(sf_drv_data_t *drvData)
{
    sf_user_config_t userConfig;

    SF_GetDefaultConfig(&userConfig);

    /* GPIOs initialization.
     * Note: GPIO settings are place in pin_mux.h file. */
    /* ACK pin. */
    drvData->gpioConfig.ackPin.gpioInstance = SF_ACK_INST;
    drvData->gpioConfig.ackPin.gpioPinNumber = SF_ACK_PIN;

    /* CS pin. */
    drvData->gpioConfig.csPin.gpioInstance = SF_CS_INST;
    drvData->gpioConfig.csPin.gpioPinNumber = SF_CS_PIN;

    SF_SetupGPIOs(&(drvData->gpioConfig));

    /* SPI initialization. */
    drvData->spiConfig.baudRate = 125000U;
    drvData->spiConfig.sourceClkHz = GET_SPI_MODULE_CLK();
    drvData->spiConfig.spiInstance = SF_SPI_INST;

    SF_SetupSPI(&(drvData->spiConfig), NULL);

    /* Sigfox driver initialization.
     * Note: drvData->gpioConfig and drvData->spiConfig structures are used
     * by SF_SetupGPIOs, SF_SetupSPI and SF_Init. */
    return SF_Init(drvData, &userConfig);
}


