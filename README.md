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
326bf36eb6e62a73e51b9a6995d156dbd905ebc6a91a3c643c0a2418a4ea8404

sha256sum eosdtgovernc/eosdtgovernc.wasm
9961ebbe4eddefb0ec12db218356c2434715b4f3c41373c1e3d5c7376b4b4c61

sha256sum eosdtliqdatr/eosdtliqdatr.wasm
0fd9c680badd4de39783a5573472ed9b876453e6074e3d40c5c66b77173e19b1

sha256sum eosdtnutoken/eosdtnutoken.wasm
662f51446c5e3dd99eba1a9423365150a47d5fa682ed7016dd3a14005ac61eb7

sha256sum eosdtorclize/eosdtorclize.wasm
455d88b5755b053b3848329fbbd8cf1bb8cb649119e4398b2f512179efc56ea0

sha256sum eosdtsttoken/eosdtsttoken.wasm
4ea0e4ff4eb5133b82e67b2078bf48618005203c5c70297a1b3da6f85db1c686

sha256sum eosdtreserve/eosdtreserve.wasm
a52cb14e8894331ed82b40f6fa3a6447f3a0fddbe2a7581450c018e115a82fa5

