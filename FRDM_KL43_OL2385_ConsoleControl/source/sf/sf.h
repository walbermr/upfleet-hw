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
 * @file sf.h
 *
 * SIGFOX driver based on AML layer supporting boards based on following NXP
 * parts: OL2361, OL2385.
 *
 * This module is common for all supported models.
 */

#ifndef SOURCE_SF_SF_H_
#define SOURCE_SF_SF_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "sf_model.h"
#include "sf_setup.h"
#include "sf_ol23xx.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/
/*!
 * @brief This macro returns true when an acknowledgement is expected for a given
 * SPI command. Otherwise it returns false.
 *
 * @param cmd Command number (see sf_spi_cmd_t enumeration).
 */
#define SF_HAS_CMD_ACK(cmd) \
    (((cmd) != sfSpiCmdWakeup) && ((cmd) != sfSpiCmdSleep) && \
     ((cmd) != sfSpiCmdContWave) && ((cmd) != sfSpiCmdTriggerWd))

/*!
 * @addtogroup sf_enum_group
 * Enumerations common for all models.
 * @{
 */
/*! @brief Specific error codes for the SIGFOX device. */
enum _sf_status_t
{
    /*! Received an error from the OL23xx device via SPI. Use the
     * SF_GetErrorCode function to obtain the error code from the last
     * ACK frame received. */
    kStatus_SF_SpiAckError = MAKE_STATUS(kStatusGroup_SF, 0U),
    /*! Size of the receive buffer is smaller than data to be received
     * (indicated by the first byte of a received acknowledgement). */
    kStatus_SF_SpiAckLength = MAKE_STATUS(kStatusGroup_SF, 1U),
    /*! SPI communication timeout. Note that some commands take extra time
     * to access SIGFOX network (for example Send Payload command). */
    kStatus_SF_SpiTimeout = MAKE_STATUS(kStatusGroup_SF, 2U),
    /*! A SPI communication test performed in the SF_Init function failed. */
    kStatus_SF_SpiTestFail = MAKE_STATUS(kStatusGroup_SF, 3U),
};

/*! @brief SPI commands of the device. */
typedef enum
{
    sfSpiCmdSendEcho = SF_CMD_SEND_ECHO_ID,     	/*!< Echo command. */
    sfSpiCmdWakeup = SF_CMD_SEND_WAKEUP_ID,     	/*!< Send Wakeup command. */
    sfSpiCmdSleep = SF_CMD_SEND_TO_SLEEP_ID,    	/*!< Send to sleep command. */
    sfSpiCmdSendPayload = SF_CMD_SEND_PAYLOAD_ID, 	/*!< Send payload command. */
    sfSpiCmdSendBit = SF_CMD_SEND_BIT_ID,       	/*!< Send bit command. */
    sfSpiCmdOutOfBand = SF_CMD_SEND_OUT_BAND_ID, 	/*!< Send Out of Band command. */
#if (SF_MODEL == SF_MODEL_OL2385)
    sfSpiCmdReceive = SF_CMD_RECEIVE_FR_ID,     	/*!< Receive Frame command
                                                     	 (available on OL2385). */
#endif
    sfSpiCmdGetInfo = SF_CMD_GET_INFO_ID,       	/*!< Get Info command. */
    sfSpiCmdGetUlFreq = SF_CMD_GET_UL_FREQ_ID,  	/*!< Set UL frequency command. */
    sfSpiCmdSetUlFreq = SF_CMD_SET_UL_FREQ_ID,  	/*!< Get UL frequency command. */
    sfSpiCmdContWave = SF_CMD_CONT_WAVE_ID,     	/*!< Continuous Wave command. */
    sfSpiCmdCheckId = SF_CMD_CHECK_ID_ID,       	/*!< Check ID key command. */
    sfSpiCmdGetDevVer = SF_CMD_GET_DEV_VER_ID,  	/*!< Get Device Version command. */
    sfSpiCmdSetWdTimer = SF_CMD_SET_WD_TIMER_ID, 	/*!< Set Watchdog Timer command. */
    sfSpiCmdGetWdTimer = SF_CMD_GET_WD_TIMER_ID, 	/*!< Get Watchdog Timer command. */
    sfSpiCmdSetReg = SF_CMD_SET_REG_ID,         	/*!< Set Register command. */
    sfSpiCmdGetReg = SF_CMD_GET_REG_ID,         	/*!< Get Register command. */
    sfSpiCmdTriggerWd = SF_CMD_TRIGGER_WD_ID,   	/*!< Trigger Watchdog command. */
    sfSpiCmdKeepAlive = SF_CMD_KEEP_ALIVE_ID,   	/*!< Keep Alive Message command. */
    sfSpiCmdSendTestMode = SF_CMD_SEND_TEST_ID, 	/*!< Send Test Mode command. */
    sfSpiCmdChangeToRCZ1 = SF_CMD_CHANGE_TO_RCZ1_ID, /*!< Change to RCZ1 command
                                                     (European ETSI standard). */
    sfSpiCmdChangeToRCZ2 = SF_CMD_CHANGE_TO_RCZ2_ID, /*!< Change to RCZ2 command
                                                     (USA FCC standard). */
    sfSpiCmdChangeToRCZ3 = SF_CMD_CHANGE_TO_RCZ3_ID, /*!< Change to RCZ3 command
                                                     (Japan/Korean ARIB standard). */
    sfSpiCmdChangeToRCZ4 = SF_CMD_CHANGE_TO_RCZ4_ID, /*!< Change to RCZ4 command
                                                     (South American FCC standard). */
} sf_spi_cmd_t;

