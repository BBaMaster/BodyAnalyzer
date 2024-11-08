#ifndef HEARTBEAT_SENSOR_H
#define HEARTBEAT_SENSOR_H

#include "os.h"
#include "max30102_interface.h"
#include "bsp_i2c.h"
#include "Timer_1.h"
#include "log_task.h"

// Threshold and Timing Constants
#define INITIAL_THRESHOLD           1500000
#define DEBOUNCE_TIME_MS            400

// Sizing and Filter Constants
#define MAX_HISTORY_SIZE            20
#define FIR_COEFF_SIZE              12
#define CIRCULAR_BUFFER_SIZE        32
#define POLLING_DELAY_MS            38

// Initial Values for Heart Rate Detection
#define INITIAL_IR_AC_MAX           20
#define INITIAL_IR_AC_MIN           -20
#define INITIAL_LAST_PEAK_TIMESTAMP 0
#define INITIAL_IR_AC_SIGNAL_CURRENT 0
#define INITIAL_IR_AC_SIGNAL_PREVIOUS 0
#define INITIAL_IR_AVG_REG          0
#define INITIAL_HEART_RATE_SUM      0
#define INITIAL_HEART_RATE_INDEX    0

// External variable declarations
extern volatile CPU_INT32U current_timestamp;
extern CPU_INT32U last_peak_timestamp;
extern CPU_INT32S IR_AC_Max;
extern CPU_INT32S IR_AC_Min;
extern CPU_INT32S IR_AC_Signal_Current;
extern CPU_INT32S IR_AC_Signal_Previous;
extern CPU_INT32S ir_avg_reg;
extern CPU_INT32S heart_rate_sum;
extern CPU_INT32U heart_rate_index;

// Function Prototypes
void MAX30102_Init(void);
static void max30102_Task(void *p_arg);
CPU_INT16S averageDCEstimator(CPU_INT32S *p, CPU_INT32U x);
CPU_INT16S lowPassFIRFilter(CPU_INT16S din);
CPU_INT32S mul16(CPU_INT16S x, CPU_INT16S y);

#endif /* HEARTBEAT_SENSOR_H */
