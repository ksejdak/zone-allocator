#!/bin/bash
# $1 - arm-none-eabi-gcc version to be installed (e.g. 3.8.2)
# $2 - Host OS (Linux or macOS)

set -ev

VERSION=${1}
if [ "${2}" == "linux" ]; then
    OS="linux"
elif [ "${2}" == "osx" ]; then
    OS="mac"
fi

if [ -z ${VERSION} ]; then
    echo "No arm-none-eabi-gcc version specified. Aborting."
    exit 1
fi

if [ -z ${OS} ]; then
    echo "No host OS specified. Aborting."
    exit 2
fi

if [ -d gcc ]; then
    exit 0;
fi

echo "Installing arm-none-eabi-gcc v${VERSION}"

case "${VERSION}" in
    "7.2.0")
        PACKAGE_NAME="gcc-arm-none-eabi-7-2017-q4-major"
        PACKAGE_BIN_NAME="${PACKAGE_NAME}-${OS}.tar.bz2"
        if [ "${OS}" == "linux" ]; then
            PACKAGE_URL="http://bit.ly/2FNac93"
        else
            PACKAGE_URL="http://bit.ly/2FPJ5dM"
        fi
        ;;
    *)
        echo "Unsupported arm-none-eabi-gcc version."
        exit 3

esac

wget --no-check-certificate --quiet ${PACKAGE_URL} -O ${PACKAGE_BIN_NAME}
mkdir -p gcc
tar --strip-components=1 -xf ${PACKAGE_BIN_NAME} -C gcc

echo "Installing arm-none-eabi-gcc v${VERSION} OK."