/*! @brief The state of the device in OL23xx software state machine. */
typedef enum
{
    sfStateIdle = 0U,               /*!< Idle state. */
    sfStateInit,                    /*!< Init state (state after initialization
                                         and in sleep mode). */
    sfStateWaitForCmd               /*!< Wait for command state (state after
                                         wake-up). */
} sf_intern_state_t;

/*! @brief The error codes of the SIGFOX device stored in the SPI
 * acknowledgement frame. */
typedef enum
{
    sfErrNone = 0x00U,                  /*!< No error */
    /*! @{
     * Open errors.
     */
    sfErrOpenMalloc = 0x10U,            /*!< Error on MANUF_API_malloc or buffer
                                             pointer NULL. */
    sfErrOpenIdPtr = 0x11U,             /*!< ID pointer NULL. */
    sfErrOpenGetSeq = 0x12U,            /*!< Error on MANUF_API_get_nv_mem w/
                                             SFX_NVMEM_SEQ_CPT. */
    sfErrOpenGetPn = 0x13U,             /*!< Error on MANUF_API_get_nv_mem w/
                                             SFX_NVMEM_PN. */
    sfErrOpenState = 0x14U,             /*!< State is not idle, library should
                                             be closed before. */
    /*! @} */
    /*! @{
     * Close errors.
     */
    sfErrCloseFree = 0x20U,             /*!< Error on MANUF_API_free. */
    sfErrCloseRfStop = 0x21U,           /*!< Error on MANUF_API_rf_stop. */
    /*! @} */
    /*! @{
     * Send frame errors.
     */
    sfErrSndFrDataLength = 0x30U,       /*!< Customer data length > 12 Bytes. */
    sfErrSndFrState = 0x31U,            /*!< State != READY, must close and
                                             reopen library. */
    sfErrSndFrResponsePtr = 0x32U,      /*!< Response data pointer NULL in case
                                             of downlink. */
    sfErrSndFrBuildUplink = 0x33U,      /*!< Build uplink frame failed. */
    sfErrSndFrSendUplink = 0x34U,       /*!< Send uplink frame failed. */
    sfErrSndFrReceive = 0x35U,          /*!< Receive downlink frame failed
                                             or timeout. */
    sfErrSndFrDelayOobAck = 0x36U,      /*!< Error on MANUF_API_delay w/
                                             SFX_DLY_OOB_ACK (Downlink). */
    sfErrSndFrBuildOobAck = 0x37U,      /*!< Build out of band frame failed
                                             (Downlink). */
    sfErrSndFrSendOobAck = 0x38U,       /*!< Send out of band frame failed
                                             (Downlink). */
    sfErrSndFrDataPtr = 0x39U,          /*!< Customer data pointer NULL. */
    sfErrSndFrCarrierSenseConfig = 0x3AU, /*!< Carrier Sense configuration need
                                             to be initialized. */
    sfErrSndFrWaitTimeout = 0x3EU,      /*!< Wait frame has returned time out. */
    sfErrSndFrInvalidFccChan = 0x3FU,   /*!< FCC invalid channel, must call
                                             SIGFOX_API_reset. */
    /*! @} */
    /*! @{
     * Send bit errors.
     */
    sfErrSendBitState = 0x41U,          /*!< State != READY, must close and
                                             reopen library. */
    sfErrSendBitResponsePtr = 0x42U,    /*!< Response data pointer NULL in case
                                             of downlink. */
    sfErrSendBitBuildUplink = 0x43U,    /*!< Build uplink frame failed. */
    sfErrSendBitSendUplink = 0x44U,     /*!< Send uplink frame failed. */
    sfErrSendBitReceive = 0x45U,        /*!< Receive downlink frame failed
                                             or timeout. */
    sfErrSendBitDelayOobAck = 0x46U,    /*!< Error on MANUF_API_delay w/
                                             SFX_DLY_OOB_ACK (Downlink). */
    sfErrSendBitBuildOobAck = 0x47U,    /*!< Build out of band frame failed
                                             (Downlink). */
    sfErrSendBitSendOobAck = 0x48U,     /*!< Send out of band frame failed
                                             (Downlink). */
    sfErrSendBitDataPtr = 0x49U,        /*!< Customer data pointer NULL. */
    sfErrSendBitWaitTimeout = 0x4EU,    /*!< Wait frame has returned time out. */
    sfErrSendBitInvalidFccChan = 0x4FU, /*!< FCC invalid channel, must call
                                             SIGFOX_API_reset. */
    /*! @} */
    /*! @{
     * Send Out of band errors.
     */
    sfErrSendOobState = 0x51U,          /*!< State != READY, must close and
                                             reopen library. */
    sfErrSendOobBuildUplink = 0x53U,    /*!< Build uplink frame failed. */
    sfErrSendOobSendUplink = 0x54U,     /*!< Send uplink frame failed. */
    sfErrSendOobInvalidFccChan = 0x5FU, /*!< Send out of band frame failed
                                             (Downlink). */
    /*! @} */
    /*! @{
     * Configuration errors.
     */
    sfErrSetStdConfigSIGFOXChan = 0x90U, /*!< Default SIGFOX channel out of range. */
    sfErrSetStdConfigSet = 0x91U,        /*!< Unable to set configuration. */
    /*! @} */
    /*! @{
     * Test mode 0 errors.
     */
    sfErrTestMode0RfInit = 0xA0U,       /*!< Error on MANUF_API_rf_init. */
    sfErrTestMode0ChangeFreq = 0xA1U,   /*!< Error on MANUF_API_change_frequency. */
    sfErrTestMode0RfSend = 0xA2U,       /*!< Error on MANUF_API_rf_send. */
    sfErrTestMode0Delay = 0xA3U,        /*!< Error on MANUF_API_delay. */
    sfErrTestMode0RfStop = 0xA4U,       /*!< Error on MANUF_API_rf_stop. */

    sfErrTestModeState = 0xB1U,         /*!< State != READY, must close and
                                             reopen library. */
    /*! @} */
    /*! @{
     * Test mode 2 errors.
     */
    sfErrTestMode2ReportTest = 0xC0U,   /*!< Error on MANUF_API_report_test_result. */
    /*! @} */
    /*! @{
     * Test mode 3 errors.
     */
    sfErrTestMode3RfInit = 0xD0U,       /*!< Error on MANUF_API_rf_init. */
    sfErrTestMode3ChangeFreq = 0xD1U,   /*!< Error on MANUF_API_change_frequency. */
    sfErrTestMode3TimerStart = 0xD2U,   /*!< Error on MANUF_API_timer_start. */
    sfErrTestMode3ReportTest = 0xD3U,   /*!< Error on MANUF_API_report_test_result. */
    sfErrTestMode3TimerStop = 0xD4U,    /*!< Error on MANUF_API_timer_stop. */
    sfErrTestMode3RfStop = 0xD5U,       /*!< Error on MANUF_API_rf_stop. */
    /*! @} */
    /*! @{
     * Test mode 4 errors.
     */
    sfErrTestMode4BuildUplink = 0xE0U,  /*!< Build uplink frame failed. */
    sfErrTestMode4SendUplink = 0xE1U,   /*!< Send uplink frame failed. */
    sfErrTestMode4ReportTest = 0xE2U,   /*!< Error on MANUF_API_report_test_result. */
    sfErrTestMode4GetRssi = 0xE3U,      /*!< Error on MANUF_API_get_rssi. */
    sfErrTestMode4Delay = 0xE4U,        /*!< Error on MANUF_API_delay. */
    /*! @} */
    /*! @{
     * Test mode 5 errors.
     */
    sfErrTestMode5RfInit = 0xF0U,       /*!< Error on MANUF_API_rf_init. */
    sfErrTestMode5ChangeFreq = 0xF1U,   /*!< Error on MANUF_API_change_frequency. */
    sfErrTestMode5BuildUplink = 0xF2U,  /*!< Build uplink frame failed. */
    sfErrTestMode5SendUplink = 0xF3U,   /*!< Send uplink frame failed. */
    sfErrTestMode5RfStop = 0xF4U        /*!< Error on MANUF_API_rf_stop. */
    /*! @} */
} sf_intern_error_t;

