# Service Script Naming Convention

This document provides a comprehensive reference for naming service scripts in the inits init system.

## Basic Format

All service scripts in `/etc/inits.d` must follow this naming pattern:

```
<runlevel><ordering-prefix>-<descriptive-name>
```

## Components

### 1. Runlevel Digit (Required)

A single digit from 0 to 9 indicating when the service runs.

**Valid:** `0`, `1`, `2`, `3`, `4`, `5`, `6`, `7`, `8`, `9`

**Common Conventions:**
- `0` - System halt/shutdown
- `1` - Single-user mode (maintenance)
- `2` - Multi-user mode without networking
- `3` - Multi-user mode with networking
- `4` - Custom/unused
- `5` - Multi-user mode with GUI
- `6` - System reboot
- `7-9` - Custom/unused

### 2. Ordering Prefix (Required)

One or more lowercase letters indicating execution order within the runlevel.

**Valid:** `a`, `b`, `c`, ..., `z`, `aa`, `ab`, ..., `zz`, `aaa`, ...

**Execution Order:**
- Services are sorted alphabetically by ordering prefix
- `a` runs before `b`, `b` before `c`, etc.
- `a` < `aa` < `ab` < `b` < `ba` < `bb` < `c`

**Best Practices:**
- Use single letters for most services: `a`, `b`, `c`
- Reserve `z` for cleanup/final services
- Use double letters for services inserted between existing ones: `aa`, `ab`
- Leave gaps in ordering for future additions

### 3. Dash Separator (Required)

A single dash character (`-`) separating the ordering prefix from the descriptive name.

### 4. Descriptive Name (Required)

A human-readable name describing the service.

**Valid Characters:** lowercase letters, numbers, hyphens

**Best Practices:**
- Use lowercase for consistency
- Use hyphens to separate words: `my-service`
- Keep names concise but descriptive
- Avoid special characters and spaces

## Examples

### Single Runlevel Services

```
1a-syslog          # Runlevel 1, first service (a), syslog daemon
3a-network         # Runlevel 3, first service (a), network initialization
3b-database        # Runlevel 3, second service (b), database server
3c-webserver       # Runlevel 3, third service (c), web server
5z-cleanup         # Runlevel 5, last service (z), cleanup tasks
```

### Multi-Runlevel Services

Services can run in multiple runlevels by including multiple runlevel-ordering pairs:

```
3a1b-multiservice  # Runs in runlevel 3 (order a) and runlevel 1 (order b)
5a3b-shared        # Runs in runlevel 5 (order a) and runlevel 3 (order b)
```

**Format:** `<rl1><order1><rl2><order2>-<name>`

**Execution:**
- In runlevel 3: uses ordering prefix `a` (first)
- In runlevel 1: uses ordering prefix `b` (second)

### Advanced Ordering

```
3a-first           # Runs first
3aa-between        # Runs between 3a and 3b
3ab-also-between   # Runs after 3aa but before 3b
3b-second          # Runs second
```

## Valid Examples

✅ **Correct:**
```
1a-syslog
3a-network
3b-database
3c-webserver
3aa-advanced
3a1b-multi
0a-shutdown
6a-reboot
5z-cleanup
```

## Invalid Examples

❌ **Incorrect:**

```
syslog              # Missing runlevel and ordering
3-network           # Missing ordering prefix
a3-network          # Runlevel must come first
3_network           # Underscore instead of dash
3A-network          # Uppercase ordering prefix
10a-service         # Runlevel must be single digit
3a_network          # Underscore instead of dash
3a network          # Space in name
3a-Network          # Uppercase in name
```

## Ordering Examples

### Execution Order in Runlevel 3

Given these service scripts:
```
3a-first
3c-third
3b-second
3aa-between-first-and-second
3z-last
```

**Execution order:**
1. `3a-first`
2. `3aa-between-first-and-second`
3. `3b-second`
4. `3c-third`
5. `3z-last`

### Multi-Runlevel Execution

