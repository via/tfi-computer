#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/syscfg.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/cortex.h>
#include "libopencm3/cm3/dwt.h"

#include "decoder.h"
#include "platform.h"
#include "scheduler.h"
#include "limits.h"
#include "sensors.h"
#include "util.h"
#include "config.h"

#include <assert.h>

static char *usart_rx_dest;
static int usart_rx_end;

/* Hardware setup:
 *  Discovery board LEDs:
 *  LD3 - Orange - PD13
 *  LD4 - Green - PD12
 *  LD5 - Red - PD14
 *  LD6 - Blue - PD15
 *
 *  Fixed pin mapping:
 *  USART2_TX - PA2 (dma1 stream 6 chan 4)
 *  USART2_RX - PA3 (dma1 stream 5 chan 4)
 *  Test trigger out - A0 (Uses TIM5)
 *  Event timer TIM2 slaved off TIM8
 *  Trigger 0 - PB3 (TIM2_CH2)
 *  Trigger 1 - PB10 (TIM2_CH3)
 *  Trigger 2 - PB11 (TIM2_CH4)
 *
 *  Configurable pin mapping:
 *  ADC: 
 *   - 1-8 maps to port A pins 4-7, port C 0-3
 *  Scheduled Outputs:
 *   - 0-15 maps to port D, pins 0-15
 *  Freq sensor:
 *   - 1-4 maps to port A, pin 8-11 (TIM1 CH1-4)
 *  PWM Outputs
 *   - 0-PC6  1-PC7 2-PC8 3-PC9 TIM3 channel 1-4
 *  GPIO (Digital Sensor or Output)
 *   - 0-15 Maps to Port E
 *
 */

static void platform_init_freqsensor(unsigned char pin) {
  uint32_t tim;
  switch(pin) {
    case 1:
      /* TIM1 CH1 */
      tim = TIM1;
      gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO8);
      gpio_set_af(GPIOA, GPIO_AF1, GPIO8);
      break;
    case 2:
      /* TIM1 CH2 */
      tim = TIM1;
      gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);
      gpio_set_af(GPIOA, GPIO_AF1, GPIO9);
      break;
    case 3:
      /* TIM1 CH3 */
      tim = TIM1;
      gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10);
      gpio_set_af(GPIOA, GPIO_AF1, GPIO10);
      break;
    case 4:
      /* TIM1 CH4 */
      tim = TIM1;
      gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11);
      gpio_set_af(GPIOA, GPIO_AF1, GPIO11);
      break;
  };
  timer_reset(tim);
  timer_set_mode(tim, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
  timer_set_period(tim, 0xFFFFFFFF);
  timer_disable_preload(tim);
  timer_continuous_mode(tim);
  /* Setup output compare registers */
  timer_disable_oc_output(tim, TIM_OC1);
  timer_disable_oc_output(tim, TIM_OC2);
  timer_disable_oc_output(tim, TIM_OC3);
  timer_disable_oc_output(tim, TIM_OC4);

  /* Set up compare */
  timer_ic_set_input(tim, TIM_IC1, TIM_IC_IN_TI1);
  timer_ic_set_filter(tim, TIM_IC1, TIM_IC_CK_INT_N_8);
  timer_ic_set_polarity(tim, TIM_IC1, TIM_IC_RISING);
  timer_set_prescaler(tim, 2*SENSOR_FREQ_DIVIDER); /* Prescale set to map up to 20kHz */
  timer_slave_set_mode(tim, TIM_SMCR_SMS_RM);
  timer_slave_set_trigger(tim, TIM_SMCR_TS_IT1FP1);
  timer_ic_enable(tim, TIM_IC1);

  timer_enable_counter(tim);
  timer_enable_irq(tim, TIM_DIER_CC1IE);

  switch(pin) {
    case 1:
    case 2:
    case 3:
    case 4:
      nvic_enable_irq(NVIC_TIM1_CC_IRQ);
      nvic_set_priority(NVIC_TIM1_CC_IRQ, 64);
      break;
  }
}

