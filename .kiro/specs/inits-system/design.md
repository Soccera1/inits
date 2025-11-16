# Design Document

## Overview

inits is implemented as a collection of 11 programs: one main C99 binary (`inits`) and 10 POSIX shell wrapper scripts (`inits0` through `inits9`). The wrapper scripts serve as the entry point, setting the runlevel and invoking the main binary. The main binary discovers, sorts, and sequentially executes service scripts from `/etc/inits.d` based on filename patterns, logs its actions to `/var/log/inits.log`, and handles graceful process termination during shutdown.

## Architecture

### High-Level Component Diagram

```
┌─────────────────┐
│  Kernel/Boot    │
│   Loader        │
└────────┬────────┘
         │ executes
         ▼
┌─────────────────┐
│ Wrapper Script  │
│  (inits0-9)     │
│                 │
│ - Set RUNLEVEL  │
│ - exec inits    │
└────────┬────────┘
         │
         ▼
┌─────────────────────────────────────┐
│         Main inits Binary           │
│                                     │
│  ┌──────────────────────────────┐  │
│  │  Initialization Module       │  │
│  │  - Read RUNLEVEL             │  │
│  │  - Open log file             │  │
│  └──────────────────────────────┘  │
│                                     │
│  ┌──────────────────────────────┐  │
│  │  Service Discovery Module    │  │
│  │  - Scan /etc/inits.d         │  │
│  │  - Filter by runlevel        │  │
│  │  - Sort by ordering prefix   │  │
│  └──────────────────────────────┘  │
│                                     │
│  ┌──────────────────────────────┐  │
│  │  Service Execution Module    │  │
│  │  - Execute scripts in order  │  │
│  │  - Wait for completion       │  │
│  │  - Log results               │  │
│  └──────────────────────────────┘  │
│                                     │
│  ┌──────────────────────────────┐  │
│  │  Shutdown Module             │  │
│  │  - Send SIGTERM              │  │
│  │  - Wait 10s                  │  │
│  │  - Send SIGKILL              │  │
│  │  - Wait 15s                  │  │
│  └──────────────────────────────┘  │
│                                     │
│  ┌──────────────────────────────┐  │
│  │  Logging Module              │  │
│  │  - Write to /var/log/inits.log│ │
│  └──────────────────────────────┘  │
└─────────────────────────────────────┘
         │
         │ executes
         ▼
┌─────────────────┐
│ Service Scripts │
│  /etc/inits.d   │
└─────────────────┘
```

### Design Rationale

1. **Separation of wrapper and main binary**: Allows the same binary to handle all runlevels while keeping the entry point simple and shell-based
2. **No configuration files**: Reduces complexity and potential failure points
3. **Filename-based service ordering**: Makes the execution order immediately visible in directory listings
4. **Sequential execution**: Simplifies implementation and makes behavior predictable
5. **Fixed shutdown timeouts**: Provides reasonable defaults without requiring configuration

## Components and Interfaces

### Wrapper Scripts (inits0 through inits9)

**Purpose**: Set the runlevel environment variable and execute the main binary

**Implementation**:
- Language: POSIX shell
- Location: `/usr/local/sbin/inits0` through `/usr/local/sbin/inits9` (default; distributions may change)
- Each script is identical except for the runlevel number

**Pseudocode**:
```sh
#!/bin/sh
RUNLEVEL=N
export RUNLEVEL
exec /usr/local/sbin/inits
```

**Interface**:
- Input: None (invoked by kernel or init system)
- Output: Executes main inits binary with RUNLEVEL set
- Environment: Sets `RUNLEVEL` to corresponding digit (0-9)

### Main inits Binary

**Purpose**: Core init system logic

**Implementation**:
- Language: C99
- Location: `/usr/local/sbin/inits` (default; distributions may change)
- Compilation: Standard C99 compiler with POSIX feature macros

**Modules**:

#### 1. Initialization Module

**Responsibilities**:
- Read and validate RUNLEVEL environment variable
- Open log file for writing
- Set up signal handlers (if needed for shutdown)

**Functions**:
```c
int init_system(void);
int get_runlevel(void);
int open_log_file(void);
```

