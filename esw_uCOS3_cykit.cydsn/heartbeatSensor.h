#ifndef HEARTBEAT_SENSOR_H
#define HEARTBEAT_SENSOR_H

#include "os.h"
#include "max30102_interface.h"
#include "bsp_i2c.h"
#include "Timer_1.h"
#include "log_task.h"
#include "data_processing_task.h"
#include "includes.h"

// Threshold and Timing Constants
#define INITIAL_THRESHOLD           1000000
#define DEBOUNCE_TIME_MS            400
#define MIN_PEAK_INTERVAL_MS 300
// Sizing and Filter Constants
#define MAX_HISTORY_SIZE            20
#define FIR_COEFF_SIZE              50
#define CIRCULAR_BUFFER_SIZE        50
#define POLLING_DELAY_MS            38

// Initial Values for Heart Rate Detection
#define INITIAL_LAST_PEAK_TIMESTAMP 0
#define INITIAL_IR_AC_SIGNAL_CURRENT 0
#define INITIAL_IR_AC_SIGNAL_PREVIOUS 0
#define INITIAL_IR_AVG_REG          0
#define INITIAL_HEART_RATE_SUM      0
#define INITIAL_HEART_RATE_INDEX    0

/* FIR Coefficients for Low Pass Filter */
static const CPU_INT16U FIRCoeffs[50] = {
    204, 261, 320, 379, 435, 486, 527, 555, 566, 557,
    525, 470, 391, 289, 167, 26, -128, -290, -456, -621,
    -778, -922, -1046, -1145, -1211, -1239, -1227, -1171, -1067, -917,
    -721, -483, -208, 88, 401, 721, 1039, 1345, 1626, 1870,
    2069, 2211, 2288, 2295, 2226, 2077, 1847, 1535, 1144, 681
};

extern CPU_INT32S Global_heart_rate;
extern CPU_INT32S Global_spO2;


// Function Prototypes
void MAX30102_Init(void);
static void max30102_Task(void *p_arg);
CPU_INT16S averageDCEstimator(CPU_INT32S *p, CPU_INT32U x);
CPU_INT16S lowPassFIRFilter(CPU_INT16S din);
CPU_INT32S mul16(CPU_INT16S x, CPU_INT16S y);

#endif /* HEARTBEAT_SENSOR_H */
