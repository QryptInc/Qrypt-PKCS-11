# Qryptoki

## Introduction

Qryptoki is a wrapper of the PKCS#11 interface that returns quantum-sourced entropy to C_GenerateRandom calls.

## Requirements
  * Supported OSes
    * Ubuntu (tested on 20.04)
    * Fedora (tested on 28)
  * Dependencies
    * CMake (>= 3.19)
    * libcurl (libcurl4-openssl-dev on Ubuntu, libcurl-devel on Fedora)
    * OpenSSL (libssl-dev, openssl-devel)
    * RapidJSON (rapidjson-dev, rapidjson-devel)
    * GTest + GMock (Easiest is to clone from source and build as standalone CMake project. See [github.com/google/googletest/tree/main/googletest](https://github.com/google/googletest/tree/main/googletest).)
  * A "base HSM" that will be used to satisfy PKCS#11 functions not handled by Qryptoki
  * An active Qrypt entropy token, attainable at [portal.qrypt.com](https://portal.qrypt.com)

## Getting Started

### Environment variables
  * Required
    * QRYPT_BASE_HSM_PATH: The absolute path to the base HSM.
    * QRYPT_EAAS_TOKEN: The Qrypt entropy token to be used by the library (except in the test suite).
  * Optional
    * QRYPT_LOG_LEVEL: The library's log level, as an integer. Follows the syslog convention (error = 3, warning = 4, info = 6, debug = 7). Default is info.
    * QRYPT_CA_CERT_PATH: A path to a custom CA certificate file. If unset, the OS-default CA certificate file will be used.

### MeteringClient
First, we build MeteringClient, a Qrypt package that accesses the Qrypt Entropy-as-a-Service (EaaS) API:

```
cd deps/qrypt/MeteringClient
mkdir build && cd build
cmake ..
make
make install
```

### Build + Test
Before we build the library, you need to give your own Qrypt entropy token to the test suite. Replace the value of VALID_TOKEN in gtests/common.h:

```
static const char *VALID_TOKEN = "PASTE A VALID QRYPT EAAS TOKEN HERE";
// ^ Change me!
```

(Don't worry! The tests only consume about 5 KB of entropy.)

After that, it's time to build the library:

```
cd ../../../..                # Back to the project root directory!
mkdir build && cd build
cmake ..
make
```

Now, we run the tests:
```
./gtests/qryptoki_gtests
```

If all tests pass, you're good to go! The library to link to is build/libqryptoki.so, and the header file with all vendor-defined macros is inc/pkcs11.h.

## Documentation, support, and feedback

Nonexistent (for now).

## Contributing

Nonexistent (for now).