/*! @brief Watchdog time. The user can use these values to set the watchdog
 * timer directly. */
typedef enum
{
#if (SF_MODEL == SF_MODEL_OL2385)
    sfWdTime_0_016 = 0X00U,         /*!< 0.016 seconds. */
    sfWdTime_0_032 = 0X01U,         /*!< 0.032 seconds. */
    sfWdTime_0_064 = 0X02U,         /*!< 0.064 seconds. */
    sfWdTime_0_128 = 0X03U,         /*!< 0.128 seconds. */
    sfWdTime_0_256 = 0X04U,         /*!< 0.256 seconds. */
    sfWdTime_0_512 = 0X05U,         /*!< 0.512 seconds. */
    sfWdTime_1_024 = 0X06U,         /*!< 1.024 seconds. */
    sfWdTime_2_048 = 0X07U,         /*!< 2.048 seconds. */
    sfWdTime_4_096 = 0X08U,         /*!< 4.096 seconds. */
    sfWdTime_8_192 = 0X09U,         /*!< 8.192 seconds. */
    sfWdTime_16_384 = 0X0AU,        /*!< 16.384 seconds. */
    sfWdTime_32_768 = 0X0BU,        /*!< 32.768 seconds. */
    sfWdTime_65_536 = 0X0CU         /*!< 65.536 seconds. */
#elif (SF_MODEL == SF_MODEL_OL2361)
    sfWdTime_0_016 = 0X00U,         /*!< 0.016 seconds. */
    sfWdTime_0_032 = 0X01U,         /*!< 0.032 seconds. */
    sfWdTime_0_065 = 0X02U,         /*!< 0.065 seconds. */
    sfWdTime_0_131 = 0X03U,         /*!< 0.131 seconds. */
    sfWdTime_0_262 = 0X04U,         /*!< 0.262 seconds. */
    sfWdTime_0_524 = 0X05U,         /*!< 0.524 seconds. */
    sfWdTime_1_048 = 0X06U,         /*!< 1.048 seconds. */
    sfWdTime_2_097 = 0X07U,         /*!< 2.097 seconds. */
    sfWdTime_4_194 = 0X08U,         /*!< 4.194 seconds. */
    sfWdTime_8_388 = 0X09U,         /*!< 8.388 seconds. */
    sfWdTime_16_777 = 0X0AU,        /*!< 16.777 seconds. */
    sfWdTime_33_554 = 0X0BU,        /*!< 33.554 seconds. */
    sfWdTime_67_108 = 0X0CU,        /*!< 67.108 seconds. */
    sfWdTime_134_217 = 0X0DU,       /*!< 134.217 seconds. */
    sfWdTime_268_435 = 0X0EU,       /*!< 268.435 seconds. */
    sfWdTime_536_87 = 0X0FU,        /*!< 536.87 seconds. */
#endif
} sf_wd_time_t;

