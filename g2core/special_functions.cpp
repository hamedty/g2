// #include "g2core.h"
#include "config.h"
#include "encoder.h"
#include "canonical_machine.h"  // needed for cm_panic() in assertions
#include "pwm_motor.h"

// gate: out7 -> 60 = PA3
// microswitch: in5 -> 24 = PA15
// holder microswitch lock: out1 -> 59 = PA4
// motors: m2, m3

#ifdef SPECIAL_FUNCTIONS
#if SPECIAL_FUNCTIONS
stat_t cm_special_function(void) {
    // if gate not open return (PA3)
    if ((REG_PIOA_PDSR & (1 << 3)) == 0) return STAT_OK;

    // if microswitch not hit return (PA15)
    if ((REG_PIOA_PDSR & (1 << 15)) != 0) return STAT_OK;

    // close gate (PA3 - to normal condition value = 0)

    // open microswitch lock (PA4), slow down motors
    REG_PIOA_CODR = 1 << 3;
    REG_PIOA_SODR = 1 << 4;
    pwm_motor_set_value_simple(1, 30);
    pwm_motor_set_value_simple(2, 30);


    return STAT_OK;
}
#endif
#endif
