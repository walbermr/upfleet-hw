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
 * @file sf.c
 *
 * SIGFOX driver based on AML layer supporting boards based on following NXP
 * parts: OL2361, OL2385.
 *
 * This module is common for all supported models.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "sf.h"
#include "../aml/spi_aml/spi_aml.h"
#include "../aml/gpio_aml.h"
#include "../aml/wait_aml/wait_aml.h"

/* Enables basic debugging messages printed to the serial console (content
 * of sent and received SPI frames). */
/* #define DEBUG_SPI_COM */
/* Enables debugging messages related to UL frequency calculation. */
/* #define DEBUG_FREQ_CALC */

#if ((defined(DEBUG_FREQ_CALC) || defined(DEBUG_SPI_COM)) && (SDK_VERSION == SDK_2_0))
/* Debug messages (for SDK 2.0 only). */
#include "fsl_debug_console.h"
#endif

/*******************************************************************************
 * Defines
 ******************************************************************************/
/*! Timeout used to wait for a level of ACK pin. It is intended for SPI commands
 * which do not cause transmission to SIGFOX network (i.e. Send Echo,...). */
#define SF_ACK_NOTRANS_TIMEOUT_US   2000000U

/*! Timeout used to wait for a level of ACK pin. It is intended for SPI commands
 * causing transmission to SIGFOX network (Send Payload, Send Bit,
 * Send Out of Band, Send Test). The Receive Frame command has higher timeout.
 * Note: waiting for an acknowledge frame can take several second.
 * Max. time is needed for Send Test Mode command (test mode number 5, TxSynth). */
#define SF_ACK_TRANS_TIMEOUT_US     15000000U

/*! Timeout used to wait for a level of ACK pin. It is intended for the
 * Receive Frame SPI command. The SIGFOX firmware has an internal timeout (60 s). */
#define SF_ACK_TRANS_RECV_TIMEOUT_US 60000000U

/*! Timeout used to wait for a level of ACK pin. It is applied after the reset
 * in the SF_Init function. */
#define SF_ACK_INIT_TIMEOUT_US      5000000U

/*! Max. value of timeout used to wait for a level of ACK pin. */
#define SF_ACK_TIMEOUT_MAX_US       100000000U

/*! Timeout measurement is split into steps. This macro defines value of the
 * step in us. */
#define SF_TIMEOUT_STEP_US          100U

/*! Delay applied after the Send Wake up command. */
#define SF_WAKEUP_DLY_MS            100U

/*! The payload used for the "Send Echo" command. */
#define SF_ECHO_PLD_ARRAY           {0x01U, 0x02U, 0x03U, 0x04U, 0x05U}

/*!
 * @brief This macro returns true when an I-frame has payload.
 * Otherwise it returns false.
 *
 * @param cmd Command number (see sf_spi_cmd_t enumeration).
 */
#define SF_HAS_CMD_PLD(cmd) \
    (((cmd) == sfSpiCmdSendEcho) || ((cmd) == sfSpiCmdSendPayload) || ((cmd) == sfSpiCmdSetUlFreq) || \
            ((cmd) == sfSpiCmdSetWdTimer) || ((cmd) == sfSpiCmdSetReg) || \
            ((cmd) == sfSpiCmdGetReg) || ((cmd) == sfSpiCmdSendTestMode))
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#if ((defined(DEBUG_FREQ_CALC) || defined(DEBUG_SPI_COM)) && (SDK_VERSION == SDK_2_0))
/*
 * @brief This function prints a frame to the serial console output.
 *
 * @param frame Pointer to the frame.
 * @param size  Size of the frame in bytes.
 */
static void SF_PrintFrame(const uint8_t *frame, uint8_t size);
#endif

/*
 * @brief This function creates the SPI frame according to the defined format.
 *
 * @param cmd        The frame command number.
 * @param payloadLen Size of the frame payload in bytes.
 * @param payload    Payload of the frame.
 * @param frame      Pointer where the resulting frame is stored. Length
 * of the array must be equal to 2 + payload length (frame header).
 */
static void SF_PackFrame(sf_spi_cmd_t cmd, uint8_t payloadLen,
        const uint8_t *payload, uint8_t *frame);

/*
 * @brief This function sends data via SPI, received data are ignored.
 *
 * @param drvData    Driver run-time data.
 * @param amlSpiData SPI transfer structure.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
static status_t SF_SendSpiData(sf_drv_data_t *drvData,
        spi_aml_transfer_t *amlSpiData);

/*
 * @brief This function receives data via SPI. Note that it sends dummy data
 * (all bits equal to 1).
 *
 * @param drvData    Driver run-time data.
 * @param amlSpiData SPI transfer structure.
 * @param tmoutUs    Timeout in ms. It is applied when waiting for the low level
 *                   of ACK pin.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
static status_t SF_ReceiveSpiData(sf_drv_data_t *drvData,
        spi_aml_transfer_t *amlSpiData, uint32_t tmoutUs);

/*
 * @brief This function waits until a GPIO pin has desired value.
 *
 * @param gpioPin   A GPIO configuration structure.
 * @param pinValExp Desired level of the pin (1 - log. 1, 0 - log. 0).
 * @param tmoutUs   Timeout in us.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
static status_t SF_WaitPinLevel(const sf_gpio_sel_t* gpioPin, uint8_t pinValExp,
        uint32_t tmoutUs);

/*
 * @brief This function creates a SPI frame, sends a SPI command to the device,
 * receives an acknowledgement (if any) and stores an error code and state value
 * from the acknowledgement.
 *
 * @param drvData    Driver run-time data.
 * @param sendLen    Length of a frame to be sent (in bytes).
 * @param recvLen    Length of a frame to be received (in bytes).
 * @param sendBuffer Pointer to a SPI send buffer.
 * @param recvBuffer Pointer to a SPI receive buffer.
 * @param tmoutUs    Timeout in ms. It is applied when waiting for the low level
 *                   of ACK pin.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
static status_t SF_SendCommandInt(sf_drv_data_t *drvData,
        uint8_t sendLen, uint8_t recvLen,
        uint8_t *sendBuffer, uint8_t *recvBuffer, uint32_t tmoutUs);

/*
 * @brief This function calculates values of SIGFOX device registers FCINT
 * and FCFRAC.
 *
 * It is common for OL2361 and OL2385.
 *
 * @param freqHz   Up-link frequency in Hz.
 * @param fcint    Pointer where the integer divider parameter FCINT.
 * @param fcfrac   Pointer where the fractional divider parameter FCFRAC.
 */
static void SF_CalcUlRegs(uint32_t freqHz, uint32_t *fcint, uint32_t *fcfrac);

/*
 * @brief This function calculates the up-link frequency with use of SIGFOX
 * device registers FREQCON0 and FREQCON1.
 *
 * @param fcint  Value of the integer divider parameter FCINT.
 * @param fcfrac Value of the fractional divider parameter FCFRAC.
 *
 * @return Up-link frequency in Hz.
 */
static uint32_t SF_CalcUlFreq(uint32_t fcint, uint32_t fcfrac);

/*!
 * @brief This function fills the SPI frame with register values.
 *
 * @param fcint  Value of FCINT register.
 * @param fcfrac Value of FCFRAC register.
 * @param frame  Pointer to the SPI frame where the register values will be stored.
 */
static void SF_StoreRegs(uint32_t fcint, uint32_t fcfrac, uint8_t *frame);

/*!
 * @brief This function fills the FCFRAC and FCINT registers using the SPI frame.
 *
 * @param frame Pointer to the SPI frame.
 * @param fcint Pointer to the FCINT register.
 * @param fcfrac Pointer to the FCFRAC register.
 */
static void SF_RestoreRegs(const uint8_t *frame, uint32_t *fcint, uint32_t *fcfrac);

