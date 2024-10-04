#include "debug.h"
#include "fs/fs.h"
#include "globals.h"
#include "proc/spinlock.h"
#include "types/list.h"
#include <stdarg.h>
#include <dev/log.h>
#include <stdio.h>
#include <string.h>

#define LOG_BUFFER_SIZE 1024

lock __log_global_lock = EMPTY_LOCK;

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

list* __listeners = NULL;

void k_dev_log_subscribe(log_listener l) {
    list_push_back(__listeners, l);
}

static size_t __k_log_write(fs_node* logdev, size_t offset, size_t size, uint8_t* buffer) {
	for(size_t i = 0; i < size; i++) {
		DEBUG_PUTCHAR(buffer[i]);
        if(__listeners) {
            foreach(l, __listeners) {
                ((log_listener)(l->value))(buffer[i]);
            }
        }
	}
	return size;
}

void k_dev_log_init() {
	__k_logdev = k_fs_alloc_fsnode("log");
	__k_logdev->ops.write = __k_log_write;
    __listeners = list_create();
	k_fs_mount_node("/dev/log", __k_logdev);
	__k_early  = 0;	
}

static char buffer1[LOG_BUFFER_SIZE] = {0};
static char buffer2[LOG_BUFFER_SIZE] = {0};

void k_dev_log(enum LOG_LEVEL level, const char* prefix, const char* format, ...) {
    LOCK(__log_global_lock);

    memset(buffer1, 0, LOG_BUFFER_SIZE);
    memset(buffer2, 0, LOG_BUFFER_SIZE);

	va_list args;
	va_start(args, format);

	vsnprintf(buffer1, LOG_BUFFER_SIZE, format, args);
	size_t bytes = 0;

    if(prefix[0] != '\0') {
        bytes = snprintf(buffer2, LOG_BUFFER_SIZE, "%s[%s] %s%s", __log_prefixes[level], prefix, buffer1, (level == PRINT ? "" : "\r\n"));
    } else {
        bytes = snprintf(buffer2, LOG_BUFFER_SIZE, "%s%s%s", __log_prefixes[level], buffer1, (level == PRINT ? "" : "\r\n"));
    }

	va_end(args);

	size_t(* __print_callback)(fs_node*, size_t, size_t, uint8_t*) = NULL;

	if(__k_early) {
		__print_callback = __k_log_write;
	} else {
		__print_callback = k_fs_write;
	}

	__print_callback(__k_logdev, 0, bytes, (uint8_t*) buffer2);

    UNLOCK(__log_global_lock);
}

EXPORT(k_dev_log)