/*! @brief Selection of the test mode used to verify the protocol, RF and
 * sensitivity. Detailed description of the modes is in the device datasheet. */
typedef enum
{
    sfTestModeTxBpsk = 0U,      /*!< Sending PRBS data in a 26 Bytes frame
                                     at constant frequency. */
    sfTestModeTxProtocol,       /*!< Test of the complete protocol in up-link. */
    sfTestModeRxProtocol,       /*!< Test of the complete protocol in down-link. */
    sfTestModeRxGfsk,           /*!< Receiving constant GFSK frames at constant
                                     frequency. */
    sfTestModeRxSensi,          /*!< Measurement of the real sensitivity of
                                     device. */
    sfTestModTxSynth            /*!< Sending SIGFOX frames with 4 Bytes payload
                                     at forced frequency. */
} sf_test_mode_t;

/*! @brief List of standards defining network communication parameters. */
typedef enum
{
    sfNetStandardETSI = 0U,	        /*!< European standard ETSI. Up-link frequency
                                         is 868 130 000 Hz, down-link frequency
                                         is 869 525 000 Hz, bit rate 100 bps. */
    sfNetStandardFCC_USA,	        /*!< USA standard FCC. Up-link frequency
                                         is 902 200 000 Hz, down-link frequency
                                         is 905 200 000 Hz, bit rate 600 bps. */
    sfNetStandardARIB,	            /*!< Japan/Korean standard ARIB. Up-link
                                         frequency is 923 200 000 Hz, down-link
                                         frequency is 922 200 000 Hz, bit rate
                                         600 bps with listen before talk. */
    sfNetStandardFCC_SouthAmerica,	/*!< South American standard FCC. Up-link
                                         frequency is 902 200 000 Hz, down-link
                                         frequency is 922 300 000 Hz, bit rate 600 bps. */

} sf_net_standard_t;
/*! @} */

