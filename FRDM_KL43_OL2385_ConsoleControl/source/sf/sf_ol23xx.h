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
 * @file sf_ol23xx.h
 *
 * Definitions of SPI frames and commands for the OL2361 and OL2385.
 */

#ifndef SOURCE_SF_SF_OL23XX_H_
#define SOURCE_SF_SF_OL23XX_H_

/*******************************************************************************
 * General definitions
 ******************************************************************************/

/*!
 * @name General bit operations */
/*! @{ */

/*!
 * @brief Macro for setting value of the bit given by the mask.
 *
 * @param Msg Message to be modified
 * @param Mask Bit selection in message
 */
#define SF_SET_BIT_VALUE(Msg, Mask) ((Msg) | (Mask))

/*!
 * @brief Macro for unsetting value of the bit given by the mask.
 *
 * @param Msg Message to be modified
 * @param Mask Bit selection in message
 */
#define SF_UNSET_BIT_VALUE(Msg, Mask) ((Msg) & ~(Mask))

/*!
 * @brief Macro for getting value of the bit given by the mask.
 *
 * @param Msg Message to be read
 * @param Mask Bit selection in message
 * @param Shift Bit shift in message
 */
#define SF_GET_BIT_VALUE(Msg, Mask, Shift) (((Msg) & (Mask)) >> (Shift))

/*!
 * @brief Macro for setting value of bits given by the mask.
 *
 * @param Msg Message to be modified
 * @param Mask Bits selection in message
 * @param Shift Bits shift in message
 * @param Val Value to be applied
 * @param Range Admissible range of value
 */
#define SF_SET_BITS_VALUE(Msg, Mask, Shift, Val, Range) (((Msg) & ~(Mask)) | (((Val) & (Range)) << (Shift)))

/*!
 * @brief Macro for getting value of bits given by the mask.
 *
 * @param Msg Message to be read
 * @param Mask Bits selection in message
 * @param Shift Bits shift in message
 */
#define SF_GET_BITS_VALUE(Msg, Mask, Shift) (((Msg) & (Mask)) >> (Shift))
/*! @} */

/*! @name Min/max macros */
/* @{ */
#if !defined(SF_MIN)
#define SF_MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#if !defined(SF_MAX)
#define SF_MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
/* @} */

/*******************************************************************************
 * Definitions of SPI frames
 ******************************************************************************/
/*!
 * @name SPI ACK frame
 * Definitions common for all ACK frames.
 */
/*! @{ */
/*! Size of the SPI ACK frame header (including length, error code and state fields)
 * in bytes. */
#define SF_ACK_HEADER_B             0x03U
/*! Max. payload size in bytes ("Get info" returns the longest payload). */
#define SF_ACK_PAYLOAD_MAX_B        SF_GET_INFO_ACK_PLD_B
/*! Max. ACK frame size in bytes. */
#define SF_ACK_SPI_MSG_MAX_B        (SF_ACK_HEADER_B + SF_ACK_PAYLOAD_MAX_B)
/*! Offset of the Length field in the ACK frame. */
#define SF_ACK_LENGTH_OF            0X00U
/*! Offset of the Error code field in the ACK frame. */
#define SF_ACK_ERROR_OF             0X01U
/*! Offset of the State field in the ACK frame. */
#define SF_ACK_STATE_OF             0x02U
/*! Offset of the Payload field in the ACK frame. */
#define SF_ACK_PAYLOAD_OF           0x03U
/*! @} */

/*!
 * @name SPI Information frame
 * Definitions common for all ACK frames.
 * @{
 */
/*! Size of the SPI Information frame header (including length and command fields)
 * in bytes. */
#define SF_INF_HEADER_B             0x02U
/*! Max. payload size in bytes (a command "Send Payload" returns up to 12 bytes). */
#define SF_INF_PAYLOAD_MAX_B        SF_SEND_PAYLOAD_INF_PLD_B
/*! Max. Information frame size in bytes. */
#define SF_INF_SPI_MSG_MAX_B        (SF_INF_HEADER_B + SF_INF_PAYLOAD_MAX_B)
/*! Offset of the Length field in the Information frame. */
#define SF_INF_LENGTH_OF            0x00U
/*! Offset of the Command field in the Information frame. */
#define SF_INF_CMD_OF               0x01U
/*! Offset of the Payload field in the Information frame. */
#define SF_INF_PAYLOAD_OF           0x02U
/*! @} */

/*!
 * @name SPI Information frame
 * The min. and max. value of the command ID.
 * @{
 */
#define SF_CMD_FIRST_ID             SF_CMD_SEND_WAKEUP_ID
#define SF_CMD_LAST_ID              SF_CMD_CHANGE_TO_RCZ4_ID
/*! @} */

/*! Max. SPI frame size in bytes. */
#define SF_SPI_MSG_MAX_B            (SF_MAX(SF_INF_SPI_MSG_MAX_B, SF_ACK_SPI_MSG_MAX_B))

