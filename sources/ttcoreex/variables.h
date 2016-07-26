#ifndef _VARIABLES_H_
#define _VARIABLES_H_ 
jmc_special_variable jmc_vars[JMC_SPECIAL_VARIABLES_NUM]={
		   {L"DATE",&variable_value_date},
		   {L"YEAR",&variable_value_year},
		   {L"MONTH",&variable_value_month},
		   {L"DAY",&variable_value_day},
		   {L"TIME",&variable_value_time},
		   {L"HOUR",&variable_value_hour},
		   {L"MINUTE",&variable_value_minute},
		   {L"SECOND",&variable_value_second},
		   {L"MILLISECOND",&variable_value_millisecond},
		   {L"TIMESTAMP",&variable_value_timestamp},
		   {L"CLOCK",&variable_value_clock},
		   {L"CLOCKMS",&variable_value_clockms},
		   
		   {L"INPUT",&variable_value_input},

		   {L"NOCOLOR", &variable_value_color_default},

		   {L"RANDOM", &variable_value_random},
		   
		   {L"HOSTNAME", &variable_value_hostname},
		   {L"HOSTIP", &variable_value_hostip},
		   {L"HOSTPORT", &variable_value_hostport},

		   {L"EOP", &variable_value_eop},
		   {L"EOL", &variable_value_eol},
		   {L"ESC", &variable_value_esc},

		   {L"PING", &variable_value_ping},
		   {L"PINGPROXY", &variable_value_ping_proxy},

		   {L"PRODUCTNAME", &variable_value_product_name},
		   {L"PRODUCTVERSION", &variable_value_product_version},

		   {L"COMMAND", &variable_value_command},

		   {L"FILENAME", &variable_value_filename}
	   };
#endif