**Error Handling**:
- If RUNLEVEL is not set or invalid, log error and exit with status 1
- If log file cannot be opened, attempt to continue but log to stderr

#### 2. Service Discovery Module

**Responsibilities**:
- Scan `/etc/inits.d` directory
- Identify service scripts matching current runlevel
- Parse filenames to extract ordering information
- Sort services by ordering prefix

**Functions**:
```c
int discover_services(int runlevel, service_list_t *services);
int parse_service_filename(const char *filename, service_info_t *info);
int sort_services(service_list_t *services);
```

**Service Filename Parsing Logic**:
1. Check if filename starts with runlevel digit
2. Extract alphabetical ordering prefix characters
3. Verify dash separator exists
4. Extract descriptive name
5. Handle multi-runlevel services (e.g., "3a1b-servicename")

**Sorting Algorithm**:
- Use standard library `qsort()` with custom comparator
- Compare ordering prefix strings lexicographically
- For multi-runlevel services, use the ordering prefix for current runlevel

#### 3. Service Execution Module

**Responsibilities**:
- Execute service scripts sequentially
- Wait for each script to complete before starting next
- Capture exit status
- Log execution events

**Functions**:
```c
int execute_service(const service_info_t *service);
int wait_for_service(pid_t pid, int *exit_status);
```

**Execution Logic**:
1. Fork child process
2. In child: exec service script with appropriate environment
3. In parent: wait for child completion
4. Log exit status
5. Continue to next service regardless of exit status

**Environment Passed to Services**:
- `RUNLEVEL`: Current runlevel
- Inherit all other environment variables from init

#### 4. Shutdown Module

**Responsibilities**:
- Send termination signals to all processes
- Implement timeout logic
- Continue shutdown even if processes remain

**Functions**:
```c
int shutdown_system(void);
int send_signal_to_all(int signal);
int wait_with_timeout(int seconds);
```

**Shutdown Sequence**:
1. Send SIGTERM to all processes (kill(-1, SIGTERM))
2. Sleep for 10 seconds
3. Send SIGKILL to all processes (kill(-1, SIGKILL))
4. Sleep for 15 seconds
5. Return control to caller (typically runlevel 0 or 6 scripts)

#### 5. Logging Module

**Responsibilities**:
- Write timestamped log entries
- Handle log file rotation (if file grows too large)
- Provide thread-safe logging (if needed)

**Functions**:
```c
void log_message(const char *format, ...);
void log_service_start(const char *service_name);
void log_service_complete(const char *service_name, int exit_status);
```

**Log Format**:
```
[YYYY-MM-DD HH:MM:SS] [LEVEL] Message
```

**Log Levels**:
- INFO: Normal operations
- ERROR: Failures that don't prevent continuation
- FATAL: Failures that require exit

## Data Models

### service_info_t Structure

```c
typedef struct {
    char filename[256];           // Full filename
    char name[256];               // Descriptive name (after dash)
    char ordering_prefix[64];     // Alphabetical ordering characters
    int runlevel;                 // Runlevel this entry applies to
    char full_path[512];          // Full path to script
} service_info_t;
```

### service_list_t Structure

```c
typedef struct {
    service_info_t *services;     // Dynamic array of services
    size_t count;                 // Number of services
    size_t capacity;              // Allocated capacity
} service_list_t;
```

### Global State

```c
static FILE *log_file = NULL;     // Log file handle
static int current_runlevel = -1; // Current runlevel
```

## Error Handling

### Critical Errors (Exit Immediately)

1. RUNLEVEL environment variable not set or invalid
2. Unable to access /etc/inits.d directory
3. Memory allocation failure

**Response**: Log error message and exit with non-zero status

### Non-Critical Errors (Log and Continue)

1. Individual service script fails to execute
2. Service script returns non-zero exit status
3. Log file cannot be opened (fall back to stderr)
4. Service script file is not executable

**Response**: Log error and continue with next service

### Shutdown Errors

1. Processes remain after SIGKILL and timeout
   - **Response**: Log warning and continue shutdown anyway

## Testing Strategy

### Unit Testing

**Approach**: Test individual modules in isolation

**Test Cases**:

1. **Initialization Module**:
   - Valid RUNLEVEL values (0-9)
   - Invalid RUNLEVEL values (empty, non-numeric, out of range)
   - Log file creation success and failure