/*******************************************************************************
 * Definitions of SPI commands
 ******************************************************************************/
/*!
 * @name Send Wake-up SPI command
 * @{
 */
/*! Send Wake-up command. */
#define SF_CMD_SEND_WAKEUP_ID       0x01U
/*! Size of the "Send Wake-up" information frame payload in bytes. */
#define SF_SEND_WAKEUP_INF_PLD_B    0x00U
/*! Size of the "Send Wake-up" information frame in bytes. */
#define SF_SEND_WAKEUP_INF_B        (SF_INF_HEADER_B + SF_SEND_WAKEUP_INF_PLD_B)
/*! Size of the "Send Wake-up" acknowledgement payload in bytes. */
#define SF_SEND_WAKEUP_ACK_PLD_B    0x00U
/*! Size of the "Send Wake-up" acknowledgement in bytes (no ACK). */
#define SF_SEND_WAKEUP_ACK_B        0x00U
/*! Size of the SPI frame in bytes. It is the max. of ACK frame and I-frame. */
#define SF_SEND_WAKEUP_FRM_B        (SF_MAX(SF_SEND_WAKEUP_INF_B, SF_SEND_WAKEUP_ACK_B))
/*! @} */

/*!
 * @name Send Echo SPI command
 * @{
 */
/*! Echo command. */
#define SF_CMD_SEND_ECHO_ID         0x02U
/*! Size of the "Send Echo" information frame payload in bytes (max. 5). */
#define SF_SEND_ECHO_INF_PLD_B      0x05U
/*! Size of the "Send Echo" information frame in bytes. */
#define SF_SEND_ECHO_INF_B          (SF_INF_HEADER_B + SF_SEND_ECHO_INF_PLD_B)
/*! Size of the "Send Echo" acknowledgement payload in bytes (max. 5). */
#define SF_SEND_ECHO_ACK_PLD_B      0x05U
/*! Size of the "Send Echo" acknowledgement in bytes. */
#define SF_SEND_ECHO_ACK_B          (SF_ACK_HEADER_B + SF_SEND_ECHO_ACK_PLD_B)
/*! Size of the SPI frame in bytes. It is the max. of ACK frame and I-frame. */
#define SF_SEND_ECHO_FRM_B          (SF_MAX(SF_SEND_ECHO_INF_B, SF_SEND_ECHO_ACK_B))
/*! @} */

/*!
 * @name Send to Sleep SPI command
 * @{
 */
/*! Send to sleep command. */
#define SF_CMD_SEND_TO_SLEEP_ID     0x03U
/*! Size of the "Send to sleep" information frame payload in bytes. */
#define SF_SEND_TO_SLEEP_INF_PLD_B  0x00U
/*! Size of the "Send to sleep" information frame in bytes. */
#define SF_SEND_TO_SLEEP_INF_B      (SF_INF_HEADER_B + SF_SEND_TO_SLEEP_INF_PLD_B)
/*! Size of the "Send to Sleep" acknowledgement payload in bytes. */
#define SF_SEND_TO_SLEEP_ACK_PLD_B  0x00U
/*! Size of the "Send to Sleep" acknowledgement in bytes. */
#define SF_SEND_TO_SLEEP_ACK_B      0x00U
/*! Size of the SPI frame in bytes. It is the max. of ACK frame and I-frame. */
#define SF_SEND_TO_SLEEP_FRM_B      (SF_MAX(SF_SEND_TO_SLEEP_INF_B, SF_SEND_TO_SLEEP_ACK_B))
/*! @} */

/*!
 * @name Send Payload SPI command
 * @{
 */
/*! Send payload command. */
#define SF_CMD_SEND_PAYLOAD_ID      0x04U
/*! Size of the "Send payload" information frame payload in bytes (max. 12). */
#define SF_SEND_PAYLOAD_INF_PLD_B   0x0CU
/*! Size of the "Send payload" information frame in bytes. */
#define SF_SEND_PAYLOAD_INF_B       (SF_INF_HEADER_B + SF_SEND_PAYLOAD_INF_PLD_B)
/*! Size of the "Send payload" acknowledgement payload in bytes. */
#define SF_SEND_PAYLOAD_ACK_PLD_B   0x00U
/*! Size of the "Send payload" acknowledgement in bytes. */
#define SF_SEND_PAYLOAD_ACK_B       (SF_ACK_HEADER_B + SF_SEND_PAYLOAD_ACK_PLD_B)
/*! Size of the SPI frame in bytes. It is the max. of ACK frame and I-frame. */
#define SF_SEND_PAYLOAD_FRM_B       (SF_MAX(SF_SEND_PAYLOAD_INF_B, SF_SEND_PAYLOAD_ACK_B))
/*! @} */

/*!
 * @name Send bit SPI command
 * @{
 */