/*
 * @brief This function stores an error code and state values from a received
 * acknowledgement.
 *
 * @param drvData   Driver run-time data.
 * @param spiRcvBuf Pointer to a SPI buffer containing received data.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
static status_t SF_StoreDevState(sf_drv_data_t *drvData, const uint8_t *spiRcvBuf);

/*
 * @brief This function stores an error code and state values from a received
 * acknowledgement.
 *
 * @param spiRcvBuf    Pointer to a SPI buffer containing received data.
 * @param recvPayload  Pointer where a resulting payload is stored.
 * @param recvPayloadBufSize Size of the paylaod buffer in bytes.
 */
static status_t SF_StorePayload(const uint8_t *spiRcvBuf, sf_msg_payload_t *recvPayload,
        uint8_t recvPayloadBufSize);

/*******************************************************************************
 * Code - internal functions
 ******************************************************************************/

#if ((defined(DEBUG_FREQ_CALC) || defined(DEBUG_SPI_COM)) && (SDK_VERSION == SDK_2_0))
/*FUNCTION**********************************************************************
 *
 * Function Name : SF_PrintFrame
 * Description   : This function prints a frame to the serial console output.
 *
 *END**************************************************************************/
static void SF_PrintFrame(const uint8_t *frame, uint8_t size)
{
    uint8_t i = 0U;

    PRINTF("0x");
    for (i = 0U; i < size; i++)
    {
        PRINTF("%02x ", *(frame + i));
    }
    PRINTF("\r\n");
}
#endif

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_PackFrame
 * Description   : This function creates the SPI frame according to the defined format.
 *
 *END**************************************************************************/
