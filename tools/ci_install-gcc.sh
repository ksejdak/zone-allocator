#!/bin/bash
# $1 - gcc version to be installed (e.g. 3.8.2)
# $2 - Host OS (Linux or macOS)

set -ev

VERSION=${1}
if [ "${2}" == "linux" ]; then
    OS="linux"
elif [ "${2}" == "osx" ]; then
    OS="mac"
fi

if [ -z ${VERSION} ]; then
    echo "No gcc version specified. Aborting."
    exit 1
fi

if [ -z ${OS} ]; then
    echo "No host OS specified. Aborting."
    exit 2
fi

MAJOR_VERSION=`echo ${VERSION} | cut -d . -f 1`
echo "Installing gcc v${MAJOR_VERSION}"

if [ "${OS}" == "linux" ]; then
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
    sudo apt-get update -qq
    sudo apt-get install gcc-${MAJOR_VERSION} -y
else
    brew install gcc@${MAJOR_VERSION}
fi

echo "Installing gcc v${MAJOR_VERSION} OK."
