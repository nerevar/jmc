#ifndef _VARIABLES_H_
#define _VARIABLES_H_ 
jmc_special_variable jmc_vars[JMC_SPECIAL_VARIABLES_NUM]={
		   {"DATE",&variable_value_date},
		   {"YEAR",&variable_value_year},
		   {"MONTH",&variable_value_month},
		   {"DAY",&variable_value_day},
		   {"TIME",&variable_value_time},
		   {"HOUR",&variable_value_hour},
		   {"MINUTE",&variable_value_minute},
		   {"SECOND",&variable_value_second},
		   {"MILLISECOND",&variable_value_millisecond},
		   {"TIMESTAMP",&variable_value_timestamp},
		   
		   {"INPUT",&variable_value_input},

		   {"NOCOLOR", &variable_value_color_default}
	   };
#endif
