# Installation Guide for inits

This guide provides detailed installation instructions for the inits init system.

## Prerequisites

Before installing inits, ensure you have:

- C99-compliant compiler (gcc or clang)
- POSIX-compliant shell (sh, bash, or dash)
- Make utility (optional but recommended)
- Root access (for system installation)

## Quick Installation

For most users, the standard installation process is:

```bash
# Build the system
make

# Install (requires root)
sudo make install
```

This installs inits to `/usr/local/sbin` with service directory at `/etc/inits.d`.

## Detailed Installation Steps

### Step 1: Build the System

```bash
make
```

This command:
1. Compiles the main `inits` binary from C source files
2. Generates wrapper scripts `inits0` through `inits9`
3. Places the binary in the project root
4. Places wrapper scripts in the `scripts/` directory

**Build Output:**
- `inits` - Main binary
- `scripts/inits0` through `scripts/inits9` - Wrapper scripts

### Step 2: Verify the Build

Check that the binary was created successfully:

```bash
./inits --help
```

Note: The binary expects the `RUNLEVEL` environment variable, so this may show an error. That's expected.

### Step 3: Install to System

```bash
sudo make install
```

This command:
1. Copies `inits` to `/usr/local/sbin/inits`
2. Copies wrapper scripts to `/usr/local/sbin/inits0` through `/usr/local/sbin/inits9`
3. Creates `/etc/inits.d` directory (if it doesn't exist)
4. Ensures `/var/log` directory exists

**Installation Locations:**
- Binary: `/usr/local/sbin/inits`
- Wrappers: `/usr/local/sbin/inits0` - `/usr/local/sbin/inits9`
- Service directory: `/etc/inits.d/`
- Log file: `/var/log/inits.log`

### Step 4: Install Example Services (Optional)

```bash
sudo cp examples/* /etc/inits.d/
sudo chmod +x /etc/inits.d/*
```

**Warning:** Review and customize example scripts before using in production.

### Step 5: Verify Installation

Check that files are in place:

```bash
ls -l /usr/local/sbin/inits*
ls -ld /etc/inits.d
```

## Custom Installation

### Installing to a Different Prefix

To install to `/usr/sbin` instead of `/usr/local/sbin`:

```bash
sudo make install PREFIX=/usr
```

To install to a custom location:

```bash
sudo make install PREFIX=/opt/inits
```

### Installing to a Staging Directory

For package creation or testing:

```bash
make install DESTDIR=/tmp/inits-staging PREFIX=/usr
```

This installs to `/tmp/inits-staging/usr/sbin/` instead of `/usr/sbin/`.

### Manual Installation

If you don't have Make or need custom installation:

```bash
# Build binary
cc -std=c99 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -pedantic \
   -Iinclude -o inits src/main.c src/service.c src/logging.c

# Install binary
sudo install -m 755 inits /usr/local/sbin/inits

# Create wrapper scripts
for i in 0 1 2 3 4 5 6 7 8 9; do
    cat > /tmp/inits$i << EOF
#!/bin/sh
RUNLEVEL=$i
export RUNLEVEL
exec /usr/local/sbin/inits
EOF
    sudo install -m 755 /tmp/inits$i /usr/local/sbin/inits$i
done

# Create directories
sudo mkdir -p /etc/inits.d
sudo mkdir -p /var/log
```

## Platform-Specific Notes

### Linux (Most Distributions)

Standard installation works on most Linux distributions:

```bash
make
sudo make install
```

### FreeBSD

FreeBSD uses a different default compiler. Use:

```bash
make CC=clang
sudo make install
```

### OpenBSD

OpenBSD requires explicit POSIX feature macros:

```bash
make CC=cc CFLAGS="-std=c99 -D_POSIX_C_SOURCE=200809L"
sudo make install
```

### macOS

macOS requires Xcode Command Line Tools:

```bash
xcode-select --install
make
sudo make install
```

## Integrating with System Boot

### As PID 1 (Primary Init)

**Warning:** Replacing your system's init is advanced and can make your system unbootable if done incorrectly. Only do this if you understand the risks.

To use inits as your system's init:

1. Install inits as described above
2. Configure your bootloader to use inits as init
3. For GRUB, add to kernel parameters: `init=/usr/local/sbin/inits3`
4. Create appropriate service scripts in `/etc/inits.d`

### As a Secondary Init System

You can use inits alongside your existing init system:

```bash
# Run a specific runlevel manually
sudo /usr/local/sbin/inits3
```

This is useful for:
- Testing inits before full deployment
- Running specific service sets
- Educational purposes

## Post-Installation

### Create Service Scripts

Create your first service script:

```bash
sudo nano /etc/inits.d/3a-myservice
```

Add content:

```bash
#!/bin/sh
echo "My service starting at runlevel $RUNLEVEL"
# Your service initialization here
exit 0
```

Make it executable:

```bash
sudo chmod +x /etc/inits.d/3a-myservice
```

### Test a Runlevel

Test your installation:

```bash
sudo /usr/local/sbin/inits3
```

Check the log:

```bash
sudo tail -f /var/log/inits.log
```

## Uninstallation

To remove inits from your system:

```bash
sudo make uninstall
```

This removes:
- `/usr/local/sbin/inits`
- `/usr/local/sbin/inits0` through `/usr/local/sbin/inits9`

This preserves:
- `/etc/inits.d/` directory and service scripts
- `/var/log/inits.log` log file

To completely remove inits including service scripts:

```bash
sudo make uninstall
sudo rm -rf /etc/inits.d
sudo rm -f /var/log/inits.log
```

## Troubleshooting Installation

### Compilation Errors

**Error:** `cc: command not found`

**Solution:** Install a C compiler:
- Debian/Ubuntu: `sudo apt-get install build-essential`
- RHEL/CentOS: `sudo yum install gcc make`
- FreeBSD: `sudo pkg install gcc`

**Error:** `fatal error: stdio.h: No such file or directory`

**Solution:** Install development headers:
- Debian/Ubuntu: `sudo apt-get install libc6-dev`
- RHEL/CentOS: `sudo yum install glibc-devel`

### Installation Errors

**Error:** `Permission denied`

**Solution:** Use `sudo` for installation:
```bash
sudo make install
```

**Error:** `cannot create directory '/usr/local/sbin'`

**Solution:** Create the directory first:
```bash
sudo mkdir -p /usr/local/sbin
sudo make install
```

### Runtime Errors

**Error:** `RUNLEVEL environment variable not set`

**Solution:** Use wrapper scripts instead of calling `inits` directly:
```bash
/usr/local/sbin/inits3  # Correct
/usr/local/sbin/inits   # Wrong - missing RUNLEVEL
```

**Error:** `Cannot open /etc/inits.d: No such file or directory`

**Solution:** Create the directory:
```bash
sudo mkdir -p /etc/inits.d
```

## Verification

After installation, verify everything is working:

```bash
# Check binary exists and is executable
ls -l /usr/local/sbin/inits

# Check wrapper scripts exist
ls -l /usr/local/sbin/inits[0-9]

# Check service directory exists
ls -ld /etc/inits.d

# Test a runlevel (with no services, should complete quickly)
sudo /usr/local/sbin/inits3

# Check log was created
ls -l /var/log/inits.log
cat /var/log/inits.log
```

## Next Steps

After installation:

1. Read the main README.md for usage instructions
2. Review example service scripts in `examples/`
3. Create your own service scripts in `/etc/inits.d`
4. Test with a non-critical runlevel first
5. Monitor `/var/log/inits.log` for issues

## Support

For issues or questions:

1. Check `/var/log/inits.log` for error messages
2. Review the README.md troubleshooting section
3. Verify service script naming conventions
4. Test service scripts manually before using with inits

## License

inits is licensed under GNU General Public License version 3.0 (GPL-3.0).