void tim1_cc_isr() {
  timeval_t t = TIM1_CCR1;
  timer_clear_flag(TIM1, TIM_SR_CC1IF);
  /*TODO: Handle pins 1-4 */
  for (int i = 0; i < NUM_SENSORS; ++i) {
    if ((config.sensors[i].method == SENSOR_FREQ) &&
        (config.sensors[i].pin == 1)) {
      config.sensors[i].raw_value = t;
    }
  }
  sensor_freq_new_data();
}

static void platform_init_eventtimer() {
  /* Set up TIM2 as 32bit clock that is slaved off TIM8*/


  timer_reset(TIM2);
  timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
  timer_slave_set_mode(TIM2, TIM_SMCR_SMS_ECM1);
  timer_slave_set_trigger(TIM2, TIM_SMCR_TS_ITR1); /* TIM2 slaved off TIM8 */
  timer_set_period(TIM2, 0xFFFFFFFF);
  timer_set_prescaler(TIM2, 0);
  timer_disable_preload(TIM2);
  timer_continuous_mode(TIM2);
  /* Setup output compare registers */
  timer_disable_oc_output(TIM2, TIM_OC1);
  timer_disable_oc_output(TIM2, TIM_OC2);
  timer_disable_oc_output(TIM2, TIM_OC3);
  timer_disable_oc_output(TIM2, TIM_OC4);
  timer_disable_oc_clear(TIM2, TIM_OC1);
  timer_disable_oc_preload(TIM2, TIM_OC1);
  timer_set_oc_slow_mode(TIM2, TIM_OC1);
  timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_FROZEN);
  /* Setup input captures for CH2-4 Triggers */
  gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO3);
  gpio_set_af(GPIOB, GPIO_AF1, GPIO3);
  timer_ic_set_input(TIM2, TIM_IC2, TIM_IC_IN_TI2);
  timer_ic_set_filter(TIM2, TIM_IC2, TIM_IC_CK_INT_N_2);
  timer_ic_set_polarity(TIM2, TIM_IC2, TIM_IC_FALLING);
  timer_ic_enable(TIM2, TIM_IC2);

  gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10);
  gpio_set_af(GPIOB, GPIO_AF1, GPIO10);
  timer_ic_set_input(TIM2, TIM_IC3, TIM_IC_IN_TI3);
  timer_ic_set_filter(TIM2, TIM_IC3, TIM_IC_CK_INT_N_2);
  timer_ic_set_polarity(TIM2, TIM_IC3, TIM_IC_FALLING);
  timer_ic_enable(TIM2, TIM_IC3);

  gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11);
  gpio_set_af(GPIOB, GPIO_AF1, GPIO11);
  timer_ic_set_input(TIM2, TIM_IC4, TIM_IC_IN_TI4);
  timer_ic_set_filter(TIM2, TIM_IC4, TIM_IC_CK_INT_N_2);
  timer_ic_set_polarity(TIM2, TIM_IC4, TIM_IC_FALLING);
  timer_ic_enable(TIM2, TIM_IC4);

  timer_enable_counter(TIM2);
  timer_enable_irq(TIM2, TIM_DIER_CC2IE);
  timer_enable_irq(TIM2, TIM_DIER_CC3IE);
  timer_enable_irq(TIM2, TIM_DIER_CC4IE);
  nvic_enable_irq(NVIC_TIM2_IRQ);
  nvic_set_priority(NVIC_TIM2_IRQ, 0);

  /* Set debug unit to stop the timer on halt */
  *((volatile uint32_t *)0xE0042008) |= 9; /*TIM2 and TIM5 */
  *((volatile uint32_t *)0xE004200C) |= 2; /* TIM8 stop */

  timer_reset(TIM8);
  timer_set_mode(TIM8, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
  timer_set_period(TIM8, 41); /* 0.25 uS */
  timer_set_prescaler(TIM8, 0);
  timer_disable_preload(TIM8);
  timer_continuous_mode(TIM8);
  timer_disable_oc_output(TIM8, TIM_OC1);
  timer_disable_oc_output(TIM8, TIM_OC2);
  timer_disable_oc_output(TIM8, TIM_OC3);
  timer_disable_oc_output(TIM8, TIM_OC4);
  timer_set_master_mode(TIM8, TIM_CR2_MMS_UPDATE);
  timer_enable_counter(TIM8);
  timer_enable_update_event(TIM8);
  timer_update_on_overflow(TIM8);
  timer_set_dma_on_update_event(TIM8);
  TIM8_DIER |= TIM_DIER_UDE; /* Enable update dma */
}


