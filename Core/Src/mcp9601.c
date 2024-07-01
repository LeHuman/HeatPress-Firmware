#include "mcp9601.h"

#include <memory.h>
#include <stdlib.h>

MCP9601 *mcp9601_new(I2C_HandleTypeDef *hi2c, const char *name, uint16_t address) {
    MCP9601 *probe = calloc(1, sizeof(MCP9601));
    probe->name = name;
    probe->handle = hi2c;
    probe->address = address;
    return probe;
}

MCP9601 *mcp9601_new_common(I2C_HandleTypeDef *hi2c, const char *name, uint16_t address, MCP9601_CONF_ADC_RES adc_res, MCP9601_CONF_AMB_RES amb_res, MCP9601_SENSE_CONF_FILTER filter, MCP9601_SENSE_CONF_TYPE type) {
    MCP9601 *probe = mcp9601_new(hi2c, name, address);
    probe->config.val.adc_measurement_resolution = adc_res;
    probe->config.val.ambient_sensor_resolution = amb_res;
    probe->sensor_config.val.filter_coefficient = filter;
    probe->sensor_config.val.thermocouple_type = type;
    return probe;
}

void mcp9601_free(MCP9601 *probe) {
    free(probe);
}

void mcp9601_init(MCP9601 *probe) {
    while (HAL_I2C_GetState(probe->handle) != HAL_I2C_STATE_READY) {
    }
    HAL_StatusTypeDef result = HAL_I2C_Mem_Write(probe->handle, probe->address, MCP9601_REG_DEVICE_CONFIGURATION, I2C_MEMADD_SIZE_8BIT, (uint8_t *)(probe->config.buf), 2, 10000);
    result |= HAL_I2C_Mem_Write(probe->handle, probe->address, MCP9601_REG_THERMOCOUPLE_SENSOR_CONFIGURATION, I2C_MEMADD_SIZE_8BIT, (uint8_t *)(probe->sensor_config.buf), 1, 10000);
    probe->failed = result;
}

void mcp9601_poll(MCP9601 *probe) {
    while (HAL_I2C_GetState(probe->handle) != HAL_I2C_STATE_READY) {
    }
    HAL_StatusTypeDef result = HAL_I2C_Mem_Read(probe->handle, probe->address, MCP9601_REG_THERMOCOUPLE_HOT_JUNCTION, I2C_MEMADD_SIZE_8BIT, (uint8_t *)(&probe->temp.buf), 2, 1000);
    result |= HAL_I2C_Mem_Read(probe->handle, probe->address, MCP9601_REG_STATUS, I2C_MEMADD_SIZE_8BIT, (uint8_t *)(&probe->status.buf), 1, 1000);
    result |= probe->status.val.open_circuit | probe->status.val.short_circuit;
    probe->failed = result;
}

void mcp9601_dma_loop(MCP9601 **probes, int len, int *counter) {
    *counter %= len;
    MCP9601 *probe = probes[*counter];
    if (probe && HAL_I2C_GetState(probe->handle) == HAL_I2C_STATE_READY) {
        HAL_StatusTypeDef result;
        switch (probe->read) {
            case MCP9601_REG_THERMOCOUPLE_HOT_JUNCTION:
                result = HAL_I2C_Mem_Read_DMA(probe->handle, probe->address, MCP9601_REG_THERMOCOUPLE_HOT_JUNCTION, I2C_MEMADD_SIZE_8BIT, (uint8_t *)(&probe->temp.buf), 2);
                probe->read = MCP9601_REG_STATUS;
                break;
            case MCP9601_REG_STATUS:
                result = HAL_I2C_Mem_Read_DMA(probe->handle, probe->address, MCP9601_REG_STATUS, I2C_MEMADD_SIZE_8BIT, (uint8_t *)(&probe->status.buf), 1);
                probe->read = MCP9601_REG_THERMOCOUPLE_HOT_JUNCTION;
                break;
        }
        probe->failed = result | probe->status.val.open_circuit | probe->status.val.short_circuit;
        ;
        *counter += 1;
    } else if (probe == NULL) {
        *counter += 1;
    }
}

const char *mcp9601_err_str(MCP9601 *probe) {
    int status = probe->status.val.open_circuit;
    status |= probe->status.val.short_circuit << 1;
    switch (status) {
        case 0:
            return "NO_ERR";
        case 1:
            return "OPEN_C";
        case 2:
            return "SHORTC";
        default:
            return "UNKOWN";
    }
}

float mcp9601_temperature(MCP9601 *therm) {
    return (therm->temp.buf[0] * 16.0f) + (therm->temp.buf[1] / 16.0f);
}