/*!
 * @addtogroup sf_struct_group
 * Structures common for all models.
 * @{
 */
/*! @brief This structure is used by the user to initialize the SIGFOX device.
 * It contains a configuration of the SIGFOX device only (no SPI,
 * etc. configuration).  */
typedef struct
{
    sf_net_standard_t netStandard;  /*!< Selection of a standard defining network
                                         communication parameters. */
    sf_wd_time_t watchdogTime;      /*!< Watchdog timer value. */
} sf_user_config_t;

/*! @brief This data structure is used by the SIGFOX driver (this is the first
 * parameter of most SIGFOX functions). */
typedef struct
{
    sf_spi_config_t spiConfig;      /*!< SPI configuration. */
    sf_gpio_config_t gpioConfig;    /*!< GPIO configuration. */
    sf_intern_state_t devState;     /*!< The state of the SIGFOX device in
                                         software state machine received in
                                         the last acknowledgement. */
    sf_intern_error_t errorCode;    /*!< The error code received in the last
                                         acknowledgement. */
} sf_drv_data_t;

/*! @brief This structure represents the payload of the SPI frame. */
typedef struct
{
    uint8_t payloadLen;             /*!< The size of the payload in bytes. */
    uint8_t *payload;               /*!< Pointer to the frame payload. */
} sf_msg_payload_t;

