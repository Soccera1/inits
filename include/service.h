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

#ifndef SERVICE_H
#define SERVICE_H

#include <stddef.h>

/**
 * Information about a single service script
 */
typedef struct {
    char filename[256];           /* Full filename */
    char name[256];               /* Descriptive name (after dash) */
    char ordering_prefix[64];     /* Alphabetical ordering characters */
    int runlevel;                 /* Runlevel this entry applies to */
    char full_path[512];          /* Full path to script */
} service_info_t;

/**
 * List of service scripts
 */
typedef struct {
    service_info_t *services;     /* Dynamic array of services */
    size_t count;                 /* Number of services */
    size_t capacity;              /* Allocated capacity */
} service_list_t;

/**
 * Discover all service scripts for the given runlevel
 * Scans /etc/inits.d directory and filters by runlevel
 * Returns 0 on success, -1 on failure
 */
int discover_services(int runlevel, service_list_t *services);

/**
 * Parse a service filename to extract runlevel and ordering information
 * Returns 1 if the service matches the given runlevel, 0 otherwise, -1 on error
 */
int parse_service_filename(const char *filename, int runlevel, service_info_t *info);

/**
 * Sort services by their ordering prefix (lexicographic order)
 */
void sort_services(service_list_t *services);

/**
 * Free memory allocated for service list
 */
void free_service_list(service_list_t *services);

/**
 * Execute a service script
 * Forks a child process and executes the service script
 * Returns the child process PID on success, -1 on failure
 */
int execute_service(const service_info_t *service);

/**
 * Wait for a service to complete
 * Waits for the child process to finish and captures exit status
 * Returns 0 on success, -1 on failure
 */
int wait_for_service(int pid, int *exit_status);

/**
 * Perform system shutdown sequence
 * Sends SIGTERM to all processes, waits 10 seconds,
 * then sends SIGKILL to remaining processes, waits 15 seconds
 * Returns 0 on success, -1 on failure
 */
int shutdown_system(void);

#endif /* SERVICE_H */
