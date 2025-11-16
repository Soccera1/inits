# inits - A Lightweight UNIX Init System

inits is a minimal init system written in C99 and POSIX shell, designed for simplicity and portability across UNIX-like systems. It provides runlevel-based service execution with predictable ordering through filename conventions.

## Features

- **Minimal Design**: No configuration files required
- **Runlevel Support**: 10 runlevels (0-9) for different system states
- **Predictable Ordering**: Service execution order determined by filename
- **Multi-Runlevel Services**: Services can run in multiple runlevels
- **Comprehensive Logging**: All actions logged to `/var/log/inits.log`
- **Graceful Shutdown**: SIGTERM followed by SIGKILL with configurable timeouts
- **POSIX Compliant**: Works on Linux, BSD, and other UNIX-like systems
- **GPL 3.0 Licensed**: Free and open source

## Requirements

- C99-compliant compiler (gcc, clang)
- POSIX-compliant shell (sh, bash, dash)
- Standard C library with POSIX extensions
- Make (optional, for automated builds)

## Building

### Quick Build

```bash
make
```

This compiles the main `inits` binary and generates wrapper scripts (`inits0` through `inits9`).

### Manual Build

If you don't have Make, you can compile manually:

```bash
cc -std=c99 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -pedantic \
   -Iinclude -o inits src/main.c src/service.c src/logging.c
```

Then create wrapper scripts manually (see `scripts/` directory for examples).

### Build Targets

- `make all` - Build binary and wrapper scripts (default)
- `make clean` - Remove build artifacts
- `make install` - Install to system (requires root)
- `make uninstall` - Remove from system (requires root)

## Installation

### Standard Installation

```bash
sudo make install
```

This installs:
- `/usr/local/sbin/inits` - Main binary
- `/usr/local/sbin/inits0` through `/usr/local/sbin/inits9` - Wrapper scripts
- `/etc/inits.d/` - Service script directory (created if needed)
- `/var/log/` - Log directory (created if needed)

### Custom Installation Prefix

To install to a different location:

```bash
sudo make install PREFIX=/usr
```

This installs to `/usr/sbin` instead of `/usr/local/sbin`.

### Uninstallation

```bash
sudo make uninstall
```

Note: This preserves `/etc/inits.d` and `/var/log` directories.

## Usage

### Runlevels

inits supports 10 runlevels (0-9). Common conventions:

- **Runlevel 0**: System halt/shutdown
- **Runlevel 1**: Single-user mode (maintenance)
- **Runlevel 2**: Multi-user mode without networking
- **Runlevel 3**: Multi-user mode with networking
- **Runlevel 4**: Unused (custom)
- **Runlevel 5**: Multi-user mode with GUI
- **Runlevel 6**: System reboot
- **Runlevels 7-9**: Custom/unused

### Starting a Runlevel

Execute the appropriate wrapper script:

```bash
/usr/local/sbin/inits3  # Start runlevel 3
```

The wrapper sets the `RUNLEVEL` environment variable and executes the main binary.

### Service Script Naming Convention

Service scripts in `/etc/inits.d` must follow this naming pattern:

```
<runlevel><ordering-prefix>-<descriptive-name>
```

**Components:**

1. **Runlevel digit** (0-9): When this service runs
2. **Ordering prefix** (a-z, aa-zz, etc.): Execution order within runlevel
3. **Dash separator** (-)
4. **Descriptive name**: Human-readable service name

**Examples:**

- `1a-syslog` - Runs in runlevel 1, first service (a)
- `3a-network` - Runs in runlevel 3, first service (a)
- `3b-database` - Runs in runlevel 3, second service (b)
- `3c-webserver` - Runs in runlevel 3, third service (c)
- `5z-cleanup` - Runs in runlevel 5, last service (z)

### Multi-Runlevel Services

Services can run in multiple runlevels by including multiple runlevel-ordering pairs:

```
3a1b-multirunlevel
```

This service runs:
- In runlevel 3 with ordering prefix 'a'
- In runlevel 1 with ordering prefix 'b'

### Service Script Format

Service scripts are executable shell scripts. They receive the `RUNLEVEL` environment variable:

