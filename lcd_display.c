/*
 * lcd_display.c
 *
 *  Created on: Apr. 1, 2020
 *      Author: Mastafa Awal. Source code for I2C functions is
 *      James Smith's I2C slides.
 */
#include "LPC804.h"
#include "lcd_display.h"

int init_i2c_lcd(void) {
	// ------------------------------------------------------------
	// Step 1: Connect I2C module to outer pins via Switch Matrix
	// PIO0_16 is connected to the SCL line
	// PIO0_10 is connected to the SDA line
	// PINASSIGN5 bits 15:8 are for SCL. Therefore fill with value 16
	// PINASSIGN5 bits 7:0 are for SDA. Therefore fill with value 10
	//
	// enable switch matrix.
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_SWM_MASK);
	// Set switch matrix
	// PINASSIGN5: clear bits 15:0 to permit the two i2c lines to be assigned.
	SWM0->PINASSIGN.PINASSIGN5 &= ~(SWM_PINASSIGN5_I2C0_SCL_IO_MASK |
	SWM_PINASSIGN5_I2C0_SDA_IO_MASK); // clear 15:0
	SWM0->PINASSIGN.PINASSIGN5 |= ((14<<SWM_PINASSIGN5_I2C0_SCL_IO_SHIFT)|
			(7<<SWM_PINASSIGN5_I2C0_SDA_IO_SHIFT));
	// disable the switch matrix

	SYSCON->SYSAHBCLKCTRL0 &= ~(SYSCON_SYSAHBCLKCTRL0_SWM_MASK);
	// ---------------- End of Switch Matrix code -----------------------------------
	// ------------------------------------------------------------
	// Step 2: Turn on the I2C module via the SYSCON clock enable pin
	// ------------------------------------------------------------
	// debug: just i2c
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_I2C0_MASK );// I2C is on
	// debug: in the demo code they don't seem to reset the i2c clock.
	// debug: just i2c
	// Put 0 in the GPIO, GPIO Interrupt and I2C reset bit to reset it.
	// Then put a 1 in the GPIO, GPIO Interrupt and I2C reset bit to allow them to operate.
	// manual: Section 6.6.10
	SYSCON->PRESETCTRL0 &= ~(SYSCON_PRESETCTRL0_I2C0_RST_N_MASK );// reset I2C(bit = 0)
	SYSCON->PRESETCTRL0 |= (SYSCON_PRESETCTRL0_I2C0_RST_N_MASK);// remove i2C reset (bit = 1)
	// ------------------------------------------------------------
	// Step 3: Choose the source of the I2C timing clock
	// see: Fig 7 of User Manual
	// ------------------------------------------------------------
	// Provide main_clk as function clock to I2C0
	// bits 2:0 are 0x1.
	// FRO is 0b000; main clock is 0b001, frg0clk is 0b010, fro_div is 0b100.
	// Set i2c to FRO (12 MHz)
	// (only bits 2:0 are used; other 29 bits are ignored)
	SYSCON->I2C0CLKSEL = 0b000;// put 000 in bits 2:1.
	// confirmed in reg view that this is FRO.
	// ------------------------------------------------------------
	// Step 4: enable primary (but not slave) functionality in the i2c module.
	//
	// ------------------------------------------------------------
	// Configure the I2C0 CFG register:
	// Primary enable = true
	// Secondary enable = true
	// Monitor enable = false
	// Time-out enable = false
	// Monitor function clock stretching = false
	//
	// Only config I2C0 as a primary
	I2C0->CFG = (I2C_CFG_MSTEN_MASK); // only as primary
	//Enable monitor
	I2C0->CFG |= (I2C_CFG_MONEN_MASK);
	// Comment out for now: therefore, no interruptions.
	// Enable the I2C0 "secondary" pending interrupt
	// I2C0->INTENSET = I2C_INTENSET_SLVPENDINGEN_MASK; // STAT_SLVPEND;
	// ------------------------------------------------------------
	// Step 5: set the i2c clock rate. (20Hz to 400kHz is typical)
	// I2C_PrimarySetBaudRate() function is helpful.
	// ------------------------------------------------------------
	// Set i2C bps to 100kHz assuming an input clock of 12MHz
	I2C_PrimarySetBaudRate(100000, 15000000);
	// after this, CLKDIV is 0x9 and MSTTIME is 0x44.
	// MSTTIME = 0x44 means: MSTSCLLOW [2:0] is CLOCKS_6
	// MSTCLHIGH [6:4] is CLOCKS_6
	return 0;
}

