/*
 * inits - A lightweight UNIX init system
 * Copyright (C) 2025
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#define _POSIX_C_SOURCE 200809L

#include "logging.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

#define LOG_FILE_PATH "/var/log/inits.log"

static FILE *log_file = NULL;

/**
 * Get current timestamp in format [YYYY-MM-DD HH:MM:SS]
 * Returns pointer to static buffer
 */
static const char *get_timestamp(void) {
    static char timestamp[32];
    time_t now;
    struct tm *tm_info;
    
    time(&now);
    tm_info = localtime(&now);
    
    strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S]", tm_info);
    return timestamp;
}

int init_logging(void) {
    log_file = fopen(LOG_FILE_PATH, "a");
    if (log_file == NULL) {
        fprintf(stderr, "%s [ERROR] Failed to open log file %s, falling back to stderr\n",
                get_timestamp(), LOG_FILE_PATH);
        return -1;
    }
    
    /* Set line buffering for immediate output */
    setvbuf(log_file, NULL, _IOLBF, 0);
    
    return 0;
}

void close_logging(void) {
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }
}

void log_message(const char *format, ...) {
    va_list args;
    FILE *output = log_file != NULL ? log_file : stderr;
    
    /* Write timestamp */
    fprintf(output, "%s ", get_timestamp());
    
    /* Write formatted message */
    va_start(args, format);
    vfprintf(output, format, args);
    va_end(args);
    
    /* Ensure newline */
    fprintf(output, "\n");
    
    /* Flush to ensure immediate write */
    fflush(output);
}

void log_service_start(const char *service_name) {
    log_message("[INFO] Starting service: %s", service_name);
}

void log_service_complete(const char *service_name, int exit_status) {
    if (exit_status == 0) {
        log_message("[INFO] Service completed successfully: %s (exit status: %d)",
                   service_name, exit_status);
    } else {
        log_message("[ERROR] Service completed with error: %s (exit status: %d)",
                   service_name, exit_status);
    }
}