/*! Send bit command. */
#define SF_CMD_SEND_BIT_ID          0x05U
/*! Size of the "Send bit" information frame payload in bytes. */
#define SF_SEND_BIT_INF_PLD_B       0x00U
/*! Size of the "Send bit" information frame in bytes. */
#define SF_SEND_BIT_INF_B           (SF_INF_HEADER_B + SF_SEND_BIT_INF_PLD_B)
/*! Size of the "Send bit" acknowledgement payload in bytes. */
#define SF_SEND_BIT_ACK_PLD_B       0x00U
/*! Size of the "Send bit" acknowledgement in bytes. */
#define SF_SEND_BIT_ACK_B           (SF_ACK_HEADER_B + SF_SEND_BIT_ACK_PLD_B)
/*! Size of the SPI frame in bytes. It is the max. of ACK frame and I-frame. */
#define SF_SEND_BIT_FRM_B           (SF_MAX(SF_SEND_BIT_INF_B, SF_SEND_BIT_ACK_B))
/*! @} */

/*!
 * @name Send Out of Band SPI command
 * @{
 */
/*! Send Out of Band command. */
#define SF_CMD_SEND_OUT_BAND_ID     0x06U
/*! Size of the "Send Out of Band" information frame payload in bytes. */
#define SF_SEND_OUT_BAND_INF_PLD_B  0x00U
/*! Size of the "Send Out of Band" information frame in bytes. */
#define SF_SEND_OUT_BAND_INF_B      (SF_INF_HEADER_B + SF_SEND_OUT_BAND_INF_PLD_B)
/*! Size of the "Send Out of Band" acknowledgement payload in bytes. */
#define SF_SEND_OUT_BAND_ACK_PLD_B  0x00U
/*! Size of the "Send Out of Band" acknowledgement in bytes. */
#define SF_SEND_OUT_BAND_ACK_B      (SF_ACK_HEADER_B + SF_SEND_OUT_BAND_ACK_PLD_B)
/*! Size of the SPI frame in bytes. It is the max. of ACK frame and I-frame. */
#define SF_SEND_OUT_BAND_FRM_B      (SF_MAX(SF_SEND_OUT_BAND_INF_B, SF_SEND_OUT_BAND_ACK_B))
/*! @} */

#if (SF_MODEL == SF_MODEL_OL2385)
/*!
 * @name Receive Frame SPI command
 * @{
 */
/*! Receive Frame command (available on OL2385). */
#define SF_CMD_RECEIVE_FR_ID        0x07U
/*! Size of the "Receive Frame" information frame payload in bytes. */
#define SF_RECEIVE_FR_INF_PLD_B     0x00U
/*! Size of the "Receive Frame" information frame in bytes. */
#define SF_RECEIVE_FR_INF_B         (SF_INF_HEADER_B + SF_RECEIVE_FR_INF_PLD_B)
/*! Size of the "Receive Frame" acknowledgement payload in bytes (max. 8). */
#define SF_RECEIVE_FR_ACK_PLD_B     0x08U
/*! Size of the "Receive Frame" acknowledgement in bytes. */
#define SF_RECEIVE_FR_ACK_B         (SF_ACK_HEADER_B + SF_RECEIVE_FR_ACK_PLD_B)
/*! Size of the SPI frame in bytes. It is the max. of ACK frame and I-frame. */
#define SF_RECEIVE_FR_FRM_B         (SF_MAX(SF_RECEIVE_FR_INF_B, SF_RECEIVE_FR_ACK_B))
/*! @} */
#endif /* END of device model check. */

/*!
 * @name Get Info SPI command
 * @{
 */
/*! Get Info command. */
#define SF_CMD_GET_INFO_ID          0x08U
/*! Size of the "Get Info" information frame payload in bytes. */
#define SF_GET_INFO_INF_PLD_B       0x00U
/*! Size of the "Get Info" information frame in bytes. */
#define SF_GET_INFO_INF_B           (SF_INF_HEADER_B + SF_GET_INFO_INF_PLD_B)
/*! Size of the "Get Info" acknowledgement payload in bytes. */
#define SF_GET_INFO_ACK_PLD_B       23U
/*! Size of the "Get Info" acknowledgement in bytes. */
#define SF_GET_INFO_ACK_B           (SF_ACK_HEADER_B + SF_GET_INFO_ACK_PLD_B)
/*! Size of the SPI frame in bytes. It is the max. of ACK frame and I-frame. */
#define SF_GET_INFO_FRM_B           SF_MAX(SF_GET_INFO_INF_B, SF_GET_INFO_ACK_B)

/*! Size of the Device ID field in the ACK frame. */
#define SF_DEV_ID_B                 4U
/*! Size of the PAC field in the ACK frame. */
#define SF_DEV_PAC_B                8U
/*! Size of the Library version field in the ACK frame. */
#define SF_LIB_VER_B                11U

/*! Offset of the Device ID field in the payload of ACK frame. */
#define SF_DEV_ID_OF                0U
/*! Offset of the PAC field in the payload of ACK frame. */
#define SF_DEV_PAC_OF               4U
/*! Offset of the Library version field in the payload of ACK frame. */
#define SF_LIB_VER_OF               12U

