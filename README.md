# eosdt
https://eosdt.com

 1. Check Ubuntu version
 
lsb_release -a

No LSB modules are available.
Distributor ID: Ubuntu
Description:    Ubuntu 18.04.1 LTS
Release:    18.04
Codename:   bionic

2.Check Contract Development Toolkit version

/usr/bin/env eosio-cpp --version

eosio-cpp version 1.4.1

3. Build contract by command

/usr/bin/env eosio-cpp eosdtcntract/eosdtcntract.cpp -o eosdtcntract/eosdtcntract.wasm

4. Check sha256 by command:

sha256sum eosdtcntract/eosdtcntract.wasm
f18f4b2745bb2abfc3ad753c2554aeeba1cbc01f73838b4e0f966437f523dd8a
sha256sum eosdtgovernc/eosdtgovernc.wasm
6f1ccaac1c92062db0d2e0ca5352e3e9415fb6d3e76fe79386b7c51fb65d2328
sha256sum eosdtliqdatr/eosdtliqdatr.wasm
9226289d399824fb0794ef49bb1b3c73c9978dc25f3f5b4cfd344c023630b01d
sha256sum eosdtnutoken/eosdtnutoken.wasm
7cfcd43fa074f6d3d43ddf486d66c0902101d1157c01ea5cc4a08bd051a53a3a
sha256sum eosdtorclize/eosdtorclize.wasm
8fe588df4d5c8f9c654b60f23b81da1a9c16df5be28b6c0ace45cea6d4c009d1
sha256sum eosdtsttoken/eosdtsttoken.wasm
34db947623376cf00f47bf7b6e792d02858a75b87112170b19680c335bc6b27e


