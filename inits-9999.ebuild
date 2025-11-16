# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=8

inherit git-r3 toolchain-funcs

DESCRIPTION="A lightweight UNIX init system with runlevel support"
HOMEPAGE="https://github.com/soccera1/inits"
EGIT_REPO_URI="https://github.com/soccera1/inits.git"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS=""
IUSE="debug examples static systemd-compat test"
RESTRICT="!test? ( test )"

DEPEND="
	systemd-compat? ( sys-apps/systemd )
"
RDEPEND="${DEPEND}"
BDEPEND="
	sys-devel/gcc
	dev-build/make
	test? ( sys-apps/coreutils )
"

src_compile() {
	local myemakeargs=(
		CC="$(tc-getCC)"
		CFLAGS="${CFLAGS}"
		LDFLAGS="${LDFLAGS}"
		PREFIX="${EPREFIX}/usr"
	)
	
	# Add debug flags if requested
	if use debug; then
		myemakeargs+=(
			CFLAGS="${CFLAGS} -DDEBUG -g -O0"
		)
	fi
	
	# Build static binary if requested
	if use static; then
		myemakeargs+=(
			LDFLAGS="${LDFLAGS} -static"
		)
	fi
	
	emake "${myemakeargs[@]}"
}

src_test() {
	if use test; then
		# Create test directory
		mkdir -p test_inits.d || die "Failed to create test directory"
		
		# Create test service scripts
		cat > test_inits.d/3a-test1 <<-EOF || die
			#!/bin/sh
			echo "Test service 1 executed"
			exit 0
		EOF
		
		cat > test_inits.d/3b-test2 <<-EOF || die
			#!/bin/sh
			echo "Test service 2 executed"
			exit 0
		EOF
		
		chmod +x test_inits.d/* || die "Failed to make test scripts executable"
		
		# Build test binary with custom service directory
		emake CC="$(tc-getCC)" \
			CFLAGS="${CFLAGS} -DINITS_DIR='\"${PWD}/test_inits.d\"'" \
			TARGET=inits_test || die "Test build failed"
		
		# Run basic tests
		einfo "Running service discovery tests..."
		RUNLEVEL=3 ./inits_test || die "Service execution test failed"
		
		# Cleanup
		rm -rf test_inits.d inits_test
	fi
}

src_install() {
	# Install main binary
	dosbin inits

	# Install wrapper scripts (already generated with correct PREFIX during compile)
	local i
	for i in {0..9}; do
		dosbin scripts/inits${i}
	done

	# Create service directory
	keepdir /etc/inits.d

	# Install documentation
	dodoc README.md INSTALL.md SERVICE-NAMING.md

	# Install example service scripts if requested
	if use examples; then
		docinto examples
		dodoc examples/README.md
		insinto /usr/share/doc/${PF}/examples
		doins examples/*-*
	fi

	# Install license
	dodoc LICENSE
	
	# Install systemd compatibility symlinks if requested
	if use systemd-compat; then
		# Create compatibility symlinks for common systemd targets
		dosym inits3 /usr/sbin/inits-multi-user.target
		dosym inits5 /usr/sbin/inits-graphical.target
		dosym inits0 /usr/sbin/inits-poweroff.target
		dosym inits6 /usr/sbin/inits-reboot.target
		dosym inits1 /usr/sbin/inits-rescue.target
	fi
}

pkg_postinst() {
	elog "inits has been installed to /usr/sbin/inits"
	elog ""
	elog "Wrapper scripts inits0 through inits9 are available in /usr/sbin/"
	elog "Service scripts should be placed in /etc/inits.d/"
	elog ""
	elog "Service script naming convention:"
	elog "  <runlevel><ordering>-<name>"
	elog "  Example: 3a-network, 3b-database"
	elog ""
	elog "See /usr/share/doc/${PF}/ for complete documentation"
	
	if use examples; then
		elog ""
		elog "Example service scripts installed to:"
		elog "  /usr/share/doc/${PF}/examples/"
	fi
	
	if use debug; then
		elog ""
		elog "Debug build enabled - binary includes debug symbols and verbose output"
	fi
	
	if use static; then
		elog ""
		elog "Static build enabled - binary is statically linked"
		elog "This is useful for rescue systems or minimal environments"
	fi
	
	if use systemd-compat; then
		elog ""
		elog "systemd compatibility symlinks installed:"
		elog "  /usr/sbin/inits-multi-user.target -> inits3"
		elog "  /usr/sbin/inits-graphical.target -> inits5"
		elog "  /usr/sbin/inits-poweroff.target -> inits0"
		elog "  /usr/sbin/inits-reboot.target -> inits6"
		elog "  /usr/sbin/inits-rescue.target -> inits1"
	fi
	
	elog ""
	ewarn "WARNING: This is experimental init system software."
	ewarn "Do NOT use as your primary init (PID 1) without thorough testing."
	ewarn "It is recommended to test with specific runlevels first."
}
