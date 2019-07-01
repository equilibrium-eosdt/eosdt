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

sha256sum eosdtcntract/eosdtcntract.wasm
970fb721489c49b773bcbca32303485c75cdabbff6d75e3ffaf934f45f5e641d
sha256sum eosdtgovernc/eosdtgovernc.wasm
2db4a06b5f951fffff8494dd0396d0247e2d82debeb3c01bf96f8fc1adc8c78a
sha256sum eosdtliqdatr/eosdtliqdatr.wasm
471fe3f96c657c55bc1fd7685b8e7da5e04afbadc284f3ca69c7cadac0db1c2f
sha256sum eosdtnutoken/eosdtnutoken.wasm
80e0ee5d2dac22ea3a5d9bffa4530422431f96faaa42beb92d7a0da605d41896
sha256sum eosdtorclize/eosdtorclize.wasm
2de9f4365df5a0824177ab7066a1ce8ebf92b29f170c168ebbd5e033b9ed5ca4
sha256sum eosdtsttoken/eosdtsttoken.wasm
c17b4299d4fef4778d59444544c1c5a91dd8bb06838be639cbfe6f657a43c4d1