/*! @brief This structure contains information about the SIGFOX device. */
typedef struct
{
    uint32_t devId;                 /*!< Device ID pre-flashed on the device
                                         (result of the "Get info" command). */
    uint8_t devPac[SF_DEV_PAC_B];   /*!< PAC code pre-flashed on the device
                                         (result of the "Get info" command). */
    uint8_t devVersion[SF_DEV_VER_B]; /*!< Device version (result of the "Get
                                         Device Version" command). */
    uint8_t libVersion[SF_LIB_VER_B]; /*!< SIGFOX library version (result of the
                                         "Get info" command). */
} sf_device_info_t;
/*! @} */

/*******************************************************************************
 * API
 ******************************************************************************/
/*!
 * @addtogroup sf_functions_group
 * @{
 */

/*!
 * @brief Initializes the SIGFOX driver based on user configuration.
 *
 * @param drvData    Driver run-time data.
 * @param userConfig User-provided configuration of the SIGFOX driver.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
status_t SF_Init(sf_drv_data_t *drvData, const sf_user_config_t *userConfig);

/*!
 * @brief Fills user configuration structure with default values.
 *
 * @param userConfig Pointer to a structure where the default SIGFOX
 *                   configuration is stored.
 */
void SF_GetDefaultConfig(sf_user_config_t *userConfig);