static uint32_t output_buffer_len;
/* Give two buffers of len size, dma will start reading from buf0 and double
 * buffer between buf0 and buf0.
 * Returns the time that buf0 starts at
 */
timeval_t init_output_thread(uint32_t *buf0, uint32_t *buf1, uint32_t len) {
  timeval_t start;

  output_buffer_len = len;
  /* dma2 stream 1, channel 7*/
  dma_stream_reset(DMA2, DMA_STREAM1);
  dma_set_priority(DMA2, DMA_STREAM1, DMA_SxCR_PL_HIGH);
  dma_enable_double_buffer_mode(DMA2, DMA_STREAM1);
  dma_set_memory_size(DMA2, DMA_STREAM1, DMA_SxCR_MSIZE_32BIT);
  dma_set_peripheral_size(DMA2, DMA_STREAM1, DMA_SxCR_PSIZE_32BIT);
  dma_enable_memory_increment_mode(DMA2, DMA_STREAM1);
  dma_set_transfer_mode(DMA2, DMA_STREAM1, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
  dma_enable_circular_mode(DMA2, DMA_STREAM1);
  dma_set_peripheral_address(DMA2, DMA_STREAM1, (uint32_t) &GPIOD_BSRR);
  dma_set_memory_address(DMA2, DMA_STREAM1, (uint32_t)buf0);
  dma_set_memory_address_1(DMA2, DMA_STREAM1, (uint32_t)buf1);
  dma_set_number_of_data(DMA2, DMA_STREAM1, len);
  dma_channel_select(DMA2, DMA_STREAM1, DMA_SxCR_CHSEL_7);
  dma_enable_direct_mode(DMA2, DMA_STREAM1);
  dma_enable_transfer_complete_interrupt(DMA2, DMA_STREAM1);

  timer_disable_counter(TIM8);
  dma_enable_stream(DMA2, DMA_STREAM1);
  start = timer_get_counter(TIM2);
  timer_enable_counter(TIM8);

  nvic_enable_irq(NVIC_DMA2_STREAM1_IRQ);

  return start;
}

/* Returns 0 if buf0 is active */
int current_output_buffer() {
  return dma_get_target(DMA2, DMA_STREAM1);
}

/* Returns the current slot that DMA is pointing at.
 * Note that the slot returned is the one queued to output next, but is already
 * in the FIFO so is considered 'done'
 */
int current_output_slot() {
  return output_buffer_len - DMA2_S1NDTR;
}

static void platform_init_pwm() {

  gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6);
  gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO7);
  gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO8);
  gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);
  gpio_set_af(GPIOC, GPIO_AF2, GPIO6);
  gpio_set_af(GPIOC, GPIO_AF2, GPIO7);
  gpio_set_af(GPIOC, GPIO_AF2, GPIO8);
  gpio_set_af(GPIOC, GPIO_AF2, GPIO9);

  timer_reset(TIM3);
  timer_disable_oc_output(TIM3, TIM_OC1);
  timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
  /* 72ish Hz, close to 60 */
  timer_set_period(TIM3, 65535);
  timer_set_prescaler(TIM3, 16);
  timer_disable_preload(TIM3);
  timer_continuous_mode(TIM3);
  /* Setup output compare registers */
  timer_ic_set_input(TIM3,  TIM_IC1, TIM_IC_OUT);
  timer_disable_oc_clear(TIM3, TIM_OC1);
  timer_disable_oc_preload(TIM3, TIM_OC1);
  timer_set_oc_slow_mode(TIM3, TIM_OC1);
  timer_set_oc_mode(TIM3, TIM_OC1, TIM_OCM_PWM1);
  timer_set_oc_value(TIM3, TIM_OC1, 0);
  timer_set_oc_polarity_high(TIM3, TIM_OC1);
  timer_enable_oc_output(TIM3, TIM_OC1);

  timer_ic_set_input(TIM3,  TIM_IC2, TIM_IC_OUT);
  timer_disable_oc_clear(TIM3, TIM_OC2);
  timer_disable_oc_preload(TIM3, TIM_OC2);
  timer_set_oc_slow_mode(TIM3, TIM_OC2);
  timer_set_oc_mode(TIM3, TIM_OC2, TIM_OCM_PWM1);
  timer_set_oc_value(TIM3, TIM_OC2, 0);
  timer_set_oc_polarity_high(TIM3, TIM_OC2);
  timer_enable_oc_output(TIM3, TIM_OC2);

  timer_ic_set_input(TIM3,  TIM_IC3, TIM_IC_OUT);
  timer_disable_oc_clear(TIM3, TIM_OC3);
  timer_disable_oc_preload(TIM3, TIM_OC3);
  timer_set_oc_slow_mode(TIM3, TIM_OC3);
  timer_set_oc_mode(TIM3, TIM_OC3, TIM_OCM_PWM1);
  timer_set_oc_value(TIM3, TIM_OC3, 0);
  timer_set_oc_polarity_high(TIM3, TIM_OC3);
  timer_enable_oc_output(TIM3, TIM_OC3);

  timer_ic_set_input(TIM3,  TIM_IC4, TIM_IC_OUT);
  timer_disable_oc_clear(TIM3, TIM_OC4);
  timer_disable_oc_preload(TIM3, TIM_OC4);
  timer_set_oc_slow_mode(TIM3, TIM_OC4);
  timer_set_oc_mode(TIM3, TIM_OC4, TIM_OCM_PWM1);
  timer_set_oc_value(TIM3, TIM_OC4, 0);
  timer_set_oc_polarity_high(TIM3, TIM_OC4);
  timer_enable_oc_output(TIM3, TIM_OC4);
  timer_enable_counter(TIM3);

}

