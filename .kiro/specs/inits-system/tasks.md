# Implementation Plan

- [x] 1. Create project structure and build system
  - Create source directory structure (src/, include/, scripts/)
  - Create Makefile with targets for build, install, and clean
  - Set up compiler flags for C99 and POSIX compliance
  - Create LICENSE file with GNU GPL 3.0 text
  - _Requirements: 6.1, 6.2, 6.3, 8.1, 8.2, 8.3_

- [x] 2. Implement logging module
  - Create header file with logging function declarations
  - Implement log file opening and initialization
  - Implement timestamped log message formatting
  - Implement log_message() with printf-style formatting
  - Implement log_service_start() and log_service_complete() functions
  - Add error handling for log file write failures (fallback to stderr)
  - _Requirements: 4.1, 4.2, 4.3, 4.4, 4.5, 4.6_

- [x] 3. Implement initialization module
  - Create main.c with main() entry point
  - Implement get_runlevel() to read and validate RUNLEVEL environment variable
  - Implement init_system() to initialize logging and validate runlevel
  - Add error handling for missing or invalid RUNLEVEL
  - Log initialization events (runlevel being executed)
  - _Requirements: 1.3, 1.4, 4.2_

- [x] 4. Implement service discovery module
  - Define service_info_t and service_list_t data structures
  - Implement discover_services() to scan /etc/inits.d directory
  - Implement parse_service_filename() to extract runlevel and ordering prefix
  - Handle multi-runlevel service filenames (e.g., "3a1b-servicename")
  - Implement service filtering by current runlevel
  - Implement sort_services() using qsort with lexicographic comparator
  - Add error handling for directory access failures
  - Log discovered services
  - _Requirements: 2.1, 2.2, 2.3, 2.4, 2.5, 3.1, 3.2, 3.3, 3.4, 3.5, 4.3_

- [x] 5. Implement service execution module
  - Implement execute_service() to fork and exec service scripts
  - Implement wait_for_service() to wait for child process completion
  - Pass RUNLEVEL environment variable to service scripts
  - Capture and log service exit status
  - Continue execution even if service fails
  - Log service execution start and completion
  - _Requirements: 2.4, 4.4, 4.5_

- [x] 6. Implement shutdown module
  - Implement shutdown_system() function
  - Implement send_signal_to_all() using kill(-1, signal)
  - Send SIGTERM to all processes
  - Implement 10-second wait after SIGTERM
  - Send SIGKILL to remaining processes
  - Implement 15-second wait after SIGKILL
  - Continue shutdown regardless of remaining processes
  - Log shutdown events
  - _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5_

- [x] 7. Create wrapper scripts
  - Create shell script template for wrapper programs
  - Generate inits0 through inits9 wrapper scripts
  - Each wrapper sets RUNLEVEL to its corresponding digit (0-9)
  - Each wrapper execs the main inits binary
  - Add GNU GPL 3.0 license headers to wrapper scripts
  - Make wrapper scripts executable
  - _Requirements: 1.1, 1.2, 6.2, 7.2, 7.3, 8.2_

- [x] 8. Integrate all modules and finalize main binary
  - Wire initialization, service discovery, execution, and shutdown modules together
  - Implement main program flow: init → discover → execute → (shutdown if applicable)
  - Add signal handlers for graceful shutdown (SIGTERM, SIGINT)
  - Ensure proper cleanup and resource deallocation
  - Verify all logging calls are in place
  - _Requirements: 1.3, 1.4, 2.4, 7.1, 7.2_

- [ ]* 9. Create unit tests
  - Write tests for get_runlevel() with valid and invalid inputs
  - Write tests for parse_service_filename() with various filename formats
  - Write tests for sort_services() with different ordering scenarios
  - Write tests for logging functions
  - Create mock service scripts for testing
  - _Requirements: All requirements_

- [ ]* 10. Create integration test suite
  - Create test directory structure with mock /etc/inits.d
  - Create test service scripts for multiple runlevels
  - Write test script to verify service execution order
  - Write test script to verify multi-runlevel service handling
  - Write test script to verify shutdown sequence
  - Verify log file contents after test runs
  - _Requirements: All requirements_

- [x] 11. Create installation and documentation
  - Update Makefile with install target
  - Create README with build instructions
  - Create README with installation instructions
  - Document service script naming conventions
  - Create example service scripts
  - Document runlevel usage
  - _Requirements: 3.1, 3.2, 3.3, 3.4, 3.5, 6.4_
