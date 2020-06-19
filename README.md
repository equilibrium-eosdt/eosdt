# Equilibrium EOSDT
EOSDT is the #1 decentralized stablecoin on EOS. Lock EOS or BTC collateral and get dollar-pegged EOSDT stablecoins at affordable APR.
https://equilibrium.io/eosdt

Technical specification is at https://knowledgebase.equilibrium.io/

## Building contracts and checking against true hashsums:
 1. Check Ubuntu version
 
lsb_release -a

No LSB modules are available.
Distributor ID: Ubuntu
Description:    Ubuntu 18.04.4 LTS
Release:    18.04
Codename:   bionic

2.Check Contract Development Toolkit version

/usr/bin/env eosio-cpp --version

eosio-cpp version 1.7.0

3. Build contract by command

/usr/bin/env eosio-cpp eosdtgovernc/eosdtgovernc.cpp -o eosdtgovernc/eosdtgovernc.wasm

4. Check sha256 by command:

sha256sum eosdtcntract/eosdtcntract.wasm
2c67124090285e41359b93cf48b3c1053f6f9264ed83873a0cda1c0ec529f565

sha256sum eosdtpbtcpos/eosdtpbtcpos.wasm
a8b3633f745e6217c6e42b60ffd9b50f80dfa09cb2cf07c62b8aa35c9b74895b

sha256sum eosdtgovernc/eosdtgovernc.wasm
4a0a06a8c226e45feb15840834fc3e5767201e65f80d85734592c5607bb9478a

sha256sum eosdtliqdatr/eosdtliqdatr.wasm
55b2fcd20e1a1f3274d0538d5aa16989c0dc7893f7c73feb16638a60cc869cc1

sha256sum eosdtnutoken/eosdtnutoken.wasm
ccc479e9e3912641af884c11248a4ff7dd90a75ef4212532cbbafdd14bf9eca4

sha256sum eosdtorclize/eosdtorclize.wasm
634fd188454d3a36c3a70cd6364cc3610facfa85d139bb457268a8da0ea869a6

sha256sum eosdtsttoken/eosdtsttoken.wasm
ebb7ccee43b403ed616c1ecd8e1c520c92d32ea8b90d103818360078f68703ea

sha256sum eosdtreserve/eosdtreserve.wasm
998686ed2c8f0d910b6360bc1467150741552903cb74fc3718537f3e5fe29cbb