void set_pwm(int output, float value) {
  int ival = value * 65535;
  switch (output) {
    case 1:
      return timer_set_oc_value(TIM3, TIM_OC1, ival);
    case 2:
      return timer_set_oc_value(TIM3, TIM_OC2, ival);
    case 3:
      return timer_set_oc_value(TIM3, TIM_OC3, ival);
    case 4:
      return timer_set_oc_value(TIM3, TIM_OC4, ival);
  }
}

static void platform_init_scheduled_outputs() {
  gpio_clear(GPIOD, 0xFFFF);
  int i;
  for (i = 0; i < config.num_events; ++i) {
    if (config.events[i].inverted && config.events[i].type) {
      set_output(config.events[i].output_id, 1);
    }
  }
  gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, 0xFFFF & ~GPIO5);
  gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, 0xFFFF & ~GPIO5);
  gpio_mode_setup(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, 0xFF);
  gpio_set_output_options(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, 0xFF);
}

static void platform_init_adc() {
  /* Set up DMA for the ADC */
  nvic_enable_irq(NVIC_ADC_IRQ);
  nvic_set_priority(NVIC_ADC_IRQ, 64);

  /* Set up ADC */
  gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, 
      GPIO4 | GPIO5 | GPIO6 | GPIO7);
  gpio_mode_setup(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, 
      GPIO0 | GPIO1 | GPIO2 | GPIO3 | GPIO4 | GPIO5);
  
  adc_off(ADC1);
  adc_off(ADC2);
  adc_off(ADC3);

  adc_enable_scan_mode(ADC1);
  adc_enable_scan_mode(ADC2);
  adc_enable_scan_mode(ADC3);

  adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_15CYC);
  adc_set_sample_time_on_all_channels(ADC2, ADC_SMPR_SMP_15CYC);
  adc_set_sample_time_on_all_channels(ADC3, ADC_SMPR_SMP_15CYC);

  adc_disable_automatic_injected_group_conversion(ADC1);
  adc_disable_automatic_injected_group_conversion(ADC2);
  adc_disable_automatic_injected_group_conversion(ADC3);

  adc_enable_eoc_interrupt_injected(ADC1);
  adc_enable_eoc_interrupt_injected(ADC2);
  adc_enable_eoc_interrupt_injected(ADC3);

  adc_eoc_after_group(ADC1);
  adc_eoc_after_group(ADC2);
  adc_eoc_after_group(ADC3);

  adc_power_on(ADC1);
  adc_power_on(ADC2);
  adc_power_on(ADC3);
  adc_set_multi_mode(ADC_CCR_MULTI_TRIPLE_INJECTED_SIMUL);

  adc_clear_overrun_flag(ADC1);
  adc_clear_overrun_flag(ADC2);
  adc_clear_overrun_flag(ADC3);

  uint8_t seq1[] = {4, 5, 6, 7};
  uint8_t seq2[] = {10, 11, 12, 13};
  uint8_t seq3[] = {14, 15}; /*TODO: these pins don't work with ADC3 */
  adc_set_injected_sequence(ADC1, 4, seq1);
  adc_set_injected_sequence(ADC2, 4, seq2);
  adc_set_injected_sequence(ADC3, 2, seq3);
}

