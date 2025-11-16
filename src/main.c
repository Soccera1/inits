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
#include "service.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

/* Global flag for signal handling */
static volatile sig_atomic_t shutdown_requested = 0;

/**
 * Signal handler for graceful shutdown
 * Sets a flag to indicate shutdown was requested
 */
static void signal_handler(int signum) {
    shutdown_requested = 1;
    /* Log to stderr since we can't safely call log_message from signal handler */
    const char *signame = (signum == SIGTERM) ? "SIGTERM" : 
                          (signum == SIGINT) ? "SIGINT" : "signal";
    write(STDERR_FILENO, "\n[INFO] Received ", 17);
    write(STDERR_FILENO, signame, strlen(signame));
    write(STDERR_FILENO, ", initiating graceful shutdown\n", 32);
}

/**
 * Set up signal handlers for graceful shutdown
 * Returns 0 on success, -1 on failure
 */
static int setup_signal_handlers(void) {
    struct sigaction sa;
    
    /* Set up signal handler structure */
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    /* Install handler for SIGTERM */
    if (sigaction(SIGTERM, &sa, NULL) < 0) {
        fprintf(stderr, "[ERROR] Failed to install SIGTERM handler\n");
        return -1;
    }
    
    /* Install handler for SIGINT */
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        fprintf(stderr, "[ERROR] Failed to install SIGINT handler\n");
        return -1;
    }
    
    return 0;
}

/**
 * Read and validate the RUNLEVEL environment variable
 * Returns the runlevel (0-9) on success, -1 on failure
 */
static int get_runlevel(void) {
    const char *runlevel_str = getenv("RUNLEVEL");
    
    /* Check if RUNLEVEL is set */
    if (runlevel_str == NULL) {
        fprintf(stderr, "[FATAL] RUNLEVEL environment variable is not set\n");
        return -1;
    }
    
    /* Check if RUNLEVEL is a single digit */
    if (strlen(runlevel_str) != 1) {
        fprintf(stderr, "[FATAL] RUNLEVEL must be a single digit (0-9), got: %s\n", 
                runlevel_str);
        return -1;
    }
    
    /* Check if RUNLEVEL is a valid digit (0-9) */
    if (runlevel_str[0] < '0' || runlevel_str[0] > '9') {
        fprintf(stderr, "[FATAL] RUNLEVEL must be between 0 and 9, got: %s\n", 
                runlevel_str);
        return -1;
    }
    
    /* Convert to integer */
    return runlevel_str[0] - '0';
}

/**
 * Initialize the init system
 * Returns 0 on success, -1 on failure
 */
static int init_system(void) {
    int runlevel;
    
    /* Get and validate runlevel */
    runlevel = get_runlevel();
    if (runlevel < 0) {
        return -1;
    }
    
    /* Initialize logging system */
    init_logging();
    
    /* Set up signal handlers for graceful shutdown */
    if (setup_signal_handlers() < 0) {
        log_message("[ERROR] Failed to set up signal handlers");
        /* Continue anyway - not critical */
    }
    
    /* Log initialization event */
    log_message("[INFO] Initializing inits for runlevel %d", runlevel);
    
    return runlevel;
}

/**
 * Main entry point
 */
int main(void) {
    int runlevel;
    service_list_t services;
    int exit_code = 0;
    
    /* Initialize the system */
    runlevel = init_system();
    if (runlevel < 0) {
        fprintf(stderr, "[FATAL] System initialization failed\n");
        return 1;
    }
    
    /* Discover services for this runlevel */
    if (discover_services(runlevel, &services) < 0) {
        log_message("[ERROR] Service discovery failed");
        close_logging();
        return 1;
    }
    
    /* Execute all discovered services in order */
    for (size_t i = 0; i < services.count; i++) {
        service_info_t *service = &services.services[i];
        int pid;
        int exit_status;
        
        /* Check if shutdown was requested via signal */
        if (shutdown_requested) {
            log_message("[INFO] Shutdown requested, stopping service execution");
            break;
        }
        
        /* Execute the service */
        pid = execute_service(service);
        if (pid < 0) {
            /* Log error but continue with next service */
            log_message("[ERROR] Failed to execute service %s", service->name);
            continue;
        }
        
        /* Wait for the service to complete */
        if (wait_for_service(pid, &exit_status) < 0) {
            /* Log error but continue with next service */
            log_message("[ERROR] Failed to wait for service %s", service->name);
            continue;
        }
        
        /* Log service completion with exit status */
        log_service_complete(service->name, exit_status);
    }
    
    log_message("[INFO] Runlevel %d initialization complete", runlevel);
    
    /* Perform shutdown sequence for runlevels 0 and 6 */
    if (runlevel == 0 || runlevel == 6) {
        log_message("[INFO] Runlevel %d detected, initiating shutdown sequence", runlevel);
        if (shutdown_system() < 0) {
            log_message("[ERROR] Shutdown sequence encountered errors");
            exit_code = 1;
        }
    }
    
    /* Check if shutdown was requested via signal */
    if (shutdown_requested) {
        log_message("[INFO] Performing graceful shutdown due to signal");
        if (shutdown_system() < 0) {
            log_message("[ERROR] Shutdown sequence encountered errors");
            exit_code = 1;
        }
    }
    
    /* Clean up resources */
    free_service_list(&services);
    close_logging();
    
    return exit_code;
}
