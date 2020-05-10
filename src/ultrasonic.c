#include "LPC804.h"
#include "ultrasonic.h"

volatile int us_state = 0;
volatile static int us_hist[GESTURE_HISTORY];
volatile static int us_read;
volatile int gest_mode = 0;
volatile int num_echos = MAX_ECHOS;
volatile int calc_flag;

void init_ultrasonic(void) {
	__disable_irq();
	NVIC_DisableIRQ(MRT0_IRQn);

	//GPIO config
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_GPIO0_MASK)
			| (SYSCON_SYSAHBCLKCTRL0_GPIO_INT_MASK);
	//Reset
//	SYSCON->PRESETCTRL0 &= ~(SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK)
//			| (SYSCON_PRESETCTRL0_GPIOINT_RST_N_MASK);
//	SYSCON->PRESETCTRL0 |= (SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK)
//			| (SYSCON_PRESETCTRL0_GPIOINT_RST_N_MASK);

	//Trigger pin is output, Echo pin is input
	GPIO->DIRSET[0] = (1UL << ULTRASONIC_TRIG);
	GPIO->CLR[0] = (1UL << ULTRASONIC_TRIG);
	GPIO->DIRCLR[0] = (1UL << ULTRASONIC_ECHO);

	//Setup rising edge interrupts on echo pin
	SYSCON->PINTSEL[0] = ULTRASONIC_ECHO;

	PINT->ISEL |= PINT_ISEL_PMODE(0); //Edge sensitive triggering
//	PINT->SIENR = 0b00000001; //Rising edge on channel 0
//	PINT->CIENF = 0b00000001; //Clear falling edge on channel 0
	PINT->IENR |= PINT_IENR_ENRL(1); //Rising edge

	PINT->IST = 0xFF; //Remove pending interrupts

//	//Enable WKT
//	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_WKT_MASK);
//
//	NVIC_DisableIRQ(WKT_IRQn);
//	//Enable low power oscillator
//	SYSCON->PDRUNCFG &= ~(SYSCON_PDRUNCFG_LPOSC_PD_MASK);
//
//	//Reset WKT
//	SYSCON->PRESETCTRL0 &= ~(SYSCON_PRESETCTRL0_WKT_RST_N_MASK);
//	SYSCON->PRESETCTRL0 |= (SYSCON_PRESETCTRL0_WKT_RST_N_MASK);
//	//Configure WKT with this low power oscillator
//	SYSCON->LPOSCCLKEN |= (1<<SYSCON_LPOSCCLKEN_WKT_SHIFT);
//	//Load WKT
//	//	//Load timer
//	WKT->CTRL = WKT_CTRL_CLKSEL_MASK;
//	WKT->COUNT = 0xFFFFFFFF; //Load max value

	//MRT Config
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_MRT_MASK);
//	SYSCON->PRESETCTRL0 &= ~(SYSCON_PRESETCTRL0_MRT_RST_N_MASK);
//	SYSCON->PRESETCTRL0 |= (SYSCON_PRESETCTRL0_MRT_RST_N_MASK);

	//Channel 0 in repeat mode
	MRT0->CHANNEL[MRT_ULTRASONIC].CTRL = (0 << MRT_CHANNEL_CTRL_MODE_SHIFT // Bit 1 is 0
	| MRT_CHANNEL_CTRL_INTEN_MASK);
	//Channel 3 in repeat mode
//	MRT0->CHANNEL[MRT_ULTRASONIC_GEST].CTRL =  (0 << MRT_CHANNEL_CTRL_MODE_SHIFT // Bit 1 is 0
//							| MRT_CHANNEL_CTRL_INTEN_MASK);
//	MRT0->CHANNEL[MRT_ULTRASONIC_GEST].INTVAL = (uint32_t) (ULTRASONIC_POLL_RELOAD) & ~(MRT_CHANNEL_INTVAL_LOAD_MASK);

	NVIC_EnableIRQ(MRT0_IRQn);
	NVIC_EnableIRQ(PIN_INT0_IRQn);
	NVIC_EnableIRQ(PIN_INT1_IRQn);
	NVIC_EnableIRQ(WKT_IRQn);
	__enable_irq();
}

int get_distance(void) {
	//Wait for calc flag lock to be released, if any
	trig_us();
	int timeout = 0;
//	while(calc_flag);
	while (calc_flag) {
		timeout++;
		if (timeout > ULTRASONIC_TIMEOUT) {
			//printf("Ultrasonic timeout!\n");
			return MAX_ECHOS;
		}
	}
	return (num_echos); //Each tick is 6us, which represents approx 1mm
}

void trig_us(void) {
	//clear previous echos
	num_echos = 0;
	//Send 10us pulse on trigger pin
	GPIO->SET[0] = (1UL << ULTRASONIC_TRIG);
	//replace with MRT later
	for (int i = 0; i < 8; i++)
		;
	GPIO->CLR[0] = (1UL << ULTRASONIC_TRIG);
	//Wait for 465 uS
	//for (int i = 0; i < 500; i++);
	calc_flag = 1;
}

void PIN_INT0_IRQHandler(void) {
	//tc = WKT->COUNT;
	//printf("PIN interrupt Handler\n");
	if (PINT->IST & (1 << 0)) {
		PINT->IST = (1 << 0);
		num_echos++;
		//If rising edge, then we trigger MRT to continue reading
		//printf("We triggered bois: Rising at time %u\n");
		MRT0->CHANNEL[MRT_ULTRASONIC].INTVAL =
				(uint32_t) (ULTRASONIC_POLL_RELOAD)
						& ~(MRT_CHANNEL_INTVAL_LOAD_MASK);
	}
}

void trig_gest() {
	if (gest_mode) {
		if (us_read > GESTURE_HISTORY) {
			//Disable MRT and set calculate
			us_state = US_STATE_CALC;
			calc_gest();
			us_read = 0;
		} else {
			us_hist[us_read] = get_distance();
			us_read++;
		}
	}
}

void calc_gest() {
	int ticks_start = 0;
	int ticks_stop = 0;
	int ticks_forcestop = 0;

	for (int i = 1; i < GESTURE_HISTORY; i++) {
		if (us_hist[i] < 15) {
			ticks_forcestop++;
			if (ticks_forcestop > 3) {
				us_state = US_STATE_GEST_STOP;
				return;
			}
		} else if (us_hist[i] > 120) {
			continue;
		}

		if (us_hist[i] <= 45) {
			ticks_stop++;
		}

		if (us_hist[i] > us_hist[i - 1]) {
			ticks_start++;
		}
		if (ticks_stop >= GESTURE_HISTORY - 3) {
			us_state = US_STATE_GEST_STOP;
		} else if (ticks_start >= GESTURE_HISTORY - 3) {
			us_state = US_STATE_GEST_START;
		} else {
			us_state = US_STATE_GEST_INGEST;
		}
	}
}

void enable_gest() {
	us_state = US_STATE_GEST_INGEST;
	gest_mode = 1;
}
void disable_gest() {
	us_state = US_STATE_IDLE;
	gest_mode = 0;
}
void print_us_hist() {
	for (int i = 0; i < 10; i++) {
		printf("Index: %d, contents: %d\n", i, us_hist[i]);
	}
}

int get_us_state() {
	return us_state;
}
