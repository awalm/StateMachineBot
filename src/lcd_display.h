/*
 * lcd_display.h
 *
 *  Created on: Apr. 1, 2020
 *      Author: masta
 */

#ifndef LCD_DISPLAY_H_
#define LCD_DISPLAY_H_

#define LCD_I2C_ADDR (0x3E)

#define MSTCTL_CONTINUE (1UL << 0) // Bit 0 of MSTCTL set ("Main" or "Primary")
#define MSTCTL_START (1UL << 1) // Bit 1 of MSTCTL set ("Main" or "Primary")
#define MSTCTL_STOP (1UL << 2) // Bit 2 of MSTTCL set ("Main" or "Primary")
#define CTL_SLVCONTINUE (1UL << 0) // Bit 0: Secondary level (SLV) Continue
#define CTL_SLVNACK (1UL << 1) // Bit 1: Secondary Level (SLV) Acknowledge
#define PRIMARY_STATE_MASK (0x7<<1) // bits 3:1 of STAT register for Main / Primary
#define I2C_STAT_MSTST_IDLE ((0b000)<<1) // Main Idle: LPC802 user manual table 187
#define I2C_STAT_MSTST_RXRDY ((0b001)<<1) // Main Receive Ready " "
#define I2C_STAT_MSTST_TXRDY ((0b010)<<1) // Main Transmit Ready " "
#define I2C_STAT_MSTST_NACK_ADD ((0b011)<<1)// Main Ack Add " "
#define I2C_STAT_MSTST_NACK_DATA ((0b100)<<1)// Main Ack signal data ” ”

//void WaitI2CPrimaryState(I2C_Type*, uint32_t);
//void I2C_PrimarySetBaudRate(uint32_t, uint32_t);
int init_i2c_lcd(void);
int i2c_lcd_begin_trans(void);
int i2c_lcd_send_data(unsigned char *pData, int len);
int i2c_lcd_send_byte(unsigned char data);
int i2c_lcd_end_trans(void);
int delayMicroseconds(int x);
void I2C_PrimarySetBaudRate(uint32_t baudRate_Bps, uint32_t srcClock_Hz);
void WaitI2CPrimaryState(I2C_Type * ptr_LPC_I2C, uint32_t state);
#endif /* LCD_DISPLAY_H_ */