2. **Service Discovery Module**:
   - Empty /etc/inits.d directory
   - Directory with services for current runlevel
   - Directory with services for other runlevels
   - Services with single-character ordering prefix
   - Services with multi-character ordering prefix (aa, ab, etc.)
   - Multi-runlevel services (e.g., 3a1b-service)
   - Invalid filenames (missing dash, no ordering prefix)

3. **Service Execution Module**:
   - Successful service execution (exit 0)
   - Failed service execution (exit non-zero)
   - Service script not found
   - Service script not executable
   - Service script that hangs (timeout handling if implemented)

4. **Shutdown Module**:
   - All processes terminate on SIGTERM
   - Some processes require SIGKILL
   - Processes remain after SIGKILL timeout

5. **Logging Module**:
   - Log file write success
   - Log file write failure (disk full, permissions)
   - Log message formatting

### Integration Testing

**Approach**: Test complete system with mock service scripts

**Test Scenarios**:

1. Boot to runlevel 3 with multiple services
2. Boot to runlevel 1 with single service
3. Boot to runlevel with no services
4. Service execution order verification
5. Multi-runlevel service execution
6. Shutdown sequence with mock processes

### System Testing

**Approach**: Test on actual system or VM

**Test Scenarios**:

1. Boot real system using inits
2. Switch between runlevels
3. Verify service execution order matches expectations
4. Verify log file contents
5. Test shutdown and reboot
6. Test with various service script types (fast, slow, failing)

### Portability Testing

**Approach**: Compile and test on multiple POSIX systems

**Target Platforms**:
- GNU/Linux (primary)
- FreeBSD (secondary)
- OpenBSD (secondary)
- Other POSIX systems (best effort)

**Test Cases**:
- Compilation with C99 compiler
- Execution of wrapper scripts with different shells
- System call compatibility
- Filesystem layout assumptions

## Security Considerations

**Note**: Security is primarily the responsibility of service scripts, but inits should follow these principles:

1. **Privilege**: inits runs as PID 1 with root privileges (required for init)
2. **Input Validation**: Validate RUNLEVEL and service filenames to prevent injection
3. **Path Safety**: Use absolute paths for all file operations
4. **Signal Handling**: Properly handle signals to prevent unexpected termination
5. **Resource Limits**: No specific limits imposed by inits (handled by service scripts)

## Performance Considerations

1. **Sequential Execution**: Services run one at a time, so total boot time is sum of individual service times
2. **Directory Scanning**: /etc/inits.d is scanned once at startup
3. **Sorting Overhead**: Minimal (typically < 100 services)
4. **Logging Overhead**: Minimal (buffered I/O)
5. **Memory Usage**: Minimal (< 1MB for typical configurations)

## Build and Installation

### Build Requirements

- C99-compliant compiler (gcc, clang)
- POSIX-compliant shell (sh, bash, dash)
- Standard C library with POSIX extensions
- Make (optional, for build automation)

### Build Process

```bash
cc -std=c99 -D_POSIX_C_SOURCE=200809L -o inits inits.c
```

### Installation

1. Copy `inits` binary to `/usr/local/sbin/inits` (default; distributions may use `/sbin/inits`)
2. Create wrapper scripts `/usr/local/sbin/inits0` through `/usr/local/sbin/inits9`
3. Set executable permissions on all files
4. Create `/etc/inits.d` directory if it doesn't exist
5. Create `/var/log` directory if it doesn't exist
6. Configure bootloader to execute appropriate wrapper (e.g., `/usr/local/sbin/inits3`)

### Packaging

- Create distribution tarball with source code
- Include LICENSE file (GNU GPL 3.0)
- Include README with build and installation instructions
- Include example service scripts

## Future Considerations

While inits is designed to be minimal, potential future enhancements could include:

1. **Parallel Execution**: Execute independent services concurrently (requires dependency tracking)
2. **Service Dependencies**: Explicit dependency declarations (conflicts with filename-based ordering)
3. **Interactive Boot**: Allow user to skip services (requires TTY handling)
4. **Service Restart**: Automatically restart failed services (requires monitoring)

**Note**: These features are intentionally excluded from the current design to maintain simplicity.
