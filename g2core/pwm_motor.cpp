#if PWM_MOTORS_AVAILABLE

#include "pwm_motor.h"
#include "gpio.h"
#define BLOCKED_COUNTER_SMALL 30000
#define BLOCKED_COUNTER_BIG   30000

pwm_motor_t pwm_motors[PWM_MOTOR_COUNT] = {
  // { 0, 0, 0, PIOC, 1 << 17, false}, // M1 - Step 4 - PC17 - D46
  { 0, 10000, 0, PIOA, 1 << 22, true}, // V3 - PA23 - D57 - Holder Jack
  { 0, 0, 0, PIOC, 1 << 19, false}, // M2 - Step 5 - PC19 - D44
  { 0, 0, 0, PIOA, 1 << 19, false}, // M3 - Step 6 - PA19 - D42
  { 0, 0, 0, PIOC, 1 << 8, false}, // M4 - Step 7 - PC8 - D40
  { 0, 0, 0, PIOC, 1 << 6, false}, // M5 - Step 8 - PC6 - D38
  { 0, 0, 0, PIOC, 1 << 4, false}, // M6 - Step 9 - PC4 - D36
  { 0, 0, 0, PIOC, 1 << 2, false}, // M7 - Step 10 - PC2 - D34
  { 0, 0, 0, PIOD, 1 << 10, false}, // M8 - Step 11 - PD10 - D32
  { 0, 0, 0, PIOD, 1 << 9, false}, // M9 - Step 12 - PD9 - D30
};

sensor_blocking_data_t sensor_blocking_data = {0, false};

void setup_pwm_motors() {
  // setup motors
  for (int i = 0; i < PWM_MOTOR_COUNT; i++) {
    pwm_motor *m = &pwm_motors[i];
    m->reg->PIO_PER  = m->reg_mask; // Enable PIO
    m->reg->PIO_OER  = m->reg_mask; // Set to OUTPUT
    m->reg->PIO_PUDR = m->reg_mask; // Disable the pull up resistor
  }
}

void pwm_motors_step() {

  bool motors_blocked_by_sensor = sensor_blocking_data.blocked_out;
  // Sensor = in4 - D27 - PD2 - active low
  bool sensor_blocked = (REG_PIOD_PDSR & (1 << 2)) == 0;

  // 1- hysthersis calcs
  if (motors_blocked_by_sensor) {
    if (sensor_blocked) {
      sensor_blocking_data.blocked_counter = BLOCKED_COUNTER_BIG;
    }
    else {
      sensor_blocking_data.blocked_counter--;
      if (sensor_blocking_data.blocked_counter <= 1) {
        motors_blocked_by_sensor = false;
        sensor_blocking_data.blocked_counter = 1;
      }
    }
  }
  else {
    if (sensor_blocked) {
      sensor_blocking_data.blocked_counter++;
      if (sensor_blocking_data.blocked_counter >= BLOCKED_COUNTER_SMALL) {
        motors_blocked_by_sensor = true;
        sensor_blocking_data.blocked_counter = BLOCKED_COUNTER_BIG;
      }

    }
    else {
      sensor_blocking_data.blocked_counter = 1;
    }
  }
  sensor_blocking_data.blocked_out = motors_blocked_by_sensor;


  // 2- Motors step
  for (int i = 0; i < PWM_MOTOR_COUNT; i++) {
    pwm_motor *m = &pwm_motors[i];
    if (!m->x_counter_on) continue;
    if (m->blockable) {
      if (motors_blocked_by_sensor) {
        m->reg->PIO_CODR = m->reg_mask; // clear
        continue;
      }
    }

    if (m->counter == 0) {
      // step
      if (m->reg->PIO_ODSR & m->reg_mask) {
        m->reg->PIO_CODR = m->reg_mask; // clear
        m->counter = m->x_counter_off;
      }
      else {
        m->reg->PIO_SODR = m->reg_mask; // set
        m->counter = m->x_counter_on;
      }
    }
    m->counter--;
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
  return STAT_OK;
}

stat_t pwm_motor_set_value_simple(uint8_t motor_index, uint32_t value) {
  bool m10 = false;

  if (motor_index > PWM_MOTOR_COUNT) return STAT_OK; // reserve == for M10
  if (motor_index == 9) {motor_index = 0; m10 = true;} // translate M10 to M1

  pwm_motor *m = &pwm_motors[motor_index];

  if(motor_index == 0) { //  1st motor
    if (m10)
      m->x_counter_off = value;
    else
      m->x_counter_on = value;

  } else {
    m->x_counter_on = value;
    m->x_counter_off = value;
  }

  if (m->x_counter_on == 0) {
    m->reg->PIO_CODR = m->reg_mask;  // clear
  }

  return STAT_OK;
}


stat_t pwm_motor_set_value(nvObj_t *nv) {
  uint8_t motor_index = nv_index_2_motor_index(nv->index);
  return pwm_motor_set_value_simple(motor_index, (uint32_t)nv->value_flt);
}


#endif // PWM_MOTORS_AVAILABLE
