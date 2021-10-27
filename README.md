# Home.

![Home](/icon.png?raw=true "Home.")

**Home.** is a open source central home automation server for Linux. It is supposed to be run on low-end devices (like RPis). At this point in time not everything is implemented and **it is not supposed to be used in the current state**.

## Getting Started

**It currently only compiles on Linux**

Some dependencies are needed beforehand:
- **CMake** version 1.8
- **boost** version 1.75.0
- **OpenSSL** version 1.1.1f
- **pthread**
- **curl**
- **sqlite3**

Download the repository using: `git clone ...`

Boostrap: `./bootstrap-debian.sh` for Debian/Ubuntu

Create build directory inside repository using `mkdir build && cd build`

Create project files using: `cmake ..`

Build project using: `make -jN` (replace N by the number of threads that should be used during compilation)

## The Plan

The plan is to provide a free, open source and fast home automation system that works with most devices. Plugins should be easy to implement and add when a specific device is not already supported. Even DIY devices should be easy to integrate into the system. An additional smartphone or web app is needed to interface with the system.

## Dependencies

- **boost** version 1.75.0
- **pthread**
- **OpenSSL**
- **jwt-cpp**:          https://github.com/Thalhammer/jwt-cpp
- **libquickmail**
- **cppcodec**:         https://github.com/tplgy/cppcodec
- **rapidjson**:        https://github.com/Tencent/rapidjson
- **spdlog**:           https://github.com/gabime/spdlog
- **stb**:              https://github.com/nothings/stb
- **xxHash**:           https://github.com/Cyan4973/xxHash
