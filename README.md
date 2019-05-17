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
85c67a0d9971b3a3b3bcdca45147fc866894fb7dcb22e8b35e8e057753ed068e
sha256sum eosdtgovernc/eosdtgovernc.wasm
ccb15f1ae6c966c9fc2364bf88da69302bfbd3ae6793a43a138ef4d23236481d
sha256sum eosdtliqdatr/eosdtliqdatr.wasm
a18e9cc3d6c8606bb7b1afa5200676d8519713ff9e0722eae022e7e3c684ba2b
sha256sum eosdtnutoken/eosdtnutoken.wasm
80e0ee5d2dac22ea3a5d9bffa4530422431f96faaa42beb92d7a0da605d41896
sha256sum eosdtorclize/eosdtorclize.wasm
0fe73cdadf260eedc5d144f95668306dcede40d797f5df3f6356b46d10b9af07
sha256sum eosdtsttoken/eosdtsttoken.wasm
c17b4299d4fef4778d59444544c1c5a91dd8bb06838be639cbfe6f657a43c4d1

