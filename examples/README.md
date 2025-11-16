# Example Service Scripts

This directory contains example service scripts demonstrating the inits service script format and naming conventions.

## Installation

To use these examples, copy them to `/etc/inits.d` and make them executable:

```bash
sudo cp examples/* /etc/inits.d/
sudo chmod +x /etc/inits.d/*
```

**Warning:** These are example scripts and may need modification for your specific system. Review and test before using in production.

## Examples

### 1a-syslog
Starts the system logging daemon in runlevel 1. This should be one of the first services to start so other services can log their activities.

**Runlevel:** 1 (single-user mode)  
**Order:** a (first)

### 3a-network
Initializes network interfaces in runlevel 3. Brings up loopback and primary network interface.

**Runlevel:** 3 (multi-user with networking)  
**Order:** a (first)

### 3b-database
Starts a database server (PostgreSQL example) in runlevel 3. Runs after network initialization.

**Runlevel:** 3 (multi-user with networking)  
**Order:** b (second)  
**Dependencies:** Requires 3a-network

### 3c-webserver
Starts a web server (Apache httpd example) in runlevel 3. Runs after network and database.

**Runlevel:** 3 (multi-user with networking)  
**Order:** c (third)  
**Dependencies:** Requires 3a-network, 3b-database

### 0a-shutdown
Gracefully stops services during system shutdown (runlevel 0). Stops services in reverse order.

**Runlevel:** 0 (halt/shutdown)  
**Order:** a (first)

### 6a-reboot
Prepares system for reboot (runlevel 6). Similar to shutdown but for reboot.

**Runlevel:** 6 (reboot)  
**Order:** a (first)

### 3a1b-multirunlevel
Demonstrates a service that runs in multiple runlevels with different ordering in each.

**Runlevels:** 1 (order b), 3 (order a)

## Customization

These scripts are templates. You'll likely need to modify them for your system:

1. **Paths:** Adjust binary paths, configuration file locations, and data directories
2. **Service Names:** Change process names in `pgrep` commands
3. **Interface Names:** Update network interface names (eth0, enp0s3, etc.)
4. **Database Systems:** Adapt for MySQL, MariaDB, SQLite, etc.
5. **Web Servers:** Adapt for nginx, lighttpd, etc.

## Service Script Guidelines

When creating your own service scripts:

1. **Shebang:** Start with `#!/bin/sh` for POSIX compatibility
2. **License:** Include GPL-3.0 license header
3. **Comments:** Document what the service does and its dependencies
4. **Idempotency:** Check if service is already running before starting
5. **Error Handling:** Exit with non-zero status on failure
6. **Logging:** Echo status messages (captured in service logs)
7. **Environment:** Use `$RUNLEVEL` variable when needed
8. **Executable:** Make script executable with `chmod +x`

## Testing

Test service scripts individually before using with inits:

```bash
# Set RUNLEVEL environment variable
export RUNLEVEL=3

# Run script manually
/etc/inits.d/3a-network

# Check exit status
echo $?
```

## Naming Convention Reference

Service script filenames must follow this pattern:

```
<runlevel><ordering>-<name>
```

- **Runlevel:** Single digit 0-9
- **Ordering:** One or more letters (a, b, c, ..., aa, ab, ...)
- **Separator:** Dash character (-)
- **Name:** Descriptive name (letters, numbers, hyphens)

**Valid Examples:**
- `1a-syslog`
- `3a-network`
- `3aa-advanced-service`
- `3a1b-multirunlevel`

**Invalid Examples:**
- `syslog` (missing runlevel and ordering)
- `3-network` (missing ordering prefix)
- `a3-network` (runlevel must come first)
- `3_network` (underscore instead of dash)

## See Also

- Main README.md for complete documentation
- `/var/log/inits.log` for service execution logs
- System-specific init documentation for your platform