/*!
 * @brief This function sends a command to the device. This function waits for
 * an acknowledgement (if any).
 *
 * @param drvData      Driver run-time data.
 * @param cmd          The frame command number.
 * @param sendPayload  Pointer to a payload to be sent. The user can set payload
 *                     data (sendPayload->payload) to NULL when the size is zero.
 * @param recvPayload  Pointer where a resulting payload is stored. It can be
 *                     NULL if the payload is not desired or present in the
 *                     ACK frame. The user can use SF_HAS_CMD_ACK to check if
 *                     a command has an acknowledgement. Size of the data variable
 *                     (recvPayload->payload) must be sufficient
 *                     (see SF_ACK_PAYLOAD_MAX_B macro). The size variable
 *                     (recvPayload->payloadLen) will be set by this function
 *                     and indicates the number of actually received bytes.
 * @param recvBufferSize Size of the receive buffer in bytes.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
status_t SF_SendCommand(sf_drv_data_t *drvData, sf_spi_cmd_t cmd,
        const sf_msg_payload_t *sendPayload, sf_msg_payload_t *recvPayload,
        uint8_t recvBufferSize);

/*!
 * @brief This function sends the "Send Wakeup" command to wake up the device
 * from power down mode.
 *
 * Note that this command does not have an acknowledgement.
 *
 * @param drvData Driver run-time data.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
status_t SF_WakeUp(sf_drv_data_t *drvData);

/*!
 * @brief This function puts the device to power off mode with use of the
 * "Send To Sleep" command. In this mode the device settings are reset to
 * default values by the SIGFOX device.
 *
 * Note: to lower power consumption of the SIGFOX device the user should set
 * MCU pins connected to the device to low except the CS pin which should be
 * asserted high.
 * Note that this command does not have an acknowledgement.
 *
 * @param drvData Driver run-time data.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
status_t SF_Sleep(sf_drv_data_t *drvData);

/*!
 * @brief This function sets uplink frequency of the SIGFOX device.
 *
 * @param drvData Driver run-time data.
 * @param freqHz  Uplink frequency in Hz.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
status_t SF_SetUlFrequency(sf_drv_data_t *drvData, uint32_t freqHz);

/*!
 * @brief This function gets uplink frequency of the SIGFOX device.
 *
 * @param drvData Driver run-time data.
 * @param freqHz  Pointer where the uplink frequency is stored in Hz.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
status_t SF_GetUlFrequency(sf_drv_data_t *drvData, uint32_t *freqHz);

/*!
 * @brief This function sends the "Send Payload" command to the device. It sends
 * user data to SIGFOX network.
 *
 * @param drvData     Driver run-time data.
 * @param sendPayload Pointer to a frame payload to be sent.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
status_t SF_SendPayload(sf_drv_data_t *drvData, const sf_msg_payload_t *sendPayload);

/*!
 * @brief This function sends the "Send Bit" command, which transmits just one
 * bit to the SIGFOX network. This is the shortest frame that SIGFOX library
 * generates. It is intended for testing.
 *
 * @param drvData Driver run-time data.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
status_t SF_SendBit(sf_drv_data_t *drvData);

/*!
 * @brief This function sends the "Send Out of band" command, which transmits
 * data to the SIGFOX network. Data is composed of information about the chip
 * itself (Voltage, Temperature). This function must be called every 24 hours
 * at least or never if an application has some energy critical constraints.
 *
 * @param drvData Driver run-time data.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
status_t SF_SendOutOfBand(sf_drv_data_t *drvData);

#if (SF_MODEL == SF_MODEL_OL2385)
/*!
 * @brief This function receives a frame from SIGFOX network with use of the
 * "Receive Frame" command. It is available for OL2385 device only. Max. size
 * of received data is eight bytes (SIGFOX limitation).
 *
 * @param drvData     Driver run-time data.
 * @param recvPayload Pointer where the paylaod from a received ACK frame is
 *                    stored.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
status_t SF_ReceiveMessage(sf_drv_data_t *drvData, sf_msg_payload_t *recvPayload);
#endif

/*!
 * @brief This function resets the SIGFOX device watchdog with use of the
 * "Trigger Watchdog" command.
 *
 * Note that this command does not have an acknowledgement.
 *
 * @param drvData Driver run-time data.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
status_t SF_TriggerWatchdog(sf_drv_data_t *drvData);

/*!
 * @brief This function checks if there is a valid key and ID combination
 * written on the device. It sends the "Check ID Key" command to the device.
 *
 * @param drvData Driver run-time data.
 * @param success Pointer to the result of the check (true - OK, false - error).
 *
 * @return Status result of the function (kStatus_Success on success).
 */
status_t SF_CheckIdKey(sf_drv_data_t *drvData, bool *success);

/*!
 * @brief This function reads device ID, PAC, SIGFOX library version
 * and device version. It uses "Get info" and "Get Device Version" commands.
 *
 * @param drvData Driver run-time data.
 * @param devInfo Pointer where resulting device information is stored.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
status_t SF_GetDeviceInfo(sf_drv_data_t *drvData, sf_device_info_t *devInfo);

/*!
 * @brief This function returns the current state of the SIGFOX device
 * software state machine. The state is updated every time when an ACK
 * SPI frame is received.
 *
 * @param drvData Driver run-time data.
 *
 * @return SIGFOX device state received in the last SPI frame.
 */
static inline sf_intern_state_t SF_GetState(sf_drv_data_t *drvData)
{
    return drvData->devState;
}

/*!
 * @brief This function returns an error code received in a SPI frame from
 * the SIGFOX device. The error code is updated every time when an ACK SPI
 * frame is received.
 *
 * @param drvData Driver run-time data.
 *
 * @return An error code received in the last SPI frame.
 */
static inline sf_intern_error_t SF_GetErrorCode(sf_drv_data_t *drvData)
{
    return drvData->errorCode;
}

