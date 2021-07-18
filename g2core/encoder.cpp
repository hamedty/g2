/*
 * encoder.c - encoder interface
 * This file is part of the g2core project
 *
 * Copyright (c) 2013 - 2018 Alden S. Hart, Jr.
 *
 * This file ("the software") is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2 as published by the
 * Free Software Foundation. You should have received a copy of the GNU General Public
 * License, version 2 along with the software.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, you may use this file as part of a software library without
 * restriction. Specifically, if other files instantiate templates or use macros or
 * inline functions from this file, or you compile this file and link it with  other
 * files to produce an executable, this file does not by itself cause the resulting
 * executable to be covered by the GNU General Public License. This exception does not
 * however invalidate any other reasons why the executable file might be covered by the
 * GNU General Public License.
 *
 * THE SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT WITHOUT ANY
 * WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
 * SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "g2core.h"
#include "config.h"
#include "encoder.h"
#include "canonical_machine.h"  // needed for cm_panic() in assertions

/**** Allocate Structures ****/

enEncoders_t en;

/************************************************************************************
 **** CODE **************************************************************************
 ************************************************************************************/

/*
 * encoder_init() - initialize encoders
 * encoder_reset() - reset encoders
 */

void encoder_init() {
    memset(&en, 0, sizeof(en));  // clear all values, pointers and status
    encoder_init_assertions();

    // by hamed
    #if ENC1_AVAILABLE
    // pinMode( 2, INPUT_PULLUP); PB25
    // pinMode(13, INPUT_PULLUP); PB27
    REG_PIOB_ODR = (1 << 25) | (1 << 27); // disable output
    REG_PIOB_PUER = (1 << 25) | (1 << 27); // enable pull up

    REG_PMC_PCER0 = PMC_PCER0_PID27;   // activate clock for TC0
    REG_TC0_CMR0  = TC_CMR_TCCLKS_XC0; // select XC0 as clock source
    REG_TC0_BMR   = TC_BMR_QDEN  | TC_BMR_POSEN | TC_BMR_EDGPHA;
    REG_TC0_CCR0  = TC_CCR_CLKEN | TC_CCR_SWTRG;
    #endif

    #if ENC2_AVAILABLE
    // pinMode(4, INPUT_PULLUP); PC26
    // pinMode(5, INPUT_PULLUP); PC25
    REG_PIOC_ODR = (1 << 25) | (1 << 26); // disable output
    REG_PIOC_PUER = (1 << 25) | (1 << 26); // enable pull up

    REG_PMC_PCER1 = PMC_PCER1_PID33;
    REG_TC2_CMR0  = TC_CMR_TCCLKS_XC0;
    REG_TC2_BMR   = TC_BMR_QDEN | TC_BMR_POSEN  | TC_BMR_EDGPHA;
    REG_TC2_CCR0  = TC_CCR_CLKEN | TC_CCR_SWTRG;
    #endif

}

void encoder_reset() { encoder_init(); }
/*
 * encoder_init_assertions() - initialize encoder assertions
 * encoder_test_assertions() - test assertions, return error code if violation exists
 */

void encoder_init_assertions() {
    en.magic_end   = MAGICNUM;
    en.magic_start = MAGICNUM;
}

stat_t encoder_test_assertions() {
    if ((BAD_MAGIC(en.magic_start)) || (BAD_MAGIC(en.magic_end))) {
        return (cm_panic(STAT_ENCODER_ASSERTION_FAILURE, "encoder_test_assertions()"));
    }
    return (STAT_OK);
}

/*
 * en_set_encoder_steps() - set encoder values to a current step count
 *
 *	Sets the encoder_position steps. Takes floating point steps as input,
 *	writes integer steps. So it's not an exact representation of machine
 *	position except if the machine is at zero.
 */

void en_set_encoder_steps(uint8_t motor, float steps) { en.en[motor].encoder_steps = (int32_t)round(steps); }

/*
 * en_read_encoder()
 *
 *	The stepper ISR count steps into steps_run(). These values are accumulated to
 *	encoder_position during LOAD (HI interrupt level). The encoder position is
 *	therefore always stable. But be advised: the position lags target and position
 *	valaes elsewhere in the system because the sample is taken when the steps for
 *	that segment are complete.
 */

float en_read_encoder(uint8_t motor) { return ((float)en.en[motor].encoder_steps); }

/*
 * en_take_encoder_snapshot()
 * en_get_encoder_snapshot_position()
 * en_get_encoder_snapshot_vector()
 *
 *  Take a snapshot of the encoder position at an exact point in time. This provides
 *  a very accurate view of step position at the time of the snapshot,  which is
 *  presumably in the middle of a switch closure interrupt. Taking the snapshot
 *  does not affect the normal accumulation run by the stepper DDA.
 *
 *  The results are in STEPS, which may need to be converted back to position using
 *  forward kinematics, depending on your use. See probe cycle for example.
 */
void en_take_encoder_snapshot() {
    for (uint8_t m = 0; m < MOTORS; m++) { en.snapshot[m] = en.en[m].encoder_steps + en.en[m].steps_run; }

    /* loop unrolled version for faster execution
        en.snapshot[MOTOR_1] = en.en[MOTOR_1].encoder_steps + en.en[MOTOR_1].steps_run;
        en.snapshot[MOTOR_2] = en.en[MOTOR_2].encoder_steps + en.en[MOTOR_2].steps_run;
        en.snapshot[MOTOR_3] = en.en[MOTOR_3].encoder_steps + en.en[MOTOR_3].steps_run;
        en.snapshot[MOTOR_4] = en.en[MOTOR_4].encoder_steps + en.en[MOTOR_4].steps_run;
        en.snapshot[MOTOR_5] = en.en[MOTOR_5].encoder_steps + en.en[MOTOR_5].steps_run;
        en.snapshot[MOTOR_6] = en.en[MOTOR_6].encoder_steps + en.en[MOTOR_6].steps_run;
    */
}

float en_get_encoder_snapshot_steps(uint8_t motor) { return (en.snapshot[motor]); }

float* en_get_encoder_snapshot_vector() { return (en.snapshot); }

/***********************************************************************************
 * CONFIGURATION AND INTERFACE FUNCTIONS
 * Functions to get and set variables from the cfgArray table
 ***********************************************************************************/

/***********************************************************************************
 * TEXT MODE SUPPORT
 * Functions to print variables from the cfgArray table
 ***********************************************************************************/

#ifdef __TEXT_MODE

#endif  // __TEXT_MODE
