#ifndef __ES1642_H
#define __ES1642_H

#include <string.h>
// 如果还没有定义 uint8_t，就定义它们
#ifndef _STDINT_H     // stdint.h 可能定义了这个

#ifndef _GCC_STDINT_H  // GCC 版本可能定义这个

// 如果没有以上这些宏，就假定 stdint.h 未包含，需要自定义
typedef unsigned char      uint8_t;
typedef signed char        int8_t;
typedef unsigned short     uint16_t;
typedef signed short       int16_t;
typedef unsigned long      uint32_t;
typedef signed long        int32_t;

#endif
#endif


// 帧头定义
#define FRAME_HEADER 0x79
// 指令字定义
#define CMD_SEND_DATA 0x14
#define CMD_RECEIVE_DATA 0x15


// typedef u8 uint8_t;
// typedef u16 uint16_t;

extern uint8_t charge_addr[6];
extern uint8_t dog_addr[6];
extern uint8_t heartTxData[1];
extern uint8_t heartRxData[1];
extern uint8_t broadcast_addr[6];


/**
 * @example 
 * @code
    uint8_t src_addr[6];
    uint8_t user_data[4];
    uint16_t user_data_len;
    int result = parse_received_data_frame(
        RX1_BUffer,
        src_addr,
        user_data,
        sizeof(user_data),
        &user_data_len);
    
    if (result == 0) {
        // 解析成功，处理数据...
    }
*/
uint8_t calculate_checksum(const uint8_t *checkFrame, uint8_t length);

uint8_t calculate_xor(const uint8_t *checkFrame, uint8_t length);

uint16_t build_data_frame(const uint8_t *dst_addr, const uint8_t *user_data, uint16_t user_data_len, uint8_t relay_depth, uint8_t *buffer, uint16_t buffer_size);

int parse_received_data_frame(const uint8_t *frame, uint8_t *src_addr, uint8_t *user_data, uint16_t user_data_buffer_size, uint16_t *user_data_len);
#endif



