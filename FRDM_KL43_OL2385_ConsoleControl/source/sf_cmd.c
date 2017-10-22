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
 * @file sf_cmd.c
 *
 * This module implements console interface for OL2385.
 * Specifically functions for command selection and related handlers.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
/* Standard libraries */
#include "stdlib.h"
#include "ctype.h"
/* SDK */
#include "fsl_common.h"
#include "fsl_debug_console.h"
/* AML */
#include "aml/common_aml.h"
/* Sigfox */
#include "sf/sf.h"
#include "sf_cmd.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/
/*! Number of available commands for OL2385. */
#define SF_CMD_NUM 25U

/*! Pointer to command handler function. */
typedef status_t (*CmdHandlerPtr)(sf_drv_data_t *sfDrvData);

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief Handler function for WakeUp command.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleWakeUp(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for Echo command.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleEcho(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for Sleep command.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleSleep(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for SendPayload command.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleSendPayload(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for SendBit command.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleSendBit(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for SendOutOfBand command.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleSendOoB(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for ReceiveFrame command.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleRcvFrame(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for GetInfo command.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleGetInfo(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for SetUlFrequency command.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleSetUlFreq(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for GetUlFrequency command.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleGetUlFreq(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for ContinuousWave command.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleContWave(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for CheckIdKey command.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleCheckIdKey(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for GetDeviceVersion command.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleGetDevVer(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for SedWdTimer command.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleSetWdTmr(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for GetWdTimer command.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleGetWdTmr(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for SetRegister command.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleSetReg(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for GetRegister command.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleGetReg(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for TriggerWd command.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleTrigWd(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for KeepAlive command.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleKeepAlive(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for TestMode command.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleTestMode(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for Change to RCZx commands.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleChangeNetStandard(sf_drv_data_t *sfDrvData, sf_net_standard_t netStandard);

/*!
 * @brief Handler function for Change to RCZ1 commands.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleChangeToRCZOne(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for Change to RCZ2 commands.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleChangeToRCZTwo(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for Change to RCZ3 commands.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleChangeToRCZThree(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function for Change to RCZ4 commands.
 *
 * @param sfDrvData Sigfox driver run-time data.
 *
 * @return Status result of the function.
 */
static status_t HandleChangeToRCZFour(sf_drv_data_t *sfDrvData);

/*!
 * @brief Handler function to send gps.
 *
 * @param sfDrvData Sigfox driver run-time data, msg.
 *
 * @return Status result of the function.
 */
static status_t HandlerSendPkg(sf_drv_data_t *sfDrvData);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*! @brief Array of pointers to command handler functions. */
static CmdHandlerPtr CmdHandlerArr[SF_CMD_NUM] = {
	HandleWakeUp,
	HandleEcho,
	HandleSleep,
	HandleSendPayload,
	HandleSendBit,
	HandleSendOoB,
	HandleRcvFrame,
	HandleGetInfo,
	HandleSetUlFreq,
	HandleGetUlFreq,
	HandleContWave,
	HandleCheckIdKey,
	HandleGetDevVer,
	HandleSetWdTmr,
	HandleGetWdTmr,
	HandleSetReg,
	HandleGetReg,
	HandleTrigWd,
	HandleKeepAlive,
	HandleTestMode,
	HandleChangeToRCZOne,
	HandleChangeToRCZTwo,
	HandleChangeToRCZThree,
	HandleChangeToRCZFour,
	HandlerSendPkg
};

/*******************************************************************************
 * Code
 ******************************************************************************/

/*FUNCTION**********************************************************************
 *
 * Function Name : PrintDeviceState
 * Description : Prints error code and internal OL2385 state which is updated
 * 				 with each ACK frame.
 *
 *END**************************************************************************/
void PrintDeviceState(sf_drv_data_t *sfDrvData)
{
	AML_ASSERT(sfDrvData != NULL);

	PRINTF("\tResponse (ACK):\r\n");
    PRINTF("\tError code:\t%u\r\n", sfDrvData->errorCode);
    PRINTF("\tDevice state:\t%u\r\n", sfDrvData->devState);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PrintPayload
 * Description : Prints message payload of the sent or received frame.
 *
 *END**************************************************************************/
void PrintPayload(const sf_msg_payload_t *msgPayload)
{
    uint8_t i;

    AML_ASSERT(msgPayload != NULL);

    PRINTF("\tLength:\t%u\r\n", msgPayload->payloadLen);
    PRINTF("\tPayload hex:\t0x", msgPayload->payloadLen);
    for (i = 0U; i < msgPayload->payloadLen; i++)
    {
        PRINTF("%02x ", *(msgPayload->payload + i));
    }
    PRINTF("\r\n");

    PRINTF("\tPayload str:\t");
    for (i = 0U; i < msgPayload->payloadLen; i++)
    {
        PRINTF("%c", *(msgPayload->payload + i));
    }
    PRINTF("\r\n");
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PrintDeviceInfo
 * Description : Prints device's ID, version and library version.
 *
 *END**************************************************************************/
void PrintDeviceInfo(const sf_device_info_t *devInfo)
{
	AML_ASSERT(devInfo != NULL);

	PRINTF("\tDevice ID:\t0x%08x\r\n", devInfo->devId);

    PRINTF("\tPAC = 0x");
    PRINTF("%02x%02x ", devInfo->devPac[7], devInfo->devPac[6]);
    PRINTF("%02x%02x ", devInfo->devPac[5], devInfo->devPac[4]);
    PRINTF("%02x%02x ", devInfo->devPac[3], devInfo->devPac[2]);
    PRINTF("%02x%02x\r\n", devInfo->devPac[1], devInfo->devPac[0]);

    PRINTF("\tDevice version = 0x");
    PRINTF("%02x%02x ", devInfo->devVersion[14], devInfo->devVersion[13]);
    PRINTF("%02x%02x ", devInfo->devVersion[12], devInfo->devVersion[11]);
    PRINTF("%02x%02x ", devInfo->devVersion[10], devInfo->devVersion[9]);
    PRINTF("%02x%02x ", devInfo->devVersion[8], devInfo->devVersion[7]);
    PRINTF("%02x%02x ", devInfo->devVersion[6], devInfo->devVersion[5]);
    PRINTF("%02x%02x ", devInfo->devVersion[4], devInfo->devVersion[3]);
    PRINTF("%02x%02x ", devInfo->devVersion[2], devInfo->devVersion[1]);
    PRINTF("%02x\r\n", devInfo->devVersion[0]);

    PRINTF("\tLibrary version = 0x");
    PRINTF("%02x%02x ", devInfo->libVersion[10], devInfo->libVersion[9]);
    PRINTF("%02x%02x ", devInfo->libVersion[8], devInfo->libVersion[7]);
    PRINTF("%02x%02x ", devInfo->libVersion[6], devInfo->libVersion[5]);
    PRINTF("%02x%02x ", devInfo->libVersion[4], devInfo->libVersion[3]);
    PRINTF("%02x%02x ", devInfo->libVersion[2], devInfo->libVersion[1]);
    PRINTF("%02x\r\n", devInfo->libVersion[0]);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PrintCommands
 * Description : Prints available console commands.
 *
 *END**************************************************************************/
void PrintCommands(void)
{
	PRINTF("****************************************************************************\r\n");
	PRINTF("*0.  Exit\r\n");
	PRINTF("*1.  Send_Wakeup\r\n");             /*!< Wakes up OL2385 */
	PRINTF("*2.  Echo_Command\r\n");            /*!< Gets upto 5 bytes from host, reverts and send them back */
	PRINTF("*3.  Send_To_Sleep\r\n");           /*!< Sends OL2385 to power off mode */
	PRINTF("*4.  Send_Payload\r\n");            /*!< Payload is sent from host to OL2385 */
	PRINTF("*5.  Send_bit\r\n");                /*!< Send a standard SIGFOX frame with null customer payload*/
	PRINTF("*6.  Send_Out_of_Band\r\n");        /*!< Send out of band message */
	PRINTF("*7.  Receive_Frame\r\n");           /*!< Receives a sigfox message */
	PRINTF("*8.  Get_info\r\n");                /*!< Gets device ID and Sigfox library version */
	PRINTF("*9.  Set_UL_frequency\r\n");        /*!< Sets Uplink Frquency of OL2385 */
	PRINTF("*10. Get_UL_frequency\r\n");        /*!< Gets Uplink Frquency of OL2385 */
	PRINTF("*11. Continuous_Wave\r\n");         /*!< Starts the carrier */
	PRINTF("*12. Check_ID_Key\r\n");            /*!< Check if ID and Key are flashed correctly on board */
	PRINTF("*13. Get_Device_Version\r\n");      /*!< Gets device version of OL2385 board */
	PRINTF("*14. Set_Watchdog_Timer\r\n");      /*!< Sets the value os Watchdog timer */
	PRINTF("*15. Get_Watchdog_Timer\r\n");      /*!< Gets the value os Watchdog timer */
	PRINTF("*16. Set_Register (not implemented in OL2385 firmware)\r\n");          /*!< Sets the value of a register */
	PRINTF("*17. Get_Register (not implemented in OL2385 firmware)\r\n");          /*!< Gets the value of a register */
	PRINTF("*18. Trigger_Watchdog\r\n");        /*!< Triggers watchdog */
	PRINTF("*19. Keep_Alive_Message (not implemented in OL2385 firmware)\r\n");    /*!< Sends a keep alive message */
	PRINTF("*20. Send_Test_Mode\r\n");        	/*!< This function is used for protocol/RF/sensitivity tests */
	PRINTF("*21. Change to RCZ1\r\n");          /*!< Sets to European standard ETSI. */
	PRINTF("*22. Change to RCZ2\r\n");          /*!< Sets to USA standard FCC. */
	PRINTF("*23. Change to RCZ3\r\n");          /*!< Sets to Japan/Korea standard ARIB. */
	PRINTF("*24. Change to RCZ4\r\n");          /*!< Sets to South American standard FCC. */
	PRINTF("*25. Send GPS data\r\n");
	PRINTF("****************************************************************************\r\n");
}

/*FUNCTION**********************************************************************
 *
 * Function Name : GetUserInput
 * Description : Reads from stdin which is attached to uart.
 *				 Input is limited by given length and user must press enter to
 *				 confirm message.
 *
 *END**************************************************************************/
uint8_t GetUserInput(char *str, uint8_t length)
{
	char c = 0;
	uint8_t read = 0;

	while (((c != '\r') && (c != '\n')) && (read < length))
	{
		c = GETCHAR();
		PUTCHAR(c);
		if ((c != '\r') && (c != '\n'))
		{
			str[read] = c;
			read++;
		}
	}

	if (read == length)
	{
		while((c != '\r') && (c != '\n'))
		{
			c = GETCHAR();
		}
		PRINTF("\r\n");
	}

	str[read] = '\0';

	return read;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : GetUserCommand
 * Description : Parses user's input to get command selection.
 *
 *END**************************************************************************/
sf_spi_cmd_t GetUserCommand(void)
{
	char cmdStr[3];
	uint8_t cmd = 0;
	uint8_t read = 0U;

	PRINTF("[Choose a command to be sent and press enter]\r\n");
	PRINTF("For list of available commands press [?]\r\n");

	while (1)
	{
		read = GetUserInput(cmdStr, 2U);

		if (cmdStr[0] == '?')
		{
			PrintCommands();
		}
		else
		{
			cmd = atoi(cmdStr);

			if ((read == 0) || (cmd > 25U) || (!isdigit((uint8_t)cmdStr[0]) || \
				((read > 1) && !isdigit((uint8_t)cmdStr[1]))))
			{
				PRINTF("Please enter valid decimal number in range (0 - 24)\r\n");
			}
			else
			{
				break;
			}
		}
	}

	return (sf_spi_cmd_t)cmd;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : ProcessCommand
 * Description : Invokes handler function of given command.
 *
 *END**************************************************************************/
status_t ProcessCommand(sf_drv_data_t *sfDrvData, uint8_t cmd)
{
	status_t status = kStatus_Success;

	//AML_ASSERT(cmd <= sfSpiCmdChangeToRCZ4);

	status = (*CmdHandlerArr[cmd - 1])(sfDrvData);

	return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HandleWakeUp
 * Description : Handler function for 'Wake up' command.
 *
 *END**************************************************************************/
static status_t HandleWakeUp(sf_drv_data_t *sfDrvData)
{
	status_t status = kStatus_Success;

	AML_ASSERT(sfDrvData != NULL);

	PRINTF("/* Wake Up command */\r\n");

	status = SF_WakeUp(sfDrvData);

	if (status != kStatus_Success)
	{
		PrintDeviceState(sfDrvData); /* Prints OL2385 firmware status. */
		PRINTF("WakeUp command failed with error code %d.\r\n", status);
	}
	else
	{
		PRINTF("WakeUp command passed.\r\n");
	}

	return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HandleEcho
 * Description : Handler function for 'Echo' command.
 *
 *END**************************************************************************/
static status_t HandleEcho(sf_drv_data_t *sfDrvData)
{
	status_t status = kStatus_Success;
	bool success = true;

	AML_ASSERT(sfDrvData != NULL);

	PRINTF("/* Echo command */\r\n");
	PRINTF("Sending '0x0102 0304 05' via SPI\r\n");

	status = SF_TestSpiCon(sfDrvData, &success);
	if (status != kStatus_Success)
	{
		PrintDeviceState(sfDrvData);
		PRINTF("Echo command failed with error code %d.\r\n", status);
	}
	else
	{
		if (!success) {
			PRINTF("Received echo is wrong.\r\n");
		}
		else
		{
			PRINTF("Received echo is: 0x0102 0304 05 (correct).\r\n");
			PRINTF("Echo command passed.\r\n");
		}
	}

	return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HandleSleep
 * Description : Handler function for 'Sleep' command.
 *
 *END**************************************************************************/
static status_t HandleSleep(sf_drv_data_t *sfDrvData)
{
	status_t status = kStatus_Success;

	AML_ASSERT(sfDrvData != NULL);

	PRINTF("/* Sleep command */\r\n");

	status = SF_Sleep(sfDrvData);

	if (status != kStatus_Success)
	{
		PrintDeviceState(sfDrvData); /* Prints OL2385 firmware status. */
		PRINTF("Sleep command failed with error code %d.\r\n", status);
	}
	else
	{
		PRINTF("Sleep command passed.\r\n");
	}

	return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HandleSendPayload
 * Description : Handler function for 'Send payload' command.
 *
 *END**************************************************************************/
static status_t HandleSendPayload(sf_drv_data_t *sfDrvData)
{
	status_t status = kStatus_Success;
	char *msg[13];
	uint8_t read;
	sf_msg_payload_t data;

	AML_ASSERT(sfDrvData != NULL);

	PRINTF("/* Send payload command */\r\n");

	PRINTF("\tPlease enter up to 12 characters of the message.\r\n");

	while (1)
	{
		read = GetUserInput((char*)msg, 12);

		if (read == 0)
		{
			PRINTF("\tEmpty string was passed.\r\n");
		}
		else
		{
			break;
		}
	}

	PRINTF("Sending payload message: %s\r\n", msg);

	data.payload = (uint8_t*)msg;
	data.payloadLen = read;

	status = SF_SendPayload(sfDrvData, &data);

	if (status != kStatus_Success)
	{
		PrintDeviceState(sfDrvData); /* Prints OL2385 firmware status. */
		PRINTF("SendPayload command failed with error code %d.\r\n", status);
	}
	else
	{
		PRINTF("SendPayload command passed.\r\n");
	}

	return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HandleSendBit
 * Description : Handler function for 'Send bit' command.
 *
 *END**************************************************************************/
static status_t HandleSendBit(sf_drv_data_t *sfDrvData)
{
	status_t status = kStatus_Success;

	AML_ASSERT(sfDrvData != NULL);

	PRINTF("/* Send bit command */\r\n");

	status = SF_SendBit(sfDrvData);

	if (status != kStatus_Success)
	{
		PrintDeviceState(sfDrvData); /* Prints OL2385 firmware status. */
		PRINTF("SendBit command failed with error code %d.\r\n", status);
	}
	else
	{
		PRINTF("SendBit command passed.\r\n");
	}

	return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HandleSendOoB
 * Description : Handler function for 'Send out of band' command.
 *
 *END**************************************************************************/
static status_t HandleSendOoB(sf_drv_data_t *sfDrvData)
{
	status_t status = kStatus_Success;

	AML_ASSERT(sfDrvData != NULL);

	PRINTF("/* Send out of band command */\r\n");

	status = SF_SendOutOfBand(sfDrvData);

	if (status != kStatus_Success)
	{
		PrintDeviceState(sfDrvData); /* Prints OL2385 firmware status. */
		PRINTF("SendOutOfBand command failed with error code %d.\r\n", status);
	}
	else
	{
		PRINTF("SendOutOfBand command passed.\r\n");
	}

	return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HandleRcvFrame
 * Description : Handler function for 'Receive frame' command.
 *
 *END**************************************************************************/
static status_t HandleRcvFrame(sf_drv_data_t *sfDrvData)
{
	status_t status = kStatus_Success;
	uint8_t data[12];
	sf_msg_payload_t msg = {12U, data};

	AML_ASSERT(sfDrvData != NULL);

	PRINTF("/* Receive frame command */\r\n");
	PRINTF("\tWaiting for data from SIGFOX network\r\n");

	status = SF_ReceiveMessage(sfDrvData, &msg);

	if (status != kStatus_Success)
	{
		PrintDeviceState(sfDrvData); /* Prints OL2385 firmware status. */
		PRINTF("ReceiveFrame command failed with error code %d.\r\n", status);
	}
	else
	{
		PrintPayload(&msg);
		PRINTF("ReceiveFrame command passed.\r\n");
	}

	return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HandleGetInfo
 * Description : Handler function for 'Get info' command.
 *
 *END**************************************************************************/
static status_t HandleGetInfo(sf_drv_data_t *sfDrvData)
{
    status_t status = kStatus_Fail;
    uint8_t rcvPayloadData[SF_ACK_SPI_MSG_MAX_B];  /* Buffer containing received payload. */
    sf_msg_payload_t sndPayload = {0};             /* Dummy data to be sent. */
    sf_msg_payload_t rcvPayload =                  /* Received payload. */
    {
            SF_ACK_SPI_MSG_MAX_B,
            rcvPayloadData
    };
    uint8_t pldOf = 0U;                            /* Offset in the payload. */

    status = SF_SendCommand(sfDrvData, SF_CMD_GET_INFO_ID, &sndPayload, &rcvPayload,
            SF_ACK_SPI_MSG_MAX_B);

    if (rcvPayload.payloadLen != SF_GET_INFO_ACK_PLD_B)
    {
        status = kStatus_Fail;
    }

    if (kStatus_Success == status)
    {
        pldOf = SF_DEV_ID_OF;
        PRINTF("\tDevice ID = 0x");
        PRINTF("%02x %02x ", rcvPayloadData[pldOf + 3], rcvPayloadData[pldOf + 2]);
        PRINTF("%02x %02x\r\n", rcvPayloadData[pldOf + 1], rcvPayloadData[pldOf + 0]);

        pldOf = SF_DEV_PAC_OF;
        PRINTF("\tPAC = 0x");
        PRINTF("%02x %02x ", rcvPayloadData[pldOf], rcvPayloadData[pldOf + 1]);
        PRINTF("%02x %02x ", rcvPayloadData[pldOf + 2], rcvPayloadData[pldOf + 3]);
        PRINTF("%02x %02x ", rcvPayloadData[pldOf + 4], rcvPayloadData[pldOf + 5]);
        PRINTF("%02x %02x\r\n", rcvPayloadData[pldOf + 6], rcvPayloadData[pldOf + 7]);

        pldOf = SF_LIB_VER_OF;
        PRINTF("\tLibrary version = 0x");
        PRINTF("%02x ", rcvPayloadData[pldOf + 10]);
        PRINTF("%02x %02x ", rcvPayloadData[pldOf + 9], rcvPayloadData[pldOf + 8]);
        PRINTF("%02x %02x ", rcvPayloadData[pldOf + 7], rcvPayloadData[pldOf + 6]);
        PRINTF("%02x %02x ", rcvPayloadData[pldOf + 5], rcvPayloadData[pldOf + 4]);
        PRINTF("%02x %02x ", rcvPayloadData[pldOf + 3], rcvPayloadData[pldOf + 2]);
        PRINTF("%02x %02x\r\n", rcvPayloadData[pldOf + 1], rcvPayloadData[pldOf]);
        PRINTF("GetInfo command passed.\r\n");
    }
    else
    {
        PrintDeviceState(sfDrvData); /* Prints OL2385 firmware status. */
        PRINTF("GetInfo command failed with error code %d.\r\n", status);
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HandleSetUlFreq
 * Description : Handler function for 'Set UL frequency' command.
 *
 *END**************************************************************************/
static status_t HandleSetUlFreq(sf_drv_data_t *sfDrvData)
{
	status_t status = kStatus_Success;
	char freqStr[10];
	uint32_t freq = 0U;
	uint8_t read = 0U;

	AML_ASSERT(sfDrvData != NULL);

	PRINTF("/* Set uplink frequency command */\r\n");
	PRINTF("\tPlease enter uplink frequency in Hz.\r\n");
	PRINTF("\tAdmissible range is from 775 000 000 Hz up to 1 100 000 000 Hz.\r\n");

	while (1)
	{
		read = GetUserInput(freqStr, 9);
		freq = atoi(freqStr);

		if ((read == 0) || (freq < SF_UL_FREQ_MIN_HZ) || (freq > SF_UL_FREQ_MAX_HZ))
		{
			PRINTF("\tPlease enter frequency in admissible range.\r\n");
		}
		else
		{
			break;
		}
	}

	PRINTF("Uplink frequency will be set to [%u Hz].\r\n", freq);

	status = SF_SetUlFrequency(sfDrvData, freq);

	if (status != kStatus_Success)
	{
		PrintDeviceState(sfDrvData); /* Prints OL2385 firmware status. */
		PRINTF("SetUlFrequency command failed with error code %d.\r\n", status);
	}
	else
	{
		PRINTF("SetUlFrequency command passed.\r\n");
	}

	return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HandleGetUlFreq
 * Description : Handler function for 'Get UL frequency' command.
 *
 *END**************************************************************************/
static status_t HandleGetUlFreq(sf_drv_data_t *sfDrvData)
{
	status_t status = kStatus_Success;
	uint32_t freq;

	AML_ASSERT(sfDrvData != NULL);

	PRINTF("/* Get uplink frequency command */\r\n");

	status = SF_GetUlFrequency(sfDrvData, &freq);

	if (status != kStatus_Success)
	{
		PrintDeviceState(sfDrvData); /* Prints OL2385 firmware status. */
		PRINTF("GetUlFrequency command failed with error code %d.\r\n", status);
	}
	else
	{
		PRINTF("Uplink frequency is set to [%d Hz].\r\n", freq);
		PRINTF("GetUlFrequency command passed.\r\n");
	}

	return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HandleContWave
 * Description : Handler function for 'Continuous wave' command.
 *
 *END**************************************************************************/
static status_t HandleContWave(sf_drv_data_t *sfDrvData)
{
	status_t status = kStatus_Success;

	AML_ASSERT(sfDrvData != NULL);

	PRINTF("/* Continuous wave command */\r\n");

	status = SF_GenerateContWave(sfDrvData);

	if (status != kStatus_Success)
	{
		PrintDeviceState(sfDrvData); /* Prints OL2385 firmware status. */
		PRINTF("GenerateContWave command failed with error code %d.\r\n", status);
	}
	else
	{
		PRINTF("GenerateContWave command passed.\r\n");
	}

	return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HandleCheckIdKey
 * Description : Handler function for 'Check ID key' command.
 *
 *END**************************************************************************/
static status_t HandleCheckIdKey(sf_drv_data_t *sfDrvData)
{
	status_t status = kStatus_Success;
	bool success = true;

	AML_ASSERT(sfDrvData != NULL);

	PRINTF("/* Check ID key command */\r\n");

	status = SF_CheckIdKey(sfDrvData, &success);

	if (status != kStatus_Success)
	{
		PrintDeviceState(sfDrvData); /* Prints OL2385 firmware status. */
		PRINTF("CheckIdKey command failed with error code %d.\r\n", status);
	}
	else
	{
		if (!success)
		{
			PRINTF("ID key is missing/corrupted.\r\n");
		}
		else
		{
			PRINTF("ID key is OK.\r\n");
			PRINTF("CheckIdKey command passed.\r\n");
		}
	}

	return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HandleGetDevVer
 * Description : Handler function for 'Get device version' command.
 *
 *END**************************************************************************/
static status_t HandleGetDevVer(sf_drv_data_t *sfDrvData)
{
	status_t status = kStatus_Success;
	sf_msg_payload_t sendPayload;
	sf_msg_payload_t rcvPayload;
	uint8_t rcvData[SF_GET_DEV_VER_ACK_PLD_B];

	AML_ASSERT(sfDrvData != NULL);

	PRINTF("/* Get device version command */\r\n");

    sendPayload.payload = NULL;
	sendPayload.payloadLen = 0U;

	rcvPayload.payload = rcvData;
	rcvPayload.payloadLen = SF_GET_DEV_VER_ACK_PLD_B;

    status = SF_SendCommand(sfDrvData, sfSpiCmdGetDevVer, &sendPayload, \
    		&rcvPayload, SF_GET_DEV_VER_ACK_PLD_B);

	if (status != kStatus_Success)
	{
		PrintDeviceState(sfDrvData); /* Prints OL2385 firmware status. */
		PRINTF("GetDeviceVersion command failed with error code %d.\r\n", status);
	}
	else if (rcvPayload.payloadLen != SF_GET_DEV_VER_ACK_PLD_B)
	{
		PRINTF("Host received less data (%d) than expected (%d).\r\n",
		        rcvPayload.payloadLen, SF_GET_DEV_VER_ACK_PLD_B);
	}
	else
	{
	    PRINTF("Device version: ");
	    PRINTF("0x%02x%02x ", rcvPayload.payload[14], rcvPayload.payload[13]);
	    PRINTF("%02x%02x ", rcvPayload.payload[12], rcvPayload.payload[11]);
	    PRINTF("%02x%02x ", rcvPayload.payload[10], rcvPayload.payload[9]);
	    PRINTF("%02x%02x ", rcvPayload.payload[8], rcvPayload.payload[7]);
	    PRINTF("%02x%02x ", rcvPayload.payload[6], rcvPayload.payload[5]);
	    PRINTF("%02x%02x ", rcvPayload.payload[4], rcvPayload.payload[3]);
	    PRINTF("%02x%02x ", rcvPayload.payload[2], rcvPayload.payload[1]);
	    PRINTF("%02x\r\n", rcvPayload.payload[0]);

		PRINTF("GetDeviceVersion command passed.\r\n");
	}

	return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HandleSetWdTmr
 * Description : Handler function for 'Set watchdog timer' command.
 *
 *END**************************************************************************/
static status_t HandleSetWdTmr(sf_drv_data_t *sfDrvData)
{
	status_t status = kStatus_Success;
	sf_msg_payload_t sendPayload;
	char 	 wdTimeoutStr[3];
	uint8_t  wdTimeout = 0U;
	uint8_t  read = 0U;

	AML_ASSERT(sfDrvData != NULL);

	PRINTF("/* Set WD timer command */\r\n");

	PRINTF("Select watchdog timeout period and press enter:\r\n");
	PRINTF("\t[0] - 0.016 seconds\r\n");
	PRINTF("\t[1] - 0.032 seconds\r\n");
	PRINTF("\t[2] - 0.064 seconds\r\n");
	PRINTF("\t[3] - 0.128 seconds\r\n");
	PRINTF("\t[4] - 0.256 seconds\r\n");
	PRINTF("\t[5] - 0.512 seconds\r\n");
	PRINTF("\t[6] - 1.024 seconds\r\n");
	PRINTF("\t[7] - 2.048 seconds\r\n");
	PRINTF("\t[8] - 4.096 seconds\r\n");
	PRINTF("\t[9] - 8.192 seconds\r\n");
	PRINTF("\t[10] - 16.384 seconds\r\n");
	PRINTF("\t[11] - 32.768 seconds\r\n");
	PRINTF("\t[12] - 65.536 seconds\r\n");

	while (1)
	{
		read = GetUserInput(wdTimeoutStr, 2);
		wdTimeout = atoi(wdTimeoutStr);

		if ((read == 0) || (wdTimeout > sfWdTime_65_536))
		{
			PRINTF("\tPlease enter valid decimal number in range (0-12)\r\n");
		}
		else
		{
			break;
		}
	}

	PRINTF("\tWD timeout option [%d] was selected.\r\n", wdTimeout);

    sendPayload.payload = &wdTimeout;
	sendPayload.payloadLen = SF_SET_WD_TIMER_INF_PLD_B;

    status = SF_SendCommand(sfDrvData, sfSpiCmdSetWdTimer, &sendPayload, NULL, 0U);

	if (status != kStatus_Success)
	{
		PrintDeviceState(sfDrvData); /* Prints OL2385 firmware status. */
		PRINTF("SetWdTimer command failed with error code %d.\r\n", status);
	}
	else
	{
		PRINTF("SetWdTimer command passed.\r\n");
	}

	return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HandleGetWdTmr
 * Description : Handler function for 'Get watchdog timer' command.
 *
 *END**************************************************************************/
static status_t HandleGetWdTmr(sf_drv_data_t *sfDrvData)
{
	status_t status = kStatus_Success;
	sf_msg_payload_t sendPayload;
	sf_msg_payload_t rcvPayload;
	uint8_t rcvData = 0;

	AML_ASSERT(sfDrvData != NULL);

	PRINTF("/* Get WD timer command */\r\n");

    sendPayload.payload = NULL;
	sendPayload.payloadLen = 0U;

	rcvPayload.payload = &rcvData;
	rcvPayload.payloadLen = 1U;

    status = SF_SendCommand(sfDrvData, sfSpiCmdGetWdTimer, &sendPayload, \
    		&rcvPayload, 1U);

	if (status != kStatus_Success)
	{
		PrintDeviceState(sfDrvData); /* Prints OL2385 firmware status. */
		PRINTF("GetWdTimer command failed with error code %d.\r\n", status);
	}
	else if (rcvPayload.payloadLen != SF_GET_WD_TIMER_ACK_PLD_B)
	{
		PRINTF("Host received less data (%d) than expected (%d).\r\n",
		        rcvPayload.payloadLen, SF_GET_WD_TIMER_ACK_PLD_B);
	}
	else
	{
		PrintPayload(&rcvPayload);
		PRINTF("GetWdTimer command passed.\r\n");
	}

	return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HandleSetReg
 * Description : Handler function for 'Set register' command.
 *
 *END**************************************************************************/
static status_t HandleSetReg(sf_drv_data_t *sfDrvData)
{
	status_t status = kStatus_Success;

	AML_ASSERT(sfDrvData != NULL);

	PRINTF("/* Set register command (not implemented) */\r\n");

	/* Not implemented in firmware. */

	return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HandleGetReg
 * Description : Handler function for 'Get register' command.
 *
 *END**************************************************************************/
static status_t HandleGetReg(sf_drv_data_t *sfDrvData)
{
	status_t status = kStatus_Success;

	AML_ASSERT(sfDrvData != NULL);

	PRINTF("/* Get register command (not implemented) */\r\n");

	/* Not implemented in firmware. */

	return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HandleTrigWd
 * Description : Handler function for 'Trigger watchdog' command.
 *
 *END**************************************************************************/
static status_t HandleTrigWd(sf_drv_data_t *sfDrvData)
{
	status_t status = kStatus_Success;

	AML_ASSERT(sfDrvData != NULL);

	PRINTF("/* Trigger WD command */\r\n");

	status = SF_TriggerWatchdog(sfDrvData);

	if (status != kStatus_Success)
	{
		PrintDeviceState(sfDrvData); /* Prints OL2385 firmware status. */
		PRINTF("TriggerWatchdog command failed with error code %d.\r\n", status);
	}
	else
	{
		PRINTF("TriggerWatchdog command passed.\r\n");
	}

	return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HandleKeepAlive
 * Description : Handler function for 'Keep alive' command.
 *
 *END**************************************************************************/
static status_t HandleKeepAlive(sf_drv_data_t *sfDrvData)
{
	status_t status = kStatus_Success;

	AML_ASSERT(sfDrvData != NULL);

	PRINTF("/* Keep alive command (not implemented) */\r\n");

	/* Not implemented in firmware. */

	return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HandleTestMode
 * Description : Handler function for 'Test mode' command.
 *
 *END**************************************************************************/
static status_t HandleTestMode(sf_drv_data_t *sfDrvData)
{
	status_t status = kStatus_Success;
	char testModeStr[2];
	uint8_t read = 0U;
	const char *testModes[6] = {"TxBpsk", "TxProtocol", "RxProtocol", "RxGfsk", "RxSensi", "TxSynth"};
	sf_test_mode_t testMode;

	AML_ASSERT(sfDrvData != NULL);

	PRINTF("/* Test mode command */\r\n");
	PRINTF("Select test mode and press enter:\r\n");
	PRINTF("\t[0] TxBpsk - Sending PRBS data in a 26 Bytes frame at constant frequency.\r\n");
	PRINTF("\t[1] TxProtocol - Test of the complete protocol in up-link.\r\n");
	PRINTF("\t[2] RxProtocol - Test of the complete protocol in down-link.\r\n");
	PRINTF("\t[3] RxGfsk - Receiving constant GFSK frames at constant frequency.\r\n");
	PRINTF("\t[4] RxSensi - Measurement of the real sensitivity of device.\r\n");
	PRINTF("\t[5] TxSynth - Sending SigFox frames with 4 Bytes payload at forced frequency.\r\n");

	while (1)
	{
		read = GetUserInput(testModeStr, 1);
		testMode = (sf_test_mode_t)atoi(testModeStr);

		if ((read == 0) || (testMode > sfTestModTxSynth))
		{
			PRINTF("\tPlease enter valid decimal number in range (0-5)\r\n");
		}
		else
		{
			break;
		}
	}

	PRINTF("\tTest mode [%s] was selected.\r\n", testModes[testMode]);

	status = SF_TestDevice(sfDrvData, testMode);

	if (status != kStatus_Success)
	{
		PrintDeviceState(sfDrvData); /* Prints OL2385 firmware status. */
		PRINTF("TestDevice command failed with error code %d.\r\n", status);
	}
	else
	{
		PRINTF("Test mode was [%s] successful.\r\n", testModes[testMode]);
	}

	return status;
}

static status_t HandleChangeNetStandard(sf_drv_data_t *sfDrvData, sf_net_standard_t netStandard)
{
    status_t status = kStatus_Success;

    AML_ASSERT(sfDrvData != NULL);

    PRINTF("/* Change to RCZ%u */\r\n", (uint8_t)netStandard + 1U);

    status = SF_ChangeNetworkStandard(sfDrvData, netStandard);

    if (status != kStatus_Success)
    {
        PrintDeviceState(sfDrvData); /* Prints OL2385 firmware status. */
        PRINTF("Change to RCZ%u command failed with error code %d.\r\n",
                netStandard + 1U, status);
    }
    else
    {
        PRINTF("Change to RCZ%u command passed.\r\n", netStandard + 1U);
    }

    return status;
}

static status_t HandleChangeToRCZOne(sf_drv_data_t *sfDrvData)
{
    return HandleChangeNetStandard(sfDrvData, sfNetStandardETSI);
}

static status_t HandleChangeToRCZTwo(sf_drv_data_t *sfDrvData)
{
    return HandleChangeNetStandard(sfDrvData, sfNetStandardFCC_USA);
}

static status_t HandleChangeToRCZThree(sf_drv_data_t *sfDrvData)
{
    return HandleChangeNetStandard(sfDrvData, sfNetStandardARIB);
}

static status_t HandleChangeToRCZFour(sf_drv_data_t *sfDrvData)
{
    return HandleChangeNetStandard(sfDrvData, sfNetStandardFCC_SouthAmerica);
}

static status_t HandlerSendPkg(sf_drv_data_t *sfDrvData)
{
	sf_msg_payload_t data;
	status_t status = kStatus_Success;

	PRINTF("Sending payload message: %s\r\n", msg);

	data.payload = (uint8_t*)msg;
	data.payloadLen = 12;

	status = SF_SendPayload(sfDrvData, &data);

	if (status != kStatus_Success)
	{
		PrintDeviceState(sfDrvData); /* Prints OL2385 firmware status. */
		PRINTF("SendPayload command failed with error code %d.\r\n", status);
	}
	else
	{
		PRINTF("SendPayload command passed.\r\n");
	}

	return status;
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
