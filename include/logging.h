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

#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>

/**
 * Initialize the logging system
 * Opens the log file at /var/log/inits.log
 * Returns 0 on success, -1 on failure
 */
int init_logging(void);

/**
 * Close the logging system
 * Closes the log file if open
 */
void close_logging(void);

/**
 * Log a formatted message with timestamp
 * Uses printf-style formatting
 * Falls back to stderr if log file is not available
 */
void log_message(const char *format, ...);

/**
 * Log the start of a service execution
 * service_name: Name of the service being started
 */
void log_service_start(const char *service_name);

/**
 * Log the completion of a service execution
 * service_name: Name of the service that completed
 * exit_status: Exit status code from the service
 */
void log_service_complete(const char *service_name, int exit_status);

#endif /* LOGGING_H */
