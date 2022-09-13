# Qrypt-PKCS-11

## Introduction

This repository contains the source code for qryptoki, a wrapper of the PKCS#11 interface that returns quantum-sourced entropy to C_GenerateRandom calls.

## Requirements
  * Supported OSes
    * Ubuntu (tested on 20.04)
    * Fedora (tested on 28)
  * Dependencies
    * CMake (>= 3.19)
    * libcurl
    * OpenSSL
    * RapidJSON
    * GTest + GMock
  * A "base HSM" that will be used to satisfy PKCS#11 functions not handled by Qryptoki
  * An active Qrypt entropy token

See the [Requirements Setup](../../wiki/Requirements-Setup) page on the GitHub wiki for help setting these up.

## Getting Started

Check out the related [Quickstarts repo](https://github.com/QryptInc/Qrypt-PKCS-11-Quickstart) for sample code, potential use cases, and examples on how to get set up (the Dockerfiles may be particularly helpful)!

### Environment variables
  * Required
    * QRYPT_BASE_HSM_PATH: The absolute path to the base HSM. (For example, if you followed the default SoftHSM install, set the variable to "/usr/local/lib/softhsm/libsofthsm2.so".)
    * QRYPT_EAAS_TOKEN: The Qrypt entropy token to be used by the library.
  * Optional
    * QRYPT_LOG_LEVEL: The library's log level, as an integer. Follows the syslog convention: error = 3, warning = 4, info = 6 (default), debug = 7.
    * QRYPT_CA_CERT_PATH: A path to a custom CA certificate file. If unset, the OS-default CA certificate file will be used.

### Build + Test

```
mkdir build && cd build   # Start in the project root directory
cmake ..
make
```

Now, we run the unit tests. (Don't worry! They only use your token for about 6KB of Qrypt entropy.)
```
src/gtests/qryptoki_gtests
```

If the unit tests pass, go ahead and install:
```
make install   # Installs to (top-level) package/ folder
```

You can now try the integration tests (which also consume 6KB of entropy):
```
cd ../integration-tests
mkdir build && cd build
cmake ..
make
./run_tests
```

If all tests pass, then you're good to go! The library to link to is package/lib/libqryptoki.so, and public header files are in package/inc.

You can track entropy usage on the [portal](https://portal.qrypt.com/).

## Documentation, support, and feedback

Check out the repo's wiki pages here on GitHub!

## Contributing

Still working on getting a contributing infrastructure set up, but we're so happy you're interested! Go ahead and publish any issues or PRs here, but understand we may be slow in responding.

## Licensing

This project is under the MIT License. See the LICENSE file for more details.

Dependencies' licenses can be found in the third-party-licenses folder.
