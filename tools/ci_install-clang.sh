#!/bin/bash
# $1 - clang version to be installed (e.g. 3.8.2)
# $2 - Host OS (Linux or macOS)

VERSION=${1}
if [ "${2}" == "linux" ]; then
    OS="linux-gnu-ubuntu-14.04"
elif [ "${2}" == "osx" ]; then
    OS="apple-darwin"
fi

if [ -z ${VERSION} ]; then
    echo "No clang version specified. Aborting."
    exit 1
fi

if [ -z ${OS} ]; then
    echo "No host OS specified. Aborting."
    exit 2
fi

echo "Installing clang v${VERSION}"

PACKAGE_NAME="clang+llvm-${VERSION}-x86_64-${OS}"
PACKAGE_BIN_NAME="${PACKAGE_NAME}.tar.xz"
PACKAGE_URL="http://releases.llvm.org/${VERSION}/${PACKAGE_BIN_NAME}"

wget --no-check-certificate ${PACKAGE_URL}
tar -xf ${PACKAGE_BIN_NAME}

export PATH=${PWD}/${PACKAGE_NAME}/bin:${PATH}

echo "Installing clang v${VERSION} OK."