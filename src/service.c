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

#include "service.h"
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifndef INITS_DIR
#define INITS_DIR "/etc/inits.d"
#endif
#define INITIAL_CAPACITY 16

/**
 * Initialize an empty service list
 */
static int init_service_list(service_list_t *services) {
    services->services = malloc(INITIAL_CAPACITY * sizeof(service_info_t));
    if (services->services == NULL) {
        log_message("[ERROR] Failed to allocate memory for service list");
        return -1;
    }
    services->count = 0;
    services->capacity = INITIAL_CAPACITY;
    return 0;
}

/**
 * Add a service to the list, expanding capacity if needed
 */
static int add_service(service_list_t *services, const service_info_t *service) {
    if (services->count >= services->capacity) {
        size_t new_capacity = services->capacity * 2;
        service_info_t *new_services = realloc(services->services, 
                                               new_capacity * sizeof(service_info_t));
        if (new_services == NULL) {
            log_message("[ERROR] Failed to expand service list capacity");
            return -1;
        }
        services->services = new_services;
        services->capacity = new_capacity;
    }
    
    services->services[services->count] = *service;
    services->count++;
    return 0;
}

/**
 * Parse a service filename to extract runlevel and ordering information
 * Handles both single-runlevel (e.g., "3a-service") and multi-runlevel (e.g., "3a1b-service")
 * Returns 1 if the service matches the given runlevel, 0 otherwise, -1 on error
 */
int parse_service_filename(const char *filename, int runlevel, service_info_t *info) {
    const char *p = filename;
    int found_runlevel = 0;
    char ordering_prefix[64] = {0};
    
    /* Parse runlevel-ordering pairs until we hit a dash */
    while (*p != '\0' && *p != '-') {
        /* Check if this is a runlevel digit */
        if (isdigit(*p)) {
            int current_runlevel = *p - '0';
            p++;
            
            /* Extract ordering prefix characters */
            const char *prefix_start = p;
            while (*p != '\0' && *p != '-' && isalpha(*p)) {
                p++;
            }
            
            /* If no alphabetical characters follow the digit, invalid format */
            if (p == prefix_start) {
                return -1;
            }
            
            /* If this runlevel matches, save the ordering prefix */
            if (current_runlevel == runlevel) {
                size_t prefix_len = p - prefix_start;
                if (prefix_len >= sizeof(ordering_prefix)) {
                    prefix_len = sizeof(ordering_prefix) - 1;
                }
                strncpy(ordering_prefix, prefix_start, prefix_len);
                ordering_prefix[prefix_len] = '\0';
                found_runlevel = 1;
            }
        } else {
            /* Invalid format: expected digit */
            return -1;
        }
    }
    
    /* Check if we found a dash separator */
    if (*p != '-') {
        return -1;
    }
    p++; /* Skip the dash */
    
    /* The rest is the descriptive name */
    if (*p == '\0') {
        return -1; /* No name after dash */
    }
    
    /* If this service matches our runlevel, populate the info structure */
    if (found_runlevel) {
        strncpy(info->filename, filename, sizeof(info->filename) - 1);
        info->filename[sizeof(info->filename) - 1] = '\0';
        
        strncpy(info->name, p, sizeof(info->name) - 1);
        info->name[sizeof(info->name) - 1] = '\0';
        
        strncpy(info->ordering_prefix, ordering_prefix, sizeof(info->ordering_prefix) - 1);
        info->ordering_prefix[sizeof(info->ordering_prefix) - 1] = '\0';
        
        info->runlevel = runlevel;
        
        snprintf(info->full_path, sizeof(info->full_path), "%s/%s", INITS_DIR, filename);
        
        return 1;
    }
    
    return 0;
}

/**
 * Comparator function for qsort - sorts by ordering prefix lexicographically
 */
static int service_comparator(const void *a, const void *b) {
    const service_info_t *service_a = (const service_info_t *)a;
    const service_info_t *service_b = (const service_info_t *)b;
    
    return strcmp(service_a->ordering_prefix, service_b->ordering_prefix);
}

/**
 * Sort services by their ordering prefix (lexicographic order)
 */
void sort_services(service_list_t *services) {
    if (services->count > 1) {
        qsort(services->services, services->count, sizeof(service_info_t), 
              service_comparator);
    }
}

/**
 * Discover all service scripts for the given runlevel
 * Scans /etc/inits.d directory and filters by runlevel
 * Returns 0 on success, -1 on failure
 */
