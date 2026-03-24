#!/bin/bash
#
# debian_builder.sh - Debian package builder functions
#
# This script provides functions to build Debian packages for dxrt-driver-dkms.

# Source common utilities if not already loaded
if [ -z "$(type -t logmsg)" ]; then
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    source "${SCRIPT_DIR}/common.sh"
fi

# Source install utilities if not already loaded
if [ -z "$(type -t check_manual_install_conflict)" ]; then
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    source "${SCRIPT_DIR}/install_utils.sh"
fi

# ============================================
# Configuration
# ============================================

PACKAGE_NAME="dxrt-driver-dkms"

# ============================================
# Helper Functions
# ============================================

function get_project_root() {
    # Assuming this script is in modules/scripts/, go up two levels
    local script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    echo "$(dirname "$(dirname "${script_dir}")")"
}

function read_debian_version() {
    local project_root=$(get_project_root)
    local release_ver_file="${project_root}/release.ver"
    
    if [[ ! -f "${release_ver_file}" ]]; then
        logerr "Version file not found: ${release_ver_file}"
        return 1
    fi
    
    # Read version and strip 'v' prefix if exists
    local version=$(cat "${release_ver_file}" | tr -d '[:space:]' | sed 's/^v//')
    
    if [[ -z "${version}" ]]; then
        logerr "Version is empty in ${release_ver_file}"
        return 1
    fi
    
    # Export for use in other functions
    export DEBIAN_PKG_VERSION="${version}"
    export DEBIAN_PKG_FULL_VERSION="${version}-2"
    
    logmsg "Debian package version: ${DEBIAN_PKG_FULL_VERSION}"
}

function validate_debian_changelog() {
    local project_root=$(get_project_root)
    local changelog="${project_root}/debian/changelog"
    
    if [[ ! -f "${changelog}" ]]; then
        logerr "Changelog file not found: ${changelog}"
        return 1
    fi
    
    # Check if first line contains the current version
    local first_line=$(head -n 1 "${changelog}")
    
    if echo "${first_line}" | grep -q "${PACKAGE_NAME} (${DEBIAN_PKG_FULL_VERSION})"; then
        logmsg "✓ Changelog is up-to-date with version ${DEBIAN_PKG_FULL_VERSION}"
        return 0
    else
        logerr "WARNING: Changelog may not be up-to-date!"
        logerr "Expected: ${PACKAGE_NAME} (${DEBIAN_PKG_FULL_VERSION})"
        logerr "Found:    ${first_line}"
        logerr ""
        logerr "Please update debian/changelog before building."
        echo ""
        echo "Example entry:"
        echo "${PACKAGE_NAME} (${DEBIAN_PKG_FULL_VERSION}) unstable; urgency=medium"
        echo ""
        echo "  * Your changes here"
        echo ""
        echo " -- Your Name <email@example.com>  $(date -R)"
        echo ""
        return 1
    fi
}

function check_debian_prerequisites() {
    logmsg "-> Checking Debian build prerequisites..."
    
    # Check if required commands exist
    if ! command -v dpkg-buildpackage &> /dev/null; then
        logerr "dpkg-buildpackage not found. Please install it:"
        logerr "  Ubuntu/Debian: sudo apt install dpkg-dev"
        return 1
    fi
    
    local project_root=$(get_project_root)
    
    # Check if debian/ directory exists
    if [[ ! -d "${project_root}/debian" ]]; then
        logerr "debian/ directory not found in ${project_root}"
        return 1
    fi
    
    logmsg "✓ Prerequisites checked"
}

# ============================================
# Main Build Function
# ============================================

