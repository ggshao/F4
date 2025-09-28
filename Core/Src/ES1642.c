#include "ES1642.h"

uint8_t charge_addr[6] = {0xC1, 0x7A, 0x1F, 0x00, 0x00, 0xFC};
uint8_t dog_addr[6] = {0x50, 0x79, 0x1F, 0x00, 0x00, 0xFC};
uint8_t broadcast_addr[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t heartTxData[1] = {0xDD};
uint8_t heartRxData[1] = {0xEE};
// 计算和校验
// L、Ctrl、Cmd、Data 所有字节算术和，不考虑溢出
uint8_t calculate_checksum(const uint8_t *checkFrame, uint8_t length) {
    uint8_t sum = 0;
    uint8_t i;
    for ( i = 0; i < length; i++) {
        sum += checkFrame[i];
    }
    return sum;
}

// 计算异或校验
uint8_t calculate_xor(const uint8_t *checkFrame, uint8_t length) {
    uint8_t xor = 0;
    uint8_t i;
    for ( i = 0; i < length; i++) {
        xor ^= checkFrame[i];
    }
    return xor;
}

/**
 * @brief 构建发送数据帧
 * 
 * @param dst_addr 目标地址(6字节)
 * @param user_data 用户数据
 * @param user_data_len 用户数据长度
 * @param relay_depth 中继深度(0-15)
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return uint16_t 实际构建的帧长度，0表示失败
 */
uint16_t build_data_frame(
    const uint8_t *dst_addr, 
    const uint8_t *user_data, 
    uint16_t user_data_len,
    uint8_t relay_depth,
    uint8_t *buffer, 
    uint16_t buffer_size) 
{
    // 计算需要的总空间
    uint16_t data_field_len = 2 + 6 + 2 + user_data_len; // Data Ctrl + Dst Addr + User Data Len + User Data
    uint16_t total_frame_len = 1 + 1 + 1 + 1 + data_field_len + 1 + 1; // Header + L + Ctrl + Cmd + Data + CSUM + CXOR
    uint16_t data_ctrl;
    uint8_t *ptr = buffer;
    uint16_t check_len;
    uint8_t csum;
    uint8_t cxor;
    // 检查缓冲区是否足够
    if (buffer_size < total_frame_len) {
        return 0;
    }
    
    
    // 1. 帧头
    *ptr++ = FRAME_HEADER;  // 79H
    
    // 2. L字段 (2字节小端)
    *ptr++ = (uint8_t)(data_field_len & 0xFF);    // 低字节
    *ptr++ = (uint8_t)(data_field_len >> 8);      // 高字节
    
    // 3. 控制字段
    *ptr++ = 0x58;  // Ctrl (Prm=1表示主动发起)
    
    // 4. 指令字
    *ptr++ = CMD_SEND_DATA;  // 14H
    
    // 5. 数据域
    // 5.1 Data Ctrl (2字节)
    data_ctrl = (relay_depth << 12); // D12-D15为中继深度
    *ptr++ = 0x44;                            // 低字节(RSV)
    *ptr++ = (uint8_t)(data_ctrl >> 8);       // 高字节
    
    // 5.2 Dst Addr (6字节)
    memcpy(ptr, dst_addr, 6);
    ptr += 6;
    
    // 5.3 User Data Len (2字节小端)
    *ptr++ = (uint8_t)(user_data_len & 0xFF);  // 低字节
    *ptr++ = (uint8_t)(user_data_len >> 8);    // 高字节
    
    // 5.4 User Data
    memcpy(ptr, user_data, user_data_len);
    ptr += user_data_len;
    
    // 计算校验
    // 校验范围: L(2字节) + Ctrl(1字节) + Cmd(1字节) + Data(n字节)
    check_len = ptr - (buffer + 1);
    csum = calculate_checksum(buffer + 1, check_len);
    cxor = calculate_xor(buffer + 1, check_len);
    
    *ptr++ = csum;  // CSUM
    *ptr++ = cxor;  // CXOR
    
    return ptr - buffer; // 返回实际帧长度
}

/**
 * @brief 解析接收数据帧
 * 0: 帧头79H 
   1: L (低字节)
   2: L (高字节)
   3: Ctrl
   4: Cmd 
   5-7: Data Ctrl (3字节)
   8-13: Src Addr (6字节)
   14-15: User Data Len (2字节)
   16-...: User Data
   ...: CSUM
   ...: CXOR
 * 
 * @param frame 接收到的帧数据
 * @param src_addr 输出参数: 源地址(6字节)
 * @param user_data 输出参数: 用户数据缓冲区
 * @param user_data_buffer_size 用户数据缓冲区大小
 * @param user_data_len 输出参数: 实际用户数据长度
 * @return int 0-成功, 其他-错误码
 */
int parse_received_data_frame(
    const uint8_t *frame, 
    uint8_t *src_addr,
    uint8_t *user_data,
    uint16_t user_data_buffer_size,
    uint16_t *user_data_len
) 
{
    uint16_t data_field_len;
    uint8_t csum;
    uint8_t cxor;
    uint8_t *datafield;
    uint16_t received_user_data_len;
    // 检查帧头
    if (frame[0] != FRAME_HEADER) return 1; // 无效帧头
    
    // 获取数据域Data长度
    data_field_len = frame[1] | (frame[2] << 8);
    
    // 计算校验
    csum = calculate_checksum(frame + 1, 4 + data_field_len ); // L + Ctrl + Cmd + Data
    cxor = calculate_xor(frame + 1, 4 + data_field_len );
    
    // 验证校验
    if (frame[5 + data_field_len] != csum || frame[5 + data_field_len + 1] != cxor) {
        return 2; // 校验失败
    }
    
    
    // 解析数据域
    datafield = frame + 5;
    // 解析Data Ctrl (3字节)(模块通知设备时为3字节)
    // 解析源地址    (6字节)
    memcpy(src_addr, datafield + 2, 6);
    
    // 解析用户数据长度 (2字节)
    received_user_data_len = datafield[8] | (datafield[9] << 8);
    
    // 检查用户数据长度是否合理
    if (2 + 6 + 2 + received_user_data_len != data_field_len) return 3; // 用户数据长度错误

    // 检查用户缓冲区是否足够
    if (received_user_data_len > user_data_buffer_size) {
        *user_data_len = 0;
        return 4; // 用户缓冲区不足
    }
    
    // 获取用户数据
    *user_data_len = received_user_data_len;
    if (received_user_data_len > 0) {
        memcpy(user_data, datafield + 2 + 6 +2, received_user_data_len);
    }
    
    return 0; // 成功
}
