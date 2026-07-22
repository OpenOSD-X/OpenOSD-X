#include <stdint.h>
#include <stdbool.h>
#include <string.h> // For memcpy, memset
#include "main.h"
#include "log.h"
#include "char_canvas.h"
#include "uart_dma.h"
#include "mspvtx.h"
#include "setting.h"
#include "msp.h"

// --- MSP Protocol Defines ---
#define MSP_HEADER '$'
#define MSP_PROTOCOL_VERSION_V1 'M'
#define MSP_PROTOCOL_VERSION_V2 'X' // Or 'V' for some implementations - check Betaflight source
#define MSP_DIRECTION_OUT '<' // To FC
#define MSP_DIRECTION_IN '>'  // From FC


// Common MSP Command IDs (ensure these values are correct as per Betaflight source)
#define MSP_API_VERSION 1
#define MSP_FC_VARIANT 2
#define MSP_FC_VERSION 3
#define MSP_BOARD_INFO 4
#define MSP_BUILD_INFO 5


// Assuming DisplayPort commands are payload of MSP_DISPLAYPORT or a similar wrapper command
// If they are direct MSP commands, their values would be different.
// The user provided these as direct values, so we might assume a simplified scenario or
// that these are sub-commands within an MSP_DISPLAYPORT message.
// For this example, let's assume they are direct MSP commands for simplicity of the initial structure,
// but typically DisplayPort OSD commands are encapsulated.

// MSP Display Port commands
typedef enum {
    MSP_DP_HEARTBEAT = 0,       // Release the display after clearing and updating
    MSP_DP_RELEASE = 1,         // Release the display after clearing and updating
    MSP_DP_CLEAR_SCREEN = 2,    // Clear the display
    MSP_DP_WRITE_STRING = 3,    // Write a string at given coordinates
    MSP_DP_DRAW_SCREEN = 4,     // Trigger a screen draw
    MSP_DP_OPTIONS = 5,         // Not used by Betaflight. Reserved by Ardupilot and INAV
    MSP_DP_SYS = 6,             // Display system element displayportSystemElement_e at given coordinates
    MSP_DP_FONTCHAR_WRITE = 7,  // New OSD chip works over MSP, enables font write over MSP
    MSP_DP_COUNT,
} displayportMspCommand_e;


#define MSP_BUFFER_SIZE 256 // Maximum MSP message size

// --- Type Definitions ---
typedef enum {
    MSP_IDLE,
    MSP_HEADER_M_X,     // 'M' or 'X'
    MSP_HEADER_ARROW, // '<' or '>'
    MSP_HEADER_FLAG,    // v2 only
    MSP_V1_SIZE,
    MSP_V1_COMMAND,
    MSP_V2_SIZE_L,
    MSP_V2_SIZE_H,
    MSP_V2_COMMAND_L,
    MSP_V2_COMMAND_H,
    MSP_PAYLOAD,
    MSP_CHECKSUM
} msp_state_t;


typedef struct {
    uint8_t buffer[MSP_BUFFER_SIZE];
    uint16_t data_size;
    uint16_t command;
    uint8_t direction; // '<' or '>'
    msp_state_t state;
    msp_protocol_version_t version;
    uint8_t payload_offset; // For MSPv2, payload starts after flag and function
} msp_message_t;

// Example VTX Config Structure (adapt to your VTX hardware)
typedef struct {
    uint8_t band;
    uint8_t channel;
    uint8_t power;
    uint8_t pitmode;
    // Add other relevant VTX settings
} vtx_config_t;

// --- Global Variables ---
msp_message_t msp_rx_message;
vtx_config_t current_vtx_config;
// Placeholder for OSD buffer
uint8_t osd_buffer[1024]; // Example size, adjust based on screen resolution / char count

// --- Function Prototypes ---

// Hardware Abstraction Layer (HAL) - TO BE IMPLEMENTED BY YOU
// Function to receive a character from FC (e.g., UART RX)
// This function should be non-blocking or use interrupts for efficiency
// Returns -1 if no char available, otherwise the char.
extern int fc_receive_char(void);

// Function to send a character to FC (e.g., UART TX)
extern void fc_send_char(char c);

// Function to send multiple characters to FC
extern void fc_send_buffer(const uint8_t *buffer, uint8_t length);

// OSD manipulation functions - TO BE IMPLEMENTED BY YOU
extern void osd_clear_screen(void);
extern void osd_write_string(uint8_t x, uint8_t y, uint8_t attributes, const char *str, uint8_t len);
extern void osd_draw_screen(void); // Renders the OSD buffer to the display

