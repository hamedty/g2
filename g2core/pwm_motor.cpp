#ifdef PWM_MOTORS_AVAILABLE
#if PWM_MOTORS_AVAILABLE

#include "pwm_motor.h"
#include "gpio.h"
#define BLOCKED_COUNTER_SMALL 20000
#define BLOCKED_COUNTER_BIG   20000

// Feeder
#if PWM_MOTORS_ARRANGEMENT == 1
pwm_motor_t pwm_motors[PWM_MOTOR_COUNT] = {
  { 0, 10000, 0, PIOA, 1 << 6, true}, // M1 - V2 - D58 - PA6
  { 0, 10000, 0, PIOA, 1 << 22, true}, // M2 - V3 - D57 - PA22
  { 0, 0, 0, PIOC, 1 << 15, false}, // M3 - Step 3 - D48 - PC15
  { 0, 0, 0, PIOC, 1 << 17, false}, // M4 - Step 4 - D46 - PC17
  { 0, 0, 0, PIOC, 1 << 19, false}, // M5 - Step 5 - D44 - PC19
  { 0, 0, 0, PIOA, 1 << 19, false}, // M6 - Step 6 - D42 - PA19
  { 0, 0, 0, PIOC, 1 << 8, false}, // M7 - Step 7 - D40 - PC8
  { 0, 0, 0, PIOC, 1 << 6, false}, // M8 - Step 8 - D38 - PC6
  { 0, 0, 0, PIOC, 1 << 4, false}, // M9 - Step 9 - D36 - PC4
};
#else
// Dosing Feeder
pwm_motor_t pwm_motors[PWM_MOTOR_COUNT] = {
  { 0, 0, 0, PIOB, 1 << 25, false} // Conveyor motor - D2 - B25
};
#endif
sensor_blocking_data_t sensor_blocking_data = {0, false};

void setup_pwm_motors() {
  // setup motors
  for (int i = 0; i < PWM_MOTOR_COUNT; i++) {
    pwm_motor *m = &pwm_motors[i];
    m->reg->PIO_PER  = m->reg_mask; // Enable PIO
    m->reg->PIO_OER  = m->reg_mask; // Set to OUTPUT
    m->reg->PIO_PUDR = m->reg_mask; // Disable the pull up resistor
  }

#ifdef PM_FEEDER
  // setup holder conveyors motor direction controller
  // M3, M4, Dir3, Dir4 - D49, D47
  // set C14, C16 as output
  REG_PIOC_PER = 1 << 14 | 1 << 16;
  REG_PIOC_OER = 1 << 14 | 1 << 16;
  REG_PIOC_PUDR = 1 << 14 | 1 << 16;

  // in(s1): Cartridge Hug sensor 1 - PC28
  // in(s2): Cartridge Hug sensor 2 - PC26
  // in(s3): holder air high level optical sensor 1 - PC25
  // in(s8): holder air high level optical sensor 2 - PC29
  // PC25, PC26, PC28, PC29
  // setup as input
#endif
}

void pwm_motors_step() {

#if PWM_MOTORS_ARRANGEMENT == 1
  bool motors_blocked_by_sensor = sensor_blocking_data.blocked_out;
  // Sensor = in8 - S3 = D5 = C25 - active low
  // holder exists -> sensor_blocked = true -> eventually motors_blocked_by_sensor = true
  // lack of holder -> sensor_blocked = false ....
  bool sensor_blocked = (REG_PIOC_PDSR & (1 << 25)) == 0;

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
        REG_PIOC_SODR = 1 << 9; // Zand feature request
        motors_blocked_by_sensor = true;
        sensor_blocking_data.blocked_counter = BLOCKED_COUNTER_BIG;
      }

    }
    else {
      sensor_blocking_data.blocked_counter = 1;
    }
  }
  sensor_blocking_data.blocked_out = motors_blocked_by_sensor;


#else
  bool motors_blocked_by_sensor = false;
#endif

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
  // always return if sensor is blocked, if asked about {m1:n}
  nv->valuetype = TYPE_INTEGER;
  nv->value_int = sensor_blocking_data.blocked_out;
  return STAT_OK;
}

stat_t pwm_motor_set_value_simple(uint8_t motor_index, uint32_t value) {
  bool m11_m12 = false;

  // if (motor_index > 11 || ) return STAT_OK; // bad motors
  if (motor_index > 9) {motor_index -= 10; m11_m12 = true;} // translate M11, M12 to M1, M2

  pwm_motor *m = &pwm_motors[motor_index];

#if PWM_MOTORS_ARRANGEMENT == 1
  if(motor_index < 2) { //  M1, M2
    if (m11_m12)
      m->x_counter_off = value;
    else
      m->x_counter_on = value;

  } else {
    m->x_counter_on = value;
    m->x_counter_off = value;
  }
#else
  m->x_counter_on = value;
  m->x_counter_off = value;
#endif


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
#endif // PWM_MOTORS_AVAILABLE
