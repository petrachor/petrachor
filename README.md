# Petrichor C++ client

This repository contains the [Petrichor](https://petrichor-dev.com) C++ Client.


## Getting Started

The easiest way to get started using Petrichor is by following the **[Installation Instructions](https://https://github.com/petrichor-dev/petrichor/wiki/Installation-Instructions)**.


## Building from source

### Get the source code

Git and GitHub is used to maintain the source code. Clone the repository by:

```shell
git clone --recursive https://github.com/petrichor-dev/petrichor.git
cd petrichor
```

The `--recursive` option is important. It orders git to clone additional 
submodules which are required to build the project.
If you missed it you can correct your mistake with command 
`git submodule update --init`.

### Install CMake

CMake is used to control the build configuration of the project. Quite recent 
version of CMake is required 
(at the time of writing [3.9.3 is the minimum](CMakeLists.txt#L25)).
We recommend installing CMake by downloading and unpacking the binary 
distribution  of the latest version available on the 
[**CMake download page**](https://cmake.org/download/).

The CMake package available in your operating system can also be installed
and used if it meets the minimum version requirement.


### Install dependencies (Linux, macOS)

The following *libraries* are required to be installed in the system in their
development variant:

- leveldb
- microhttpd

They usually can be installed using system-specific package manager.
Examples for some systems:

Operating system | Installation command
---------------- | --------------------
Debian-based     | `sudo apt-get install libleveldb-dev libmicrohttpd-dev`
RedHat-based     | `dnf install leveldb-devel libmicrohttpd-devel`
macOS            | `brew install leveldb libmicrohttpd`


### Install dependencies (Windows)

We provide prebuilt dependencies required to build the project. Download them
with the [scripts/install_deps.bat](scripts/install_deps.bat) script.

```shell
scripts/install_deps.bat
```


### Build

Configure the project build with the following command. It will create the 
`build` directory with the configuration.

```shell
mkdir build; cd build  # Create a build directory.
cmake ..               # Configure the project.
cmake --build .        # Build all default targets.
```

On **Windows** Visual Studio 2015 is required. You should generate Visual Studio 
solution file (.sln) for 64-bit architecture by adding 
`-G "Visual Studio 14 2015 Win64"` argument to the CMake configure command.
After configuration is completed the `cpp-ethereum.sln` can be found in the
`build` directory.

```shell
cmake .. -G "Visual Studio 14 2015 Win64"
```


## Documentation



## License

[![License](https://img.shields.io/github/license/ethereum/cpp-ethereum.svg)](LICENSE)

All contributions are made under the [GNU General Public License v3](https://www.gnu.org/licenses/gpl-3.0.en.html). See [LICENSE](LICENSE).
