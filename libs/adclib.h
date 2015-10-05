/**
 * adclib.h
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2014
 *
 * Trivial abstraction to simplify use of BeagleBone ADC.
 *
 * You may need to change the following constants depending on your BBB:
 *
 * ADC_DIR_PREFIX
 */

#ifndef _ADCLIB_H_INCLUDED
#define _ADCLIB_H_INCLUDED

#define ADC_DRIVER          "cape-bone-iio"

#define ADC_DIR_PREFIX      "/sys/devices/ocp.2/helper.14/"       // trailing slash is required, the '2' and '14' may vary depending on your BBB
#define ADC_0_DIR           "AIN0"
#define ADC_1_DIR           "AIN1"
#define ADC_2_DIR           "AIN2"
#define ADC_3_DIR           "AIN3"
#define ADC_4_DIR           "AIN4"
#define ADC_5_DIR           "AIN5"
#define ADC_6_DIR           "AIN6"
#define ADC_7_DIR           "AIN7"

int adc_init();
int adc_get_value(int adc);
int adc_sample(int adc, int samplesRequested);
int adc_compare(const void *a, const void *b);

#endif // _ADCLIB_H_INCLUDED