static void platform_init_usart() {
  /* USART initialization */
  nvic_enable_irq(NVIC_USART2_IRQ);
  nvic_set_priority(NVIC_USART2_IRQ, 64);

  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO3);
  gpio_set_output_options(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_25MHZ, GPIO3);

  /* Setup USART2 TX and RX pin as alternate function. */
  gpio_set_af(GPIOA, GPIO_AF7, GPIO2);
  gpio_set_af(GPIOA, GPIO_AF7, GPIO3);

  /* Setup USART2 parameters. */
  usart_set_baudrate(USART2, config.console.baud);
  usart_set_databits(USART2, config.console.data_bits);
  switch (config.console.stop_bits) {
    case 1:
      usart_set_stopbits(USART2, USART_STOPBITS_1);
      break;
    default:
      while (1);
  }
  usart_set_mode(USART2, USART_MODE_TX_RX);
  switch (config.console.parity) {
    case 'N':
      usart_set_parity(USART2, USART_PARITY_NONE);
      break;
    case 'O':
      usart_set_parity(USART2, USART_PARITY_ODD);
      break;
    case 'E':
      usart_set_parity(USART2, USART_PARITY_EVEN);
      break;
  }
  usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

  /* Finally enable the USART. */
  usart_enable(USART2);
  usart_rx_reset();
}

void platform_init() {

  /* 168 Mhz clock */
  rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

  /* Enable clocks for subsystems */
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_GPIOC);
  rcc_periph_clock_enable(RCC_GPIOD);
  rcc_periph_clock_enable(RCC_GPIOE);
  rcc_periph_clock_enable(RCC_SYSCFG);
  rcc_periph_clock_enable(RCC_ADC1);
  rcc_periph_clock_enable(RCC_ADC2);
  rcc_periph_clock_enable(RCC_ADC3);
  rcc_periph_clock_enable(RCC_USART2);
  rcc_periph_clock_enable(RCC_DMA1);
  rcc_periph_clock_enable(RCC_DMA2);
  rcc_periph_clock_enable(RCC_TIM1);
  rcc_periph_clock_enable(RCC_TIM2);
  rcc_periph_clock_enable(RCC_TIM3);
  rcc_periph_clock_enable(RCC_TIM8);

  scb_set_priority_grouping(SCB_AIRCR_PRIGROUP_GROUP16_NOSUB);
  platform_init_scheduled_outputs();
  platform_init_eventtimer();
  platform_init_adc();
  platform_init_usart();
  platform_init_pwm();

  for (int i = 0; i < NUM_SENSORS; ++i) {
    if (config.sensors[i].method == SENSOR_FREQ) {
      platform_init_freqsensor(config.sensors[i].pin);
    }
    if (config.sensors[i].method == SENSOR_DIGITAL) {
      gpio_mode_setup(GPIOE, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, 
          (1 << config.sensors[i].pin));
    }
  }
  dwt_enable_cycle_counter();
}