/*!
 * @brief This macro returns a 32 bit value containing device ID
 * from the "Get Info" acknowledgement.
 *
 * @param payload Pointer to the Device ID field in the acknowledgement payload
 * (8 bit pointer).
 */
#define SF_GET_DEV_ID(payload) \
    (((uint32_t)(*(payload))) | \
     ((uint32_t)(*((payload) + 1)) << 8U) | \
     ((uint32_t)(*((payload) + 2)) << 16U) | \
     ((uint32_t)(*((payload) + 3)) << 24U))
/*! @} */

/*!
 * @name Set UL frequency SPI command
 * @{
 */
/*! UL frequency offset of the high byte of the FREQCON0 register (2 bytes)
 * in the payload. */
#define SF_UL_FREQCON0_HIGH_OF      0x00U
/*! UL frequency offset of the low byte of the FREQCON0 register (2 bytes)
 * in the payload. */
#define SF_UL_FREQCON0_LOW_OF       0x01U
/*! UL frequency offset of the high byte of the FREQCON1 register (2 bytes)
 * in the payload. */
#define SF_UL_FREQCON1_HIGH_OF      0x02U
/*! UL frequency offset of the low byte of the FREQCON1 register (2 bytes)
 * in the payload. */
#define SF_UL_FREQCON1_LOW_OF       0x03U

/*! Set UL frequency command. */
#define SF_CMD_SET_UL_FREQ_ID       0x09U
/*! Size of the "Set UL frequency" information frame payload in bytes. */
#define SF_SET_UL_FREQ_INF_PLD_B    0x04U
/*! Size of the "Set UL frequency" information frame in bytes. */
#define SF_SET_UL_FREQ_INF_B        (SF_INF_HEADER_B + SF_SET_UL_FREQ_INF_PLD_B)
/*! Size of the "Set UL frequency" acknowledgement payload in bytes. */
#define SF_SET_UL_FREQ_ACK_PLD_B    0x00U
/*! Size of the "Set UL frequency" acknowledgement in bytes. */
#define SF_SET_UL_FREQ_ACK_B        (SF_ACK_HEADER_B + SF_SET_UL_FREQ_ACK_PLD_B)
/*! Size of the SPI frame in bytes. It is the max. of ACK frame and I-frame. */
#define SF_SET_UL_FREQ_FRM_B        (SF_MAX(SF_SET_UL_FREQ_INF_B, SF_SET_UL_FREQ_ACK_B))

/*! "Set UL frequency" offset of the high byte of the FREQCON0 register (2 bytes)
 * in the I-frame. */
#define SF_SET_UL_FREQCON0_HIGH_OF  (SF_UL_FREQCON0_HIGH_OF + SF_INF_PAYLOAD_OF)
/*! "Set UL frequency" offset of the low byte of the FREQCON0 register (2 bytes)
 * in the I-frame. */
#define SF_SET_UL_FREQCON0_LOW_OF   (SF_UL_FREQCON0_LOW_OF + SF_INF_PAYLOAD_OF)
/*! "Set UL frequency" offset of the high byte of the FREQCON1 register (2 bytes)
 * in the I-frame. */
#define SF_SET_UL_FREQCON1_HIGH_OF  (SF_UL_FREQCON1_HIGH_OF + SF_INF_PAYLOAD_OF)
/*! "Set UL frequency" offset of the low byte of the FREQCON1 register (2 bytes)
 * in the I-frame. */
#define SF_SET_UL_FREQCON1_LOW_OF   (SF_UL_FREQCON1_LOW_OF + SF_INF_PAYLOAD_OF)
/*! @} */

/*!
 * @name Get UL frequency SPI command
 * @{
 */
/*! Get UL frequency command. */
#define SF_CMD_GET_UL_FREQ_ID       0x0AU
/*! Size of the "Get UL frequency" information frame payload in bytes. */
#define SF_GET_UL_FREQ_INF_PLD_B    0x00U
/*! Size of the "Get UL frequency" information frame in bytes. */
#define SF_GET_UL_FREQ_INF_B        (SF_INF_HEADER_B + SF_GET_UL_FREQ_INF_PLD_B)
/*! Size of the "Get UL frequency" acknowledgement payload in bytes. */
#define SF_GET_UL_FREQ_ACK_PLD_B    0x04U
/*! Size of the "Get UL frequency" acknowledgement in bytes. */
#define SF_GET_UL_FREQ_ACK_B        (SF_ACK_HEADER_B + SF_GET_UL_FREQ_ACK_PLD_B)
/*! Size of the SPI frame in bytes. It is the max. of ACK frame and I-frame. */
#define SF_GET_UL_FREQ_FRM_B        (SF_MAX(SF_GET_UL_FREQ_INF_B, SF_GET_UL_FREQ_ACK_B))

/*! "Get UL frequency" offset of the high byte of the FREQCON0 register (2 bytes)
 * in the ACK frame. */