// VTX control functions - TO BE IMPLEMENTED BY YOU
extern void vtx_apply_config(const vtx_config_t *config);
extern void vtx_get_config(vtx_config_t *config);
extern uint8_t vtx_get_band_info(uint8_t index, uint8_t* band_name, uint8_t* frequencies); // Example
extern bool vtx_set_band_info(uint8_t index, const uint8_t* band_name, const uint8_t* frequencies); // Example
extern uint8_t vtx_get_power_level_info(uint8_t index, uint16_t* power_value); // Example
extern bool vtx_set_power_level_info(uint8_t index, uint16_t power_value); // Example

// EEPROM functions - TO BE IMPLEMENTED BY YOU
extern void eeprom_save_settings(void);
extern void system_reboot(void);


// MSP Processing Functions
void msp_init(void);
//void msp_process_input(void);
void msp_process_message(void);
void msp_send_reply(uint16_t command, const uint8_t *payload, uint8_t payload_size, msp_protocol_version_t version);
void msp_send_error(uint16_t command, msp_protocol_version_t version); // Simplified error, V2 has more specific error codes
uint8_t msp_calculate_checksum_v1(const uint8_t *data, uint8_t length);
uint8_t msp_calculate_checksum_v2(const uint8_t *data, uint8_t length);


// --- Function Implementations ---

/**
 * @brief Initializes the MSP processing system.
 */
void msp_init(void) 
{
    msp_rx_message.state = MSP_IDLE;
    msp_rx_message.data_size = 0;
    // Initialize VTX config (e.g., load from EEPROM or defaults)
    // vtx_get_config(&current_vtx_config); // Example
}

/**
 * @brief Calculates MSP V1 checksum (XOR sum of size, command, and payload).
 * @param data Pointer to the start of data (size byte).
 * @param length Length of the data including size, command, and payload.
 * @return Calculated checksum.
 */