Given this service script:
```
3a1b-multiservice
```

**In runlevel 1:**
- Executes with ordering prefix `b`
- Runs after `1a-*` services
- Runs before `1c-*` services

**In runlevel 3:**
- Executes with ordering prefix `a`
- Runs first (before `3b-*` services)

## Best Practices

### Planning Service Order

1. **List dependencies:** Identify which services depend on others
2. **Assign runlevels:** Determine which runlevels need each service
3. **Order services:** Assign ordering prefixes based on dependencies
4. **Leave gaps:** Use `a`, `c`, `e` instead of `a`, `b`, `c` for future additions

### Example Service Ordering

For a typical web application stack:

```
3a-network         # Network must be first
3b-database        # Database needs network
3c-cache           # Cache needs network
3d-application     # Application needs database and cache
3e-webserver       # Web server needs application
3z-monitoring      # Monitoring runs last
```

### Inserting New Services

If you need to add a service between `3a-network` and `3b-database`:

```
3a-network         # Existing
3aa-dns            # New service inserted
3b-database        # Existing
```

### Multi-Runlevel Services

Use multi-runlevel services when:
- Service is needed in multiple runlevels
- Service needs different ordering in each runlevel
- You want to avoid duplicate scripts

Example:
```
3a1a-logging       # Logging first in both runlevels
3b1b5a-monitoring  # Monitoring in runlevels 1, 3, and 5
```

## Verification

### Check Service Order

List services for a specific runlevel:

```bash
ls /etc/inits.d/3* | sort
```

This shows the execution order for runlevel 3.

### Test Service Naming

Before creating a service, verify the name is valid:

```bash
# Valid pattern: <digit><letters>-<name>
echo "3a-myservice" | grep -E '^[0-9][a-z]+-[a-z0-9-]+$'
```

## Common Patterns

### System Services

```
1a-syslog          # Logging
1b-udev            # Device management
1c-mount           # Filesystem mounting
```

### Network Services

```
3a-network         # Network initialization
3b-firewall        # Firewall rules
3c-dns             # DNS resolver
```

### Application Services

```
3d-database        # Database server
3e-cache           # Cache server
3f-application     # Application server
3g-webserver       # Web server
```

### Shutdown Services

```
0a-stop-services   # Stop application services
0b-unmount         # Unmount filesystems
0c-shutdown        # Final shutdown
```

## Troubleshooting

### Service Not Running

**Problem:** Service script exists but doesn't execute

**Check:**
1. Filename matches pattern: `<digit><letters>-<name>`
2. Runlevel digit matches the runlevel you're starting
3. Script is executable: `chmod +x /etc/inits.d/3a-myservice`
4. Script is in `/etc/inits.d` directory

### Wrong Execution Order

**Problem:** Services run in unexpected order

**Check:**
1. List services: `ls /etc/inits.d/3* | sort`
2. Verify ordering prefixes are alphabetically correct
3. Remember: `a` < `aa` < `ab` < `b`

### Service Runs in Wrong Runlevel

**Problem:** Service runs in unexpected runlevel

**Check:**
1. Verify runlevel digit in filename
2. For multi-runlevel services, check all runlevel digits
3. Example: `3a1b-service` runs in both runlevel 3 and 1

## Reference

### Complete Naming Pattern

```
^[0-9][a-z]+-[a-z0-9-]+$
```

**Breakdown:**
- `^` - Start of string
- `[0-9]` - Single digit (runlevel)
- `[a-z]+` - One or more lowercase letters (ordering)
- `-` - Dash separator
- `[a-z0-9-]+` - One or more lowercase letters, digits, or hyphens (name)
- `$` - End of string

### Multi-Runlevel Pattern

```
^([0-9][a-z]+)+-[a-z0-9-]+$
```

**Example:** `3a1b5c-service`
- Runlevel 3, order a
- Runlevel 1, order b
- Runlevel 5, order c

## See Also

- README.md - Complete documentation
- examples/ - Example service scripts
- INSTALL.md - Installation instructions
