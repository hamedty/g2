// #include "g2core.h"
#include "config.h"
#include "encoder.h"
#include "canonical_machine.h"  // needed for cm_panic() in assertions
#include "pwm_motor.h"

#ifdef SPECIAL_FUNCTIONS
void holder_gate_contorl(void)  {
  /*
  Gate 1
  gate: out4 -> D56 = PA23
  microswitch: in5 -> 24 = PA15
  holder microswitch lock: out6 -> D54 = PA16
  */

  // if "gate open" and "microswitch hit"
  if ((REG_PIOA_PDSR & (1 << 23)) &&
      ((REG_PIOA_PDSR & (1 << 15)) == 0))
  {
    // close gate to normal condition value = 0
    // open microswitch lock
    REG_PIOA_CODR = 1 << 23;
    REG_PIOA_SODR = 1 << 16;
  }


  /*
  Gate 2
  gate: out5 -> D55 = PA24
  microswitch: in9 -> D08 = PC22
  holder microswitch lock: out7 -> D60 = PA3
  */

  // if "gate open" and "microswitch hit"
  if ((REG_PIOA_PDSR & (1 << 24)) &&
      ((REG_PIOC_PDSR & (1 << 22)) == 0))
  {
    // close gate to normal condition value = 0
    // open microswitch lock
    REG_PIOA_CODR = 1 << 24;
    REG_PIOA_SODR = 1 << 3;
  }


}

uint32_t holder_motor_in_reverse_counter1 = 0;
uint32_t holder_motor_in_reverse_counter2 = 0;
void holder_motor_direction() {
  bool motor_in_reverse;

  // motor direction = Conveyor1 Motor M3 -> DIR 3 -> D49 -> PC14
  motor_in_reverse = (REG_PIOC_PDSR & (1 << 14));
  if (!motor_in_reverse) {
    holder_motor_in_reverse_counter1 = 0;
  } else {
    holder_motor_in_reverse_counter1++;
    if (holder_motor_in_reverse_counter1 > 10000){
      // clear reverse
      REG_PIOC_CODR = 1 << 14;
    }
  }

  // motor direction = Conveyor2 Motor M4 -> DIR 4 -> D47 -> PC16
  motor_in_reverse = (REG_PIOC_PDSR & (1 << 16));
  if (!motor_in_reverse) {
    holder_motor_in_reverse_counter2 = 0;
  } else {
    holder_motor_in_reverse_counter2++;
    if (holder_motor_in_reverse_counter2 > 10000){
      // clear reverse
      REG_PIOC_CODR = 1 << 16;
    }
  }


}

#define HOLDER_LOW_Q_MAX 40000
#define HOLDER_LOW_Q_MIN 1
void holder_low_q_detection () {
  bool no_holder;

  // holder line 1 = in4 = D27 = PD2
  no_holder = (REG_PIOD_PDSR & (1 << 2));
  if (no_holder)
    if (cfg.user_data_a[2] < HOLDER_LOW_Q_MAX)
      cfg.user_data_a[2]++;
  else
    if (cfg.user_data_a[2] > HOLDER_LOW_Q_MIN)
      cfg.user_data_a[2]--;

  // holder line 2 = in7(s4) = D6 = PC24
  no_holder = (REG_PIOC_PDSR & (1 << 24));
  if (no_holder)
    if (cfg.user_data_a[3] < HOLDER_LOW_Q_MAX)
      cfg.user_data_a[3]++;
  else
    if (cfg.user_data_a[3] > HOLDER_LOW_Q_MIN)
      cfg.user_data_a[3]--;
}

stat_t cm_special_function(void) {
#ifdef PM_FEEDER
  holder_gate_contorl();
  holder_motor_direction();
  holder_low_q_detection();
#endif // PM_FEEDER

  return STAT_OK;

}


// // blink
// REG_PIOC_SODR = 1 << 28;
// for (uint32_t i =0; i<1000000; i++) {z = z+i;}
// REG_PIOC_CODR = 1 << 28;

#endif // SPECIAL_FUNCTIONS
