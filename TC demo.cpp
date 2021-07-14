#include <Arduino.h>

#define MAX_V 20000000
#define MIN_V 2000000

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  // Configure
  REG_PMC_PCER0 = PMC_PCER0_PID27; // activate clock for TC0
  TC_Configure(TC0,
               0,
               TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK1);

  // Set RC
  // TC_SetRC(TC0, 0, 10000000); // aproximately 200ms
  REG_TC0_RC0 = MAX_V;

  // enable interrupt
  TC0->TC_CHANNEL[0].TC_IER = TC_IER_CPCS;
  NVIC_ClearPendingIRQ(TC0_IRQn);
  NVIC_EnableIRQ(TC0_IRQn);

  // Start Timer
  REG_TC0_CCR0 = TC_CCR_CLKEN | TC_CCR_SWTRG;
  TC_Start(TC0, 0);
}

void loop() {}

volatile bool value = 0;
uint32_t mask       = 1 << 27;
void TC0_Handler() {
  TC_GetStatus(TC0, 0);

  if (value) {
    REG_PIOB_SODR = mask;
  } else {
    REG_PIOB_CODR = mask;
  }
  value = !value;
  uint32_t v = REG_TC0_RC0;

  if (v > MIN_V) {
    REG_TC0_RC0 = v - MIN_V;
  } else {
    REG_TC0_RC0 = MAX_V;
  }
}