#define SF_GET_UL_FREQCON0_HIGH_OF  (SF_UL_FREQCON0_HIGH_OF + SF_ACK_PAYLOAD_OF)
/*! "Get UL frequency" offset of the low byte of the FREQCON0 register (2 bytes)
 * in the ACK frame. */
#define SF_GET_UL_FREQCON0_LOW_OF   (SF_UL_FREQCON0_LOW_OF + SF_ACK_PAYLOAD_OF)
/*! "Get UL frequency" offset of the high byte of the FREQCON1 register (2 bytes)
 * in the ACK frame. */
#define SF_GET_UL_FREQCON1_HIGH_OF  (SF_UL_FREQCON1_HIGH_OF + SF_ACK_PAYLOAD_OF)
/*! "Get UL frequency" offset of the low byte of the FREQCON1 register (2 bytes)
 * in the ACK frame. */
#define SF_GET_UL_FREQCON1_LOW_OF   (SF_UL_FREQCON1_LOW_OF + SF_ACK_PAYLOAD_OF)

/*!
 * @brief This macro returns value of the FREQCON0 register stored in the
 * payload of the acknowledment frame.
 *
 * @param payload Pointer to the acknowledgement payload (8 bit).
 */
#define SF_GET_FREQCON0(payload) \
    (((uint16_t)(*((payload) + SF_UL_FREQCON0_HIGH_OF)) << 8U) | \
      (uint16_t)(*((payload) + SF_UL_FREQCON0_LOW_OF)))

/*!
 * @brief This macro returns value of the FREQCON0 register stored in the
 * payload of the acknowledment frame.
 *
 * @param payload Pointer to the acknowledgement payload (8 bit).
 */
#define SF_GET_FREQCON1(payload) \
    (((uint16_t)(*((payload) + SF_UL_FREQCON1_HIGH_OF)) << 8U) | \
      (uint16_t)(*((payload) + SF_UL_FREQCON1_LOW_OF)))
/*! @} */

/*!
 * @name Continuous Wave SPI command
 * @{
 */
/*! Continuous Wave command. */
#define SF_CMD_CONT_WAVE_ID         0x0BU
/*! Size of the "Continuous Wave" information frame payload in bytes. */
#define SF_CONT_WAVE_INF_PLD_B      0x00U
/*! Size of the "Continuous Wave" information frame in bytes. */
#define SF_CONT_WAVE_INF_B          (SF_INF_HEADER_B + SF_CONT_WAVE_INF_PLD_B)
/*! Size of the "Continuous Wave" acknowledgement payload in bytes. */
#define SF_CONT_WAVE_ACK_PLD_B      0x00U
/*! Size of the "Continuous Wave" acknowledgement in bytes. */
#define SF_CONT_WAVE_ACK_B          0x00U
/*! Size of the SPI frame in bytes. It is the max. of ACK frame and I-frame. */
#define SF_CONT_WAVE_FRM_B          (SF_MAX(SF_CONT_WAVE_INF_B, SF_CONT_WAVE_ACK_B))
/*! @} */

/*!
 * @name Check ID key SPI command
 * @{
 */
/*! Check ID key command. */
#define SF_CMD_CHECK_ID_ID          0x0CU
/*! Size of the "Check ID key" information frame payload in bytes. */
#define SF_CHECK_ID_INF_PLD_B       0x00U
/*! Size of the "Check ID key" information frame in bytes. */
#define SF_CHECK_ID_INF_B           (SF_INF_HEADER_B + SF_CHECK_ID_INF_PLD_B)
/*! Size of the "Check ID key" acknowledgement payload in bytes. */
#define SF_CHECK_ID_ACK_PLD_B       0x01U
/*! Size of the "Check ID key" acknowledgement in bytes. */
#define SF_CHECK_ID_ACK_B           (SF_ACK_HEADER_B + SF_CHECK_ID_ACK_PLD_B)
/*! Size of the SPI frame in bytes. It is the max. of ACK frame and I-frame. */
#define SF_CHECK_ID_FRM_B           (SF_MAX(SF_CHECK_ID_INF_B, SF_CHECK_ID_ACK_B))
/*! @} */

/*!
 * @name Get Device Version SPI command
 * @{
 */
/*! Get Device Version command. */
#define SF_CMD_GET_DEV_VER_ID       0x0DU
/*! Size of the "Get Device Version" information frame payload in bytes. */
#define SF_GET_DEV_VER_INF_PLD_B    0x00U
/*! Size of the "Get Device Version" information frame in bytes. */
#define SF_GET_DEV_VER_INF_B        (SF_INF_HEADER_B + SF_GET_DEV_VER_INF_PLD_B)
/*! Size of the "Get Device Version" acknowledgement payload in bytes. */
#define SF_GET_DEV_VER_ACK_PLD_B    0x0FU
/*! Size of the "Get Device Version" acknowledgement in bytes. */
#define SF_GET_DEV_VER_ACK_B        (SF_ACK_HEADER_B + SF_GET_DEV_VER_ACK_PLD_B)
/*! Size of the SPI frame in bytes. It is the max. of ACK frame and I-frame. */
#define SF_GET_DEV_VER_FRM_B        (SF_MAX(SF_GET_DEV_VER_INF_B, SF_GET_DEV_VER_ACK_B))

