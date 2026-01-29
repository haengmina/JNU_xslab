#include <use_sensor.h>
#include <cmath>

static const float TEMP_VDD_MV = 3300.0f;
static const float PULL_DOWN_R = 1000.0f;
static const float R0 = 10000.0f;
static const float B_K = 3435.0f;
static const float T0 = 298.15f;

static const uint32_t TEMP_UPDATE_MS = 1000;
static const uint32_t HUM_UPDATE_MS = 1000;

static bool hum_state = false;
static bool temp_state = false;

static float T_mV = 0.0f;
static float H_mV = 0.0f;

static float AM1011A_temp(int v1)
{
    T_mV = Sensor_Interface_Board::get_Voltage_mV(v1);

    if (T_mV <= 0.0f || T_mV >= TEMP_VDD_MV) {
        return 0.0f;
    }

    float r_ntc = PULL_DOWN_R * (TEMP_VDD_MV / T_mV - 1.0f);
    if (r_ntc <= 0.0f) {
        return 0.0f;
    }

    float inv_T = 1.0f / T0 + (1.0f / B_K) * logf(r_ntc / R0);

    float T_K = 1.0f / inv_T;
    float T_C = T_K - 273.15f;

    return T_C;
}

void use_sensor(int v1, float t, int v2, float h)
{
    uint32_t now = HAL_GetTick();
    static uint32_t last_ms = 0;

    if (now - last_ms < TEMP_UPDATE_MS) {
        return;
    }
    last_ms = now;

    float sc_temp = AM1011A_temp(v1);

    if (sc_temp >= t) {
        temp_state = true;
    } else {
        temp_state = false;
    }

    H_mV = Sensor_Interface_Board::get_Voltage_mV(v2);

    float sc_hum = H_mV * 100.0f / 3000.0f;

    if (sc_hum >= h) {
        hum_state = true;
    } else {
        hum_state = false;
    }

    if (hum_state || temp_state) {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
    }
}
