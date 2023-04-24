#ifndef application_HEADER_H
#define application_HEADER_H

#include "global.h"
#include "fpga_reg.h"
#include "ep.h"
#include "tcp_server.h"
#include "serial-dev.h"

///////////////////////////////////////////////////////////////////////////////
/* 1 Dimension */
typedef struct {
    double x;  /* state */
    double A;  /* x(n)=A*x(n-1)+u(n),u(n)~N(0,q) */
    double H;  /* z(n)=H*x(n)+w(n),w(n)~N(0,r)   */
    double q;  /* process(predict) noise convariance */
    double r;  /* measure noise convariance */
    double p;  /* estimated error convariance */
    double gain;
} kalman1_state;

/* 2 Dimension */
typedef struct {
    float x[2];     /* state: [0]-angle [1]-diffrence of angle, 2x1 */
    float A[2][2];  /* X(n)=A*X(n-1)+U(n),U(n)~N(0,q), 2x2 */
    float H[2];     /* Z(n)=H*X(n)+W(n),W(n)~N(0,r), 1x2   */
    float q[2];     /* process(predict) noise convariance,2x1 [q0,0; 0,q1] */
    float r;        /* measure noise convariance */
    float p[2][2];  /* estimated error convariance,2x2 [p0 p1; p2 p3] */
    float gain[2];  /* 2x1 */
} kalman2_state; 

extern void kalman1_init(kalman1_state *state, double init_x, double init_p);
extern float kalman1_filter(kalman1_state *state, double z_measure);
extern void kalman2_init(kalman2_state *state, double *init_x, double (*init_p)[2]);
extern float kalman2_filter(kalman2_state *state, double z_measure);
///////////////////////////////////////////////////////////////////////////////


void *fifo_0_empty();

void trigger_count_clear();
void trigger_auto_run();
void fifo_reset();
void gpio_out_reg();
void real_time_sensing();
void uart_gpio_init();
void uart_gpio_0_on();
void uart_gpio_1_on();
void uart_gpio_2_on();
void uart_gpio_0_off();
void uart_gpio_1_off();
void uart_gpio_2_off();
void uart_gpio_all_on();
void uart_gpio_all_off();
void conversion_fifo_read_ext(int sel,int *count,REGISTER_CONFIG_CS0 *fifo_read,ADC_CAL *adc_cal,u_int8_t cal_flag1,u_int8_t cal_flag2,u_int8_t cal_flag3);
void conversion_fifo_read_sum_value(int sel,ADC_CAL *adc_cal,u_int8_t cal_flag1,u_int8_t cal_flag2,u_int8_t cal_flag3);
void formula_calculation_init();
void uart_gpio_ctl(unsigned short gpio);
void uart_to_i2c_ctl(unsigned char ld, unsigned value);
void ld_current_ctrl_init();
void ld_current_value_save();



void adc_cal_load();
#endif