void usart_rx_reset() {
  usart_rx_dest = config.console.rxbuffer;
  usart_rx_end = 0;
  usart_enable_rx_interrupt(USART2);
}


void usart2_isr() {
  if (usart_rx_end) {
    /* Already have unprocessed line in buffer */
    (void)usart_recv(USART2);
    return;
  }

  *usart_rx_dest = usart_recv(USART2);
  if (*usart_rx_dest == '\r') {
    *(usart_rx_dest + 1) = '\0';
    usart_rx_end = 1;
  }

  usart_rx_dest++;
  if (!usart_rx_end && 
      usart_rx_dest == config.console.rxbuffer + CONSOLE_BUFFER_SIZE - 1) {
    /* Buffer full */
    usart_rx_reset();
    return;
  }
}

int usart_tx_ready() {
  return usart_get_flag(USART2, USART_SR_TC);
}

int usart_rx_ready() {
  return usart_rx_end;
}

void usart_tx(char *buf, unsigned short len) {
  usart_enable_tx_dma(USART2);
  dma_stream_reset(DMA1, DMA_STREAM6);
  dma_set_priority(DMA1, DMA_STREAM6, DMA_SxCR_PL_LOW);
  dma_set_memory_size(DMA1, DMA_STREAM6, DMA_SxCR_MSIZE_8BIT);
  dma_set_peripheral_size(DMA1, DMA_STREAM6, DMA_SxCR_PSIZE_8BIT);
  dma_enable_memory_increment_mode(DMA1, DMA_STREAM6);
  dma_set_transfer_mode(DMA1, DMA_STREAM6, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
  dma_set_peripheral_address(DMA1, DMA_STREAM6, (uint32_t) &USART2_DR);
  dma_set_memory_address(DMA1, DMA_STREAM6, (uint32_t) buf);
  dma_set_number_of_data(DMA1, DMA_STREAM6, len);
  dma_enable_transfer_complete_interrupt(DMA1, DMA_STREAM6);
  dma_channel_select(DMA1, DMA_STREAM6, DMA_SxCR_CHSEL_4);
  dma_enable_stream(DMA1, DMA_STREAM6);

}

void adc_gather() {
  adc_start_conversion_injected(ADC1);
}

void tim2_isr() {
  if (timer_get_flag(TIM2, TIM_SR_CC1IF)) {
    timer_clear_flag(TIM2, TIM_SR_CC1IF);
    scheduler_callback_timer_execute();
  }
  if (timer_get_flag(TIM2, TIM_SR_CC2IF)) {
    timer_clear_flag(TIM2, TIM_SR_CC2IF);
    config.decoder.last_t0 = TIM2_CCR2;
    config.decoder.needs_decoding_t0 = 1;
  }
  if (timer_get_flag(TIM2, TIM_SR_CC3IF)) {
    timer_clear_flag(TIM2, TIM_SR_CC3IF);
    config.decoder.last_t1 = TIM2_CCR3;
    config.decoder.needs_decoding_t1 = 1;
  }
}


void adc_isr(void) {
  uint16_t adc_inputs[10];

  ADC1_SR &= ~(ADC_SR_JEOC);
  ADC2_SR &= ~(ADC_SR_JEOC);
  ADC3_SR &= ~(ADC_SR_JEOC);

  adc_inputs[0] = adc_read_injected(ADC1, 1);
  adc_inputs[1] = adc_read_injected(ADC1, 2);
  adc_inputs[2] = adc_read_injected(ADC1, 3);
  adc_inputs[3] = adc_read_injected(ADC1, 4);
  adc_inputs[4] = adc_read_injected(ADC2, 1);
  adc_inputs[5] = adc_read_injected(ADC2, 2);
  adc_inputs[6] = adc_read_injected(ADC2, 3);
  adc_inputs[7] = adc_read_injected(ADC2, 4);
  adc_inputs[8] = adc_read_injected(ADC2, 1);
  adc_inputs[9] = adc_read_injected(ADC2, 2);

  for (int i = 0; i < NUM_SENSORS; ++i) {
    if (config.sensors[i].method == SENSOR_ADC) {
      config.sensors[i].raw_value = 
        (uint32_t)adc_inputs[config.sensors[i].pin - 1];
    }
  }

  sensor_adc_new_data();
}

void dma2_stream1_isr(void) {
  if (dma_get_interrupt_flag(DMA2, DMA_STREAM1, DMA_TCIF)) {
      dma_clear_interrupt_flags(DMA2, DMA_STREAM1, DMA_TCIF);
    scheduler_buffer_swap();
  }
}

void enable_interrupts() {
  cm_enable_interrupts();
}

void disable_interrupts() {
  cm_disable_interrupts();
}

int interrupts_enabled() {
  return !cm_is_masked_interrupts();
}

void set_event_timer(timeval_t t) {
  timer_set_oc_value(TIM2, TIM_OC1, t);
  timer_enable_irq(TIM2, TIM_DIER_CC1IE);
}

timeval_t get_event_timer() {
  return TIM2_CCR1;
}

void clear_event_timer() {
  timer_clear_flag(TIM2, TIM_SR_CC1IF);
}

void disable_event_timer() {
  timer_disable_irq(TIM2, TIM_DIER_CC1IE);
  timer_clear_flag(TIM2, TIM_SR_CC1IF);
}

timeval_t current_time() {
  return timer_get_counter(TIM2);
}

int get_output(int output) {
  return gpio_get(GPIOD, (1 << output)) != 0;
}

void set_output(int output, char value) {
  if (value) {
    gpio_set(GPIOD, (1 << output));
  } else {
    gpio_clear(GPIOD, (1 << output));
  }
}

void set_gpio(int output, char value) {
  if (value) {
    gpio_set(GPIOE, (1 << output));
  } else {
    gpio_clear(GPIOE, (1 << output));
  }
}

void enable_test_trigger(trigger_type trig, unsigned int rpm) {
  if (trig != FORD_TFI) {
    return;
  }

  timeval_t t = time_from_rpm_diff(rpm, 45);

  /* Set up TIM5 as 32bit clock */
  rcc_periph_clock_enable(RCC_TIM5);
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO0);
  gpio_set_af(GPIOA, GPIO_AF2, GPIO0);
  timer_reset(TIM5);
  timer_disable_oc_output(TIM5, TIM_OC1);
  timer_set_mode(TIM5, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
  timer_set_period(TIM5, (unsigned int)t);
  timer_set_prescaler(TIM5, 20); /* Primary clock is still 168 Mhz */
  timer_disable_preload(TIM5);
  timer_continuous_mode(TIM5);
  /* Setup output compare registers */
  timer_ic_set_input(TIM5,  TIM_IC1, TIM_IC_OUT);
  timer_disable_oc_clear(TIM5, TIM_OC1);
  timer_disable_oc_preload(TIM5, TIM_OC1);
  timer_set_oc_slow_mode(TIM5, TIM_OC1);
  timer_set_oc_mode(TIM5, TIM_OC1, TIM_OCM_TOGGLE);
  timer_set_oc_value(TIM5, TIM_OC1, t);
  timer_set_oc_polarity_high(TIM5, TIM_OC1);
  timer_enable_oc_output(TIM5, TIM_OC1);
  timer_enable_counter(TIM5);
}

extern unsigned _configdata_loadaddr, _sconfigdata, _econfigdata;
void platform_load_config() {
  volatile unsigned *src, *dest;
  for (src = &_configdata_loadaddr, dest = &_sconfigdata;
      dest < &_econfigdata;
      src++, dest++) {
    *dest = *src;
  }
}

void platform_save_config() {
  volatile unsigned *src, *dest;
  int n_sectors, conf_bytes;
  flash_unlock();

  /* configrom is sectors 1 through 3, each 16k,
   * compute how many we need to erase */
  conf_bytes = ((char*)&_econfigdata - (char*)&_sconfigdata);
  n_sectors = (conf_bytes + 16385) / 16386;

  flash_erase_sector(n_sectors, 2);
  for (dest = &_configdata_loadaddr, src = &_sconfigdata;
      src < &_econfigdata;
      src++, dest++) {
    flash_program_word((uint32_t)dest, *src);
  }

  flash_lock();
}
