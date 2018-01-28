#!/bin/bash
# $1 - clang version to be installed (e.g. 3.8.2)
# $2 - Host OS (Linux or macOS)

set -ev

VERSION=${1}
OS=${2}

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
    case "${VERSION}" in
    "3.9" | "4.0" | "5.0")
        sudo add-apt-repository ppa:llvm-toolchain-trusty-${VERSION} -y
        ;;
    *)
        echo "Unsupported clang version."
        exit 3
        ;;
    esac

    sudo apt-get update -qq
    sudo apt-get install clang-${VERSION} -y
else
    brew install llvm@${MAJOR_VERSION}
fi

echo "Installing gcc v${VERSION} OK."