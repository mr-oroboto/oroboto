/**
 * adclib.cpp
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2014
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "adclib.h"
#include "logger.h"
#include "sysfslib.h"

/**
 * Initialise the ADC subsystem.
 *
 * @return int
 */
int adc_init()
{
    return sysfs_write(SLOTS_FILE, ADC_DRIVER);
}

/**
 * Read the value from an ADC.
 *
 * It is recommended to use adc_sample() in preference to this as the ADC values can sometimes be erroneous.
 *
 * @see adc_sample()
 *
 * @param int adc      the ADC channel to read
 *
 * @return int
 */
int adc_get_value(int adc)
{
    char    ctrlFile[1024];
    char    adcVal[16];
    int 	bytesRead;

    switch (adc)
    {
        case 0:
            snprintf(ctrlFile, sizeof(ctrlFile), "%s%s", ADC_DIR_PREFIX, ADC_0_DIR);
            break;
        case 1:
            snprintf(ctrlFile, sizeof(ctrlFile), "%s%s", ADC_DIR_PREFIX, ADC_1_DIR);
            break;
        case 2:
            snprintf(ctrlFile, sizeof(ctrlFile), "%s%s", ADC_DIR_PREFIX, ADC_2_DIR);
            break;
        case 3:
            snprintf(ctrlFile, sizeof(ctrlFile), "%s%s", ADC_DIR_PREFIX, ADC_3_DIR);
            break;
        case 4:
            snprintf(ctrlFile, sizeof(ctrlFile), "%s%s", ADC_DIR_PREFIX, ADC_4_DIR);
            break;
        case 5:
            snprintf(ctrlFile, sizeof(ctrlFile), "%s%s", ADC_DIR_PREFIX, ADC_5_DIR);
            break;
        case 6:
            snprintf(ctrlFile, sizeof(ctrlFile), "%s%s", ADC_DIR_PREFIX, ADC_6_DIR);
            break;
        case 7:
            snprintf(ctrlFile, sizeof(ctrlFile), "%s%s", ADC_DIR_PREFIX, ADC_7_DIR);
            break;

        default:
        	Logger::getInstance()->error("adc::adc_get_value: unknown ADC");
            return -1;
    }

    memset(adcVal, 0, sizeof(adcVal));

    bytesRead = sysfs_read(ctrlFile, adcVal, sizeof(adcVal));

    if (bytesRead < 0)
    {
    	Logger::getInstance()->error("adc::adc_get_value: failed to read ADC sysfs file");
        return -1;
    }

    if (bytesRead >= sizeof(adcVal))
    {
    	Logger::getInstance()->error("adc::adc_get_value: overflow");
        return -1;
    }
    else
    {
    	adcVal[bytesRead+1] = '\0';
    }

    return atoi(adcVal);
}

/**
 * Read the value from an ADC by taking several samples and using a median filter to choose the sample to return.
 *
 * @param int adc               the ADC channel to read
 * @param int samples_requested the number of samples to take
 *
 * @return int
 */
int adc_sample(int adc, int samplesRequested)
{
    char ctrlFile[1024];
    char adcVal[16];

    int          fd, i, samplesStored;
    int *        sampleBuf;
    unsigned int bytesRead;

    switch (adc)
    {
        case 0:
            snprintf(ctrlFile, sizeof(ctrlFile), "%s%s", ADC_DIR_PREFIX, ADC_0_DIR);
            break;
        case 1:
            snprintf(ctrlFile, sizeof(ctrlFile), "%s%s", ADC_DIR_PREFIX, ADC_1_DIR);
            break;
        case 2:
            snprintf(ctrlFile, sizeof(ctrlFile), "%s%s", ADC_DIR_PREFIX, ADC_2_DIR);
            break;
        case 3:
            snprintf(ctrlFile, sizeof(ctrlFile), "%s%s", ADC_DIR_PREFIX, ADC_3_DIR);
            break;
        case 4:
            snprintf(ctrlFile, sizeof(ctrlFile), "%s%s", ADC_DIR_PREFIX, ADC_4_DIR);
            break;
        case 5:
            snprintf(ctrlFile, sizeof(ctrlFile), "%s%s", ADC_DIR_PREFIX, ADC_5_DIR);
            break;
        case 6:
            snprintf(ctrlFile, sizeof(ctrlFile), "%s%s", ADC_DIR_PREFIX, ADC_6_DIR);
            break;
        case 7:
            snprintf(ctrlFile, sizeof(ctrlFile), "%s%s", ADC_DIR_PREFIX, ADC_7_DIR);
            break;

        default:
        	Logger::getInstance()->error("adc::adc_sample: unknown ADC");
            return -1;
    }

    if ((fd = sysfs_open_read(ctrlFile, O_RDONLY)) < 0)
    {
    	Logger::getInstance()->error("adc::adc_sample: failed to open ADC sysfs control file");
        return -1;
    }

    // Allocate the sample buffer
    sampleBuf = (int*)malloc(samplesRequested * sizeof(int));
    if (sampleBuf == NULL)
    {
    	Logger::getInstance()->error("adc::adc_sample: failed to allocate sample buffer, out of memory?");
        sysfs_close(fd);
        return -1;
    }

    memset(sampleBuf, 0, samplesRequested * sizeof(int));

    for (i = 0, samplesStored = 0; samplesStored < samplesRequested && (i < samplesRequested * 2); i++)
    {
        usleep(1000);  // 1ms
        memset(adcVal, 0, sizeof(adcVal));

        lseek(fd, 0, SEEK_SET);

        if ((bytesRead = read(fd, adcVal, sizeof(adcVal))) <= 0)
        {
        	Logger::getInstance()->debug("adc::adc_sample: failed to take sample, trying again");
            continue;
        }

        if (bytesRead >= sizeof(adcVal))
        {
        	Logger::getInstance()->notice("adc::adc_sample: overflow, trying again");
            continue;
        }

        adcVal[bytesRead+1] = '\0';
        sampleBuf[samplesStored] = atoi(adcVal);

        samplesStored++;
    }

    // sort to find outliers and select median
    qsort(sampleBuf, samplesStored, sizeof(int), adc_compare);

    i = sampleBuf[samplesStored / 2];

    free(sampleBuf);
    sysfs_close(fd);

    return i;
}

/**
 * QuickSort comparison function.
 *
 * @param void *
 * @param void *
 *
 * @return int
 */
int adc_compare(const void *a, const void *b)
{
    return (*(int*)a - *(int*)b);
}

