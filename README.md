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

eosio-cpp version 1.6.1

3. Build contract by command

/usr/bin/env eosio-cpp eosdtgovernc/eosdtgovernc.cpp -o eosdtgovernc/eosdtgovernc.wasm

4. Check sha256 by command:

eosdtcntract/eosdtcntract.wasm
b9105dcaa3a63a25d1d4e1933bc3fe609b9c052f43727ac3c86291838148651d

eosdtgovernc/eosdtgovernc.wasm
eadb120a2d95984d8340f2d683e82066627a25a32cf88d56bb1a454b52237cc7

eosdtliqdatr/eosdtliqdatr.wasm
d00edde229b518e650d7ec0971af2d7694e31ea0280c394f9a5cc9d7192a4f5d

eosdtnutoken/eosdtnutoken.wasm
f5578a8a452c489a9fc8b8c339a61747dc9e025626ac5db311d9800db81364d0

eosdtorclize/eosdtorclize.wasm
29c1c839a0c089956748982ef367a3f5e290d56125c1f8de181226b5f85c74e4

eosdtsttoken/eosdtsttoken.wasm
7f7ae2f398d7b4bf708a06223b54db34f4e7690bb38dc7019b37cd90a0588a74

sha256sum eosdtreserve/eosdtreserve.wasm
7f7ae2f398d7b4bf708a06223b54db34f4e7690bb38dc7019b37cd90a0588a74
