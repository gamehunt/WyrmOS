#include "debug.h"
#include "fs/fs.h"
#include "globals.h"
#include <stdarg.h>
#include <dev/log.h>
#include <stdio.h>
#include <string.h>

#define LOG_BUFFER_SIZE 1024

static fs_node* __k_logdev = NULL;
const char* __log_prefixes[] = {
	[PRINT]    = "",
	[DEBUG]    = "[D] ",
	[VERBOSE]  = "[V] ",
	[INFO]     = "[I] ",
	[WARNING]  = "[W] ",
	[ERROR]    = "[E] ",
	[CRITICAL] = "[E]! "
};

static size_t __k_log_write(fs_node* logdev, size_t offset, size_t size, uint8_t* buffer) {
	for(size_t i = 0; i < size; i++) {
		DEBUG_PUTCHAR(buffer[i]);
	}
	return size;
}

void k_dev_log_init() {
	__k_logdev = k_fs_alloc_fsnode("log");
	__k_logdev->ops.write = __k_log_write;
	k_fs_mount_node("/dev/log", __k_logdev);
	__k_early  = 0;	
}

void k_dev_log(enum LOG_LEVEL level, const char* format, ...) {
	char buffer1[LOG_BUFFER_SIZE] = {0};
	char buffer2[LOG_BUFFER_SIZE] = {0};

	va_list args;
	va_start(args, format);

	vsnprintf(buffer1, LOG_BUFFER_SIZE, format, args);
	size_t bytes = snprintf(buffer2, LOG_BUFFER_SIZE, "%s%s%s", __log_prefixes[level], buffer1, (level == PRINT ? "" : "\r\n"));

	va_end(args);

	size_t(* __print_callback)(fs_node*, size_t, size_t, uint8_t*) = NULL;

	if(__k_early) {
		__print_callback = __k_log_write;
	} else {
		__print_callback = k_fs_write;
	}

	__print_callback(__k_logdev, 0, bytes, (uint8_t*) buffer2);
}

EXPORT(k_dev_log)
