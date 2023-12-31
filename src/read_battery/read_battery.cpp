#include <esp_adc_cal.h>
#include "read_battery.h"

float read_battery() {
    uint32_t value = 0;
    int rounds = 11;
    esp_adc_cal_characteristics_t adc_chars;

    // battery voltage divided by 2 can be measured at GPIO34, which equals ADC1_CHANNEL6
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    // switch (esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars)) {
    //     case ESP_ADC_CAL_VAL_EFUSE_TP:
    //         Serial.println("Characterized using Two Point Value");
    //         break;
    //     case ESP_ADC_CAL_VAL_EFUSE_VREF:
    //         Serial.printf("Characterized using eFuse Vref (%d mV)\r\n", adc_chars.vref);
    //         break;
    //     default:
    //         Serial.printf("Characterized using Default Vref (%d mV)\r\n", 1100);
    // }

    // to avoid noise, sample the pin several times and average the result
    for (int i = 1; i <= rounds; i++) {
        value += adc1_get_raw(ADC1_CHANNEL_6);
    }
    value /= (uint32_t)rounds;

    // due to the voltage divider (1M+1M) values must be multiplied by 2
    // and convert mV to V
    return esp_adc_cal_raw_to_voltage(value, &adc_chars) * 2.0 / 1000.0;
}