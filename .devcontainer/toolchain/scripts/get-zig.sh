#!/usr/bin/env bash

set -o errexit
set -o pipefail
set -o nounset
# set -o xtrace

ZIG_VERSION=0.13.0

remove_zig() {
    rm -rf /usr/local/bin/zio
    rm -rf /usr/local/lib/zig
    rm -rf /usr/local/share/zig
}

install_zig() {
    remove_zig
    ARCH=$(uname -m)
    TMP=$(mktemp -d)
    echo "Installing Zig ${ZIG_VERSION} for ${ARCH}"
    echo "Downloading files to ${TMP}"
    curl -sSfL https://ziglang.org/download/${ZIG_VERSION}/zig-linux-${ARCH}-${ZIG_VERSION}.tar.xz | tar -xJ -C $TMP
    cp -a $TMP/zig-linux-${ARCH}-${ZIG_VERSION}/zig /usr/local/bin/zig
    cp -a $TMP/zig-linux-${ARCH}-${ZIG_VERSION}/lib /usr/local/lib/zig
    mkdir -p /usr/local/share/zig
    cp -a $TMP/zig-linux-${ARCH}-${ZIG_VERSION}/doc /usr/local/share/zig
    cp -a $TMP/zig-linux-${ARCH}-${ZIG_VERSION}/README.md /usr/local/share/zig
    cp -a $TMP/zig-linux-${ARCH}-${ZIG_VERSION}/LICENSE /usr/local/share/zig
    # rm -rf $TMP
}

install_zig