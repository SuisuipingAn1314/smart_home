#include "main.h"
#include "adc.h"
#include "light_sensor.h"

void Light_Init(void)
{
	
}

uint16_t Light_Get(uint16_t num)
{
	uint16_t light_val = 0;
	
	if(num == 1)
	{
		HAL_ADC_Start(&hadc1);
		
		HAL_ADC_PollForConversion(&hadc1, 1000);
		
		light_val = 4095 - HAL_ADC_GetValue(&hadc1);
		
		HAL_ADC_Stop(&hadc1);
	}
	
	return light_val;
}
