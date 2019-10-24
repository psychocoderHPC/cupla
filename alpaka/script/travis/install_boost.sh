#!/bin/bash

#
# Copyright 2017-2019 Benjamin Worpitz
#
# This file is part of Alpaka.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

source ./script/travis/set.sh

: "${BOOST_ROOT?'BOOST_ROOT must be specified'}"
: "${ALPAKA_CI_BOOST_LIB_DIR?'ALPAKA_CI_BOOST_LIB_DIR must be specified'}"
if [ "$TRAVIS_OS_NAME" = "linux" ]
then
    : "${ALPAKA_CI_STDLIB?'ALPAKA_CI_STDLIB must be specified'}"
fi
: "${CMAKE_BUILD_TYPE?'CMAKE_BUILD_TYPE must be specified'}"
: "${CXX?'CXX must be specified'}"
: "${CC?'CC must be specified'}"
: "${ALPAKA_CI_INSTALL_FIBERS?'ALPAKA_CI_INSTALL_FIBERS must be specified'}"

git clone -b "${ALPAKA_CI_BOOST_BRANCH}" --quiet --recursive --single-branch --depth 1 https://github.com/boostorg/boost.git "${BOOST_ROOT}"

# Bootstrap boost.
if [ "$TRAVIS_OS_NAME" = "windows" ]
then
    (cd "${BOOST_ROOT}"; ./bootstrap.bat)
else
    (cd "${BOOST_ROOT}"; sudo ./bootstrap.sh --with-toolset="${CC}")
fi
(cd "${BOOST_ROOT}"; cat ./bootstrap.log)

# Create file links.
if [ "$TRAVIS_OS_NAME" = "windows" ]
then
    (cd "${BOOST_ROOT}"; ./b2 headers)
else
    (cd "${BOOST_ROOT}"; sudo ./b2 headers)
fi

# Only build boost if we need some of the non-header-only libraries
if [ "${ALPAKA_CI_INSTALL_FIBERS}" == "ON" ]
then
    # Prepare the library destination directory.
    mkdir -p "${ALPAKA_CI_BOOST_LIB_DIR}"

    # Create the boost build command.
    ALPAKA_BOOST_B2=""
    ALPAKA_BOOST_B2_CFLAGS=""
    ALPAKA_BOOST_B2_CXXFLAGS=""

    if [ "$TRAVIS_OS_NAME" = "linux" ] || [ "$TRAVIS_OS_NAME" = "osx" ]
    then
        ALPAKA_BOOST_B2+="sudo "
    fi
    ALPAKA_BOOST_B2+="./b2 -j1"

    if [ "$TRAVIS_OS_NAME" = "linux" ] || [ "$TRAVIS_OS_NAME" = "osx" ]
    then
        ALPAKA_BOOST_B2_CFLAGS+="-fPIC"
        ALPAKA_BOOST_B2_CXXFLAGS+="-fPIC"
    fi

    if [ "$TRAVIS_OS_NAME" = "windows" ]
    then
        ALPAKA_BOOST_B2+=" --layout=versioned --toolset=msvc-14.1"
    else
        ALPAKA_BOOST_B2+=" --layout=tagged --toolset=${CC}"
    fi

    # TODO: Win32: adress-model=32
    ALPAKA_BOOST_B2+=" architecture=x86 address-model=64 link=static threading=multi runtime-link=shared"

    if [ "$TRAVIS_OS_NAME" = "windows" ]
    then
        ALPAKA_BOOST_B2+=" define=_CRT_NONSTDC_NO_DEPRECATE define=_CRT_SECURE_NO_DEPRECATE define=_SCL_SECURE_NO_DEPRECAT define=BOOST_USE_WINFIBERS define=_ENABLE_EXTENDED_ALIGNED_STORAGE"
    fi

    if [ "${CMAKE_BUILD_TYPE}" == "Debug" ]
    then
      ALPAKA_BOOST_B2+=" variant=debug"
    else
      ALPAKA_BOOST_B2+=" variant=release"
    fi

    # Clang is not supported by the FindBoost script.
    # boost (especially old versions) produces too much warnings when using clang (newer versions) so that the 4 MiB log is too short.
    if [ "${CXX}" == "clang++" ]
    then
        ALPAKA_BOOST_B2_CXXFLAGS+=" -Wunused-private-field -Wno-unused-local-typedef -Wno-c99-extensions -Wno-variadic-macros"
    fi
    # Select the libraries required.
    # If the variable is not set, the backend will most probably be used by default so we install it.
    if [ "${ALPAKA_CI_INSTALL_FIBERS}" == "ON" ]
    then
        if [ "$TRAVIS_OS_NAME" = "linux" ]
        then
            ALPAKA_BOOST_B2_CXXFLAGS+=" -std=c++11"
        fi
        ALPAKA_BOOST_B2+=" --with-fiber --with-context --with-thread --with-atomic --with-system --with-chrono --with-date_time"
    fi
    if [ "${ALPAKA_BOOST_B2_CFLAGS}" != "" ]
    then
        ALPAKA_BOOST_B2+=' cflags="'
        ALPAKA_BOOST_B2+="${ALPAKA_BOOST_B2_CFLAGS}"
        ALPAKA_BOOST_B2+='"'
    fi
    if [ "${ALPAKA_BOOST_B2_CXXFLAGS}" != "" ]
    then
        ALPAKA_BOOST_B2+=' cxxflags="'
        ALPAKA_BOOST_B2+="${ALPAKA_BOOST_B2_CXXFLAGS}"
        if [ "$TRAVIS_OS_NAME" = "linux" ]
        then
            if [ "${ALPAKA_CI_STDLIB}" == "libc++" ]
            then
                ALPAKA_BOOST_B2+=" -stdlib=libc++"
            fi
        fi
        ALPAKA_BOOST_B2+='"'
    fi

    if [ "$TRAVIS_OS_NAME" = "linux" ]
    then
        if [ "${ALPAKA_CI_STDLIB}" == "libc++" ]
        then
            ALPAKA_BOOST_B2+=' linkflags="-stdlib=libc++"'
        fi
    fi

    ALPAKA_BOOST_B2+=" --stagedir=${ALPAKA_CI_BOOST_LIB_DIR} stage"

    # Build boost.
    #echo "ALPAKA_BOOST_B2=${ALPAKA_BOOST_B2}"
    (cd "${BOOST_ROOT}"; eval "${ALPAKA_BOOST_B2}")

    # Clean the intermediate build files.
    if [ "$TRAVIS_OS_NAME" = "windows" ]
    then
        rm -rf bin.v2
    else
        sudo rm -rf bin.v2
    fi
fi