/*! Size of the Device version field in the ACK frame. */
#define SF_DEV_VER_B                15U

/*! Offset of the Device version field in the ACK frame. */
#define SF_DEV_VER_OF               0U
/*! @} */

/*!
 * @name Set Watchdog SPI command
 * @{
 */
/*! Set Watchdog Timer command. */
#define SF_CMD_SET_WD_TIMER_ID      0x0EU
/*! Size of the "Get Device Version" information frame payload in bytes. */
#define SF_SET_WD_TIMER_INF_PLD_B   0x01U
/*! Size of the "Get Device Version" information frame in bytes. */
#define SF_SET_WD_TIMER_INF_B       (SF_INF_HEADER_B + SF_SET_WD_TIMER_INF_PLD_B)
/*! Size of the "Get Device Version" acknowledgement payload in bytes. */
#define SF_SET_WD_TIMER_ACK_PLD_B   0x00U
/*! Size of the "Get Device Version" acknowledgement in bytes. */
#define SF_SET_WD_TIMER_ACK_B       (SF_ACK_HEADER_B + SF_SET_WD_TIMER_ACK_PLD_B)
/*! Size of the SPI frame in bytes. It is the max. of ACK frame and I-frame. */
#define SF_SET_WD_TIMER_FRM_B       (SF_MAX(SF_SET_WD_TIMER_INF_B, SF_SET_WD_TIMER_ACK_B))
/*! @} */

/*!
 * @name Get Watchdog SPI command
 * @{
 */
/*! Get Watchdog Timer command. */
#define SF_CMD_GET_WD_TIMER_ID      0x0FU
/*! Size of the "Get Watchdog Timer" information frame payload in bytes. */
#define SF_GET_WD_TIMER_INF_PLD_B   0x00U
/*! Size of the "Get Watchdog Timer" information frame in bytes. */
#define SF_GET_WD_TIMER_INF_B       (SF_INF_HEADER_B + SF_GET_WD_TIMER_INF_PLD_B)
/*! Size of the "Get Watchdog Timer" acknowledgement payload in bytes. */
#define SF_GET_WD_TIMER_ACK_PLD_B   0x01U
/*! Size of the "Get Watchdog Timer" acknowledgement in bytes. */
#define SF_GET_WD_TIMER_ACK_B       (SF_ACK_HEADER_B + SF_GET_WD_TIMER_ACK_PLD_B)
/*! Size of the SPI frame in bytes. It is the max. of ACK frame and I-frame. */
#define SF_GET_WD_TIMER_FRM_B       (SF_MAX(SF_GET_WD_TIMER_INF_B, SF_GET_WD_TIMER_ACK_B))
/*! @} */

/*!
 * @name Set Register SPI command
 * @{
 */
/*! Set Register command. */
#define SF_CMD_SET_REG_ID           0x10U
/*! Size of the "Set Register" information frame payload in bytes. */
#define SF_SET_REG_INF_PLD_B        0x03U
/*! Size of the "Set Register" information frame in bytes. */
#define SF_SET_REG_INF_B            (SF_INF_HEADER_B + SF_SET_REG_INF_PLD_B)
/*! Size of the "Set Register" acknowledgement payload in bytes. */
#define SF_SET_REG_ACK_PLD_B        0x00U
/*! Size of the "Set Register" acknowledgement in bytes. */
#define SF_SET_REG_ACK_B            (SF_ACK_HEADER_B + SF_SET_REG_ACK_PLD_B)
/*! Size of the SPI frame in bytes. It is the max. of ACK frame and I-frame. */
#define SF_SET_REG_FRM_B            (SF_MAX(SF_SET_REG_INF_B, SF_SET_REG_ACK_B))
/*! @} */

/*!
 * @name Get Register SPI command
 * @{
 */
/*! Get Register command. */
#define SF_CMD_GET_REG_ID           0x11U
/*! Size of the "Get Register" information frame payload in bytes. */
#define SF_GET_REG_INF_PLD_B        0x01U
/*! Size of the "Get Register" information frame in bytes. */
#define SF_GET_REG_INF_B            (SF_INF_HEADER_B + SF_GET_REG_INF_PLD_B)
/*! Size of the "Get Register" acknowledgement payload in bytes. */
#define SF_GET_REG_ACK_PLD_B        0x02U
/*! Size of the "Get Register" acknowledgement in bytes. */
#define SF_GET_REG_ACK_B            (SF_ACK_HEADER_B + SF_GET_REG_ACK_PLD_B)
/*! Size of the SPI frame in bytes. It is the max. of ACK frame and I-frame. */
#define SF_GET_REG_FRM_B            (SF_MAX(SF_GET_REG_INF_B, SF_GET_REG_ACK_B))
/*! @} */

