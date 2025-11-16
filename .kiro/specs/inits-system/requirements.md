# Requirements Document

## Introduction

inits is a lightweight UNIX init system written in C99 and shell, licensed under GNU GPL 3.0. The system provides minimal initialization functionality through a main binary and wrapper programs that handle runlevel-based service execution. Services are shell scripts organized by runlevel and execution order, with the init system responsible for sequential execution and process termination during shutdown.

## Glossary

- **inits**: The main init system binary that executes services based on runlevel
- **Wrapper Program**: A shell script (inits0 through inits9) that sets RUNLEVEL environment variable and executes the main inits binary
- **Service Script**: A shell script in /etc/inits.d that performs initialization tasks
- **Runlevel**: A numeric value (0-9) indicating the system state and which services to execute
- **Service Ordering Prefix**: Alphabetical characters in a service script filename that determine execution order within a runlevel
- **RUNLEVEL Environment Variable**: An environment variable containing the numeric runlevel value (0-9)
- **Service Discovery**: The process of identifying which service scripts to execute for a given runlevel

## Requirements

### Requirement 1: Runlevel Selection

**User Story:** As a system administrator, I want the init system to execute services based on the wrapper program invoked, so that I can control which runlevel the system boots into.

#### Acceptance Criteria

1. WHEN a wrapper program (inits0 through inits9) is executed, THE Wrapper Program SHALL set the RUNLEVEL environment variable to its corresponding numeric value (0-9)
2. WHEN the RUNLEVEL environment variable is set, THE Wrapper Program SHALL execute the main inits binary
3. WHEN the inits binary starts, THE inits binary SHALL read the RUNLEVEL environment variable
4. THE inits binary SHALL support runlevel values from 0 through 9 inclusive

### Requirement 2: Service Discovery and Execution

**User Story:** As a system administrator, I want services to be executed in a predictable order based on their filenames, so that I can control service dependencies through naming conventions.

#### Acceptance Criteria

1. WHEN inits executes, THE inits binary SHALL discover all service scripts in the /etc/inits.d directory
2. WHEN discovering service scripts, THE inits binary SHALL identify scripts whose filename begins with the current runlevel digit
3. WHEN multiple service scripts match the current runlevel, THE inits binary SHALL sort them alphabetically by their ordering prefix characters
4. THE inits binary SHALL execute discovered service scripts sequentially in sorted order
5. WHEN a service script filename contains multiple runlevel prefixes, THE inits binary SHALL execute that script when any of its specified runlevels matches the current RUNLEVEL

### Requirement 3: Service Script Naming Convention

**User Story:** As a system administrator, I want to name service scripts with runlevel and ordering information, so that the init system can determine when and in what order to execute them.

#### Acceptance Criteria

1. THE Service Script filename SHALL begin with a runlevel digit (0-9)
2. THE Service Script filename SHALL contain one or more alphabetical characters immediately following the runlevel digit to indicate execution order
3. THE Service Script filename SHALL contain a dash character following the ordering prefix
4. THE Service Script filename SHALL contain a descriptive name following the dash character
5. WHEN a service belongs to multiple runlevels, THE Service Script filename SHALL contain multiple runlevel-and-ordering-prefix pairs before the dash

### Requirement 4: Logging

**User Story:** As a system administrator, I want the init system to log its actions, so that I can troubleshoot boot issues.

#### Acceptance Criteria

1. THE inits binary SHALL write log entries to /var/log/inits.log
2. WHEN inits starts execution, THE inits binary SHALL log the runlevel being executed
3. WHEN inits discovers a service script, THE inits binary SHALL log the service script filename
4. WHEN inits executes a service script, THE inits binary SHALL log the execution start
5. WHEN a service script completes execution, THE inits binary SHALL log the completion and exit status
6. THE inits binary SHALL NOT log output or errors from service scripts themselves

### Requirement 5: Shutdown Process

**User Story:** As a system administrator, I want the init system to gracefully terminate processes during shutdown, so that data is not corrupted.

#### Acceptance Criteria

1. WHEN shutdown is initiated, THE inits binary SHALL send SIGTERM to all remaining processes
2. WHEN SIGTERM has been sent, THE inits binary SHALL wait 10 seconds before proceeding
3. WHEN the 10-second wait completes, THE inits binary SHALL send SIGKILL to all processes that are still running
4. WHEN SIGKILL has been sent, THE inits binary SHALL wait 15 seconds before proceeding
5. WHEN the 15-second wait completes, THE inits binary SHALL continue with shutdown regardless of remaining processes

### Requirement 6: Portability and Standards Compliance

**User Story:** As a developer, I want the init system to be written in standard C99 and POSIX shell, so that it can run on multiple UNIX-like systems.

#### Acceptance Criteria

1. THE inits binary SHALL be written in C99-compliant code
2. THE Wrapper Programs SHALL be written using POSIX-compliant shell syntax
3. THE inits binary SHALL use only POSIX-standard system calls and library functions
4. THE inits binary SHALL assume the presence of /etc directory for configuration

### Requirement 7: Minimal Configuration

**User Story:** As a system administrator, I want the init system to operate without configuration files, so that the system is simple to understand and maintain.

#### Acceptance Criteria

1. THE inits binary SHALL NOT read any configuration files
2. THE inits binary SHALL determine all behavior from the RUNLEVEL environment variable and service script filenames
3. THE Wrapper Programs SHALL contain only the logic to set RUNLEVEL and execute the inits binary

### Requirement 8: Licensing

**User Story:** As a user of free software, I want the init system to be licensed under GNU GPL 3.0, so that I have the freedom to modify and redistribute it.

#### Acceptance Criteria

1. THE inits binary source code SHALL include GNU GPL 3.0 license headers
2. THE Wrapper Programs SHALL include GNU GPL 3.0 license headers
3. THE distribution SHALL include a copy of the GNU GPL 3.0 license text