static void SF_PackFrame(sf_spi_cmd_t cmd, uint8_t payloadLen,
        const uint8_t *payload, uint8_t *frame)
{
    /* Preconditions. */
    AML_ASSERT((cmd >= SF_CMD_FIRST_ID) && (cmd <= SF_CMD_LAST_ID));
    AML_ASSERT(payloadLen <= SF_INF_PAYLOAD_MAX_B);
    AML_ASSERT(frame != NULL);

    *(frame + SF_INF_LENGTH_OF) = payloadLen + SF_INF_HEADER_B;
    *(frame + SF_INF_CMD_OF) = (uint8_t)cmd;

    if ((payloadLen > 0U) && (payload != NULL))
    {   /* Store the payload. */
        memcpy((void*)(frame + SF_INF_PAYLOAD_OF),
                (void*)payload,
                (size_t)payloadLen);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_SendSpiData
 * Description   : This function sends data via SPI, received data are ignored.
 *
 *END**************************************************************************/
static status_t SF_SendSpiData(sf_drv_data_t *drvData,
        spi_aml_transfer_t *amlSpiData)
{
    status_t      status = kStatus_Fail;
    sf_gpio_sel_t *csPin = NULL;    /* SPI CS pin settings. */

    /* Preconditions. */
    AML_ASSERT(drvData != NULL);
    AML_ASSERT(amlSpiData != NULL);
    AML_ASSERT(amlSpiData->dataSize > 0U);
    /* Previous transfer must be complete. */
    AML_ASSERT(GPIO_AML_ReadInput(drvData->gpioConfig.csPin.gpioInstance,
            drvData->gpioConfig.csPin.gpioPinNumber) != 0U);

    csPin = &(drvData->gpioConfig.csPin);

    /* Wait until ACK pin is high to be sure that the previous transmission is
     * complete. */
    status = SF_WaitPinLevel(&(drvData->gpioConfig.ackPin), 1U,
            SF_ACK_NOTRANS_TIMEOUT_US);

    if (kStatus_Success == status)
    {
        /* Set CS pin low. */
        SPI_AML_MasterSelectDevice(csPin->gpioInstance, csPin->gpioPinNumber,
                spiPcsActiveLow);

        /* Wait until ACK pin goes low. */
        status = SF_WaitPinLevel(&(drvData->gpioConfig.ackPin), 0U,
                SF_ACK_NOTRANS_TIMEOUT_US);
    }

    if (kStatus_Success == status)
    {
        /* Send data and receive dummy data. */
        status = SPI_AML_MasterTransfer(drvData->spiConfig.spiInstance, amlSpiData);
    }

    if (kStatus_Success == status)
    {
        /* Wait until ACK goes high. */
        status = SF_WaitPinLevel(&(drvData->gpioConfig.ackPin), 1U,
                SF_ACK_NOTRANS_TIMEOUT_US);
    }

    /* Set CS pin high (even if there is an error). */
    SPI_AML_MasterUnselectDevice(csPin->gpioInstance, csPin->gpioPinNumber,
            spiPcsActiveLow);

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_ReceiveSpiData
 * Description   : This function receives data via SPI. Note that it sends dummy
 *                 data (all bits equal to 1).
 *
 *END**************************************************************************/
static status_t SF_ReceiveSpiData(sf_drv_data_t *drvData,
        spi_aml_transfer_t *amlSpiData, uint32_t tmoutUs)
{
    status_t      status = kStatus_Fail;
    sf_gpio_sel_t *csPin = &(drvData->gpioConfig.csPin); /* SPI CS pin settings. */
    uint8_t       dataSizeOld = 0U;    /* Expected size of a received frame */

    /* Preconditions. */
    AML_ASSERT(drvData != NULL);
    AML_ASSERT(amlSpiData != NULL);
    AML_ASSERT(amlSpiData->dataSize > 0U);
    /* Previous transfer must be complete. */
    AML_ASSERT(GPIO_AML_ReadInput(csPin->gpioInstance, csPin->gpioPinNumber) != 0U);

    dataSizeOld = amlSpiData->dataSize;

    /* Send dummy data (all bits equal to 1) */
    memset((void *)amlSpiData->txBuffer, 0xFFU, (size_t)(amlSpiData->dataSize));

    /* Wait until ACK pin goes low. It takes long time for some commands causing
     * SIGFOX communication (for example Send Payload). It is several seconds
     * in contrast to other commands (Send Echo,...) which take a few milliseconds. */
    status = SF_WaitPinLevel(&(drvData->gpioConfig.ackPin), 0U,
            tmoutUs);

    if (kStatus_Success == status)
    {
        /* Set CS pin low. */
        SPI_AML_MasterSelectDevice(csPin->gpioInstance, csPin->gpioPinNumber,
                spiPcsActiveLow);

        /* Receive the first byte with length of the frame. */
        amlSpiData->dataSize = 1U;
        status = SPI_AML_MasterTransfer(drvData->spiConfig.spiInstance, amlSpiData);
    }

    if (kStatus_Success == status)
    {
        /* Update the SPI data structure to receive the rest of the ACK frame. */
        amlSpiData->dataSize = *(amlSpiData->rxBuffer + SF_ACK_LENGTH_OF);
        if ((amlSpiData->dataSize > dataSizeOld) || (amlSpiData->dataSize == 0U))
        {   /* Size of the receive buffer is smaller than data to be received. */
            /* Set CS pin back to high. */
            SPI_AML_MasterUnselectDevice(csPin->gpioInstance, csPin->gpioPinNumber,
                    spiPcsActiveLow);
            status = kStatus_SF_SpiAckLength;
        }
    }

    if (kStatus_Success == status)
    {
        amlSpiData->dataSize -= 1;
        amlSpiData->txBuffer += 1;
        amlSpiData->rxBuffer += 1;

        /* Receive the rest of the frame. */
        status = SPI_AML_MasterTransfer(drvData->spiConfig.spiInstance, amlSpiData);

        /* Set position of SPI buffers back. */
        amlSpiData->dataSize += 1;
        amlSpiData->txBuffer -= 1;
        amlSpiData->rxBuffer -= 1;
    }

    if (kStatus_Success == status)
    {
        /* Wait until ACK goes high. */
        status = SF_WaitPinLevel(&(drvData->gpioConfig.ackPin), 1U, SF_ACK_NOTRANS_TIMEOUT_US);
    }
    /* Set CS pin high (even it there is an error). */
    SPI_AML_MasterUnselectDevice(csPin->gpioInstance, csPin->gpioPinNumber,
            spiPcsActiveLow);

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_WaitPinLevel
 * Description   : This function waits until a GPIO pin has desired value.
 *
 *END**************************************************************************/
static status_t SF_WaitPinLevel(const sf_gpio_sel_t* gpioPin, uint8_t pinValExp,
        uint32_t tmoutUs)
{
    volatile bool pinVal = false;       /* Value of a pin. */
    uint32_t      tmoutUsTemp = 0U;          /* Current time in us. */

    AML_ASSERT(gpioPin != NULL);
    /* Max. timeout is 20 s (it covers all cases). */
    AML_ASSERT((tmoutUs > 0U) && (tmoutUs < SF_ACK_TIMEOUT_MAX_US));

    do
    {
        pinVal = GPIO_AML_ReadInput(gpioPin->gpioInstance, gpioPin->gpioPinNumber);
        tmoutUsTemp += SF_TIMEOUT_STEP_US;

        /* WaitUS function ensures the timeout value is not less than a desired
         * time (tmoutUs). */
        WAIT_AML_WaitUs(SF_TIMEOUT_STEP_US);
    }
    while ((pinVal != pinValExp) && (tmoutUs > tmoutUsTemp));

    return (tmoutUs > tmoutUsTemp) ? kStatus_Success : kStatus_SF_SpiTimeout;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_SendCommandInt
 * Description   : This function creates SPI frame, sends a SPI command to the
 *                 device, receives an acknowledgement (if any) and stores
 *                 an error code and state value from the ACK.
 *
 *END**************************************************************************/
static status_t SF_SendCommandInt(sf_drv_data_t *drvData,
        uint8_t sendLen, uint8_t recvLen,
        uint8_t *sendBuffer, uint8_t *recvBuffer, uint32_t tmoutUs)
{
    status_t           status = kStatus_Success;
    spi_aml_transfer_t amlSpiData =    /* SPI transmission structure. */
    {
            sendBuffer,
            recvBuffer,
            sendLen,
            0x00U
    };

    /* Preconditions. */
    AML_ASSERT(drvData != NULL);
    AML_ASSERT(sendLen > 0U);
    AML_ASSERT(sendBuffer != NULL);

#if defined(DEBUG_SPI_COM) && (SDK_VERSION == SDK_2_0)
    /* Note: SF_ReceiveSpiData function changes sendBuffer (sets all bits to 1). */
    PRINTF("Sent frame: ");
    SF_PrintFrame(amlSpiData.txBuffer, sendLen);
#endif

    if ((status = SF_SendSpiData(drvData, &amlSpiData)) != kStatus_Success)
    {
        return status;
    }

    if (recvLen > 0U)
    {   /* Note: some commands do not have an acknowledge frame. */
        AML_ASSERT(recvBuffer != NULL);

        /* Receive and process an ACK frame. */
        amlSpiData.dataSize = recvLen;
        if ((status = SF_ReceiveSpiData(drvData, &amlSpiData, tmoutUs)) == kStatus_Success)
        {   /* Store the error code and state from the ACK frame. */
            status = SF_StoreDevState(drvData, amlSpiData.rxBuffer);
        }
    }

#if defined(DEBUG_SPI_COM) && (SDK_VERSION == SDK_2_0)
    if (recvLen > 0U)
    {
        PRINTF("Received frame: ");
        SF_PrintFrame(amlSpiData.rxBuffer, amlSpiData.dataSize);
    }
#endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_CalcUlRegs
 * Description   : This function calculates values of SIGFOX device registers
 *                 FCINT and FCFRAC.
 *
 *END**************************************************************************/
static void SF_CalcUlRegs(uint32_t freqHz, uint32_t *fcint, uint32_t *fcfrac)
{
    AML_ASSERT((freqHz >= SF_UL_FREQ_MIN_HZ) && (freqHz <= SF_UL_FREQ_MAX_HZ));

    /* Calculate FCINT from an up-link frequency.
     * FCINT = roundDown(ftx / fx0)
     */
    (*fcint) = (SF_ULFREQ_FACTOR * freqHz) / SF_ULFREQ_F_X0;

    /* Calculate FCRAC from an up-link frequency.
     * Original formula:
     *   FCFRAC = (ftx - FCINT * fx0) * 2^19
     *   FCFRAC = FCFRAC / fx0
     *   Round FCFRAC to the nearest integer (implicit rounding-down used instead).
     */
    (*fcfrac) = (SF_ULFREQ_FACTOR * freqHz) - ((*fcint) * SF_ULFREQ_F_X0);
    (*fcfrac) = (uint32_t)(((uint64_t)(*fcfrac) << 19U) / SF_ULFREQ_F_X0);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_CalcUlFreq
 * Description   : This function calculates the up-link frequency with use
 *                 of SIGFOX device registers FCFRAC and FCINT.
 *
 *END**************************************************************************/
static uint32_t SF_CalcUlFreq(uint32_t fcint, uint32_t fcfrac)
{
    uint32_t freqHz = 0U;   /* Up-link frequency in Hz. */

    freqHz = ((5266U * fcfrac) + 2048U) >> 13U;

    freqHz += 52U * fcfrac;
    freqHz += fcint * SF_ULFREQ_F_X0;

    return freqHz;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_StoreRegs
 * Description   : This function fills the SPI frame with register values.
 *
 *END**************************************************************************/
static void SF_StoreRegs(uint32_t fcint, uint32_t fcfrac, uint8_t *frame)
{
    AML_ASSERT(frame != NULL);

#if SF_MODEL == SF_MODEL_OL2361

    uint16_t freqcon0 = 0U;     /* Value of register FREQCON0 (OL2361 only). */
    uint16_t freqcon1 = 0U;     /* Value of register FREQCON1 (OL2361 only). */

    /* FREQCON0[15:7] = FCRAC[8:0]. */
    freqcon0 = (fcfrac & SF_FCFRAC_LH_MASK) << SF_FREQCON0_FCFRAC_LH_SHIFT;
    /* Enable fractional part. */
    freqcon0 |= SF_FREQCON0_FCDISFRAC_MASK;

    /* FREQCON1[15:10] = FCINT[5:0]. */
    freqcon1 = (fcint & SF_FCINT_MASK) << SF_FREQCON1_FCINT_SHIFT;
    /* FREQCON1[9:8] = FCFRAC[18:17]. */
    freqcon1 |= ((fcfrac & SF_FCFRAC_HH_MASK) >>
            (SF_FCFRAC_HH_SHIFT - SF_FREQCON1_FCFRAC_HH_SHIFT));
    /* FREQCON1[7:0] = FCFRAC[16:9]. */
    freqcon1 |= ((fcfrac & SF_FCFRAC_HL_MASK) >> (SF_FCFRAC_HL_SHIFT));

    frame[SF_SET_UL_FREQCON0_HIGH_OF] = SF_GET_BITS_VALUE(freqcon0, 0xFF00U, 8U);
    frame[SF_SET_UL_FREQCON0_LOW_OF] = SF_GET_BITS_VALUE(freqcon0, 0xFFU, 0U);
    frame[SF_SET_UL_FREQCON1_HIGH_OF] = SF_GET_BITS_VALUE(freqcon1, 0xFF00U, 8U);
    frame[SF_SET_UL_FREQCON1_LOW_OF] = SF_GET_BITS_VALUE(freqcon1, 0xFFU, 0U);

#else

    frame[SF_SET_UL_FREQCON0_HIGH_OF] = fcint;
    frame[SF_SET_UL_FREQCON0_LOW_OF] = (fcfrac & 0xFF0000) >> 16U;
    frame[SF_SET_UL_FREQCON1_HIGH_OF] = (fcfrac & 0xFF00) >> 8U;
    frame[SF_SET_UL_FREQCON1_LOW_OF] = fcfrac & 0xFFU;

#endif
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_RestoreRegs
 * Description   : This function fills the FCFRAC and FCINT registers using
 *                 the SPI frame.
 *
 *END**************************************************************************/
static void SF_RestoreRegs(const uint8_t *frame, uint32_t *fcint, uint32_t *fcfrac)
{
#if SF_MODEL == SF_MODEL_OL2361

    uint16_t freqcon0 = 0U;     /* Value of register FREQCON0 (OL2361 only). */
    uint16_t freqcon1 = 0U;     /* Value of register FREQCON1 (OL2361 only). */

    /* Extract FREQCON0 and FReQCON1 register values from the frame. */
    freqcon0 = SF_GET_FREQCON0(frame + SF_ACK_PAYLOAD_OF);
    freqcon1 = SF_GET_FREQCON1(frame + SF_ACK_PAYLOAD_OF);

    /* Extract FCINT from the FREQCON1 register. */
    /* FCINT[5:0] = FREQCON1[15:10]. */
    (*fcint) = (freqcon1 & SF_FREQCON1_FCINT_MASK) >> SF_FREQCON1_FCINT_SHIFT;

    /* Extract FCFRAC from FREQCON0 and FREQCON1 registers. */
    /* FCRAC[8:0] = FREQCON0[15:7]. */
    (*fcfrac) = (freqcon0 & SF_FREQCON0_FCFRAC_LH_MASK) >> SF_FREQCON0_FCFRAC_LH_SHIFT;
    /* FCFRAC[16:9] = FREQCON1[7:0]. */
    (*fcfrac) |= ((freqcon1 & SF_FREQCON1_FCFRAC_HL_MASK) << SF_FCFRAC_HL_SHIFT);
    /* FCFRAC[18:17] = FREQCON1[9:8]. */
    (*fcfrac) |= ((freqcon1 & SF_FREQCON1_FCFRAC_HH_MASK) <<
            (SF_FCFRAC_HH_SHIFT - SF_FREQCON1_FCFRAC_HH_SHIFT));

#else

    (*fcint) = frame[SF_GET_UL_FREQCON0_HIGH_OF];
    (*fcfrac) = frame[SF_GET_UL_FREQCON0_LOW_OF] << 16U;
    (*fcfrac) |= frame[SF_GET_UL_FREQCON1_HIGH_OF] << 8U;
    (*fcfrac) |= frame[SF_GET_UL_FREQCON1_LOW_OF];

#endif
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_StoreDevState
 * Description   : This function stores an error code and state values from
 *                 a received acknowledgement.
 *
 *END**************************************************************************/
static status_t SF_StoreDevState(sf_drv_data_t *drvData, const uint8_t *spiRcvBuf)
{
    AML_ASSERT(drvData != NULL);
    AML_ASSERT(spiRcvBuf != NULL);

    /* Store the error code and state from the ACK frame. */
    drvData->devState = (sf_intern_state_t)(*(spiRcvBuf + SF_ACK_STATE_OF));
    drvData->errorCode = (sf_intern_error_t)(*(spiRcvBuf + SF_ACK_ERROR_OF));

    /* Note: the user can obtain a value of the error code with use
     * of the SF_GetErrorCode function. */
    return (drvData->errorCode == sfErrNone) ? kStatus_Success : kStatus_SF_SpiAckError;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_StorePayload
 * Description   : This function stores an error code and state values from
 *                 a received acknowledgement.
 *
 *END**************************************************************************/
static status_t SF_StorePayload(const uint8_t *spiRcvBuf, sf_msg_payload_t *recvPayload,
        uint8_t recvPayloadBufSize)
{
    status_t status = kStatus_Fail;

    AML_ASSERT(spiRcvBuf != NULL);
    AML_ASSERT(recvPayload != NULL);
    AML_ASSERT((recvPayload->payload) != NULL);

    /* Get payload length. */
    recvPayload->payloadLen = (uint8_t)(*(spiRcvBuf + SF_ACK_LENGTH_OF));

    /* Subtract size of the ACK header. */
    recvPayload->payloadLen -= SF_ACK_HEADER_B;

    if (recvPayload->payloadLen > recvPayloadBufSize)
    {
        status = kStatus_SF_SpiAckLength;
    }
    else
    {
        status = kStatus_Success;
    }

    if ((kStatus_Success == status) && (recvPayload->payloadLen > 0U))
    {
        memcpy((void*)recvPayload->payload,
                (void*)(spiRcvBuf + SF_ACK_PAYLOAD_OF),
                (size_t)recvPayload->payloadLen);
    }

    return status;
}

/*******************************************************************************
 * Code - general functions
 ******************************************************************************/
/*FUNCTION**********************************************************************
 *
 * Function Name : SF_Init
 * Description   : Initializes the SIGFOX driver based on user configuration.
 *
 *END**************************************************************************/
status_t SF_Init(sf_drv_data_t *drvData, const sf_user_config_t *userConfig)
{
    status_t status = kStatus_Success;
    bool     spiTestSuccess = false;           /* Result of SPI test. */
    uint8_t  spiSndBuf[SF_SET_WD_TIMER_FRM_B]; /* SPI send buffer. */
    uint8_t  spiRcvBuf[SF_SET_WD_TIMER_FRM_B]; /* SPI receive buffer. */

    /* Preconditions. */
    AML_ASSERT(drvData != NULL);
    AML_ASSERT(userConfig != NULL);

    drvData->devState = sfStateInit;
    drvData->errorCode = sfErrNone;

    /* Wait until ACK pin is high (Sigfox device is ready). */
    status = SF_WaitPinLevel(&(drvData->gpioConfig.ackPin), 1U,
            SF_ACK_INIT_TIMEOUT_US);

    if (kStatus_Success == status)
    {
        status = SF_WakeUp(drvData);
    }

    /* Test SPI communication. */
    if (kStatus_Success == status)
    {
        status = SF_TestSpiCon(drvData, &spiTestSuccess);
    }
    if ((kStatus_Success == status) && (spiTestSuccess == false))
    {
        status = kStatus_SF_SpiTestFail;
    }

    /* Initialize registers of the Sigfox device. */
    if (kStatus_Success == status)
    {
        status = SF_ChangeNetworkStandard(drvData, userConfig->netStandard);
    }

    /* Set watchdog timer. */
    if (kStatus_Success == status)
    {
        SF_PackFrame(sfSpiCmdSetWdTimer, SF_SET_WD_TIMER_INF_PLD_B,
                (uint8_t*)(&(userConfig->watchdogTime)), (uint8_t *)spiSndBuf);
        status = SF_SendCommandInt(drvData, SF_SET_WD_TIMER_INF_B,
                SF_SET_WD_TIMER_ACK_B, spiSndBuf, spiRcvBuf, SF_ACK_NOTRANS_TIMEOUT_US);
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_GetDefaultConfig
 * Description   : Fills user configuration structure with default values.
 *
 *END**************************************************************************/
void SF_GetDefaultConfig(sf_user_config_t *userConfig)
{
    AML_ASSERT(userConfig != NULL);

    userConfig->netStandard = sfNetStandardFCC_USA;	//USA default
#if (SF_MODEL == SF_MODEL_OL2385)
    userConfig->watchdogTime = sfWdTime_65_536;     /* 65 seconds for OL2385. */
#elif (SF_MODEL == SF_MODEL_OL2361)
    userConfig->watchdogTime = sfWdTime_67_108;     /* 67 seconds for OL2361. */
#endif
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_SendCommand
 * Description   : This function sends a command to the device. This function
 *                 waits for an acknowledgement (if any).
 *
 *END**************************************************************************/
status_t SF_SendCommand(sf_drv_data_t *drvData, sf_spi_cmd_t cmd,
        const sf_msg_payload_t *sendPayload, sf_msg_payload_t *recvPayload,
        uint8_t recvBufferSize)
{
    status_t status = kStatus_Success;
    uint8_t  spiSndBuf[SF_SPI_MSG_MAX_B]; /* SPI receive buffer. */
    uint8_t  spiRcvBuf[SF_SPI_MSG_MAX_B]; /* SPI send buffer. */
    uint8_t  recvSize = 0U;               /* Expected size of an ACK frame. */

    /* Preconditions. */
    AML_ASSERT(drvData != NULL);
    AML_ASSERT((cmd >= SF_CMD_FIRST_ID) && (cmd <= SF_CMD_LAST_ID));
    AML_ASSERT(sendPayload != NULL);
    AML_ASSERT(sendPayload->payloadLen <= SF_INF_PAYLOAD_MAX_B);
    /* Payload data variable must be non-null when the command contains paylaod. */
    AML_ASSERT((SF_HAS_CMD_PLD(cmd) && (sendPayload->payload != NULL)) ||
            (SF_HAS_CMD_PLD(cmd) == false));

    /* When a command has an acknowledgement then the actual ACK frame size is
     * defined by the first byte of the ACK and updated later. */
    recvSize = (SF_HAS_CMD_ACK(cmd)) ? SF_SPI_MSG_MAX_B : 0U;

    SF_PackFrame(cmd, sendPayload->payloadLen, sendPayload->payload, (uint8_t*)spiSndBuf);
    status = SF_SendCommandInt(drvData, sendPayload->payloadLen + SF_INF_HEADER_B,
            recvSize, spiSndBuf, spiRcvBuf, SF_ACK_TRANS_RECV_TIMEOUT_US);

    if ((recvPayload != NULL) && (kStatus_Success == status))
    {   /* Store the payload and its length. */
        status = SF_StorePayload(spiRcvBuf, recvPayload, recvBufferSize);
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_WakeUp
 * Description   : This function sends the "Send Wakeup" command to wake up the
 *                 device from power down mode.
 *
 *END**************************************************************************/
status_t SF_WakeUp(sf_drv_data_t *drvData)
{
    status_t status = kStatus_Fail;
    uint8_t  spiSndBuf[SF_SEND_WAKEUP_INF_B]; /* SPI send buffer. */

    /* Preconditions. */
    AML_ASSERT(drvData != NULL);

    SF_PackFrame(sfSpiCmdWakeup, SF_SEND_WAKEUP_INF_PLD_B, NULL, (uint8_t*)spiSndBuf);
    status = SF_SendCommandInt(drvData, SF_SEND_WAKEUP_INF_B, SF_SEND_WAKEUP_ACK_B,
            spiSndBuf, NULL, SF_ACK_NOTRANS_TIMEOUT_US);

    /* Delay is needed after wake up. */
    WAIT_AML_WaitMs(SF_WAKEUP_DLY_MS);

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_Sleep
 * Description   : This function puts the device to power off mode with use
 *                 of the "Send To Sleep" command.
 *
 *END**************************************************************************/
status_t SF_Sleep(sf_drv_data_t *drvData)
{
    uint8_t spiSndBuf[SF_SEND_TO_SLEEP_INF_B]; /* SPI receive buffer. */

    /* Preconditions. */
    AML_ASSERT(drvData != NULL);

    SF_PackFrame(sfSpiCmdSleep, SF_SEND_TO_SLEEP_INF_PLD_B, NULL, (uint8_t*)spiSndBuf);
    return SF_SendCommandInt(drvData, SF_SEND_TO_SLEEP_INF_B, SF_SEND_TO_SLEEP_ACK_B,
            spiSndBuf, NULL, SF_ACK_NOTRANS_TIMEOUT_US);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_SetUlFrequency
 * Description   : This function sets uplink frequency of the SIGFOX device.
 *
 *END**************************************************************************/
status_t SF_SetUlFrequency(sf_drv_data_t *drvData, uint32_t freqHz)
{
    status_t status = kStatus_Success;
    uint8_t  spiSndBuf[SF_SET_UL_FREQ_FRM_B]; /* SPI receive buffer. */
    uint8_t  spiRcvBuf[SF_SET_UL_FREQ_FRM_B]; /* SPI send buffer. */
    uint32_t fcint = 0U;        /* Value of the integer divider parameter FCINT. */
    uint32_t fcfrac = 0U;       /* Value of the fractional divider parameter FCFRAC. */

    /* Preconditions. */
    AML_ASSERT(drvData != NULL);
    AML_ASSERT((freqHz >= SF_UL_FREQ_MIN_HZ) && (freqHz <= SF_UL_FREQ_MAX_HZ));

    /* Calculate value of the FREQCON0/1 registers. */
    SF_CalcUlRegs(freqHz, &fcint, &fcfrac);

    /* Store the registers into a SPI frame. */

    SF_PackFrame(sfSpiCmdSetUlFreq, SF_SET_UL_FREQ_INF_PLD_B, NULL, (uint8_t*)spiSndBuf);
    SF_StoreRegs(fcint, fcfrac, (uint8_t*)spiSndBuf);

#if defined(DEBUG_FREQ_CALC) && (SDK_VERSION == SDK_2_0)
    PRINTF("Freq   = %u Hz\r\n", freqHz);
    PRINTF("FCFRAC = %u\r\n", fcfrac);
    PRINTF("FCINT  = %u\r\n", fcint);
    PRINTF("Frame  = 0x%02x %02x %02x %02x %02x %02x\r\n", spiSndBuf[0], spiSndBuf[1],
            spiSndBuf[2], spiSndBuf[3], spiSndBuf[4], spiSndBuf[5]);
#endif

    status = SF_SendCommandInt(drvData, SF_SET_UL_FREQ_INF_B, SF_SET_UL_FREQ_ACK_B,
            spiSndBuf, spiRcvBuf, SF_ACK_NOTRANS_TIMEOUT_US);

    if ((spiRcvBuf[SF_ACK_LENGTH_OF] != SF_SET_UL_FREQ_ACK_B) && (kStatus_Success == status))
    {   /* An acknowledgement does not have expected length. */
        status = kStatus_SF_SpiAckLength;
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_GetUlFrequency
 * Description   : This function gets uplink frequency of the SIGFOX device.
 *
 *END**************************************************************************/
status_t SF_GetUlFrequency(sf_drv_data_t *drvData, uint32_t *freqHz)
{
    status_t status = kStatus_Success;
    uint8_t  spiSndBuf[SF_GET_UL_FREQ_FRM_B]; /* SPI receive buffer. */
    uint8_t  spiRcvBuf[SF_GET_UL_FREQ_FRM_B]; /* SPI send buffer. */
    uint32_t fcint = 0U;        /* Value of the integer divider parameter FCINT. */
    uint32_t fcfrac = 0U;       /* Value of the fractional divider parameter FCFRAC. */

    /* Preconditions. */
    AML_ASSERT(drvData != NULL);
    AML_ASSERT(freqHz != NULL);

    SF_PackFrame(sfSpiCmdGetUlFreq, SF_GET_UL_FREQ_INF_PLD_B, NULL, (uint8_t*)spiSndBuf);
    status = SF_SendCommandInt(drvData, SF_GET_UL_FREQ_INF_B, SF_GET_UL_FREQ_ACK_B,
            spiSndBuf, spiRcvBuf, SF_ACK_NOTRANS_TIMEOUT_US);

    if ((spiRcvBuf[SF_ACK_LENGTH_OF] != SF_GET_UL_FREQ_ACK_B) &&
            (kStatus_Success == status))
    {   /* An acknowledgement does not have expected length. */
        status = kStatus_SF_SpiAckLength;
    }

    if (kStatus_Success == status)
    {
        SF_RestoreRegs(spiRcvBuf, &fcint, &fcfrac);
        *freqHz = SF_CalcUlFreq(fcint, fcfrac);
    }

#if defined(DEBUG_FREQ_CALC) && (SDK_VERSION == SDK_2_0)
    PRINTF("Freq   = %u Hz\r\n", *freqHz);
    PRINTF("FCFRAC = %u\r\n", fcfrac);
    PRINTF("FCINT  = %u\r\n", fcint);
    PRINTF("FRAME  = 0x%02x %02x %02x %02x %02x %02x %02x\r\n", spiRcvBuf[0], spiRcvBuf[1],
            spiRcvBuf[2], spiRcvBuf[0 + 3], spiRcvBuf[1 + 3], spiRcvBuf[2 + 3],
            spiRcvBuf[3 + 3]);
#endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_SendPayload
 * Description   : This function sends the "Send Payload" command to the device.
 *                 It sends user data to SIGFOX network.
 *
 *END**************************************************************************/
status_t SF_SendPayload(sf_drv_data_t *drvData, const sf_msg_payload_t *sendPayload)
{
    status_t status = kStatus_Fail;
    uint8_t  spiSndBuf[SF_SEND_PAYLOAD_FRM_B]; /* SPI receive buffer. */
    uint8_t  spiRcvBuf[SF_SEND_PAYLOAD_FRM_B]; /* SPI send buffer. */

    /* Preconditions. */
    AML_ASSERT(drvData != NULL);
    AML_ASSERT(sendPayload != NULL);
    AML_ASSERT((sendPayload->payloadLen > 0U) && (sendPayload->payload != NULL));

    SF_PackFrame(sfSpiCmdSendPayload, sendPayload->payloadLen, sendPayload->payload,
            (uint8_t*)spiSndBuf);
    status = SF_SendCommandInt(drvData, SF_INF_HEADER_B + sendPayload->payloadLen,
            SF_SEND_PAYLOAD_ACK_B, spiSndBuf, spiRcvBuf, SF_ACK_TRANS_TIMEOUT_US);

    if ((spiRcvBuf[SF_ACK_LENGTH_OF] != SF_SEND_PAYLOAD_ACK_B) &&
            (kStatus_Success == status))
    {   /* An acknowledgement does not have expected length. */
        status = kStatus_SF_SpiAckLength;
    }
    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_SendBit
 * Description   : This function sends the "Send Bit" command, which transmits
 *                 just one bit to the SIGFOX network. This is the shortest frame
 *                 that SIGFOX library generates.
 *
 *END**************************************************************************/
status_t SF_SendBit(sf_drv_data_t *drvData)
{
    status_t status = kStatus_Fail;
    uint8_t  spiSndBuf[SF_SEND_BIT_FRM_B]; /* SPI receive buffer. */
    uint8_t  spiRcvBuf[SF_SEND_BIT_FRM_B]; /* SPI send buffer. */

    /* Preconditions. */
    AML_ASSERT(drvData != NULL);

    SF_PackFrame(sfSpiCmdSendBit, SF_SEND_BIT_INF_PLD_B, NULL, (uint8_t*)spiSndBuf);
    status = SF_SendCommandInt(drvData, SF_SEND_BIT_INF_B, SF_SEND_BIT_ACK_B,
            spiSndBuf, spiRcvBuf, SF_ACK_TRANS_TIMEOUT_US);

    if ((spiRcvBuf[SF_ACK_LENGTH_OF] != SF_SEND_BIT_ACK_B) && (kStatus_Success == status))
    {   /* An acknowledgement does not have expected length. */
        status = kStatus_SF_SpiAckLength;
    }
    return status;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : SF_SendOutOfBand
 * Description   : This function sends the "Send Out of band" command, which
 *                 transmits data to the SIGFOX network. Data is composed
 *                 of information about the chip itself (Voltage, Temperature).
 *                 This function must be called every 24 hours at least or never
 *                 if an application has some energy critical constraints.
 *
 *END**************************************************************************/
status_t SF_SendOutOfBand(sf_drv_data_t *drvData)
{
    status_t status = kStatus_Fail;
    uint8_t  spiSndBuf[SF_SEND_OUT_BAND_FRM_B]; /* SPI receive buffer. */
    uint8_t  spiRcvBuf[SF_SEND_OUT_BAND_FRM_B]; /* SPI send buffer. */

    /* Preconditions. */
    AML_ASSERT(drvData != NULL);

    SF_PackFrame(sfSpiCmdOutOfBand, SF_SEND_OUT_BAND_INF_PLD_B, NULL, (uint8_t*)spiSndBuf);
    status = SF_SendCommandInt(drvData, SF_SEND_OUT_BAND_INF_B, SF_SEND_OUT_BAND_ACK_B,
            spiSndBuf, spiRcvBuf, SF_ACK_TRANS_TIMEOUT_US);

    if ((spiRcvBuf[SF_ACK_LENGTH_OF] != SF_SEND_OUT_BAND_ACK_B) &&
            (kStatus_Success == status))
    {   /* An acknowledgement does not have expected length. */
        status = kStatus_SF_SpiAckLength;
    }
    return status;
}

#if (SF_MODEL == SF_MODEL_OL2385)
/*FUNCTION**********************************************************************
 *
 * Function Name : SF_ReceiveMessage
 * Description   : This function receives a frame from SIGFOX network with use
 *                 of the "Receive Frame" command. It is available for OL2385
 *                 device only. Max. size of received data is eight bytes
 *                 (SIGFOX limitation).
 *
 *END**************************************************************************/
status_t SF_ReceiveMessage(sf_drv_data_t *drvData, sf_msg_payload_t *recvPayload)
{
    status_t status = kStatus_Success;
    uint8_t  spiSndBuf[SF_RECEIVE_FR_FRM_B]; /* SPI receive buffer. */
    uint8_t  spiRcvBuf[SF_RECEIVE_FR_FRM_B]; /* SPI send buffer. */

    /* Preconditions. */
    AML_ASSERT(drvData != NULL);
    AML_ASSERT(recvPayload != NULL);

    SF_PackFrame(sfSpiCmdReceive, SF_RECEIVE_FR_INF_PLD_B, NULL, (uint8_t*)spiSndBuf);
    status = SF_SendCommandInt(drvData, SF_RECEIVE_FR_INF_B, SF_RECEIVE_FR_ACK_B,
            spiSndBuf, spiRcvBuf, SF_ACK_TRANS_RECV_TIMEOUT_US);

    if ((spiRcvBuf[SF_ACK_LENGTH_OF] != SF_RECEIVE_FR_ACK_B) && (kStatus_Success == status))
    {   /* An acknowledgement does not have expected length. */
        status = kStatus_SF_SpiAckLength;
    }

    if (kStatus_Success == status)
    {
        recvPayload->payloadLen = spiRcvBuf[SF_ACK_LENGTH_OF] - SF_ACK_HEADER_B;
        if (recvPayload->payloadLen > 0U)
        {
            memcpy((void*)recvPayload->payload,
                    (void*)(spiRcvBuf + SF_ACK_PAYLOAD_OF),
                    (size_t)(recvPayload->payloadLen));
        }
    }

    return status;
}
#endif

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_TriggerWatchdog
 * Description   : This function resets the SIGFOX device watchdog with use
 *                 of the "Trigger Watchdog" command.
 *
 *END**************************************************************************/
status_t SF_TriggerWatchdog(sf_drv_data_t *drvData)
{
    uint8_t spiSndBuf[SF_TRIGGER_WD_INF_B]; /* SPI receive buffer. */

    /* Preconditions. */
    AML_ASSERT(drvData != NULL);

    SF_PackFrame(sfSpiCmdTriggerWd, SF_TRIGGER_WD_INF_PLD_B, NULL, (uint8_t*)spiSndBuf);
    return SF_SendCommandInt(drvData, SF_TRIGGER_WD_INF_B, SF_TRIGGER_WD_ACK_B,
            spiSndBuf, NULL, SF_ACK_NOTRANS_TIMEOUT_US);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_CheckIdKey
 * Description   : This function checks if there is a valid key and ID combination
 * written on the device. It sends the "Check ID Key" command to the device.
 *
 *END**************************************************************************/
status_t SF_CheckIdKey(sf_drv_data_t *drvData, bool *success)
{
    status_t status = kStatus_Fail;
    uint8_t  spiSndBuf[SF_CHECK_ID_FRM_B]; /* SPI receive buffer. */
    uint8_t  spiRcvBuf[SF_CHECK_ID_FRM_B]; /* SPI send buffer. */

    /* Preconditions. */
    AML_ASSERT(drvData != NULL);

    SF_PackFrame(sfSpiCmdCheckId, SF_CHECK_ID_INF_PLD_B, NULL, (uint8_t*)spiSndBuf);
    status = SF_SendCommandInt(drvData, SF_CHECK_ID_INF_B, SF_CHECK_ID_ACK_B,
            spiSndBuf, spiRcvBuf, SF_ACK_NOTRANS_TIMEOUT_US);

    if ((spiRcvBuf[SF_ACK_LENGTH_OF] != SF_CHECK_ID_ACK_B) && (kStatus_Success == status))
    {   /* An acknowledgement does not have expected length. */
        status = kStatus_SF_SpiAckLength;
    }

    if (kStatus_Success == status)
    {
        (*success) = (spiRcvBuf[SF_ACK_PAYLOAD_OF] != 0U) ? true : false;
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_GetDeviceInfo
 * Description   : This function reads device ID, SIGFOX library version and
 *                 device version. It uses "Get info" and "Get Device Version"
 *                 commands.
 *
 *END**************************************************************************/
status_t SF_GetDeviceInfo(sf_drv_data_t *drvData, sf_device_info_t *devInfo)
{
    status_t status = kStatus_Success;
    uint8_t  spiSndBuf[SF_GET_INFO_FRM_B]; /* SPI receive buffer. */
    uint8_t  spiRcvBuf[SF_GET_INFO_FRM_B]; /* SPI send buffer. */

    /* Preconditions. */
    AML_ASSERT(drvData != NULL);
    AML_ASSERT(devInfo != NULL);
    AML_ASSERT(devInfo->libVersion != NULL);

    /* Send the Get Info command. */
    SF_PackFrame(sfSpiCmdGetInfo, SF_GET_INFO_INF_PLD_B, NULL, (uint8_t*)spiSndBuf);
    status = SF_SendCommandInt(drvData, SF_GET_INFO_INF_B, SF_GET_INFO_ACK_B,
            spiSndBuf, spiRcvBuf, SF_ACK_NOTRANS_TIMEOUT_US);

    /* Process the response. */
    if (kStatus_Success == status)
    {
        if (spiRcvBuf[SF_ACK_LENGTH_OF] != SF_GET_INFO_ACK_B)
        {   /* An acknowledgement does not have expected length. */
            status = kStatus_SF_SpiAckLength;
        }
        else
        {
            /* Device ID bytes are in reverse order. */
            devInfo->devId = SF_GET_DEV_ID(spiRcvBuf +
                    (SF_DEV_ID_OF + SF_ACK_PAYLOAD_OF));
            memcpy(devInfo->devPac, spiRcvBuf + (SF_DEV_PAC_OF + SF_ACK_PAYLOAD_OF),
                    SF_DEV_PAC_B);
            memcpy(devInfo->libVersion, spiRcvBuf +
                    (SF_LIB_VER_OF + SF_ACK_PAYLOAD_OF), SF_LIB_VER_B);
        }
    }

    /* Send the Get Device version command. */
    if (kStatus_Success == status)
    {
        SF_PackFrame(sfSpiCmdGetDevVer, SF_GET_DEV_VER_INF_PLD_B, NULL, (uint8_t*)spiSndBuf);
        status = SF_SendCommandInt(drvData, SF_GET_DEV_VER_INF_B, SF_GET_DEV_VER_ACK_B,
                spiSndBuf, spiRcvBuf, SF_ACK_NOTRANS_TIMEOUT_US);
    }

    /* Process the response. */
    if (kStatus_Success == status)
    {
        if (spiRcvBuf[SF_ACK_LENGTH_OF] != SF_GET_DEV_VER_ACK_B)
        {   /* An acknowledgement does not have expected length. */
            status = kStatus_SF_SpiAckLength;
        }
        else
        {
            memcpy(devInfo->devVersion, spiRcvBuf + (SF_ACK_PAYLOAD_OF + SF_DEV_VER_OF),
                    SF_DEV_VER_B);
        }
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_TestSpiCon
 * Description   : This function tests if the SPI bus is working. It uses the
 *                 "Echo" command to send data (see SF_ECHO_DATA macro) and
 *                 checks if the device replies with the inverted payload.
 *
 *END**************************************************************************/
status_t SF_TestSpiCon(sf_drv_data_t *drvData, bool *success)
{
    status_t      status = kStatus_Success;
    uint8_t       spiSndBuf[SF_SEND_ECHO_FRM_B]; /* SPI receive buffer. */
    uint8_t       spiRcvBuf[SF_SEND_ECHO_FRM_B]; /* SPI send buffer. */
    const uint8_t payload[] = SF_ECHO_PLD_ARRAY; /* I-frame payload. */
    /* Size of the payload in bytes. */
    const uint8_t payloadLen = (uint8_t)sizeof(payload) / (uint8_t)sizeof(payload[0]);
    uint8_t       i = 0U;

    /* Preconditions. */
    AML_ASSERT(drvData != NULL);
    AML_ASSERT(success != NULL);
    AML_ASSERT((payloadLen > 0U) && (payloadLen <= SF_SEND_ECHO_INF_PLD_B));

    *success = false;

    /* Send the Echo command. */
    SF_PackFrame(sfSpiCmdSendEcho, payloadLen, (uint8_t*)payload, (uint8_t*)spiSndBuf);
    status = SF_SendCommandInt(drvData, SF_SEND_ECHO_INF_B, payloadLen + SF_ACK_HEADER_B,
            spiSndBuf, spiRcvBuf, SF_ACK_NOTRANS_TIMEOUT_US);

    if (kStatus_Success == status)
    {
        if (spiRcvBuf[SF_ACK_LENGTH_OF] != (payloadLen + SF_ACK_HEADER_B))
        {   /* An acknowledgement does not have expected length. */
            status = kStatus_SF_SpiAckLength;
        }
    }

    if (kStatus_Success == status)
    {
        /* Check an acknowledgement which should contain an inverted payload
         * of the I-frame sent. */
        *success = true;
        for (i = 0U; i < payloadLen; i++)
        {
            if (((uint8_t)(~spiRcvBuf[SF_ACK_PAYLOAD_OF + i])) != payload[i])
            {
                *success = false;
                break;
            }
        }
    }
    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_TestDevice
 * Description   : This function executes a test of uplink and downlink
 *                 connectivity using the "Send Test Mode" command. Returned
 *                 status represents result of the test (success or fail),
 *                 the user can obtain details using SF_GetErrorCode function.
 *
 *END**************************************************************************/
status_t SF_TestDevice(sf_drv_data_t *drvData, sf_test_mode_t testMode)
{
    status_t status = kStatus_Success;
    uint8_t spiSndBuf[SF_SEND_TEST_FRM_B]; /* SPI receive buffer. */
    uint8_t spiRcvBuf[SF_SEND_TEST_FRM_B]; /* SPI send buffer. */

    /* Preconditions. */
    AML_ASSERT(drvData != NULL);

    SF_PackFrame(sfSpiCmdSendTestMode, SF_SEND_TEST_INF_PLD_B, NULL,
            (uint8_t*)spiSndBuf);
    spiSndBuf[SF_TEST_MODE_HIGH_OF] = (uint16_t)testMode >> 8U;
    spiSndBuf[SF_TEST_MODE_LOW_OF] = (uint16_t)testMode & 0xFFU;
    spiSndBuf[SF_TEST_CFG_OF] = 0U;

    status = SF_SendCommandInt(drvData, SF_SEND_TEST_INF_B, SF_SEND_TEST_ACK_B,
            spiSndBuf, spiRcvBuf, SF_ACK_TRANS_TIMEOUT_US);

    if ((spiRcvBuf[SF_ACK_LENGTH_OF] != SF_SEND_TEST_ACK_B) && (kStatus_Success))
    {   /* An acknowledgement does not have expected length. */
        status = kStatus_SF_SpiAckLength;
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_GenerateContWave
 * Description   : This function sends the "Continuous Wave" command, which can
 *                 be used for testing. It generates a continuous transmission
 *                 at last set frequency. The signal can be then analyzed using
 *                 spectrum analyzer.
 *
 *END**************************************************************************/
status_t SF_GenerateContWave(sf_drv_data_t *drvData)
{
    uint8_t spiSndBuf[SF_CONT_WAVE_INF_B]; /* SPI receive buffer. */

    /* Preconditions. */
    AML_ASSERT(drvData != NULL);

    SF_PackFrame(sfSpiCmdContWave, SF_CONT_WAVE_INF_PLD_B, NULL, (uint8_t*)spiSndBuf);
    return SF_SendCommandInt(drvData, SF_CONT_WAVE_INF_B, SF_CONT_WAVE_ACK_B,
            spiSndBuf, NULL, SF_ACK_TRANS_TIMEOUT_US);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_SendCommandNonBlock
 * Description   : This function changes up-link and down-link frequency
 *                 and bit rate.
 *
 *END**************************************************************************/
status_t SF_ChangeNetworkStandard(sf_drv_data_t *drvData,
        sf_net_standard_t standardSel)
{
    status_t status = kStatus_Success;
    uint8_t spiSndBuf[SF_CHANGE_TO_RCZX_FRM_B]; /* SPI receive buffer. */
    uint8_t spiRcvBuf[SF_CHANGE_TO_RCZX_FRM_B]; /* SPI send buffer. */
    sf_spi_cmd_t cmd = sfSpiCmdChangeToRCZ1;    /* SPI command ID. */

    /* Preconditions. */
    AML_ASSERT(drvData != NULL);
    AML_ASSERT((sfNetStandardETSI <= standardSel) ||
            (sfNetStandardFCC_SouthAmerica >= standardSel));

    /* Get command ID. */
    cmd = (sf_spi_cmd_t)(SF_CMD_CHANGE_TO_RCZ1_ID + (uint8_t)standardSel);

    /* Send the command. */
    SF_PackFrame(cmd, SF_CHANGE_TO_RCZX_INF_PLD_B, NULL, (uint8_t*)spiSndBuf);
    status = SF_SendCommandInt(drvData, SF_CHANGE_TO_RCZX_INF_B, SF_CHANGE_TO_RCZX_ACK_B,
            spiSndBuf, spiRcvBuf, SF_ACK_NOTRANS_TIMEOUT_US);

    /* Check the response. */
    if (kStatus_Success == status)
    {
        if (spiRcvBuf[SF_ACK_LENGTH_OF] != SF_CHANGE_TO_RCZX_ACK_B)
        {   /* An acknowledgement does not have expected length. */
            status = kStatus_SF_SpiAckLength;
        }
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_SendCommandNonBlock
 * Description   : This function sends a command to the device. It does not wait
 *                 for an acknowledgement.
 *
 *END**************************************************************************/
status_t SF_SendCommandNonBlock(sf_drv_data_t *drvData, sf_spi_cmd_t cmd,
        const sf_msg_payload_t *sendPayload)
{
    uint8_t  spiSndBuf[SF_INF_SPI_MSG_MAX_B]; /* SPI receive buffer. */
    uint8_t  spiRcvBuf[SF_INF_SPI_MSG_MAX_B]; /* SPI send buffer. */
    spi_aml_transfer_t amlSpiData = { };  /* SPI transmission structure. */

    /* Preconditions. */
    AML_ASSERT(drvData != NULL);
    AML_ASSERT((cmd >= SF_CMD_FIRST_ID) && (cmd <= SF_CMD_LAST_ID));
    AML_ASSERT(sendPayload != NULL);
    AML_ASSERT(sendPayload->payloadLen <= SF_INF_PAYLOAD_MAX_B);

    amlSpiData.txBuffer = spiSndBuf;
    amlSpiData.rxBuffer = spiRcvBuf;
    amlSpiData.dataSize = sendPayload->payloadLen + SF_INF_HEADER_B;

    SF_PackFrame(cmd, sendPayload->payloadLen, sendPayload->payload, (uint8_t*)spiSndBuf);
    return SF_SendSpiData(drvData, &amlSpiData);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_IsAckFrameReady
 * Description   : This function checks if the SIGFOX device is ready to send an
 *                 acknowledgement. It checks ACK pin value, which is active low.
 *
 *END**************************************************************************/
bool SF_IsAckFrameReady(sf_drv_data_t *drvData)
{
    AML_ASSERT(drvData != NULL);

    return (GPIO_AML_ReadInput(drvData->gpioConfig.ackPin.gpioInstance,
            drvData->gpioConfig.ackPin.gpioPinNumber) == 0U);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : SF_ReadAckFrameNonBlock
 * Description   : This function receives an acknowledge frame.
 *
 *END**************************************************************************/
status_t SF_ReadAckFrameNonBlock(sf_drv_data_t *drvData,
        sf_msg_payload_t *recvPayload, uint8_t recvBufferSize)
{
    status_t status = kStatus_Success;
    uint8_t  spiSndBuf[SF_ACK_SPI_MSG_MAX_B]; /* SPI receive buffer. */
    uint8_t  spiRcvBuf[SF_ACK_SPI_MSG_MAX_B]; /* SPI send buffer. */
    spi_aml_transfer_t amlSpiData =           /* SPI transmission structure. */
    {
            (uint8_t*)spiSndBuf,
            (uint8_t*)spiRcvBuf,
            SF_ACK_SPI_MSG_MAX_B,
            0x00U
    };

    /* Preconditions. */
    AML_ASSERT(drvData != NULL);
    AML_ASSERT(recvPayload != NULL);
    AML_ASSERT(recvPayload->payload != NULL);

    /* Receive and process an ACK frame. */
    status = SF_ReceiveSpiData(drvData, &amlSpiData, SF_ACK_TRANS_RECV_TIMEOUT_US);

    /* Store items from the ACK frame. */
    if (kStatus_Success == status)
    {
        status = SF_StoreDevState(drvData, amlSpiData.rxBuffer);
    }
    if (kStatus_Success == status)
    {
        status = SF_StorePayload(amlSpiData.rxBuffer, recvPayload, recvBufferSize);
    }

    return status;
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
