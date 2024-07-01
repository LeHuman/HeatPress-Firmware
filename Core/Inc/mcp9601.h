#pragma once

#include <stdint.h>

#include "main.h"

#define MCP9601_REG_THERMOCOUPLE_HOT_JUNCTION 0b00000000         // Thermocouple Hot-Junction register, TH
#define MCP9601_REG_JUNCTIONS_TEMPERATURE_DELTA 0b00000001       // Junctions Temperature Delta register, TΔ
#define MCP9601_REG_COLD_JUNCTION_TEMPERATURE 0b00000010         // Cold-Junction Temperature register, TC
#define MCP9601_REG_RAW_ADC_DATA 0b00000011                      // Raw ADC Data register
#define MCP9601_REG_STATUS 0b00000100                            // STATUS register
#define MCP9601_REG_THERMOCOUPLE_SENSOR_CONFIGURATION 0b00000101 // Thermocouple Sensor Configuration register
#define MCP9601_REG_DEVICE_CONFIGURATION 0b00000110              // Device Configuration register
#define MCP9601_REG_ALERT_1_CONFIGURATION 0b00001000             // Alert 1 Configuration register
#define MCP9601_REG_ALERT_2_CONFIGURATION 0b00001001             // Alert 2 Configuration register
#define MCP9601_REG_ALERT_3_CONFIGURATION 0b00001010             // Alert 3 Configuration register
#define MCP9601_REG_ALERT_4_CONFIGURATION 0b00001011             // Alert 4 Configuration register
#define MCP9601_REG_ALERT_1_HYSTERESIS 0b00001100                // Alert 1 Hysteresis register, THYST1
#define MCP9601_REG_ALERT_2_HYSTERESIS 0b00001101                // Alert 2 Hysteresis register, THYST2
#define MCP9601_REG_ALERT_3_HYSTERESIS 0b00001110                // Alert 3 Hysteresis register, THYST3
#define MCP9601_REG_ALERT_4_HYSTERESIS 0b00001111                // Alert 4 Hysteresis register, THYST4
#define MCP9601_REG_TEMPERATURE_ALERT_1_LIMIT 0b00010000         // Temperature Alert 1 Limit register, TALERT1
#define MCP9601_REG_TEMPERATURE_ALERT_2_LIMIT 0b00010001         // Temperature Alert 2 Limit register, TALERT2
#define MCP9601_REG_TEMPERATURE_ALERT_3_LIMIT 0b00010010         // Temperature Alert 3 Limit register, TALERT3
#define MCP9601_REG_TEMPERATURE_ALERT_4_LIMIT 0b00010011         // Temperature Alert 4 Limit register, TALERT4
#define MCP9601_REG_DEVICE_ID_REVISION 0b00100000                // Device ID/Revision register

typedef enum {
    amb_res_0_0625 = 0,
    amb_res_0_25 = 1,
} MCP9601_CONF_AMB_RES;

typedef enum {
    adc_res_18 = 0b00,
    adc_res_16 = 0b01,
    adc_res_14 = 0b10,
    adc_res_12 = 0b11,
} MCP9601_CONF_ADC_RES;

typedef enum {
    burst_s_1 = 0b000,
    burst_s_2 = 0b001,
    burst_s_4 = 0b010,
    burst_s_8 = 0b011,
    burst_s_16 = 0b100,
    burst_s_32 = 0b101,
    burst_s_64 = 0b110,
    burst_s_128 = 0b111,
} MCP9601_CONF_BURST_SAMP;

typedef enum {
    mode_normal = 0b00,
    mode_shutdown = 0b01,
    mode_burst = 0b10,
    // mode_unimplemented = 0b11,
} MCP9601_CONF_MODE;

typedef enum {
    type_k = 0b000,
    type_j = 0b001,
    type_t = 0b010,
    type_n = 0b011,
    type_s = 0b100,
    type_e = 0b101,
    type_b = 0b110,
    type_r = 0b111,
} MCP9601_SENSE_CONF_TYPE;

/**
 *
 * Y = k * X + (1 - k) * Y_-1
 * k = 2 / (2^n + 1)
 *
 * Where:
 *  Y = New filtered temperature in TΔ
 *  X = Current, unfiltered hot-junction
 *  temperatures
 *  Y_-1 = Previous filtered temperature
 *  n = User-selectable filter coefficient
 */
typedef enum {
    filter_0 = 0b000,
    filter_1 = 0b001,
    filter_2 = 0b010,
    filter_3 = 0b011,
    filter_4 = 0b100,
    filter_5 = 0b101,
    filter_6 = 0b110,
    filter_7 = 0b111,
} MCP9601_SENSE_CONF_FILTER;

typedef struct MCP9601 {
    const char *name;
    uint16_t address;
    I2C_HandleTypeDef *handle;

    uint8_t failed;
    uint16_t read;

    union MCP9601_TEMP {
        uint8_t buf[2];
        uint16_t all;
        // struct MCP9601_TEMP_VAL {
        //     uint8_t f : 4;
        //     int16_t i : 12;
        // } val;
    } temp;

    union MCP9601_STATUS {
        uint8_t buf[2];
        struct MCP9601_STATUS_VAL {
            uint8_t alert_1 : 1;
            uint8_t alert_2 : 1;
            uint8_t alert_3 : 1;
            uint8_t alert_4 : 1;
            uint8_t open_circuit : 1;
            uint8_t short_circuit : 1;
            uint8_t th_update : 1;
            uint8_t burst_complete : 1;
        } val;
    } status;

    union MCP9601_CONF {
        uint8_t buf[2];

        struct MCP9601_CONF_VAL {
            MCP9601_CONF_MODE shutdown_mode : 2;
            MCP9601_CONF_BURST_SAMP burst_mode_temperature_samples : 3;
            MCP9601_CONF_ADC_RES adc_measurement_resolution : 2;
            MCP9601_CONF_AMB_RES ambient_sensor_resolution : 1;
        } val;
    } config;

    union MCP9601_SENSE_CONF {
        uint8_t buf[2];

        struct MCP9601_SENSE_CONF_VAL {
            MCP9601_SENSE_CONF_FILTER filter_coefficient : 3;
            uint8_t nil_1 : 1;
            MCP9601_SENSE_CONF_TYPE thermocouple_type : 3;
            uint8_t nil_0 : 1;
        } val;
    } sensor_config;
} MCP9601;

MCP9601 *mcp9601_new(I2C_HandleTypeDef *hi2c, const char *name, uint16_t address);
MCP9601 *mcp9601_new_common(I2C_HandleTypeDef *hi2c, const char *name, uint16_t address, MCP9601_CONF_ADC_RES adc_res, MCP9601_CONF_AMB_RES amb_res, MCP9601_SENSE_CONF_FILTER filter, MCP9601_SENSE_CONF_TYPE type);
void mcp9601_free(MCP9601 *probe);

void mcp9601_init(MCP9601 *probe);
void mcp9601_poll(MCP9601 *probe);
void mcp9601_dma_loop(MCP9601 **probes, int len, int *counter);
const char *mcp9601_err_str(MCP9601 *probe);

float mcp9601_temperature(MCP9601 *therm);
