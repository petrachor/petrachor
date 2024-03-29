#------------------------------------------------------------------------------
# Top-level CMake file for cpp-ethereum.
#
# The documentation for cpp-ethereum is hosted at http://cpp-ethereum.org
#
# ------------------------------------------------------------------------------
# This file is part of cpp-ethereum.
#
# cpp-ethereum is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# cpp-ethereum is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>
#
# (c) 2014-2016 cpp-ethereum contributors.
#------------------------------------------------------------------------------
add_definitions(-DPUBLICCHAIN=$ENV{PUBLICCHAIN})
if(PUBLICCHAIN)
    add_definitions(-DNETWORKP2PPORT=40404 )
else()
    add_definitions(-DNETWORKP2PPORT=41414 )
endif()

cmake_minimum_required(VERSION 3.9.3)

include(cmake/cable/bootstrap.cmake)
include(CableBuildInfo)
include(CableBuildType)
include(CableToolchains)
include(GNUInstallDirs)

cable_configure_toolchain(DEFAULT cxx14-pic)

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    # The default toolchain file configures compilers and build environment.
    # This configuration is also used by hunter to build dependencies.
    # CMake will cache this value, not need to explictly specify CACHE param.
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/toolchain.cmake)
endif()

set(ETH_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake" CACHE PATH "The path to the cmake directory")
list(APPEND CMAKE_MODULE_PATH ${ETH_CMAKE_DIR})

set(CPP_ETHEREUM_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE PATH "Path to the root directory for petrachor")

# set cmake_policies
include(EthPolicy)
eth_policy()

if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE STRING "" FORCE)
endif()

set(HUNTER_CONFIGURATION_TYPES Release)
set(HUNTER_JOBS_NUMBER 4)
set(HUNTER_CACHE_SERVERS "https://github.com/ethereum/hunter-cache")
include(HunterGate)
HunterGate(
        URL "https://github.com/ruslo/hunter/archive/v0.23.211.tar.gz"
        SHA1 "c022f0c2480e835a6bafdd734035ec2071b51897"
        LOCAL
)

if(HUNTER_ENABLED)
    find_package(PythonInterp QUIET)
    if(PYTHONINTERP_FOUND)
        hunter_gate_self(
                "${HUNTER_CACHED_ROOT}"
                "${HUNTER_VERSION}"
                "${HUNTER_SHA1}"
                hunter_dir
        )
        set(hunter_upload_script "${hunter_dir}/maintenance/upload-cache-to-github.py")
        set(hunter_cache_dir "${HUNTER_GATE_ROOT}/_Base/Cache")
        set(hunter_tmp_dir "${HUNTER_GATE_ROOT}/tmp")
        add_custom_target(
                hunter_upload_cache
                ${PYTHON_EXECUTABLE} ${hunter_upload_script}
                --username hunter-cache-bot
                --repo-owner ethereum
                --repo hunter-cache
                --cache-dir ${hunter_cache_dir}
                --temp-dir ${hunter_tmp_dir}
        )
    endif()
endif()

add_subdirectory(evmc)

# project name and version should be set after cmake_policy CMP0048
project(petrachor VERSION "1.1.0")
cable_add_buildinfo_library(PROJECT_NAME petrachor)

if (NOT EXISTS ${CMAKE_SOURCE_DIR}/evmjit/.git)
    message(FATAL_ERROR "Git submodules not initialized, execute:\n  git submodule update --init")
endif()

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY On)

include(EthCcache)
include(EthOptions)
include(EthCompilerSettings)
include(EthExecutableHelper)
include(EthDependencies)
include(EthUtils)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
hunter_add_package(Boost COMPONENTS program_options filesystem system thread context log)
find_package(Boost CONFIG REQUIRED program_options filesystem system thread context log)

hunter_add_package(jsoncpp)
find_package(jsoncpp CONFIG REQUIRED)

hunter_add_package(Snappy)
find_package(Snappy CONFIG)

hunter_add_package(cryptopp)
find_package(cryptopp CONFIG REQUIRED)
message("Crypto++: ${HUNTER_cryptopp_VERSION}")

hunter_add_package(libscrypt)
find_package(libscrypt CONFIG REQUIRED)

hunter_add_package(ethash)
find_package(ethash CONFIG REQUIRED)

#include(ProjectCryptopp)
include(ProjectJsonRpcCpp)
include(ProjectSecp256k1)
include(ProjectSnark)
include(ProjectPairing)

find_package(Threads)

if(MINIUPNPC)
    find_package(Miniupnpc 1.8.2013 REQUIRED)
endif()

configure_project()

include_directories("${CMAKE_SOURCE_DIR}")

add_subdirectory(libaleth-interpreter)
add_subdirectory(libdevcore)
add_subdirectory(libdevcrypto)
add_subdirectory(libp2p)
add_subdirectory(libethcore)
add_subdirectory(libevm)
add_subdirectory(libethereum)
add_subdirectory(libethashseal)

add_subdirectory(libwhisper)
add_subdirectory(libwebthree)
add_subdirectory(libweb3jsonrpc)
add_subdirectory(websocket-api)

if (EVMJIT)
    add_subdirectory(evmjit)
endif()

add_subdirectory(petrachor)

if (TOOLS)
    add_subdirectory(petrachor-key)
    add_subdirectory(ethvm)
    add_subdirectory(rlp)
endif()

#if (TESTS)
#    enable_testing()
#    add_subdirectory(test)
#endif()

# TODO: Split out json_spirit, libscrypt and sec256k1.
add_subdirectory(utils)
