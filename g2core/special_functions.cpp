// #include "g2core.h"
#include "config.h"
#include "encoder.h"
#include "canonical_machine.h"  // needed for cm_panic() in assertions
#include "pwm_motor.h"

#ifdef SPECIAL_FUNCTIONS
void holder_gate_contorl(void)  {
  /*
  gate: out7 -> 60 = PA3
  microswitch: in5 -> 24 = PA15
  holder microswitch lock: out1 -> 59 = PA4
  motors: m2, m3
  */
  // if gate not open return (PA3)
  if ((REG_PIOA_PDSR & (1 << 3)) == 0) return;

  // if microswitch not hit return (PA15)
  if ((REG_PIOA_PDSR & (1 << 15)) != 0) return;

  // close gate (PA3 - to normal condition value = 0)

  // open microswitch lock (PA4), slow down motors
  REG_PIOA_CODR = 1 << 3;
  REG_PIOA_SODR = 1 << 4;
  pwm_motor_set_value_simple(1, 30);
  pwm_motor_set_value_simple(2, 30);
  return;
}

uint32_t holder_motor_in_reverse_counter = 0;
void holder_motor_direction() {
  /* motor direction = Conveyor Motor M7 -> DIR 10 -> D35 -> PC3
  */
  bool motor_in_reverse = (REG_PIOC_PDSR & (1 << 9));
  if (!motor_in_reverse) {
    holder_motor_in_reverse_counter = 0;
    return;
  }
  holder_motor_in_reverse_counter++;
  if (holder_motor_in_reverse_counter > 10000){
    // clear reverse
    REG_PIOC_CODR = 1 << 9;
  }
}

stat_t cm_special_function(void) {
#ifdef PM_FEEDER
  holder_gate_contorl();
  holder_motor_direction();
#endif // PM_FEEDER

  return STAT_OK;

}


// // blink
// REG_PIOC_SODR = 1 << 28;
// for (uint32_t i =0; i<1000000; i++) {z = z+i;}
// REG_PIOC_CODR = 1 << 28;

#endif // SPECIAL_FUNCTIONS