/*!
 * @name Trigger Watchdog SPI command
 * @{
 */
/*! Trigger Watchdog command. */
#define SF_CMD_TRIGGER_WD_ID        0x12U
/*! Size of the "Trigger Watchdog" information frame payload in bytes. */
#define SF_TRIGGER_WD_INF_PLD_B     0x00U
/*! Size of the "Trigger Watchdog" information frame in bytes. */
#define SF_TRIGGER_WD_INF_B         (SF_INF_HEADER_B + SF_TRIGGER_WD_INF_PLD_B)
/*! Size of the "Trigger Watchdog" acknowledgement payload in bytes. */
#define SF_TRIGGER_WD_ACK_PLD_B     0x00U
/*! Size of the "Trigger Watchdog" acknowledgement in bytes. */
#define SF_TRIGGER_WD_ACK_B         0x00U
/*! Size of the SPI frame in bytes. It is the max. of ACK frame and I-frame. */
#define SF_TRIGGER_WD_FRM_B         (SF_MAX(SF_TRIGGER_WD_INF_B, SF_TRIGGER_WD_ACK_B))
/*! @} */

/*!
 * @name Keep Alive Message SPI command
 * @{
 */
/*! Keep Alive Message command. */
#define SF_CMD_KEEP_ALIVE_ID        0x13U
/*! Size of the "Keep Alive Message" information frame payload in bytes. */
#define SF_KEEP_ALIVE_INF_PLD_B     0x00U
/*! Size of the "Keep Alive Message" information frame in bytes. */
#define SF_KEEP_ALIVE_INF_B         (SF_INF_HEADER_B + SF_KEEP_ALIVE_INF_PLD_B)
/*! Size of the "Keep Alive Message" acknowledgement payload in bytes. */
#define SF_KEEP_ALIVE_ACK_PLD_B     0x00U
/*! Size of the "Keep Alive Message" acknowledgement in bytes. */
#define SF_KEEP_ALIVE_ACK_B         (SF_ACK_HEADER_B + SF_KEEP_ALIVE_ACK_PLD_B)
/*! Size of the SPI frame in bytes. It is the max. of ACK frame and I-frame. */
#define SF_KEEP_ALIVE_FRM_B         (SF_MAX(SF_KEEP_ALIVE_INF_B, SF_KEEP_ALIVE_ACK_B))
/*! @} */

/*!
 * @name Test Mode SPI command
 * @{
 */
/*! Send Test Mode command. */
#define SF_CMD_SEND_TEST_ID         0x14U
/*! Size of the "Send Test Mode" information frame payload in bytes. */
#define SF_SEND_TEST_INF_PLD_B      0x03U
/*! Size of the "Send Test Mode" information frame in bytes. */
#define SF_SEND_TEST_INF_B          (SF_INF_HEADER_B + SF_SEND_TEST_INF_PLD_B)
/*! Size of the "Send Test Mode" acknowledgement payload in bytes. */
#define SF_SEND_TEST_ACK_PLD_B      0x00U
/*! Size of the "Send Test Mode" acknowledgement in bytes. */
#define SF_SEND_TEST_ACK_B          (SF_ACK_HEADER_B + SF_SEND_TEST_ACK_PLD_B)
/*! Size of the SPI frame in bytes. It is the max. of ACK frame and I-frame. */
#define SF_SEND_TEST_FRM_B          (SF_MAX(SF_SEND_TEST_INF_B, SF_SEND_TEST_ACK_B))

/*! Offset of the Mode field high byte in the Information message. */
#define SF_TEST_MODE_HIGH_OF        (0U + SF_INF_PAYLOAD_OF)
/*! Offset of the Mode field low byte in the Information message. */
#define SF_TEST_MODE_LOW_OF         (1U + SF_INF_PAYLOAD_OF)
/*! Offset of the Config field (1 bytes) in the Information message. */
#define SF_TEST_CFG_OF              (2U + SF_INF_PAYLOAD_OF)
/*! @} */


/*!
 * @name Change To RCZX commands
 * Definition of macros for Change To RCZ1, Change To RCZ2, Change To RCZ3 and
 * Change To RCZ4 commands.
 * @{
 */
/*! Change To RCZ1 command. */
#define SF_CMD_CHANGE_TO_RCZ1_ID    0x15U
/*! Change To RCZ2 command. */
#define SF_CMD_CHANGE_TO_RCZ2_ID    0x16U
/*! Change To RCZ3 command. */
#define SF_CMD_CHANGE_TO_RCZ3_ID    0x17U
/*! Change To RCZ4 command. */
#define SF_CMD_CHANGE_TO_RCZ4_ID    0x18U
/*! Size of the "Change To RCZX" information frame payload in bytes. */
#define SF_CHANGE_TO_RCZX_INF_PLD_B 0x00U
/*! Size of the "Change To RCZX" information frame in bytes. */
#define SF_CHANGE_TO_RCZX_INF_B     (SF_INF_HEADER_B + SF_CHANGE_TO_RCZX_INF_PLD_B)
/*! Size of the "Change To RCZX" acknowledgement payload in bytes. */
#define SF_CHANGE_TO_RCZX_ACK_PLD_B 0x00U
/*! Size of the "Change To RCZX" acknowledgement in bytes. */
#define SF_CHANGE_TO_RCZX_ACK_B     (SF_ACK_HEADER_B + SF_CHANGE_TO_RCZX_ACK_PLD_B)
/*! Size of the SPI frame in bytes. It is the max. of ACK frame and I-frame. */
#define SF_CHANGE_TO_RCZX_FRM_B     SF_MAX(SF_CHANGE_TO_RCZX_INF_B, SF_CHANGE_TO_RCZX_ACK_B)
/*! @} */