```bash
#!/bin/sh
# Example service script

echo "Starting my service at runlevel $RUNLEVEL"

# Your initialization code here
# ...

exit 0
```

**Important:**
- Scripts must be executable (`chmod +x`)
- Scripts should exit with status 0 on success
- Non-zero exit status is logged but doesn't stop other services
- Scripts run sequentially (not in parallel)

## Example Service Scripts

See the `examples/` directory for sample service scripts:

- `examples/1a-syslog` - System logging daemon
- `examples/3a-network` - Network initialization
- `examples/3b-database` - Database server
- `examples/3c-webserver` - Web server
- `examples/0a-shutdown` - Shutdown services

## Logging

All inits actions are logged to `/var/log/inits.log`:

```
[2024-11-16 10:30:15] [INFO] Executing runlevel 3
[2024-11-16 10:30:15] [INFO] Discovered service: 3a-network
[2024-11-16 10:30:15] [INFO] Discovered service: 3b-database
[2024-11-16 10:30:15] [INFO] Starting service: 3a-network
[2024-11-16 10:30:16] [INFO] Service completed: 3a-network (exit status: 0)
[2024-11-16 10:30:16] [INFO] Starting service: 3b-database
[2024-11-16 10:30:18] [INFO] Service completed: 3b-database (exit status: 0)
```

Service script output is not logged by inits. Services should handle their own logging.

## Shutdown Process

When shutdown is initiated (typically via runlevel 0 or 6 services):

1. SIGTERM sent to all processes
2. Wait 10 seconds
3. SIGKILL sent to remaining processes
4. Wait 15 seconds
5. Continue shutdown

This ensures graceful termination with a fallback to forced termination.

## Troubleshooting

### Service Not Executing

Check:
1. Filename follows naming convention: `<runlevel><ordering>-<name>`
2. Script is executable: `chmod +x /etc/inits.d/3a-myservice`
3. Script is in `/etc/inits.d` directory
4. Runlevel digit matches the runlevel you're starting
5. Check `/var/log/inits.log` for errors

### Service Execution Order Wrong

- Services execute in alphabetical order by ordering prefix
- Use `ls /etc/inits.d/3*` to see execution order for runlevel 3
- Rename services to adjust order (e.g., `3a-first`, `3b-second`, `3c-third`)

### Log File Not Created

- Ensure `/var/log` directory exists and is writable
- If log file can't be opened, inits logs to stderr instead
- Check permissions: `ls -ld /var/log`

### Service Fails But System Continues

This is expected behavior. inits logs the failure and continues with remaining services. Check:
- Service script exit status in `/var/log/inits.log`
- Service's own log files for details

## Architecture

inits consists of:

1. **Main Binary** (`inits`): Core init system logic
   - Service discovery and sorting
   - Sequential execution
   - Logging
   - Shutdown handling

2. **Wrapper Scripts** (`inits0`-`inits9`): Runlevel entry points
   - Set RUNLEVEL environment variable
   - Execute main binary

3. **Service Scripts** (`/etc/inits.d/*`): User-provided initialization scripts
   - Perform actual system initialization
   - Start daemons and services

## Design Philosophy

inits follows these principles:

- **Simplicity**: No configuration files, behavior determined by filenames
- **Predictability**: Sequential execution, clear ordering
- **Transparency**: Comprehensive logging of all actions
- **Robustness**: Continue on service failures, graceful shutdown
- **Portability**: Standard C99 and POSIX compliance

## License

inits is licensed under the GNU General Public License version 3.0 (GPL-3.0).

See the LICENSE file for full license text.

## Contributing

Contributions are welcome! Please ensure:

- Code follows C99 and POSIX standards
- Changes maintain simplicity and minimalism
- All code includes appropriate GPL-3.0 headers
- Documentation is updated for user-facing changes

## Author

inits was created as a minimal, educational init system demonstrating core init system concepts.

## Documentation

- **README.md** (this file) - Overview and quick start guide
- **INSTALL.md** - Detailed installation instructions
- **SERVICE-NAMING.md** - Complete service script naming reference
- **examples/README.md** - Example service scripts documentation
- **LICENSE** - GNU GPL 3.0 license text

## See Also

- System V init
- BSD init
- systemd
- OpenRC
- runit
