#include "log.h"

t_log* completar_logger(char* mensaje,char* programa, t_log_level log_level)
{
	t_log* logger;
	//logger = log_create(config_get_string_value(t_config*, char LOG_FILE),programa, 1, log_level);
	log_info(logger,mensaje);
	return logger;
}