/*!
 * @brief This function tests if the SPI bus is working. It uses the "Echo"
 * command to send data (see SF_ECHO_DATA macro) and checks if the device
 * replies with the inverted payload.
 *
 * @param drvData Driver run-time data.
 * @param success Pointer to the result of the check (true - OK, false - error).
 *
 * @return Status result of the function (kStatus_Success on success).
 */
status_t SF_TestSpiCon(sf_drv_data_t *drvData, bool *success);

/*!
 * @brief This function executes a test of uplink and downlink connectivity
 * using the "Send Test Mode" command. Returned status represents result
 * of the test (success or fail), the user can obtain details using the
 * SF_GetErrorCode function.
 *
 * @param drvData  Driver run-time data.
 * @param testMode Test mode selection.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
status_t SF_TestDevice(sf_drv_data_t *drvData, sf_test_mode_t testMode);

/*!
 * @brief This function sends the "Continuous Wave" command, which can be used
 * for testing. It generates a continuous transmission at last set frequency.
 * The signal can be then analyzed using spectrum analyzer.
 *
 * Note that this command does not have an acknowledgement.
 *
 * @param drvData Driver run-time data.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
status_t SF_GenerateContWave(sf_drv_data_t *drvData);

/*!
 * @brief This function changes up-link and down-link frequency and bit rate.
 *
 * It enables to set European ETSI standard (default setting), USA FCC standard,
 * Japanese/Korean ARIB standard and South American FCC standard. It uses Change
 * To RCZ1, Change To RCZ2, Change To RCZ3 and Change To RCZ4 commands to set
 * an appropriate standard.
 *
 * @param drvData     Driver run-time data.
 * @param standardSel Selection of a standard defining network communication
 * parameters.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
status_t SF_ChangeNetworkStandard(sf_drv_data_t *drvData,
        sf_net_standard_t standardSel);

/*!
 * @brief This function sends a command to the device. It does not wait for
 * an acknowledgement.
 *
 * The user can use SF_HAS_CMD_ACK macro to figure out if a command
 * has ACK. Then he can utilize SF_IsAckFrameReady and SF_ReadAckFrameNonBlock
 * functions to receive the ACK.
 *
 * @param drvData     Driver run-time data.
 * @param cmd         The frame command number.
 * @param sendPayload Pointer to a frame payload to be sent. The user can set
 * payload data (sendPayload->payload) to NULL when the size is zero.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
status_t SF_SendCommandNonBlock(sf_drv_data_t *drvData, sf_spi_cmd_t cmd,
        const sf_msg_payload_t *sendPayload);

/*!
 * @brief This function checks if the SIGFOX device is ready to send an
 * acknowledgement. It checks ACK pin value, which is active low.
 *
 * The user can use SF_HAS_CMD_ACK macro to check if a command has an
 * acknowledgement.
 *
 * @param drvData     Driver run-time data.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
bool SF_IsAckFrameReady(sf_drv_data_t *drvData);

/*!
 * @brief This function receives an acknowledge frame.
 *
 * It should be used in conjunction with SF_SendCommandNonBlock and
 * SF_IsAckFrameReady functions. The user can use SF_HAS_CMD_ACK macro to check
 * if a command has an acknowledgement.
 *
 * @param drvData      Driver run-time data.
 * @param recvPayload  Pointer where a resulting payload is stored. Size of the
 *                     data variable (recvPayload->payload) must be sufficient
 *                     (see SF_ACK_PAYLOAD_MAX_B macro). The size variable
 *                     (recvPayload->payloadLen) will be set by this function
 *                     and indicates the number of actually received bytes.
 * @param recvBufferSize Size of the receive buffer in bytes.
 *
 * @return Status result of the function (kStatus_Success on success).
 */
status_t SF_ReadAckFrameNonBlock(sf_drv_data_t *drvData,
        sf_msg_payload_t *recvPayload, uint8_t recvBufferSize);
/*! @} */

#endif /* SOURCE_SF_SF_H_ */

/*******************************************************************************
 * EOF
 ******************************************************************************/
