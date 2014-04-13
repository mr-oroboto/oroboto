/**
 * adclib.cpp
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2014
 *
 * Trivial abstraction to simplify use of BeagleBone ADC.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "adclib.h"
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
    char         ctrl_file[1024];
    char         adc_val[16];
    unsigned int bytes_read;

    switch (adc)
    {
        case 0:
            snprintf(ctrl_file, sizeof(ctrl_file), "%s%s", ADC_DIR_PREFIX, ADC_0_DIR);
            break;
        case 1:
            snprintf(ctrl_file, sizeof(ctrl_file), "%s%s", ADC_DIR_PREFIX, ADC_1_DIR);
            break;
        case 2:
            snprintf(ctrl_file, sizeof(ctrl_file), "%s%s", ADC_DIR_PREFIX, ADC_2_DIR);
            break;
        case 3:
            snprintf(ctrl_file, sizeof(ctrl_file), "%s%s", ADC_DIR_PREFIX, ADC_3_DIR);
            break;
        case 4:
            snprintf(ctrl_file, sizeof(ctrl_file), "%s%s", ADC_DIR_PREFIX, ADC_4_DIR);
            break;
        case 5:
            snprintf(ctrl_file, sizeof(ctrl_file), "%s%s", ADC_DIR_PREFIX, ADC_5_DIR);
            break;
        case 6:
            snprintf(ctrl_file, sizeof(ctrl_file), "%s%s", ADC_DIR_PREFIX, ADC_6_DIR);
            break;
        case 7:
            snprintf(ctrl_file, sizeof(ctrl_file), "%s%s", ADC_DIR_PREFIX, ADC_7_DIR);
            break;

        default:
            fprintf(stderr, "adc_get_value: unknown ADC\n");
            return -1;
    }

    memset(adc_val, 0, sizeof(adc_val));

    bytes_read = sysfs_read(ctrl_file, adc_val, sizeof(adc_val));

    if (bytes_read < 0)
    {
        fprintf(stderr, "adc_get_value: failed to read ADC sysfs file\n");
        return -1;
    }

    if (bytes_read >= sizeof(adc_val))
    {
        fprintf(stderr, "adc_get_value: overflow\n");
        return -1;
    }
    else
    {
        adc_val[bytes_read+1] = '\0';
    }

    return atoi(adc_val);
}

/**
 * Read the value from an ADC by taking several samples and using a median filter to choose the sample to return.
 *
 * @param int adc               the ADC channel to read
 * @param int samples_requested the number of samples to take
 *
 * @return int
 */
int adc_sample(int adc, int samples_requested)
{
    char ctrl_file[1024];
    char adc_val[16];

    int          fd, i, samples_stored;
    int *        sample_buf;
    unsigned int bytes_read;

    switch (adc)
    {
        case 0:
            snprintf(ctrl_file, sizeof(ctrl_file), "%s%s", ADC_DIR_PREFIX, ADC_0_DIR);
            break;
        case 1:
            snprintf(ctrl_file, sizeof(ctrl_file), "%s%s", ADC_DIR_PREFIX, ADC_1_DIR);
            break;
        case 2:
            snprintf(ctrl_file, sizeof(ctrl_file), "%s%s", ADC_DIR_PREFIX, ADC_2_DIR);
            break;
        case 3:
            snprintf(ctrl_file, sizeof(ctrl_file), "%s%s", ADC_DIR_PREFIX, ADC_3_DIR);
            break;
        case 4:
            snprintf(ctrl_file, sizeof(ctrl_file), "%s%s", ADC_DIR_PREFIX, ADC_4_DIR);
            break;
        case 5:
            snprintf(ctrl_file, sizeof(ctrl_file), "%s%s", ADC_DIR_PREFIX, ADC_5_DIR);
            break;
        case 6:
            snprintf(ctrl_file, sizeof(ctrl_file), "%s%s", ADC_DIR_PREFIX, ADC_6_DIR);
            break;
        case 7:
            snprintf(ctrl_file, sizeof(ctrl_file), "%s%s", ADC_DIR_PREFIX, ADC_7_DIR);
            break;

        default:
            fprintf(stderr, "adc_sample: unknown ADC\n");
            return -1;
    }

    if ((fd = sysfs_open_read(ctrl_file, O_RDONLY)) < 0)
    {
        fprintf(stderr, "adc_sample: Failed to open ADC sysfs control file\n");
        return -1;
    }

    // Allocate the sample buffer
    sample_buf = (int*)malloc(samples_requested * sizeof(int));
    if (sample_buf == NULL)
    {
        fprintf(stderr, "adc_sample: Failed to allocate sample buffer, out of memory?\n");
        sysfs_close(fd);
        return -1;
    }

    memset(sample_buf, 0, samples_requested * sizeof(int));

    for (i = 0, samples_stored = 0; samples_stored < samples_requested && (i < samples_requested * 2); i++)
    {
        usleep(1000);  // 1ms
        memset(adc_val, 0, sizeof(adc_val));

        lseek(fd, 0, SEEK_SET);

        if ((bytes_read = read(fd, adc_val, sizeof(adc_val))) <= 0)
        {
            fprintf(stderr, "adc_sample: Failed to take sample, trying again\n");
            continue;
        }

        if (bytes_read >= sizeof(adc_val))
        {
            fprintf(stderr, "adc_sample: overflow, trying again\n");
            continue;
        }

        adc_val[bytes_read+1] = '\0';
        sample_buf[samples_stored] = atoi(adc_val);

        samples_stored++;
    }

    // sort to find outliers and select median
    qsort(sample_buf, samples_stored, sizeof(int), adc_compare);

    i = sample_buf[samples_stored / 2];

    free(sample_buf);
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