function build_debian_package() {
    logmsg "\n*** Building Debian Package ***\n"
    
    local project_root=$(get_project_root)
    local parent_dir="$(dirname "${project_root}")"
    local release_dir="${project_root}/release"
    
    # Check prerequisites
    if ! check_debian_prerequisites; then
        return 1
    fi
    
    # Read and validate version
    if ! read_debian_version; then
        return 1
    fi
    
    # Validate changelog (warning only, don't fail)
    validate_debian_changelog || true
    
    # Build the package
    logmsg "-> Building in: ${project_root}"
    logmsg "-> Running dpkg-buildpackage..."
    logmsg "   This may take a few minutes..."
    logmsg ""
    
    cd "${project_root}"
    
    # Run dpkg-buildpackage
    # -us: unsigned source
    # -uc: unsigned changes
    # -b: binary-only build
    if dpkg-buildpackage -us -uc -b > /tmp/dpkg-build-$$.log 2>&1; then
        logmsg "✓ Package built successfully"
    else
        logerr "Package build failed!"
        logerr "Check the log: /tmp/dpkg-build-$$.log"
        tail -n 50 /tmp/dpkg-build-$$.log
        return 1
    fi
    
    # Find the generated .deb file
    local deb_file=$(find "${parent_dir}" -maxdepth 1 -name "${PACKAGE_NAME}_${DEBIAN_PKG_FULL_VERSION}_*.deb" -type f | head -n 1)
    
    if [[ -z "${deb_file}" ]]; then
        logerr "Debian package file not found in ${parent_dir}!"
        logerr "Expected pattern: ${PACKAGE_NAME}_${DEBIAN_PKG_FULL_VERSION}_*.deb"
        return 1
    fi
    
    logmsg "-> Found package: $(basename ${deb_file})"
    
    # Create release directory structure
    local version_dir="${release_dir}/${DEBIAN_PKG_VERSION}"
    mkdir -p "${version_dir}"
    
    # Move .deb file to release directory
    logmsg "-> Moving to release/${DEBIAN_PKG_VERSION}/"
    mv "${deb_file}" "${version_dir}/"
    
    # Update 'latest' symlink
    cd "${release_dir}"
    ln -sfn "${DEBIAN_PKG_VERSION}" latest
    
    logmsg "✓ Package installed to release/${DEBIAN_PKG_VERSION}/"
    logmsg "✓ Symlink 'latest' -> ${DEBIAN_PKG_VERSION}"
    
    # Cleanup all build artifacts
    logmsg "-> Cleaning up build artifacts..."
    
    # Remove build artifacts from parent directory
    cd "${parent_dir}"
    rm -f ${PACKAGE_NAME}_*.buildinfo 2>/dev/null || true
    rm -f ${PACKAGE_NAME}_*.changes 2>/dev/null || true
    rm -f ${PACKAGE_NAME}_*.deb 2>/dev/null || true
    
    # Remove debian temporary build directories
    cd "${project_root}"
    rm -rf debian/${PACKAGE_NAME}/ 2>/dev/null || true
    rm -rf debian/.debhelper/ 2>/dev/null || true
    rm -f debian/files 2>/dev/null || true
    rm -f debian/*.substvars 2>/dev/null || true
    rm -f debian/*.log 2>/dev/null || true
    
    # Remove build logs
    rm -f /tmp/dpkg-build-*.log 2>/dev/null || true
    
    logmsg "✓ Build artifacts cleaned up"
    
    # Show summary
    logmsg "\n*** Debian Package Build Completed ***"
    logmsg ""
    logmsg "  Package: ${PACKAGE_NAME}"
    logmsg "  Version: ${DEBIAN_PKG_FULL_VERSION}"
    logmsg "  Location: release/${DEBIAN_PKG_VERSION}/"
    logmsg ""
    logmsg "  To install:"
    logmsg "    ./build.sh -c install-package"
    logmsg "  Or manually:"
    logmsg "    sudo dpkg -i release/${DEBIAN_PKG_VERSION}/${PACKAGE_NAME}_${DEBIAN_PKG_FULL_VERSION}_*.deb"
    logmsg ""
    
    cd "${project_root}"
    return 0
}

# Install the built Debian package
# Args:
#   $1: Package version (optional, defaults to "latest")
install_debian_package() {
    local pkg_version="${1:-latest}"
    local project_root=$(get_project_root)
    local pkg_dir
    
    logmsg "\n*** Installing Debian Package ***"
    
    # Debug: Check if function is available
    if type check_manual_install_conflict >/dev/null 2>&1; then
        logmsg "-> Checking for manual installation conflicts..."
        check_manual_install_conflict
    else
        logerr "WARNING: check_manual_install_conflict function not found!"
        logerr "This should not happen. Please report this bug."
    fi
    
    # Determine package directory
    if [[ "${pkg_version}" == "latest" ]]; then
        pkg_dir="${project_root}/release/latest"
        
        # Check if latest symlink exists
        if [[ ! -L "${pkg_dir}" ]]; then
            logerr "No 'latest' package found. Build a package first with: ./build.sh -c debian-package"
            return 1
        fi
        logmsg "-> Using latest version"
    else
        pkg_dir="${project_root}/release/${pkg_version}"
        
        # Check if version directory exists
        if [[ ! -d "${pkg_dir}" ]]; then
            logerr "Package version '${pkg_version}' not found in release/"
            logmsg ""
            
            # Check if release directory exists and show available versions
            if [[ -d "${project_root}/release" ]]; then
                local available_versions=$(ls -1 "${project_root}/release/" 2>/dev/null | grep -v "^latest$" | sort -V)
                
                if [[ -n "${available_versions}" ]]; then
                    logmsg "  Available versions:"
                    echo "${available_versions}" | sed 's/^/    /'
                else
                    logmsg "  No packages found. Build a package first with: ./build.sh -c debian-package"
                fi
            else
                logmsg "  No packages found. Build a package first with: ./build.sh -c debian-package"
            fi
            
            logmsg ""
            return 1
        fi
        logmsg "-> Using version: ${pkg_version}"
    fi
    
    # Find the .deb file (use -L to follow symlinks)
    local deb_file=$(find -L "${pkg_dir}" -maxdepth 1 -name "*.deb" -type f | head -1)
    
    if [[ -z "${deb_file}" ]]; then
        logerr "No .deb file found in ${pkg_dir}"
        return 1
    fi
    
    logmsg "-> Installing: ${deb_file}"
    
    # Install the package
    if sudo dpkg -i "${deb_file}"; then
        logmsg "✓ Package installed successfully"
        logmsg ""
        logmsg "  To check status:"
        logmsg "    lsmod | grep -E 'dxrt|dx_dma' or"
        logmsg "    dkms status"
        return 0
    else
        logerr "Failed to install package"
        logmsg "  Try fixing dependencies with:"
        logmsg "    sudo apt-get install -f"
        return 1
    fi
}

# Uninstall the Debian package
uninstall_debian_package() {
    local package_name="dxrt-driver-dkms"
    
    logmsg "\n*** Uninstalling Debian Package ***"
    
    # Check if package is installed
    if ! dpkg -l | grep -q "^ii.*${package_name}"; then
        logmsg "Package ${package_name} is not installed"
        return 0
    fi
    
    logmsg "-> Removing package and configuration files: ${package_name}"
    
    # Purge package completely (removes package + configuration files)
    if sudo dpkg -P "${package_name}"; then
        logmsg "✓ Package and configuration files removed successfully"
        return 0
    else
        logerr "Failed to uninstall package"
        return 1
    fi
}