uint8_t msp_calculate_checksum_v1(const uint8_t *data, uint8_t length) 
{
    uint8_t checksum = 0;
    for (uint8_t i = 0; i < length; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

/**
 * @brief Calculates MSP V2 checksum (CRC8_DVB_S2).
 * @param data Pointer to the start of data (flag byte for outgoing, first byte after cmd for incoming).
 * @param length Length of the data to checksum.
 * @return Calculated CRC8 checksum.
 *
 * @note Betaflight uses CRC8_DVB_S2 with polynomial 0xD5.
 * The checksum includes the header ($, X/V, flags, command, size) and payload.
 * This implementation needs a proper CRC8 function.
 */
uint8_t msp_calculate_checksum_v2(const uint8_t *data, uint8_t length) 
{
    uint8_t crc = 0;
    for (uint8_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0xD5;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}


/**
 * @brief Parses an incoming character from the FC.
 * @param c The character received.
 */
bool msp_parse_char(uint8_t c) 
{
    static uint8_t bytes_received = 0;
    bool ret = false;

    //DEBUG_PRINTF("state:%d rx:%x", (uint8_t)msp_rx_message.state, c);
    switch (msp_rx_message.state) {
        case MSP_IDLE:
            if (c == MSP_HEADER) { // Could be V1 or V2 start
                msp_rx_message.state = MSP_HEADER_M_X;
                bytes_received = 0;
                msp_rx_message.buffer[bytes_received++] = c; // Store '$'
            }
            break;

        case MSP_HEADER_M_X:
            if (c == MSP_PROTOCOL_VERSION_V1) {
                msp_rx_message.version = MSP_V1;
                msp_rx_message.payload_offset = 5;
                msp_rx_message.state = MSP_HEADER_ARROW;
                msp_rx_message.buffer[bytes_received++] = c;
            } else if (c == MSP_PROTOCOL_VERSION_V2) {
                msp_rx_message.version = MSP_V2;
                msp_rx_message.payload_offset = 8; // head + flag +com +size
                msp_rx_message.state = MSP_HEADER_ARROW;
                msp_rx_message.buffer[bytes_received++] = c;
            }else{
                msp_rx_message.state = MSP_IDLE;
            }
            break;

        case MSP_HEADER_ARROW:
            if (c == MSP_DIRECTION_IN || c == MSP_DIRECTION_OUT ) {
                msp_rx_message.direction = c;
                msp_rx_message.buffer[bytes_received++] = c; // Store '<' or '>'
                if (msp_rx_message.version == MSP_V1){
                    msp_rx_message.state = MSP_V1_SIZE;
                }else{
                    msp_rx_message.state = MSP_HEADER_FLAG;
                }
            } else {
                msp_rx_message.state = MSP_IDLE;
                DEBUG_PRINTF("ARROE->IDLE");
            }
            break;

        case MSP_V1_SIZE:
            msp_rx_message.data_size = c;
            msp_rx_message.buffer[bytes_received++] = c; // Store size
            msp_rx_message.state = MSP_V1_COMMAND;
            break;

        case MSP_V1_COMMAND:
            msp_rx_message.command = c;
            msp_rx_message.buffer[bytes_received++] = c;
            if (msp_rx_message.data_size == 0) { // No payload
                msp_rx_message.state = MSP_CHECKSUM;
            } else {
                msp_rx_message.state = MSP_PAYLOAD;
            }
            break;

        case MSP_HEADER_FLAG:
            msp_rx_message.buffer[bytes_received++] = c;
            msp_rx_message.state = MSP_V2_COMMAND_L;
            break;

        case MSP_V2_COMMAND_L:
            msp_rx_message.command = c;
            msp_rx_message.buffer[bytes_received++] = c;
            msp_rx_message.state = MSP_V2_COMMAND_H;
            break;

        case MSP_V2_COMMAND_H:
            msp_rx_message.command |= c<<8;
            msp_rx_message.buffer[bytes_received++] = c;
            msp_rx_message.state = MSP_V2_SIZE_L;
            break;

        case MSP_V2_SIZE_L:
            msp_rx_message.data_size = c;
            msp_rx_message.buffer[bytes_received++] = c; // Store size
            msp_rx_message.state = MSP_V2_SIZE_H;
            break;

        case MSP_V2_SIZE_H:
            msp_rx_message.data_size |= c<<8;
            msp_rx_message.buffer[bytes_received++] = c; // Store size
            if (msp_rx_message.data_size == 0) { // No payload
                msp_rx_message.state = MSP_CHECKSUM;
            } else {
                msp_rx_message.state = MSP_PAYLOAD;
            }
            break;

        case MSP_PAYLOAD:
            msp_rx_message.buffer[bytes_received++] = c;
            if (bytes_received >= msp_rx_message.data_size + msp_rx_message.payload_offset) {
                msp_rx_message.state = MSP_CHECKSUM;
            }
            break;

        case MSP_CHECKSUM: 
            {
                uint8_t calculated_checksum;
                msp_rx_message.buffer[bytes_received++] = c;
                if (msp_rx_message.version == MSP_V1) {
                    calculated_checksum = msp_calculate_checksum_v1(&msp_rx_message.buffer[3], msp_rx_message.data_size + 2);
                } else {
                    calculated_checksum = msp_calculate_checksum_v2(&msp_rx_message.buffer[3], msp_rx_message.data_size + 5);
#if 0
                    DEBUG_PRINTTIMESTAMP();
                    DEBUG_PRINTRAW("rx_v2(%04x):", msp_rx_message.command);
                    for (int x=0; x<msp_rx_message.data_size + msp_rx_message.payload_offset; x++){
                        DEBUG_PRINTRAW(" %02x", msp_rx_message.buffer[x]);
                    }
                    DEBUG_PRINTRAW("\n");
#endif
                }

                if (calculated_checksum == c) {

                    msp_process_message();
                    ret = true; // Message processed successfully
                } else {
                    // Checksum error
                    DEBUG_PRINTF("Checksum error");

#if 0
        DEBUG_PRINTTIMESTAMP();
        DEBUG_PRINTRAW("rx_err(%04x):", msp_rx_message.command);
        for (int x=0; x<msp_rx_message.data_size + msp_rx_message.payload_offset; x++){
            DEBUG_PRINTRAW(" %02x", msp_rx_message.buffer[x]);
        }
        DEBUG_PRINTRAW("\n");
#endif


                }
                msp_rx_message.state = MSP_IDLE;
            }
            break;

        default:
            msp_rx_message.state = MSP_IDLE;
            break;
    }
    return ret;
}


#if 0
/**
 * @brief Main loop to process incoming characters from FC.
 */
void msp_process_input(void) 
{
    int char_in = fc_receive_char();
    if (char_in != -1) {
        msp_parse_char((uint8_t)char_in);
    }
}
#endif


/**
 * @brief Processes a fully received and validated MSP message.
 */
void msp_process_message(void)
{
    static bool msp_set_osd_canvas_recv = false;
//    uint8_t *payload = msp_rx_message.buffer + 5 + msp_rx_message.payload_offset; // Adjust offset for V2 if needed

    // Note: DisplayPort commands are often subcommands within a generic MSP_DISPLAYPORT message.
    // If MSP_DP_... are direct MSP commands, this structure is okay.
    // If they are subcommands, you'd first check for MSP_DISPLAYPORT, then parse its payload for the DP command.
    // This example assumes they are direct top-level commands as per the user's list.

#if 0
        DEBUG_PRINTTIMESTAMP();
        DEBUG_PRINTRAW("rx(%04x):", msp_rx_message.command);
        for (int x=0; x<msp_rx_message.data_size + msp_rx_message.payload_offset; x++){
            DEBUG_PRINTRAW(" %02x", msp_rx_message.buffer[x]);
        }
        DEBUG_PRINTRAW("\n");
#else
        //DEBUG_PRINTF("rx(%04x-%d):", msp_rx_message.command, (uint8_t)msp_rx_message.version);
#endif

    switch (msp_rx_message.command) {
        case MSP_DISPLAYPORT:
            switch(msp_rx_message.buffer[msp_rx_message.payload_offset]){
                case MSP_DP_HEARTBEAT:
                    //DEBUG_PRINTF("MSP_HEARTBEAT");
                    {
                        if ( msp_set_osd_canvas_recv == false /*&& video_format != VIDEO_UNKNOWN*/ ){
                            DEBUG_PRINTF("MSP_HEARTBEAT");
                            uint8_t data[2] = {COLUMN_SIZE,0};
                            data[1] = (setting()->videoFormat == VIDEO_PAL) ? ROW_SIZE_PAL : ROW_SIZE_NTSC;
                            msp_send_reply(MSP_SET_OSD_CANVAS, data, sizeof(data), msp_rx_message.version);
                        }
                    }
                    break;

                case MSP_DP_CLEAR_SCREEN:
                    //DEBUG_PRINTF("MSP_CLEAR_SCREEN");
                    charCanvasClear();
                    break;

                case MSP_DP_WRITE_STRING:
                    //DEBUG_PRINTF("MSP_WRITE_STRING:%c", msp_rx_message.buffer[ msp_rx_message.payload_offset +4 ]);
                    charCanvasWrite(
                        msp_rx_message.buffer[ msp_rx_message.payload_offset +1],
                        msp_rx_message.buffer[ msp_rx_message.payload_offset +2 ],
                        msp_rx_message.buffer[ msp_rx_message.payload_offset +3 ],
                        &msp_rx_message.buffer[ msp_rx_message.payload_offset +4 ],
                        msp_rx_message.data_size -4);
                    break;

                case MSP_DP_DRAW_SCREEN:
                    //DEBUG_PRINTF("MSP_DRAW_SCREEN");
                    charCanvasDraw();
                    break;
                case MSP_DP_FONTCHAR_WRITE:
                    //DEBUG_PRINTF("MSP_FONTCHAR_WRITE");
                    writeFlashFont(msp_rx_message.buffer[ msp_rx_message.payload_offset +2]<<8 | msp_rx_message.buffer[ msp_rx_message.payload_offset +1], &msp_rx_message.buffer[ msp_rx_message.payload_offset +4]);
                    break;
            }
            break;
        case MSP_SET_OSD_CANVAS:
            DEBUG_PRINTF("MSP_SET_OSD_CANVAS");
            msp_set_osd_canvas_recv = true;
            break;
#ifndef TARGET_NOVTX
        case MSP_VTX_CONFIG:
            DEBUG_PRINTF("MSP_VTX_CONFIG");
            mspvtx_VtxConfig( &msp_rx_message.buffer[msp_rx_message.payload_offset] );
            break;

        case MSP_SET_VTX_CONFIG:
        case MSP_SET_VTXTABLE_BAND:
        case MSP_SET_VTXTABLE_POWERLEVEL:
        case MSP_EEPROM_WRITE:
            mspvtx_Ack(msp_rx_message.command);
            break;

        case MSP_VTXTABLE_BAND: // Get VTX Band/Channel table entry
            mspvtx_VtxTableBand( &msp_rx_message.buffer[msp_rx_message.payload_offset] );
            break;

        case MSP_VTXTABLE_POWERLEVEL: // Get VTX Power Level table entry
            mspvtx_VtxTablePowerLevel( &msp_rx_message.buffer[msp_rx_message.payload_offset] );
            break;
#endif
        case MSP_REBOOT:
            rebootDfu();
            break;

        // Handle other MSP commands if necessary (e.g., MSP_API_VERSION for compatibility checks)
        case MSP_API_VERSION:
//            uint8_t api_reply[] = {1, 16}; // MSP Protocol Version (e.g., 1.16, V1: byte 0, V2: byte 1)
//                                           // Adjust this according to the MSP versions you fully support.
//                                           // For V2, it's often like { MSP_PROTOCOL_VERSION, API_VERSION_MAJOR, API_VERSION_MINOR }
//            msp_send_reply(MSP_API_VERSION, api_reply, sizeof(api_reply), msp_rx_message.version);
            break;


        default:
            // Unknown command, could send an error or ignore
            // msp_send_error(msp_rx_message.command);
            break;
    }
    // Reset message state for next message
    msp_rx_message.state = MSP_IDLE;
    msp_rx_message.data_size = 0;
}


/**
 * @brief Sends an MSP reply to the FC.
 * @param command The MSP command ID being replied to.
 * @param payload Pointer to the payload data.
 * @param payload_size Size of the payload data.
 */
void msp_send_reply(uint16_t command, const uint8_t *payload, uint8_t payload_size, msp_protocol_version_t version)
{
    static uint8_t tx_buffer[MSP_BUFFER_SIZE];
    uint8_t tx_index = 0;

    if (version == MSP_V1) { // Assuming reply should match received version
        tx_buffer[tx_index++] = MSP_HEADER;      // $
        tx_buffer[tx_index++] = MSP_PROTOCOL_VERSION_V1;                // M
        tx_buffer[tx_index++] = MSP_DIRECTION_OUT; // > (direction to FC)
        tx_buffer[tx_index++] = payload_size;
        tx_buffer[tx_index++] = (uint8_t)(command & 0xff);
        if (payload && payload_size > 0) {
            memcpy(&tx_buffer[tx_index], payload, payload_size);
            tx_index += payload_size;
        }
        tx_buffer[tx_index++] = msp_calculate_checksum_v1(&tx_buffer[3], payload_size + 2); // Checksum size, cmd, payload
    } else { // MSP_V2_UNFRAMED (Simplified for now)
        // Proper MSPv2 framing: $ X flags funcL funcH sizeL sizeH ... payload ... crc
        // This simplified version will likely NOT work with a real Betaflight MSPv2 master
        // without implementing the full V2 header structure (flags, 2-byte cmd, 2-byte size).
        // For a V2 reply, command is typically split into two bytes.
        // The `command` parameter here is a single byte.
        // This needs significant rework for proper V2.

        tx_buffer[tx_index++] = MSP_HEADER;         // $
        tx_buffer[tx_index++] = MSP_PROTOCOL_VERSION_V2; // 'X' or 'V'
        tx_buffer[tx_index++] = MSP_DIRECTION_OUT;
        tx_buffer[tx_index++] = 0;                      // flag
        tx_buffer[tx_index++] = (uint8_t)(command & 0xff);               // Command LSB
        tx_buffer[tx_index++] = (uint8_t)((command >> 8) &0xff);                     // Command MSB
        tx_buffer[tx_index++] = payload_size & 0xFF;   // Payload Size LSB
        tx_buffer[tx_index++] = (payload_size >> 8) & 0xFF; // Payload Size MSB

        if (payload && payload_size > 0) {
            memcpy(&tx_buffer[tx_index], payload, payload_size);
            tx_index += payload_size;
        }
        // Checksum for V2 covers: flags, funcL, funcH, sizeL, sizeH, payload
        // The current msp_calculate_checksum_v2 is a V1 placeholder and INCORRECT for V2.
        // You MUST use a CRC8_DVB_S2 checksum for the range specified in Betaflight's MSPv2.
        // The range usually starts from the 'flags' byte.
        tx_buffer[tx_index++] = msp_calculate_checksum_v2(&tx_buffer[3], payload_size + 5); // Checksum from flags byte
    }
#if 1
    DEBUG_PRINTTIMESTAMP();
    DEBUG_PRINTRAW("tx:");
    for (int x=0; x<tx_index; x++){
        DEBUG_PRINTRAW(" %02x", tx_buffer[x]);
    }
    DEBUG_PRINTRAW("\n");
#endif
    uart_send(tx_buffer, tx_index);
}