int discover_services(int runlevel, service_list_t *services) {
    DIR *dir;
    struct dirent *entry;
    
    /* Initialize the service list */
    if (init_service_list(services) < 0) {
        return -1;
    }
    
    /* Open the directory */
    dir = opendir(INITS_DIR);
    if (dir == NULL) {
        log_message("[ERROR] Failed to open directory %s", INITS_DIR);
        free(services->services);
        services->services = NULL;
        return -1;
    }
    
    /* Scan directory entries */
    while ((entry = readdir(dir)) != NULL) {
        service_info_t info;
        int result;
        
        /* Skip . and .. */
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        /* Parse the filename */
        result = parse_service_filename(entry->d_name, runlevel, &info);
        
        if (result == 1) {
            /* Service matches our runlevel, add it to the list */
            if (add_service(services, &info) < 0) {
                closedir(dir);
                free_service_list(services);
                return -1;
            }
            log_message("[INFO] Discovered service: %s (ordering: %s)", 
                       info.name, info.ordering_prefix);
        } else if (result == -1) {
            /* Invalid filename format, log warning but continue */
            log_message("[WARN] Invalid service filename format: %s", entry->d_name);
        }
        /* result == 0 means service doesn't match our runlevel, skip silently */
    }
    
    closedir(dir);
    
    /* Sort the services by ordering prefix */
    sort_services(services);
    
    log_message("[INFO] Discovered %zu service(s) for runlevel %d", 
               services->count, runlevel);
    
    return 0;
}

/**
 * Free memory allocated for service list
 */
void free_service_list(service_list_t *services) {
    if (services->services != NULL) {
        free(services->services);
        services->services = NULL;
    }
    services->count = 0;
    services->capacity = 0;
}

/**
 * Execute a service script
 * Forks a child process and executes the service script
 * Returns the child process PID on success, -1 on failure
 */
int execute_service(const service_info_t *service) {
    pid_t pid;
    
    /* Log service execution start */
    log_service_start(service->name);
    
    /* Fork a child process */
    pid = fork();
    
    if (pid < 0) {
        /* Fork failed */
        log_message("[ERROR] Failed to fork process for service %s", service->name);
        return -1;
    } else if (pid == 0) {
        /* Child process - execute the service script */
        
        /* Execute the service script with sh */
        /* The RUNLEVEL environment variable is already set and will be inherited */
        execl("/bin/sh", "sh", service->full_path, (char *)NULL);
        
        /* If execl returns, it failed */
        fprintf(stderr, "[ERROR] Failed to execute service %s: %s\n", 
                service->name, service->full_path);
        exit(127); /* Exit with standard "command not found" status */
    }
    
    /* Parent process - return the child PID */
    return pid;
}

/**
 * Wait for a service to complete
 * Waits for the child process to finish and captures exit status
 * Returns 0 on success, -1 on failure
 */
int wait_for_service(int pid, int *exit_status) {
    int status;
    pid_t result;
    
    /* Wait for the child process to complete */
    result = waitpid(pid, &status, 0);
    
    if (result < 0) {
        log_message("[ERROR] Failed to wait for process %d", pid);
        return -1;
    }
    
    /* Extract the exit status */
    if (WIFEXITED(status)) {
        *exit_status = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        /* Process was terminated by a signal */
        *exit_status = 128 + WTERMSIG(status);
    } else {
        /* Unknown termination status */
        *exit_status = -1;
    }
    
    return 0;
}

/**
 * Send a signal to all processes except init
 * Uses kill(-1, signal) to broadcast signal
 * Returns 0 on success, -1 on failure
 */
static int send_signal_to_all(int signal) {
    const char *signal_name = (signal == SIGTERM) ? "SIGTERM" : 
                              (signal == SIGKILL) ? "SIGKILL" : "signal";
    
    log_message("[INFO] Sending %s to all processes", signal_name);
    
    /* Send signal to all processes except init (PID 1) */
    if (kill(-1, signal) < 0) {
        log_message("[ERROR] Failed to send %s to all processes", signal_name);
        return -1;
    }
    
    return 0;
}

/**
 * Perform system shutdown sequence
 * Sends SIGTERM to all processes, waits 10 seconds,
 * then sends SIGKILL to remaining processes, waits 15 seconds
 * Returns 0 on success, -1 on failure
 */
int shutdown_system(void) {
    log_message("[INFO] Initiating system shutdown sequence");
    
    /* Send SIGTERM to all processes */
    if (send_signal_to_all(SIGTERM) < 0) {
        /* Log error but continue with shutdown */
        log_message("[WARN] Failed to send SIGTERM, continuing shutdown");
    }
    
    /* Wait 10 seconds for processes to terminate gracefully */
    log_message("[INFO] Waiting 10 seconds for processes to terminate");
    sleep(10);
    
    /* Send SIGKILL to remaining processes */
    if (send_signal_to_all(SIGKILL) < 0) {
        /* Log error but continue with shutdown */
        log_message("[WARN] Failed to send SIGKILL, continuing shutdown");
    }
    
    /* Wait 15 seconds for processes to be killed */
    log_message("[INFO] Waiting 15 seconds for remaining processes");
    sleep(15);
    
    /* Continue shutdown regardless of remaining processes */
    log_message("[INFO] Shutdown sequence complete");
    
    return 0;
}
