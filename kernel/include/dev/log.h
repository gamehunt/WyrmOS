#ifndef __K_DEV_LOG_H
#define __K_DEV_LOG_H 1

struct _fs_node;

enum LOG_LEVEL {
	PRINT,
	DEBUG,
	VERBOSE,
	INFO,
	WARNING,
	ERROR,
	CRITICAL
};

void             k_dev_log_init();
struct _fs_node* k_dev_log_get();
void             k_dev_log(enum LOG_LEVEL level, const char* format, ...);

#define k_print(...)   k_dev_log(PRINT, __VA_ARGS__)
#define k_debug(...)   k_dev_log(DEBUG, __VA_ARGS__)
#define k_verbose(...) k_dev_log(VERBOSE, __VA_ARGS__)
#define k_info(...)    k_dev_log(INFO, __VA_ARGS__)
#define k_warn(...)    k_dev_log(WARNING, __VA_ARGS__)
#define k_error(...)   k_dev_log(ERROR, __VA_ARGS__)
#define k_crit(...)    k_dev_log(CRITICAL, __VA_ARGS__)

#endif