/*!
 * @name UL Frequency definitions
 * Following definitions are used in UL frequency formulas.
 * @{
 */
/*! Value of a crystal frequency used in UL formulas. */
#define SF_ULFREQ_F_X0              27600000U
/*! FIXDIV_DIV2_EN value, which is defined in OL2361 datasheet. Note that
 * OL2385 formulas uses a factor which is similar to this value. */
#define SF_ULFREQ_FIXDIV_DIV2_EN    0U
/*! This factor is used in UL frequency formulas. */
#define SF_ULFREQ_FACTOR            (1U + SF_ULFREQ_FIXDIV_DIV2_EN)
/*! @} */

/*!
 * @name UL Frequency FCFRAC
 * Masks and shifts for FCFRAC value.
 * @{
 */
/*! Mask for frequency control fractional part low-high bits (FCFRAC[8:0]). */
#define SF_FCFRAC_LH_MASK           0x1FFU
/*! Mask for frequency control fractional part high-low bits (FCFRAC[16:9]). */
#define SF_FCFRAC_HL_MASK           0x1FE00U
/*! Mask for frequency control fractional part high-high bits (FCFRAC[18:17]). */
#define SF_FCFRAC_HH_MASK           0x60000U

/*! Shift for frequency control fractional part low-high bits. */
#define SF_FCFRAC_LH_SHIFT          0x00U
/*! Shift for frequency control fractional part high-low bits. */
#define SF_FCFRAC_HL_SHIFT          0x09U
/*! Shift for frequency control fractional part high-high bits. */
#define SF_FCFRAC_HH_SHIFT          0x11U
/*! @} */

/*!
 * @name UL Frequency FCINT
 * Masks and shifts for FCINT value.
 * @{
 */
/*! Mask for integer part frequency control word bits. */
#define SF_FCINT_MASK               0x3FU

/*! Shift for integer part frequency control word bits. */
#define SF_FCINT_SHIFT              0x00U
/*! @} */

/*!
 * @name UL Frequency register FREQCON0
 * FREQCON0 register masks and shifts
 * @{
 */
/*! Mask for frequency control fractional part low-high bits of FREQCON0 register. */
#define SF_FREQCON0_FCFRAC_LH_MASK  0xFF80U
/*! Mask for frequency control disable fractional of FREQCON0 register. */
#define SF_FREQCON0_FCDISFRAC_MASK  0x01U

/*! Shift for frequency control fractional part low-high bits of FREQCON0 register. */
#define SF_FREQCON0_FCFRAC_LH_SHIFT 0x07U
/*! Shift for frequency control disable fractional of FREQCON0 register. */
#define SF_FREQCON0_FCDISFRAC_SHIFT 0x00U
/*! @} */

/*!
 * @name UL Frequency register FREQCON1
 * FREQCON1 register masks and shifts
 * @{
 */
/*! Mask for integer part frequency control word of FREQCON1 register. */
#define SF_FREQCON1_FCINT_MASK      0xFC00U
/*! Mask for frequency control fractional part high-high bits of FREQCON1 register. */
#define SF_FREQCON1_FCFRAC_HH_MASK  0x300U
/*! Mask for frequency control fractional part high-low bits of FREQCON1 register. */
#define SF_FREQCON1_FCFRAC_HL_MASK  0xFFU

/*! Shift for integer part frequency control word of FREQCON1 register. */
#define SF_FREQCON1_FCINT_SHIFT     0x0AU
/*! Shift for frequency control fractional part high-high bits of FREQCON1 register. */
#define SF_FREQCON1_FCFRAC_HH_SHIFT 0x08U
/*! Shift for frequency control fractional part high-low bits of FREQCON1 register. */
#define SF_FREQCON1_FCFRAC_HL_SHIFT 0x00U
/*! @} */

/*!
 * @name Max. and min. uplink frequency.
 * @{
 */
/* Min. uplink frequency in Hz. */
#define SF_UL_FREQ_MIN_HZ           775000000U
/* Max. uplink frequency in Hz. */
#define SF_UL_FREQ_MAX_HZ           1100000000U

/*! @} */

#endif /* SOURCE_SF_SF_OL23XX_H_ */

/*******************************************************************************
 * EOF
 ******************************************************************************/