int i2c_lcd_begin_trans(void) {
	WaitI2CPrimaryState(I2C0, I2C_STAT_MSTST_IDLE); // Wait for the Primary's state to be idle
	I2C0->MSTDAT = (LCD_I2C_ADDR << 1)| 0; // Address with 0 for RWn bit (WRITE)
	I2C0->MSTCTL = MSTCTL_START; // Start the transaction by setting the
	// MSTSTART bit to 1 in the Primary control register
	return 0;
}

int i2c_lcd_send_data(unsigned char *pData, int len) {
	for (int i = 0; i < len; ++i) {
		WaitI2CPrimaryState(I2C0, I2C_STAT_MSTST_TXRDY);
		I2C0->MSTDAT = pData[i];
		I2C0->MSTCTL = MSTCTL_CONTINUE;
	}
	return 0;
}

int i2c_lcd_send_byte(unsigned char data) {
	WaitI2CPrimaryState(I2C0, I2C_STAT_MSTST_TXRDY);
	I2C0->MSTDAT = data;
	I2C0->MSTCTL = MSTCTL_CONTINUE;
	return 0;
}
int i2c_lcd_end_trans(void) {
	WaitI2CPrimaryState(I2C0, I2C_STAT_MSTST_TXRDY);
	I2C0->MSTCTL = MSTCTL_STOP;
	return 0;
}

void I2C_PrimarySetBaudRate(uint32_t baudRate_Bps, uint32_t srcClock_Hz)
{
	uint32_t scl, divider;
	uint32_t best_scl, best_div;
	uint32_t err, best_err;
	best_err = 0;
	for (scl = 9; scl >= 2; scl--){
		/* calculated ideal divider value for given scl */
		divider = srcClock_Hz / (baudRate_Bps * scl * 2u);
		/* adjust it if it is out of range */
		divider = (divider > 0x10000u) ? 0x10000 : divider;
		/* calculate error */
		err = srcClock_Hz - (baudRate_Bps * scl * 2u * divider);
		if ((err < best_err) || (best_err == 0)){
			best_div = divider;
			best_scl = scl;
			best_err = err;
		}
		if ((err == 0) || (divider >= 0x10000u)){
			/* either exact value was found
			or divider is at its max (it would even greater in the next iteration for sure) */
			break;
		}
	}
	// Assign Clock Divider value, using included in LPC802.h
	I2C0->CLKDIV = I2C_CLKDIV_DIVVAL(best_div - 1);
	// Assign Primary timing configuration, using two macros include in LPC802.h
	I2C0->MSTTIME =
	I2C_MSTTIME_MSTSCLLOW(best_scl - 2u) | I2C_MSTTIME_MSTSCLHIGH(best_scl - 2u);
}


void WaitI2CPrimaryState(I2C_Type * ptr_LPC_I2C, uint32_t state) {
	// Check the Primary Pending bit (bit 0 of the i2c stat register)
	// Wait for MSTPENDING bit set in STAT register
	int counter;

	while(!(ptr_LPC_I2C->STAT & I2C_STAT_MSTPENDING_MASK)); // Wait

	if((ptr_LPC_I2C->STAT & PRIMARY_STATE_MASK) != state) // If primary state mismatch
	{
		//GPIO->DIRCLR[0] = (1UL<<LED_USER2); // turn on LED on PIO0_9 (LED_USER2)
		printf("Primary state mismatch!\n");
		while(1); // die here and debug the problem
	}
	return; // If no mismatch, return
}
int delayMicroseconds(int x) {
	//Simple naive delay implementation =(
	for (int i = 0; i < x; i++);
	return 0;
}

