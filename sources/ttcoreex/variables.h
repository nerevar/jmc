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
		   {"CLOCK",&variable_value_clock},
		   {"CLOCKMS",&variable_value_clockms},
		   
		   {"INPUT",&variable_value_input},

		   {"NOCOLOR", &variable_value_color_default},

		   {"RANDOM", &variable_value_random},
		   
		   {"HOSTNAME", &variable_value_hostname},
		   {"HOSTIP", &variable_value_hostip},
		   {"HOSTPORT", &variable_value_hostport},

		   {"EOP", &variable_value_eop},
		   {"EOL", &variable_value_eol},
		   {"ESC", &variable_value_esc},

		   {"PING", &variable_value_ping},
		   {"PINGPROXY", &variable_value_ping_proxy}
	   };
#endif
