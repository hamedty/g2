#if PWM_MOTORS_AVAILABLE

#include "pwm_motor.h"

pwm_motor_t pwm_motors[PWM_MOTOR_COUNT] = {
  { 10000, 0, PIOC, 1 << 6  }, // PC6 - step8 - D38
  { 10000, 0, PIOB, 1 << 27 }, // PB27 - D13
};

void setup_pwm_motors() {
  for (int i = 0; i < PWM_MOTOR_COUNT; i++) {
    pwm_motor *m = &pwm_motors[i];
    m->reg->PIO_PER  = m->reg_mask; // Enable PIO
    m->reg->PIO_OER  = m->reg_mask; // Set to OUTPUT
    m->reg->PIO_PUDR = m->reg_mask; // Disable the pull up resistor
  }
}

void pwm_motors_step() {
  for (int i = 0; i < PWM_MOTOR_COUNT; i++) {
    pwm_motor *m = &pwm_motors[i];

    if (m->x_counter) {
      m->counter++;

      if (m->counter >= m->x_counter) {
        // step
        if (m->reg->PIO_ODSR & m->reg_mask) {
          m->reg->PIO_CODR = m->reg_mask;
        }
        else {
          m->reg->PIO_SODR = m->reg_mask;
        }

        m->counter = 0;
      }
    }
  }
}

uint8_t nv_index_2_motor_index(const index_t index)
{
  const char *ptr = cfgArray[index].token;

  ptr++;
  return atoi(ptr) - 1;
}

void pwm_motor_print_out(nvObj_t *nv) {
  // sprintf(cs.out_buf, fmt_gpio_out, nv->token, (int)nv->value_int);
  // xio_writeline(cs.out_buf);
}

stat_t pwm_motor_get_value(nvObj_t *nv)
{
  nv->value_flt = 10;

  //     uint8_t output_num = _io(nv->index);
  //
  //     ioMode outMode = d_out[output_num].mode;
  //     if (outMode == IO_MODE_DISABLED) {
  //         nv->valuetype = TYPE_NULL;   // reports back as NULL
  //     } else {
  //         nv->valuetype = TYPE_FLOAT;
  //         nv->precision = 2;
  //
  //         switch (output_num+1) {                 // add 1 to get logical pin
  // numbers
  //             case 1:  { nv->value_flt = (float)output_1_pin; } break;
  //             case 2:  { nv->value_flt = (float)output_2_pin; } break;
  //             case 3:  { nv->value_flt = (float)output_3_pin; } break;
  //             case 4:  { nv->value_flt = (float)output_4_pin; } break;
  //             case 5:  { nv->value_flt = (float)output_5_pin; } break;
  //             case 6:  { nv->value_flt = (float)output_6_pin; } break;
  //             case 7:  { nv->value_flt = (float)output_7_pin; } break;
  //             case 8:  { nv->value_flt = (float)output_8_pin; } break;
  //             case 9:  { nv->value_flt = (float)output_9_pin; } break;
  //             case 10: { nv->value_flt = (float)output_10_pin; } break;
  //             case 11: { nv->value_flt = (float)output_11_pin; } break;
  //             case 12: { nv->value_flt = (float)output_12_pin; } break;
  //             case 13: { nv->value_flt = (float)output_13_pin; } break;
  // #ifdef FEEDER
  //             case 14: { nv->value_flt = (float)output_14_pin; } break;
  // #endif
  //             default: { nv->valuetype = TYPE_NULL;  }    // reports back as
  // NULL
  //         }
  //         if (outMode == IO_ACTIVE_LOW) {
  //             nv->value_flt = 1.0 - nv->value_flt;        // invert output
  // sense
  //         }
  //     }
  return STAT_OK;
}

stat_t pwm_motor_set_value(nvObj_t *nv)
{
  uint8_t motor_index = nv_index_2_motor_index(nv->index);

  if (motor_index >= PWM_MOTOR_COUNT) return STAT_OK;

  pwm_motor *m = &pwm_motors[motor_index];
  m->x_counter = (uint32_t)nv->value_flt;
  return STAT_OK;
}

#endif // PWM_MOTORS_AVAILABLE
