/*
 * ir_mrt.c
 *
 *  Created on: Apr. 4, 2020
 *      Author: masta
 */
#include "LPC804.h"
#include "ir.h"

//This flag stores the current state
volatile int raw_buf[68];
volatile int ir_state = 0;
int ir_btn = 0;

int decode_err = 0;
int us_buf[68];
//stores how many times the previous state has been repeated
volatile int prev_state_ticks = 1;
volatile int curr_pin_state = 0;
volatile int bits_read = 0;
volatile int key_repeat = 0;
volatile int button_pressed;
volatile int prev_pin_state = 0;


int init_ir(void) {
	__disable_irq(); //Turn IRQs off
	NVIC_DisableIRQ(PIN_INT1_IRQn);
	NVIC_DisableIRQ(WKT_IRQn);

	//Start GPIO
	SYSCON->SYSAHBCLKCTRL0 |= ( SYSCON_SYSAHBCLKCTRL0_GPIO0_MASK | // GPIO is on
			SYSCON_SYSAHBCLKCTRL0_GPIO_INT_MASK); // GPIO Interrupt is on

	//Set IRPIN as input
	GPIO->DIRCLR[0] = (1UL << IR_PIN);
	SYSCON->PINTSEL[1] = IR_PIN;
	PINT->ISEL = (1 << PINT_ISEL_PMODE(0)); //Edge sensitive triggering
	PINT->SIENF = 0b00000010; //Falling edge on channel 1
	PINT->CIENR = 0b00000010; //Clear  rising edge on channel 1
	PINT->IENF = (1 << PINT_IENF_ENAF(1)); //Falling edge
	PINT->IST = 0xFFFF; //Remove pending interrupts

	//MRT Config
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_MRT_MASK);

	//Channel 1 for IR in repeat mode.
	MRT0->CHANNEL[MRT_IR].CTRL = (0 << MRT_CHANNEL_CTRL_MODE_SHIFT // Bit 1 is 0
	| MRT_CHANNEL_CTRL_INTEN_MASK);

	NVIC_EnableIRQ(WKT_IRQn);
	NVIC_EnableIRQ(PIN_INT1_IRQn);
	NVIC_EnableIRQ(MRT0_IRQn);
	__enable_irq();
	return 0;
}

void PIN_INT1_IRQHandler() {
	//printf("We handling bois\n");
	//Enable MRT to check state every 50us
	MRT0->CHANNEL[MRT_IR].INTVAL = (uint32_t) (MRT_POLL_RELOAD)
			& ~(MRT_CHANNEL_INTVAL_LOAD_MASK);
	//Falling edge, so this means that pin state must be low when this interrupt is polled
	//But just in case, for debugging reasons, get actual state
	//Disable GPIO interrupt on this pin - re-enabled again in MRT handler after processing
	ir_state = IR_STATE_START_PULSE; //Initialize start pulse
	bits_read = 0;
	key_repeat = 0;
	//is complete
	PINT->IST = (1 << 1); //Clear interrupt

	NVIC_DisableIRQ(PIN_INT1_IRQn);
}

void calc_us_buf() {
	for (int i = 0; i < 68; i++) {
		us_buf[i] = raw_buf[i] * USEC_PER_TICK;
	}
}

void decode() {
	int prev_ticks = 0;
	int curr_ticks = 0;
	int decoded_output = 0;

	if (key_repeat == 1) {
		ir_btn = decoded_output;
		ir_state = IR_STATE_BTN_PRESSED;
		return;
	}

	ir_state = IR_STATE_DECODING;
	for (int i = 3; i < 67; i=i+2) {
		prev_ticks = raw_buf[i - 1];
		curr_ticks = raw_buf[i];
		if((prev_ticks >= (BIT_PULSE_TICKS_LOW)) && (prev_ticks <= (BIT_PULSE_TICKS_HIGH))) {
			//If previous is a bit pulse, check whether or not the next space is a logic low
			//or logic high by checking it's space
			if (((-1)*curr_ticks >= (LOG_LOW_TICKS_LOW)) && ((-1)*curr_ticks <= (LOG_LOW_TICKS_HIGH))) {
				//decoded_buf[decoded_buf_idx] = 0;
				decoded_output = (decoded_output << 1) | 0;
			} else if (((-1)*curr_ticks >= (LOG_HIGH_TICKS_LOW)) && ((-1)*curr_ticks <= (LOG_HIGH_TICKS_HIGH))) {
				//decoded_buf[decoded_buf_idx] = 1;
				decoded_output = (decoded_output << 1) | 1;
			} else {
				decode_err = 1;
			}
			//decoded_buf_idx = decoded_buf_idx + 2;
			ir_btn = decoded_output;
		}
	}
	ir_state = IR_STATE_BTN_PRESSED;
}
void print_us_buf() {
	for (int i = 0; i < 68; i++) {
		printf(" %d ", us_buf[i]);
	}
}

int get_ir_state() {
	return ir_state;
}
int get_ir_btn() {
	if (ir_state == IR_STATE_FINISH_RECV) {
		decode();
	}
	return ir_btn;
}
void reset_ir_state() {
	MRT0->CHANNEL[MRT_IR].INTVAL = 0;
	NVIC_EnableIRQ(PIN_INT1_IRQn);
	prev_pin_state = 0;
	curr_pin_state = 0;
	prev_state_ticks = 0;
	bits_read = 0;
	ir_btn = 0;
	ir_state = IR_STATE_IDLE;
}
void clear_err() {
	ir_state = IR_STATE_IDLE;
}
